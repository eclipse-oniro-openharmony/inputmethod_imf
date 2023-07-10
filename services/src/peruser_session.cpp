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
#include "ime_info_inquirer.h"
#include "input_client_proxy.h"
#include "input_control_channel_proxy.h"
#include "input_data_channel_proxy.h"
#include "input_method_agent_proxy.h"
#include "input_method_core_proxy.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "parcel.h"
#include "sys/prctl.h"
#include "system_ability_definition.h"
#include "unistd.h"
#include "want.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
constexpr uint32_t IME_RESTART_TIMES = 5;
constexpr uint32_t IME_RESTART_INTERVAL = 300;
PerUserSession::PerUserSession(int32_t userId) : userId_(userId), imsDeathRecipient_(new InputDeathRecipient())
{
}

PerUserSession::~PerUserSession()
{
    imsDeathRecipient_ = nullptr;
}

int PerUserSession::AddClient(sptr<IRemoteObject> inputClient, const InputClientInfo &clientInfo, ClientAddEvent event)
{
    IMSA_HILOGD("PerUserSession, run in");
    auto cacheInfo = GetClientInfo(inputClient);
    if (cacheInfo != nullptr) {
        IMSA_HILOGI("client info is exist, not need add.");
        if (event == START_LISTENING) {
            cacheInfo->eventFlag = clientInfo.eventFlag;
        }
        if (event == PREPARE_INPUT) {
            cacheInfo->attribute = clientInfo.attribute;
        }
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
    IMSA_HILOGD("add client end");
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
    auto clientInfo = GetClientInfo(client);
    if (clientInfo == nullptr) {
        IMSA_HILOGD("client already removed");
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGD("start removing client of pid: %{public}d", clientInfo->pid);
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
    if (clientInfo->eventFlag != EventStatusManager::NO_EVENT_ON && !isClientDied) {
        IMSA_HILOGD("need to notify, do not remove");
        clientInfo->isShowKeyboard = false;
        clientInfo->isValid = false;
        return ErrorCode::NO_ERROR;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx);
    if (clientInfo->deathRecipient != nullptr) {
        IMSA_HILOGI("deathRecipient remove");
        client->RemoveDeathRecipient(clientInfo->deathRecipient);
        clientInfo->deathRecipient = nullptr;
    }
    mapClients_.erase(client);
    IMSA_HILOGD("client[%{public}d] is removed successfully", clientInfo->pid);
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
int32_t PerUserSession::ShowKeyboard(const sptr<IInputDataChannel> &channel, const sptr<IInputClient> &inputClient,
    bool isShowKeyboard, bool attachFlag)
{
    IMSA_HILOGD("PerUserSession, run in");
    if (inputClient == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto core = GetImsCore(CURRENT_IME);
    if (core == nullptr) {
        IMSA_HILOGE("Aborted! imsCore[%{public}d] is nullptr", CURRENT_IME);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    int32_t ret = core->ShowKeyboard(channel, isShowKeyboard, attachFlag);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to show keyboard, ret: %{public}d", ret);
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
        return ErrorCode::ERROR_NULL_POINTER;
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
void PerUserSession::OnImsDied(const sptr<IInputMethodCore> &remote)
{
    if (remote == nullptr) {
        return;
    }
    ClearImeData(CURRENT_IME);
    if (GetCurrentClient() == nullptr) {
        IMSA_HILOGD("not bound to a client, no need to restart at once");
        return;
    }
    if (!IsRestartIme(CURRENT_IME)) {
        IMSA_HILOGI("ime deaths over max num");
        return;
    }
    IMSA_HILOGI("user %{public}d ime died, restart!", userId_);
    StartInputService(ImeInfoInquirer::GetInstance().GetStartedIme(userId_), true);
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
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        IMSA_HILOGE("client info not found");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    return ShowKeyboard(clientInfo->channel, client, true, false);
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
        IMSA_HILOGD("client not found");
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
    return AddClient(clientInfo.client->AsObject(), clientInfo, PREPARE_INPUT);
}

int32_t PerUserSession::SendAgentToSingleClient(const sptr<IInputClient> &client)
{
    IMSA_HILOGD("PerUserSession::SendAgentToSingleClient");
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGI("agent is nullptr");
        InputMethodSysEvent::CreateComponentFailed(userId_, ErrorCode::ERROR_NULL_POINTER);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return client->OnInputReady(agent);
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
int32_t PerUserSession::OnStartInput(const sptr<IInputClient> &client, bool isShowKeyboard, bool attachFlag)
{
    IMSA_HILOGD("start input with keyboard[%{public}d], attchFlag[%{public}d]", isShowKeyboard, attachFlag);
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    if (GetImsCore(CURRENT_IME) == nullptr) {
        IMSA_HILOGI("current ime is empty, try to restart it");
        if (!StartInputService(ImeInfoInquirer::GetInstance().GetStartedIme(userId_), true)) {
            IMSA_HILOGE("failed to restart ime");
            return ErrorCode::ERROR_IME_START_FAILED;
        }
    }

    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        IMSA_HILOGE("client not found");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    // build channel from imc to ima
    int32_t ret = SendAgentToSingleClient(client);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    // build channel from ima to imc
    return ShowKeyboard(clientInfo->channel, client, isShowKeyboard, attachFlag);
}

int32_t PerUserSession::OnSetCoreAndAgent(const sptr<IInputMethodCore> &core, const sptr<IInputMethodAgent> &agent)
{
    IMSA_HILOGD("PerUserSession::SetCoreAndAgent Start");
    if (core == nullptr || agent == nullptr) {
        IMSA_HILOGE("PerUserSession::SetCoreAndAgent core or agent nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    if (imsDeathRecipient_ == nullptr || core->AsObject() == nullptr) {
        IMSA_HILOGE("imsDeathRecipient_ or core as object is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    imsDeathRecipient_->SetDeathRecipient([this, core](const wptr<IRemoteObject> &) { this->OnImsDied(core); });
    if (!core->AsObject()->AddDeathRecipient(imsDeathRecipient_)) {
        IMSA_HILOGE("failed to add death recipient");
        return ErrorCode::ERROR_ADD_DEATH_RECIPIENT_FAILED;
    }
    SetImsCore(CURRENT_IME, core);
    SetAgent(agent);

    int ret = InitInputControlChannel();
    IMSA_HILOGI("init input control channel ret: %{public}d", ret);
    auto client = GetCurrentClient();
    if (client != nullptr) {
        auto clientInfo = GetClientInfo(client->AsObject());
        if (clientInfo != nullptr) {
            ret = OnStartInput(clientInfo->client, clientInfo->isShowKeyboard, true);
            IMSA_HILOGI("start input ret: %{public}d", ret);
        }
    }
    bool isStarted = GetImsCore(CURRENT_IME) != nullptr && GetAgent() != nullptr;
    isImeStarted_.SetValue(isStarted);
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::InitInputControlChannel()
{
    IMSA_HILOGD("PerUserSession::InitInputControlChannel");
    sptr<IInputControlChannel> inputControlChannel = new InputControlChannelStub(userId_);
    auto core = GetImsCore(CURRENT_IME);
    if (core == nullptr) {
        IMSA_HILOGE("PerUserSession::InitInputControlChannel core is nullptr");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    return core->InitInputControlChannel(
        inputControlChannel, ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->imeId);
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
    SetImsCore(CURRENT_IME, nullptr);
    SetAgent(nullptr);
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
    auto core = GetImsCore(index);
    if (core != nullptr) {
        core->AsObject()->RemoveDeathRecipient(imsDeathRecipient_);
        SetImsCore(index, nullptr);
    }
    SetAgent(nullptr);
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

int32_t PerUserSession::OnSwitchIme(const Property &property, const SubProperty &subProperty, bool isSubtypeSwitch)
{
    IMSA_HILOGD("PerUserSession::OnSwitchIme");
    if (isSubtypeSwitch) {
        sptr<IInputMethodCore> core = GetImsCore(CURRENT_IME);
        if (core == nullptr) {
            IMSA_HILOGE("imsCore is nullptr");
            return ErrorCode::ERROR_IME_NOT_STARTED;
        }
        int32_t ret = core->SetSubtype(subProperty);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("PerUserSession::SetSubtype failed, ret %{public}d", ret);
            return ret;
        }
    }
    std::lock_guard<std::recursive_mutex> lock(mtx);
    for (const auto &client : mapClients_) {
        auto clientInfo = client.second;
        if (clientInfo == nullptr || !EventStatusManager::IsImeChangeOn(clientInfo->eventFlag)) {
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
    return ErrorCode::NO_ERROR;
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

sptr<IInputMethodAgent> PerUserSession::GetAgent()
{
    std::lock_guard<std::mutex> lock(agentLock_);
    return agent_;
}

void PerUserSession::SetAgent(sptr<IInputMethodAgent> agent)
{
    std::lock_guard<std::mutex> lock(agentLock_);
    agent_ = agent;
}

void PerUserSession::OnFocused(int32_t pid, int32_t uid)
{
    if (IsCurrentClient(pid, uid)) {
        IMSA_HILOGD("pid[%{public}d] same as current client", pid);
        return;
    }
    auto client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGD("no client in bound state");
        return;
    }
    IMSA_HILOGI("focus shifts to pid: %{public}d, start unbinding", pid);
    UnbindClient(client);
    InputMethodSysEvent::OperateSoftkeyboardBehaviour(IME_HIDE_UNFOCUSED);
}

void PerUserSession::OnUnfocused(int32_t pid, int32_t uid)
{
    if (IsCurrentClient(pid, uid)) {
        IMSA_HILOGD("pid[%{public}d] same as current client", pid);
        return;
    }
    for (auto &mapClient : mapClients_) {
        if (mapClient.second->pid == pid) {
            IMSA_HILOGI("clear unfocused client info: %{public}d", pid);
            UnbindClient(mapClient.second->client);
            InputMethodSysEvent::OperateSoftkeyboardBehaviour(IME_HIDE_UNFOCUSED);
            break;
        }
    }
}

void PerUserSession::UnbindClient(const sptr<IInputClient> &client)
{
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr");
        return;
    }
    int32_t ret = client->OnInputStop();
    IMSA_HILOGI("OnInputStop ret: %{public}d", ret);
    ret = OnReleaseInput(client);
    IMSA_HILOGI("release input ret: %{public}d", ret);
}

bool PerUserSession::IsCurrentClient(int32_t pid, int32_t uid)
{
    auto client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGD("no client in bound state");
        return false;
    }
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        IMSA_HILOGE("failed to get client info");
        return false;
    }
    return clientInfo->pid == pid && clientInfo->uid == uid;
}

sptr<AAFwk::IAbilityManager> PerUserSession::GetAbilityManagerService()
{
    IMSA_HILOGD("InputMethodSystemAbility::GetAbilityManagerService start");
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        IMSA_HILOGE("SystemAbilityManager is nullptr.");
        return nullptr;
    }
    auto abilityMsObj = samgr->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (abilityMsObj == nullptr) {
        IMSA_HILOGE("Failed to get ability manager service.");
        return nullptr;
    }
    return iface_cast<AAFwk::IAbilityManager>(abilityMsObj);
}

bool PerUserSession::StartInputService(const std::string &imeName, bool isRetry)
{
    IMSA_HILOGI("start ime: %{public}s with isRetry: %{public}d", imeName.c_str(), isRetry);
    std::string::size_type pos = imeName.find('/');
    if (pos == std::string::npos) {
        IMSA_HILOGE("invalid ime name");
        return false;
    }
    auto abms = GetAbilityManagerService();
    if (abms == nullptr) {
        IMSA_HILOGE("failed to get ability manager service");
        return false;
    }
    IMSA_HILOGI("ime: %{public}s", imeName.c_str());
    AAFwk::Want want;
    want.SetElementName(imeName.substr(0, pos), imeName.substr(pos + 1));
    isImeStarted_.Clear(false);
    if (abms->StartAbility(want) != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to start ability");
    } else if (isImeStarted_.GetValue()) {
        IMSA_HILOGI("ime started successfully");
        return true;
    }
    if (isRetry) {
        IMSA_HILOGE("failed to start ime, begin to retry five times");
        auto retryTask = [this, imeName]() {
            pthread_setname_np(pthread_self(), "ImeRestart");
            BlockRetry(IME_RESTART_INTERVAL, IME_RESTART_TIMES,
                [this, imeName]() { return StartInputService(imeName, false); });
        };
        std::thread(retryTask).detach();
    }
    return false;
}

bool PerUserSession::CheckFocused(uint32_t tokenId)
{
    auto client = GetCurrentClient();
    if (client == nullptr) {
        return false;
    }
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        return false;
    }
    return clientInfo->tokenID == tokenId;
}

int32_t PerUserSession::OnPanelStatusChange(const InputWindowStatus &status, const InputWindowInfo &windowInfo)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    for (const auto &client : mapClients_) {
        auto clientInfo = client.second;
        if (clientInfo == nullptr) {
            IMSA_HILOGD("client nullptr or no need to notify");
            continue;
        }
        if (status == InputWindowStatus::SHOW && !EventStatusManager::IsImeShowOn(clientInfo->eventFlag)) {
            IMSA_HILOGD("has no imeShow callback");
            continue;
        }
        if (status == InputWindowStatus::HIDE && !EventStatusManager::IsImeHideOn(clientInfo->eventFlag)) {
            IMSA_HILOGD("has no imeHide callback");
            continue;
        }
        int32_t ret = clientInfo->client->OnPanelStatusChange(status, { windowInfo });
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("OnPanelStatusChange failed, ret: %{public}d", ret);
            continue;
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnUpdateListenEventFlag(const InputClientInfo &clientInfo)
{
    auto remoteClient = clientInfo.client->AsObject();
    auto ret = AddClient(remoteClient, clientInfo, START_LISTENING);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("AddClient failed");
        return ret;
    }
    auto info = GetClientInfo(remoteClient);
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx);
    if (info->eventFlag == EventStatusManager::NO_EVENT_ON && !info->isValid) {
        remoteClient->RemoveDeathRecipient(info->deathRecipient);
        info->deathRecipient = nullptr;
        mapClients_.erase(remoteClient);
    }
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS
