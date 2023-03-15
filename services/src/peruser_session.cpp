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

void RemoteObjectDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    IMSA_HILOGE("Start");
    if (handler_ != nullptr) {
        handler_(remote);
    }
}

void RemoteObjectDeathRecipient::SetDeathRecipient(RemoteDiedHandler handler)
{
    handler_ = handler;
}

PerUserSession::PerUserSession(int userId) : userId_(userId), imsDeathRecipient(new RemoteObjectDeathRecipient())
{
}

PerUserSession::~PerUserSession()
{
    imsDeathRecipient = nullptr;
}

int PerUserSession::AddClient(sptr<IRemoteObject> inputClient, const ClientInfo &clientInfo)
{
    IMSA_HILOGD("PerUserSession::AddClient");
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto cacheClient = GetClientInfo(inputClient);
    if (cacheClient != nullptr) {
        IMSA_HILOGE("PerUserSession::client info is exist, not need add.");
        return ErrorCode::NO_ERROR;
    }
    auto info = std::make_shared<ClientInfo>(clientInfo);
    mapClients.insert({ inputClient, info });
    info->deathRecipient->SetDeathRecipient(
        [this, info](const wptr<IRemoteObject> &) { this->OnClientDied(info->client); });
    sptr<IRemoteObject> obj = info->client->AsObject();
    if (obj == nullptr) {
        IMSA_HILOGE("PerUserSession::AddClient inputClient AsObject is nullptr");
        return ErrorCode::ERROR_REMOTE_CLIENT_DIED;
    }
    bool ret = obj->AddDeathRecipient(info->deathRecipient);
    IMSA_HILOGI("Add death recipient %{public}s", ret ? "success" : "failed");
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
    info->isShowKeyBoard = isShowKeyboard;
}

/** Remove an input client
 * @param inputClient remote object handler of the input client
 * @return ErrorCode::NO_ERROR no error
 * @return ErrorCode::ERROR_CLIENT_NOT_FOUND client is not found
 */
