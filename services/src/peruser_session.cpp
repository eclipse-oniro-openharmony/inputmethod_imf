/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "peruser_session.h"

#include <vector>

#include "ability_connect_callback_proxy.h"
#include "ability_manager_interface.h"
#include "element_name.h"
#include "ime_cfg_manager.h"
#include "input_client_proxy.h"
#include "input_control_channel_proxy.h"
#include "input_data_channel_proxy.h"
#include "input_method_agent_proxy.h"
#include "input_method_core_proxy.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "parcel.h"
#include "system_ability_definition.h"
#include "sys/prctl.h"
#include "unistd.h"
#include "utils.h"
#include "want.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
PerUserSession::PerUserSession(int userId) : userId_(userId), imsDeathRecipient_(new InputDeathRecipient())
{
}

PerUserSession::~PerUserSession()
{
    imsDeathRecipient_ = nullptr;
}

int PerUserSession::AddClient(sptr<IRemoteObject> inputClient, const InputClientInfo &clientInfo)
{
    IMSA_HILOGD("PerUserSession, run in");
    auto cacheInfo = GetClientInfo(inputClient);
    if (cacheInfo != nullptr) {
        IMSA_HILOGI("client info is exist, not need add.");
        cacheInfo->isValid = cacheInfo->isValid || clientInfo.isValid;
        cacheInfo->isToNotify = cacheInfo->isToNotify || clientInfo.isToNotify;
        return ErrorCode::NO_ERROR;
    }

    auto info = std::make_shared<InputClientInfo>(clientInfo);
    info->deathRecipient->SetDeathRecipient(
        [this, info](const wptr<IRemoteObject> &) { this->OnClientDied(info->client); });
    auto obj = info->client->AsObject();
    if (obj == nullptr) {
        IMSA_HILOGE("client obj is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    if (!obj->AddDeathRecipient(info->deathRecipient)) {
        IMSA_HILOGI("failed to add client death recipient");
        return ErrorCode::ERROR_CLIENT_ADD_FAILED;
    }

    std::lock_guard<std::recursive_mutex> lock(mtx);
    mapClients_.insert({ inputClient, info });
    return ErrorCode::NO_ERROR;
}

void PerUserSession::UpdateClient(sptr<IRemoteObject> inputClient, bool isShowKeyboard)
{
    IMSA_HILOGD("PerUserSession::start");
    auto info = GetClientInfo(inputClient);
    if (info == nullptr) {
        IMSA_HILOGE("client info is not exist.");
        return;
    }
    info->isValid = true;
    info->isShowKeyboard = isShowKeyboard;
}

int32_t PerUserSession::RemoveClient(const sptr<IRemoteObject> &client, bool isClientDied)
{
    IMSA_HILOGD("start");
    auto clientInfo = GetClientInfo(client);
    if (clientInfo == nullptr) {
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    // if current client is removed, hide keyboard and clear channel
    auto currentClient = GetCurrentClient();
    if (currentClient != nullptr && client == currentClient->AsObject()) {
        int32_t ret = HideKeyboard(currentClient);
        IMSA_HILOGI("hide keyboard ret: %{public}d", ret);
        SetCurrentClient(nullptr);
        ret = ClearDataChannel(clientInfo->channel);
        IMSA_HILOGI("clear data channel ret: %{public}d", ret);
    }

    // if client still need to be notified event, do not remove, update info
    if (clientInfo->isToNotify && !isClientDied) {
        clientInfo->isShowKeyboard = false;
        clientInfo->isValid = false;
        return ErrorCode::NO_ERROR;
    }
    client->RemoveDeathRecipient(clientInfo->deathRecipient);
    std::lock_guard<std::recursive_mutex> lock(mtx);
    mapClients_.erase(client);
    return ErrorCode::NO_ERROR;
}

/** Show keyboard
 * @param inputClient the remote object handler of the input client.
 * @return ErrorCode::NO_ERROR no error
 * @return ErrorCode::ERROR_IME_NOT_STARTED ime not started
 * @return ErrorCode::ERROR_KBD_IS_OCCUPIED keyboard is showing by other client
 * @return ErrorCode::ERROR_CLIENT_NOT_FOUND the input client is not found
 * @return ErrorCode::ERROR_IME_START_FAILED failed to start input method service
 * @return ErrorCode::ERROR_KBD_SHOW_FAILED failed to show keyboard
 * @return other errors returned by binder driver
 */
int32_t PerUserSession::ShowKeyboard(
    const sptr<IInputDataChannel>& channel, const sptr<IInputClient> &inputClient, bool isShowKeyboard)
{
    IMSA_HILOGD("PerUserSession, run in");
    if (inputClient == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto core = GetImsCore(CURRENT_IME);
    if (core == nullptr) {
        IMSA_HILOGE("Aborted! imsCore[%{public}d] is nullptr", CURRENT_IME);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto subProperty = GetCurrentSubProperty();
    int32_t ret = core->ShowKeyboard(channel, isShowKeyboard, subProperty);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to show keybaord, ret: %{public}d", ret);
        return ErrorCode::ERROR_KBD_SHOW_FAILED;
    }
    UpdateClient(inputClient->AsObject(), isShowKeyboard);
    SetCurrentClient(inputClient);
    return ErrorCode::NO_ERROR;
}

/** hide keyboard
 * @param inputClient the remote object handler of the input client.
 * @return ErrorCode::NO_ERROR no error
 * @return ErrorCode::ERROR_IME_NOT_STARTED ime not started
 * @return ErrorCode::ERROR_KBD_IS_NOT_SHOWING keyboard has not been showing
 * @return ErrorCode::ERROR_CLIENT_NOT_FOUND the input client is not found
 * @return ErrorCode::ERROR_KBD_HIDE_FAILED failed to hide keyboard
 * @return other errors returned by binder driver
 */
int32_t PerUserSession::HideKeyboard(const sptr<IInputClient> &inputClient)
{
    IMSA_HILOGD("PerUserSession::HideKeyboard");
    auto client = GetCurrentClient();
    if (client == nullptr || inputClient != client) {
        IMSA_HILOGE("not current client");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    sptr<IInputMethodCore> core = GetImsCore(CURRENT_IME);
    if (core == nullptr) {
        IMSA_HILOGE("imsCore is nullptr");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    if (inputClient != nullptr) {
        UpdateClient(inputClient->AsObject(), false);
    }
    bool ret = core->HideKeyboard(1);
    if (!ret) {
        IMSA_HILOGE("core->HideKeyboard failed");
        return ErrorCode::ERROR_KBD_HIDE_FAILED;
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::ClearDataChannel(const sptr<IInputDataChannel> &channel)
{
    sptr<IInputMethodCore> core = GetImsCore(CURRENT_IME);
    if (core == nullptr || channel == nullptr) {
        IMSA_HILOGE("imsCore or channel is nullptr");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    return core->ClearDataChannel(channel);
}

/** Handle the situation a remote input client died.
 * It's called when a remote input client died
 * @param the remote object handler of the input client died.
 */
void PerUserSession::OnClientDied(sptr<IInputClient> remote)
{
    IMSA_HILOGI("userId: %{public}d", userId_);
    if (remote == nullptr) {
        return;
    }
    RemoveClient(remote->AsObject(), true);
}

/** Handle the situation a input method service died
 * It's called when an input method service died
 * @param the remote object handler of input method service who died.
 */
void PerUserSession::OnImsDied(sptr<IInputMethodCore> remote)
{
    if (remote == nullptr) {
        return;
    }
    int index = 0;
    for (int i = 0; i < MAX_IME; i++) {
        sptr<IInputMethodCore> core = GetImsCore(i);
        if (core == remote) {
            index = i;
            break;
        }
    }
    ClearImeData(index);
    if (!IsRestartIme(index)) {
        IMSA_HILOGI("Restart ime over max num");
        return;
    }
    IMSA_HILOGI("user %{public}d ime died, restart!", userId_);
    auto *msg = new (std::nothrow) Message(MSG_ID_START_INPUT_SERVICE, nullptr);
    if (msg == nullptr) {
        IMSA_HILOGE("msg is nullptr");
        return;
    }
    usleep(MAX_RESET_WAIT_TIME);
    MessageHandler::Instance()->SendMessage(msg);
}

void PerUserSession::UpdateCurrentUserId(int32_t userId)
{
    userId_ = userId;
}

/** Hide current keyboard
 * @param flag the flag to hide keyboard.
 */
int PerUserSession::OnHideKeyboardSelf()
{
    IMSA_HILOGI("PerUserSession::OnHideKeyboardSelf");
    sptr<IInputClient> client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGE("current client is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    return HideKeyboard(client);
}

int PerUserSession::OnShowKeyboardSelf()
{
    IMSA_HILOGD("PerUserSession::OnShowKeyboardSelf");
    sptr<IInputClient> client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGE("current client is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    auto clientInfo = GetClientInfo(client);
    if (clientInfo == nullptr) {
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    return ShowKeyboard(clientInfo->channel, client, true);
}

/** Get ime index for the input client
 * @param inputClient the remote object handler of an input client.
 * @return 0 - default ime
 * @return 1 - security ime
 * @return -1 - input client is not found
 */
int PerUserSession::GetImeIndex(const sptr<IInputClient> &inputClient)
{
    if (inputClient == nullptr) {
        IMSA_HILOGW("PerUserSession::GetImeIndex inputClient is nullptr");
        return -1;
    }
    auto clientInfo = GetClientInfo(inputClient->AsObject());
    if (clientInfo == nullptr) {
        IMSA_HILOGW("PerUserSession::clientInfo is nullptr");
        return -1;
    }
    if (clientInfo->attribute.GetSecurityFlag()) {
        return SECURITY_IME;
    }
    return CURRENT_IME;
}

/** Get ClientInfo
 * @param inputClient the IRemoteObject remote handler of given input client
 * @return a pointer of ClientInfo if client is found
 *         null if client is not found
 * @note the clientInfo pointer should not be freed by caller
 */
std::shared_ptr<InputClientInfo> PerUserSession::GetClientInfo(sptr<IRemoteObject> inputClient)
{
    if (inputClient == nullptr) {
        IMSA_HILOGE("inputClient is nullptr");
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto it = mapClients_.find(inputClient);
    if (it == mapClients_.end()) {
        IMSA_HILOGE("client not found");
        return nullptr;
    }
    return it->second;
}

/** Prepare input. Called by an input client.
    \n Run in work thread of this user
    \param the parameters from remote client
    \return ErrorCode
    */
int32_t PerUserSession::OnPrepareInput(const InputClientInfo &clientInfo)
{
    IMSA_HILOGD("PerUserSession::OnPrepareInput Start\n");
    return AddClient(clientInfo.client->AsObject(), clientInfo);
}

int32_t PerUserSession::SendAgentToSingleClient(const InputClientInfo &clientInfo)
{
    IMSA_HILOGD("PerUserSession::SendAgentToSingleClient");
    if (imsAgent == nullptr) {
        IMSA_HILOGI("PerUserSession::SendAgentToSingleClient imsAgent is nullptr");
        CreateComponentFailed(userId_, ErrorCode::ERROR_NULL_POINTER);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return clientInfo.client->OnInputReady(imsAgent);
}

/** Release input. Called by an input client.Run in work thread of this user
 * @param the parameters from remote client
 * @return ErrorCode
 */
int32_t PerUserSession::OnReleaseInput(const sptr<IInputClient>& client)
{
    IMSA_HILOGI("PerUserSession::Start");
    if (client == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    RemoveClient(client->AsObject(), false);
    return ErrorCode::NO_ERROR;
}

/** Start input. Called by an input client. Run in work thread of this user
 * @param the parameters from remote client
 * @return ErrorCode
 */
int32_t PerUserSession::OnStartInput(sptr<IInputClient> client, bool isShowKeyboard)
{
    IMSA_HILOGI("PerUserSession::OnStartInput");
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    // bind imc to ima
    int32_t ret = SendAgentToSingleClient(*clientInfo);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    // bind ima to imc
    return ShowKeyboard(clientInfo->channel, client, isShowKeyboard);
}

int32_t PerUserSession::OnSetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent)
{
    IMSA_HILOGD("PerUserSession::SetCoreAndAgent Start\n");
    if (core == nullptr || agent == nullptr) {
        IMSA_HILOGE("PerUserSession::SetCoreAndAgent core or agent nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    SetImsCore(CURRENT_IME, core);
    if (imsDeathRecipient_ != nullptr && core->AsObject() != nullptr) {
        imsDeathRecipient_->SetDeathRecipient([this, core](const wptr<IRemoteObject> &) { this->OnImsDied(core); });
        bool ret = core->AsObject()->AddDeathRecipient(imsDeathRecipient_);
        IMSA_HILOGI("Add death recipient %{public}s", ret ? "success" : "failed");
    }
    imsAgent = agent;
    InitInputControlChannel();
    SendAgentToAllClients();
    auto client = GetCurrentClient();
    if (client != nullptr) {
        auto clientInfo = GetClientInfo(client->AsObject());
        if (clientInfo != nullptr) {
            IMSA_HILOGI("PerUserSession::Bind IMC to IMA");
            OnStartInput(clientInfo->client, clientInfo->isShowKeyBoard);
        }
    }
    return ErrorCode::NO_ERROR;
}

void PerUserSession::SendAgentToAllClients()
{
    IMSA_HILOGD("PerUserSession::SendAgentToAllClients");
    if (imsAgent == nullptr) {
        IMSA_HILOGE("PerUserSession::SendAgentToAllClients imsAgent is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx);
    for (auto & mapClient : mapClients_) {
        auto clientInfo = mapClient.second;
        if (clientInfo != nullptr || clientInfo->isValid) {
            clientInfo->client->OnInputReady(imsAgent);
        }
    }
}

void PerUserSession::InitInputControlChannel()
{
    IMSA_HILOGD("PerUserSession::InitInputControlChannel");
    sptr<IInputControlChannel> inputControlChannel = new InputControlChannelStub(userId_);
    sptr<IInputMethodCore> core = GetImsCore(CURRENT_IME);
    if (core == nullptr) {
        IMSA_HILOGE("PerUserSession::InitInputControlChannel core is nullptr");
        return;
    }
    int ret =
        core->InitInputControlChannel(inputControlChannel, ImeCfgManager::GetInstance().GetImeCfg(userId_).currentIme);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGI("PerUserSession::InitInputControlChannel fail %{public}d", ret);
    }
}

/** Stop input. Called by an input client. Run in work thread of this user
 * @param the parameters from remote client
 * @return ErrorCode
 */
int32_t PerUserSession::OnStopInput(sptr<IInputClient> client)
{
    IMSA_HILOGD("PerUserSession::OnStopInput");
    return HideKeyboard(client);
}

void PerUserSession::StopInputService(std::string imeId)
{
    IMSA_HILOGI("PerUserSession::StopInputService");
    sptr<IInputMethodCore> core = GetImsCore(CURRENT_IME);
    if (core == nullptr) {
        IMSA_HILOGE("imsCore[0] is nullptr");
        return;
    }
    IMSA_HILOGI("Remove death recipient");
    core->AsObject()->RemoveDeathRecipient(imsDeathRecipient_);
    core->StopInputService(imeId);
}

bool PerUserSession::IsRestartIme(uint32_t index)
{
    IMSA_HILOGD("PerUserSession::IsRestartIme");
    std::lock_guard<std::mutex> lock(resetLock);
    auto now = time(nullptr);
    if (difftime(now, manager[index].last) > IME_RESET_TIME_OUT) {
        manager[index] = { 0, now };
    }
    ++manager[index].num;
    return manager[index].num <= MAX_RESTART_NUM;
}

void PerUserSession::ClearImeData(uint32_t index)
{
    IMSA_HILOGI("Clear ime...index = %{public}d", index);
    sptr<IInputMethodCore> core = GetImsCore(index);
    if (core != nullptr) {
        core->AsObject()->RemoveDeathRecipient(imsDeathRecipient_);
        SetImsCore(index, nullptr);
    }
}

void PerUserSession::SetCurrentClient(sptr<IInputClient> client)
{
    IMSA_HILOGI("set current client");
    std::lock_guard<std::mutex> lock(clientLock_);
    currentClient_ = client;
}

sptr<IInputClient> PerUserSession::GetCurrentClient()
{
    std::lock_guard<std::mutex> lock(clientLock_);
    return currentClient_;
}

void PerUserSession::OnInputMethodSwitched(const Property &property, const SubProperty &subProperty)
{
    IMSA_HILOGD("PerUserSession::OnInputMethodSwitched");
    std::lock_guard<std::recursive_mutex> lock(mtx);
    for (const auto &client : mapClients_) {
        auto clientInfo = client.second;
        if (clientInfo == nullptr || !clientInfo->isToNotify) {
            IMSA_HILOGD("client nullptr or no need to notify");
            continue;
        }
        int32_t ret = clientInfo->client->OnSwitchInput(property, subProperty);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE(
                "OnSwitchInput failed, ret: %{public}d, uid: %{public}d", ret, static_cast<int32_t>(clientInfo->uid));
            continue;
        }
    }
    if (subProperty.name != currentSubProperty.name) {
        SetCurrentSubProperty(subProperty);
        return;
    }
    SetCurrentSubProperty(subProperty);
    sptr<IInputMethodCore> core = GetImsCore(CURRENT_IME);
    if (core == nullptr) {
        IMSA_HILOGE("imsCore is nullptr");
        return;
    }
    int32_t ret = core->SetSubtype(subProperty);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("PerUserSession::SetSubtype failed, ret %{public}d", ret);
    }
}

SubProperty PerUserSession::GetCurrentSubProperty()
{
    IMSA_HILOGD("PerUserSession::GetCurrentSubProperty");
    std::lock_guard<std::mutex> lock(propertyLock_);
    return currentSubProperty;
}

void PerUserSession::SetCurrentSubProperty(const SubProperty &subProperty)
{
    IMSA_HILOGD("PerUserSession::SetCurrentSubProperty");
    std::lock_guard<std::mutex> lock(propertyLock_);
    currentSubProperty = subProperty;
}

sptr<IInputMethodCore> PerUserSession::GetImsCore(int32_t index)
{
    std::lock_guard<std::mutex> lock(imsCoreLock_);
    if (!IsValid(index)) {
        return nullptr;
    }
    return imsCore[index];
}

void PerUserSession::SetImsCore(int32_t index, sptr<IInputMethodCore> core)
{
    std::lock_guard<std::mutex> lock(imsCoreLock_);
    if (!IsValid(index)) {
        return;
    }
    imsCore[index] = core;
}

int32_t PerUserSession::OnUnfocused(int32_t pid, int32_t uid)
{
    auto client = GetCurrentClient();
    if (client == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto clientInfo = GetClientInfo(currentClient_->AsObject());
    if (clientInfo == nullptr) {
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    if (clientInfo->pid != pid || clientInfo->uid != uid) {
        IMSA_HILOGD("not current client, no need to handle");
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGD("current client is unfocused, start unbinding");
    int32_t ret = client->OnInputStop();
    IMSA_HILOGI("OnInputStop ret: %{public}d", ret);
    return OnReleaseInput(client);
}
} // namespace MiscServices
} // namespace OHOS