void PerUserSession::RemoveClient(sptr<IRemoteObject> inputClient)
{
    IMSA_HILOGD("start");
    auto client = GetCurrentClient();
    if (client != nullptr && client->AsObject() == inputClient) {
        int ret = HideKeyboard(client);
        IMSA_HILOGI("hide keyboard ret: %{public}d", ret);
        SetCurrentClient(nullptr);
    }
    auto info = GetClientInfo(inputClient);
    if (info == nullptr) {
        IMSA_HILOGE("client info is not exist.");
        return;
    }
    inputClient->RemoveDeathRecipient(info->deathRecipient);
    std::lock_guard<std::recursive_mutex> lock(mtx);
    mapClients.erase(inputClient);
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
int PerUserSession::ShowKeyboard(const sptr<IInputClient> &inputClient, bool isShowKeyboard)
{
    IMSA_HILOGD("PerUserSession::ShowKeyboard");
    if (inputClient == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto clientInfo = GetClientInfo(inputClient->AsObject());
    int index = GetImeIndex(inputClient);
    if (index < 0 || index >= MAX_IME || clientInfo == nullptr) {
        IMSA_HILOGE("PerUserSession::ShowKeyboard Aborted! index = -1 or clientInfo is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    sptr<IInputMethodCore> core = GetImsCore(CURRENT_IME);
    if (core == nullptr) {
        IMSA_HILOGE("PerUserSession::ShowKeyboard Aborted! imsCore[%{public}d] is nullptr", index);
        return ErrorCode::ERROR_NULL_POINTER;
    }

    auto subProperty = GetCurrentSubProperty();
    int32_t ret = core->ShowKeyboard(clientInfo->channel, isShowKeyboard, subProperty);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("PerUserSession, failed ret: %{public}d", ret);
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
int PerUserSession::HideKeyboard(const sptr<IInputClient> &inputClient)
{
    IMSA_HILOGD("PerUserSession::HideKeyboard");
    sptr<IInputMethodCore> core = GetImsCore(CURRENT_IME);
    if (core == nullptr) {
        IMSA_HILOGE("PerUserSession::HideKeyboard imsCore is nullptr");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    if (inputClient != nullptr) {
        UpdateClient(inputClient->AsObject(), false);
    }
    bool ret = core->HideKeyboard(1);
    if (!ret) {
        IMSA_HILOGE("PerUserSession::HideKeyboard [imsCore->hideKeyboard] failed");
        return ErrorCode::ERROR_KBD_HIDE_FAILED;
    }
    return ErrorCode::NO_ERROR;
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
    RemoveClient(remote->AsObject());
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
    return ShowKeyboard(client, true);
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
std::shared_ptr<ClientInfo> PerUserSession::GetClientInfo(sptr<IRemoteObject> inputClient)
{
    if (inputClient == nullptr) {
        IMSA_HILOGE("PerUserSession::GetClientInfo inputClient is nullptr");
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto it = mapClients.find(inputClient);
    if (it == mapClients.end()) {
        IMSA_HILOGE("PerUserSession::GetClientInfo client not found");
        return nullptr;
    }
    return it->second;
}

/** Prepare input. Called by an input client.
    \n Run in work thread of this user
    \param the parameters from remote client
    \return ErrorCode
    */
int32_t PerUserSession::OnPrepareInput(const ClientInfo &clientInfo)
{
    IMSA_HILOGD("PerUserSession::OnPrepareInput Start\n");
    int ret = AddClient(clientInfo.client->AsObject(), clientInfo);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("PerUserSession::OnPrepareInput %{public}d", ret);
        return ret;
    }
    SendAgentToSingleClient(clientInfo);
    return ErrorCode::NO_ERROR;
}

void PerUserSession::SendAgentToSingleClient(const ClientInfo &clientInfo)
{
    IMSA_HILOGD("PerUserSession::SendAgentToSingleClient");
    if (imsAgent == nullptr) {
        IMSA_HILOGI("PerUserSession::SendAgentToSingleClient imsAgent is nullptr");
        CreateComponentFailed(userId_, ErrorCode::ERROR_NULL_POINTER);
        return;
    }
    clientInfo.client->OnInputReady(imsAgent);
}

/** Release input. Called by an input client.Run in work thread of this user
 * @param the parameters from remote client
 * @return ErrorCode
 */
int32_t PerUserSession::OnReleaseInput(sptr<IInputClient> client)
{
    IMSA_HILOGI("PerUserSession::Start");
    if (client == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    RemoveClient(client->AsObject());
    return ErrorCode::NO_ERROR;
}

/** Start input. Called by an input client. Run in work thread of this user
 * @param the parameters from remote client
 * @return ErrorCode
 */
int32_t PerUserSession::OnStartInput(sptr<IInputClient> client, bool isShowKeyboard)
{
    IMSA_HILOGI("PerUserSession::OnStartInput");
    return ShowKeyboard(client, isShowKeyboard);
}

int32_t PerUserSession::OnSetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent)
{
    IMSA_HILOGD("PerUserSession::SetCoreAndAgent Start\n");
    if (core == nullptr || agent == nullptr) {
        IMSA_HILOGE("PerUserSession::SetCoreAndAgent core or agent nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    SetImsCore(CURRENT_IME, core);
    if (imsDeathRecipient != nullptr && core->AsObject() != nullptr) {
        imsDeathRecipient->SetDeathRecipient([this, core](const wptr<IRemoteObject> &) { this->OnImsDied(core); });
        bool ret = core->AsObject()->AddDeathRecipient(imsDeathRecipient);
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
    for (auto it = mapClients.begin(); it != mapClients.end(); ++it) {
        auto clientInfo = it->second;
        if (clientInfo != nullptr) {
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
    core->AsObject()->RemoveDeathRecipient(imsDeathRecipient);
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
        core->AsObject()->RemoveDeathRecipient(imsDeathRecipient);
        SetImsCore(index, nullptr);
    }
}

void PerUserSession::SetCurrentClient(sptr<IInputClient> client)
{
    IMSA_HILOGI("set current client");
    std::lock_guard<std::mutex> lock(clientLock_);
    currentClient = client;
}

sptr<IInputClient> PerUserSession::GetCurrentClient()
{
    std::lock_guard<std::mutex> lock(clientLock_);
    return currentClient;
}

void PerUserSession::OnInputMethodSwitched(const Property &property, const SubProperty &subProperty)
{
    IMSA_HILOGD("PerUserSession::OnInputMethodSwitched");
    std::lock_guard<std::recursive_mutex> lock(mtx);
    for (const auto &client : mapClients) {
        auto clientInfo = client.second;
        if (clientInfo == nullptr) {
            IMSA_HILOGD("PerUserSession::clientInfo is nullptr");
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
} // namespace MiscServices
} // namespace OHOS
