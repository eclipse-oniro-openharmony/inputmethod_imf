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
#include "ability_manager_client.h"
#include "identity_checker_impl.h"
#include "ime_connection.h"
#include "ime_info_inquirer.h"
#include "input_control_channel_stub.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "mem_mgr_client.h"
#include "on_demand_start_stop_sa.h"
#include "os_account_adapter.h"
#include "scene_board_judgement.h"
#include "security_mode_parser.h"
#include "system_ability_definition.h"
#include "wms_connection_observer.h"
#include "dm_common.h"
#include "display_manager.h"
#include "parameters.h"

namespace OHOS {
namespace MiscServices {
using namespace std::chrono;
using namespace MessageID;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Rosen;
constexpr int64_t INVALID_PID = -1;
constexpr uint32_t STOP_IME_TIME = 600;
constexpr const char *STRICT_MODE = "strictMode";
constexpr const char *ISOLATED_SANDBOX = "isolatedSandbox";
constexpr uint32_t CHECK_IME_RUNNING_RETRY_INTERVAL = 60;
constexpr uint32_t CHECK_IME_RUNNING_RETRY_TIMES = 10;
constexpr int32_t MAX_RESTART_NUM = 3;
constexpr int32_t IME_RESET_TIME_OUT = 3;
constexpr int32_t MAX_RESTART_TASKS = 2;
const std::string FOLD_SCREEN_TYPE = OHOS::system::GetParameter("const.window.foldscreen.type", "0,0,0,0");
PerUserSession::PerUserSession(int userId) : userId_(userId) { }

PerUserSession::PerUserSession(int32_t userId, const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler)
    : userId_(userId), eventHandler_(eventHandler)
{
    // if bms not start, AppMgrClient::GetProcessRunningInfosByUserId will blocked
    if (IsSaReady(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID)) {
        auto bundleNames = ImeInfoInquirer::GetInstance().GetRunningIme(userId_);
        if (!bundleNames.empty()) {
            runningIme_ = bundleNames[0]; // one user only has one ime at present
        }
    }
}

PerUserSession::~PerUserSession() { }

int PerUserSession::AddClientInfo(
    sptr<IRemoteObject> inputClient, const InputClientInfo &clientInfo, ClientAddEvent event)
{
    IMSA_HILOGD("PerUserSession start.");
    auto clientGroup = GetClientGroup(clientInfo.displayGroupID);
    if (clientGroup != nullptr) {
        return clientGroup->AddClientInfo(inputClient, clientInfo, event);
    }
    clientGroup = std::make_shared<ClientGroup>(clientInfo.displayId);
    auto ret = clientGroup->AddClientInfo(inputClient, clientInfo, event);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to AddClientInfo: %{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> lock(clientGroupLock_);
    clientGroupMap_.insert(std::make_pair(clientInfo.displayGroupID, clientGroup));
    IMSA_HILOGI("add client group end.");
    return ErrorCode::NO_ERROR;
}

uint64_t PerUserSession::GetDisplayGroupId(uint64_t displayId)
{
    IMSA_HILOGD("displayId: %{public} " PRIu64 "", displayId);
    if (displayId == DEFAULT_DISPLAY_ID || virtualScreenDisplayId_.size() == 0) {
        return DEFAULT_DISPLAY_ID;
    }
    if (virtualScreenDisplayId_.find(displayId) != virtualScreenDisplayId_.end()) {
        return displayId;
    }
    return DEFAULT_DISPLAY_ID;
}

std::shared_ptr<ClientGroup> PerUserSession::GetClientGroup(uint64_t displayId)
{
    DisplayId displayGroupId = GetDisplayGroupId(displayId);
    std::lock_guard<std::mutex> lock(clientGroupLock_);
    if (displayGroupId == DEFAULT_DISPLAY_ID) {
        return clientGroupMap_[DEFAULT_DISPLAY_ID];
    }
    auto iter = clientGroupMap_.find(displayGroupId);
    if (iter == clientGroupMap_.end()) {
        IMSA_HILOGD("client group with displayId: %{public}" PRIu64 "", displayId);
        return nullptr;
    }
    return iter->second;
}

std::shared_ptr<ClientGroup> PerUserSession::GetClientGroup(sptr<IRemoteObject> client)
{
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(clientGroupLock_);
    auto iter = std::find_if(clientGroupMap_.begin(), clientGroupMap_.end(),
        [client](const auto &clientGroup) { return clientGroup.second->IsClientExist(client); });
    if (iter == clientGroupMap_.end()) {
        IMSA_HILOGD("not found");
        return nullptr;
    }
    return iter->second;
}

int32_t PerUserSession::HideKeyboard(
    const sptr<IInputClient> &currentClient, const std::shared_ptr<ClientGroup> &clientGroup)
{
    IMSA_HILOGD("PerUserSession::HideKeyboard start.");
    if (currentClient == nullptr) {
        IMSA_HILOGE("current client is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto clientInfo = clientGroup->GetClientInfo(currentClient->AsObject());
    if (clientInfo == nullptr) {
        IMSA_HILOGE("client info is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    auto data = GetReadyImeData(clientInfo->bindImeType);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist!", clientInfo->bindImeType);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    auto ret = RequestIme(data, RequestType::NORMAL, [&data] { return data->core->HideKeyboard(); });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to hide keyboard, ret: %{public}d!", ret);
        return ErrorCode::ERROR_KBD_HIDE_FAILED;
    }
    bool isShowKeyboard = false;
    clientGroup->UpdateClientInfo(currentClient->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, isShowKeyboard } });
    RestoreCurrentImeSubType();
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::ShowKeyboard(const sptr<IInputClient> &currentClient,
    const std::shared_ptr<ClientGroup> &clientGroup, int32_t requestKeyboardReason)
{
    if (currentClient == nullptr || clientGroup == nullptr) {
        IMSA_HILOGE("client is nullptr");
        return ErrorCode::ERROR_IMSA_NULLPTR;
    }
    auto clientInfo = clientGroup->GetClientInfo(currentClient->AsObject());
    if (clientInfo == nullptr) {
        IMSA_HILOGE("client info is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    auto data = GetReadyImeData(clientInfo->bindImeType);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist!", clientInfo->bindImeType);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    auto ret = RequestIme(data, RequestType::REQUEST_SHOW, [&data] { return data->core->ShowKeyboard(); });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to show keyboard, ret: %{public}d!", ret);
        return ErrorCode::ERROR_KBD_SHOW_FAILED;
    }
    bool isShowKeyboard = true;
    clientGroup->UpdateClientInfo(currentClient->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, isShowKeyboard } });
    NotifyInputStartToClients(clientInfo->config.windowId, clientGroup, requestKeyboardReason);
    return ErrorCode::NO_ERROR;
}

/** Handle the situation a remote input client died.
 * It's called when a remote input client died
 * @param the remote object handler of the input client died.
 */
void PerUserSession::OnClientDied(sptr<IInputClient> remote)
{
    auto clientGroup = GetClientGroup(remote);
    if (clientGroup == nullptr) {
        return;
    }
    if (IsNotifyInputStop(remote, clientGroup)) {
        NotifyInputStopToClients(clientGroup);
    }
    if (remote == nullptr) {
        return;
    }
    auto clientInfo = clientGroup->GetClientInfo(remote->AsObject());
    IMSA_HILOGI("userId: %{public}d.", userId_);
    if (IsSameClient(remote, clientGroup->GetCurrentClient())) {
        if (clientInfo != nullptr) {
            StopImeInput(clientInfo->bindImeType, clientInfo->channel);
        }
        clientGroup->SetCurrentClient(nullptr);
        RestoreCurrentImeSubType();
    }
    if (IsSameClient(remote, clientGroup->GetInactiveClient())) {
        if (clientInfo != nullptr) {
            StopImeInput(clientInfo->bindImeType, clientInfo->channel);
        }
        clientGroup->SetInactiveClient(nullptr);
        RestoreCurrentImeSubType();
    }
    clientGroup->RemoveClientInfo(remote->AsObject(), true);
}

/** Handle the situation that an ime died
 * It's called when an ime died
 * @param the remote object handler of the ime who died.
 */
void PerUserSession::OnImeDied(const sptr<IInputMethodCore> &remote, ImeType type)
{
    if (remote == nullptr) {
        return;
    }
    IMSA_HILOGI("type: %{public}d.", type);
    auto imeData = GetImeData(type);
    if (imeData != nullptr && imeData->imeStatus == ImeStatus::EXITING) {
        RemoveImeData(type, true);
        InputTypeManager::GetInstance().Set(false); // 如果当前是协同也需要置为false吗
        NotifyImeStopFinished();
        IMSA_HILOGI("%{public}d not current imeData.", type);
        return;
    }
    RemoveImeData(type, true);
    if (!OsAccountAdapter::IsOsAccountForeground(userId_)) {
        IMSA_HILOGW("userId:%{public}d in background, no need to restart ime.", userId_);
        return;
    }
    auto clientInfo = GetCurClientInfo();
    if (clientInfo != nullptr && clientInfo->bindImeType == type) {
        NotifyInputStopToClients();
        StopClientInput(clientInfo);
        if (type == ImeType::IME) {
            StartImeInImeDied();
        }
        return;
    }
    auto currentImeInfo = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    if (currentImeInfo == nullptr) {
        IMSA_HILOGE("currentImeInfo is nullptr!");
        return;
    }
    auto defaultImeInfo = ImeInfoInquirer::GetInstance().GetDefaultImeCfgProp();
    if (defaultImeInfo == nullptr) {
        IMSA_HILOGE("defaultImeInfo is nullptr!");
        return;
    }
    if (type == ImeType::IME && currentImeInfo->bundleName == defaultImeInfo->name) {
        StartImeInImeDied();
    }
}

int32_t PerUserSession::RemoveIme(const sptr<IInputMethodCore> &core, ImeType type)
{
    if (core == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto data = GetReadyImeData(type);
    if (data == nullptr || data->core->AsObject() != core->AsObject()) {
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }

    auto clientInfo = GetCurClientInfo();
    if (clientInfo != nullptr && clientInfo->bindImeType == type) {
        UnBindClientWithIme(clientInfo);
    }
    RemoveImeData(type, true);
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnHideCurrentInput(uint64_t displayId)
{
    auto clientGroup = GetClientGroup(displayId);
    if (clientGroup == nullptr) {
        IMSA_HILOGE("client group not found");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    sptr<IInputClient> client = clientGroup->GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGE("current client is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    return HideKeyboard(client, clientGroup);
}

int32_t PerUserSession::OnShowCurrentInput(uint64_t displayId)
{
    IMSA_HILOGD("PerUserSession::OnShowCurrentInput start.");
    auto clientGroup = GetClientGroup(displayId);
    if (clientGroup == nullptr) {
        IMSA_HILOGE("client group not found");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    sptr<IInputClient> client = clientGroup->GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGE("current client is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    return ShowKeyboard(client, clientGroup);
}

int32_t PerUserSession::OnHideInput(sptr<IInputClient> client)
{
    IMSA_HILOGD("PerUserSession::OnHideInput start.");
    auto clientGroup = GetClientGroup(client);
    if (clientGroup == nullptr) {
        IMSA_HILOGE("client group not found");
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    if (!IsSameClient(client, clientGroup->GetCurrentClient())) {
        IMSA_HILOGE("client is not current client!");
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    return HideKeyboard(client, clientGroup);
}

int32_t PerUserSession::OnShowInput(sptr<IInputClient> client, int32_t requestKeyboardReason)
{
    IMSA_HILOGD("PerUserSession::OnShowInput start.");
    auto clientGroup = GetClientGroup(client);
    if (clientGroup == nullptr) {
        IMSA_HILOGE("client group not found");
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    if (!IsSameClient(client, clientGroup->GetCurrentClient())) {
        IMSA_HILOGE("client is not current client!");
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    return ShowKeyboard(client, clientGroup, requestKeyboardReason);
}

void PerUserSession::OnHideSoftKeyBoardSelf()
{
    IMSA_HILOGD("PerUserSession::OnHideSoftKeyBoardSel start.");
    auto clientGroup = GetClientGroup(DEFAULT_DISPLAY_ID);
    if (clientGroup == nullptr) {
        IMSA_HILOGE("current client is nullptr!");
        return;
    }
    sptr<IInputClient> client = clientGroup->GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGE("current client is nullptr!");
        return;
    }
    clientGroup->UpdateClientInfo(client->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, false } });
    RestoreCurrentImeSubType();
}

int32_t PerUserSession::OnRequestShowInput(uint64_t displayId)
{
    IMSA_HILOGD("PerUserSession::OnRequestShowInput start.");
    auto clientGroup = GetClientGroup(displayId);
    if (clientGroup == nullptr) {
        IMSA_HILOGE("no current client");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    ImeType type = ImeType::NONE;
    if (GetDisplayGroupId(displayId) == DEFAULT_DISPLAY_ID) {
        type = ImeType::IME;
    } else if (displayId == aiAgentDisplayId_) {
        type = ImeType::AI_AGENT_IME;
    }
    auto data = GetReadyImeData(type);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d doesn't exist!", ImeType::IME);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    auto ret = RequestIme(data, RequestType::REQUEST_SHOW, [&data] { return data->core->ShowKeyboard(); });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to show keyboard, ret: %{public}d!", ret);
        return ErrorCode::ERROR_KBD_SHOW_FAILED;
    }
    InputMethodSysEvent::GetInstance().ReportImeState(
        ImeState::BIND, data->pid, ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName);
    Memory::MemMgrClient::GetInstance().SetCritical(getpid(), true, INPUT_METHOD_SYSTEM_ABILITY_ID);
    auto currentClient = clientGroup->GetCurrentClient();
    if (currentClient != nullptr) {
        clientGroup->UpdateClientInfo(currentClient->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, true } });
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnRequestHideInput(int32_t callingPid, uint64_t displayId)
{
    IMSA_HILOGD("PerUserSession::OnRequestHideInput start.");
    ImeType imeType = GetCurDisplayImeType(displayId);
    auto data = GetReadyImeData(imeType);
    if (data != nullptr) {
        auto ret = RequestIme(data, RequestType::REQUEST_HIDE, [&data] { return data->core->HideKeyboard(); });
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("failed to hide keyboard, ret: %{public}d!", ret);
            return ErrorCode::ERROR_KBD_HIDE_FAILED;
        }
    }

    auto clientGroup = GetClientGroup(displayId);
    if (clientGroup != nullptr) {
        auto currentClient = clientGroup->GetCurrentClient();
        auto currentClientInfo = clientGroup->GetCurrentClientInfo();
        if (currentClientInfo != nullptr && currentClient != nullptr) {
            clientGroup->UpdateClientInfo(currentClient->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, false } });
        }
        auto inactiveClient = clientGroup->GetInactiveClient();
        if (inactiveClient != nullptr) {
            RemoveClient(inactiveClient, clientGroup, false, true);
        }
    }
    RestoreCurrentImeSubType();
    NotifyInputStopToClients(clientGroup);
    return ErrorCode::NO_ERROR;
}

/** Get ClientInfo
 * @param inputClient the IRemoteObject remote handler of given input client
 * @return a pointer of ClientInfo if client is found
 *         null if client is not found
 * @note the clientInfo pointer should not be freed by caller
 */
std::shared_ptr<InputClientInfo> PerUserSession::GetClientInfo(sptr<IRemoteObject> inputClient)
{
}

int32_t PerUserSession::OnPrepareInput(const InputClientInfo &clientInfo)
{
    IMSA_HILOGD("PerUserSession::OnPrepareInput start");
    return AddClientInfo(clientInfo.client->AsObject(), clientInfo, PREPARE_INPUT);
}

/** Release input. Called by an input client.Run in work thread of this user
 * @param the parameters from remote client
 * @return ErrorCode
 */
int32_t PerUserSession::OnReleaseInput(const sptr<IInputClient> &client)
{
    IMSA_HILOGD("PerUserSession::OnReleaseInput start");
    auto clientGroup = GetClientGroup(client);
    if (clientGroup == nullptr) {
        IMSA_HILOGD("client not found");
        return ErrorCode::NO_ERROR;
    }
    bool isReady = IsNotifyInputStop(client, clientGroup);
    int32_t ret = RemoveClient(client, clientGroup, true);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("remove client failed");
        return ret;
    }
    if (isReady) {
        NotifyInputStopToClients(clientGroup);
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::RemoveClient(const sptr<IInputClient> &client, const std::shared_ptr<ClientGroup> &clientGroup,
    bool isUnbindFromClient, bool isInactiveClient, bool isNotifyClientAsync)
{
    if (client == nullptr || clientGroup == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    // if client is current client, unbind firstly
    auto clientInfo = clientGroup->GetClientInfo(client->AsObject());
    if (IsSameClient(client, clientGroup->GetCurrentClient())) {
        UnBindClientWithIme(clientInfo, isUnbindFromClient, isNotifyClientAsync);
        clientGroup->SetCurrentClient(nullptr);
        RestoreCurrentImeSubType();
        StopClientInput(clientInfo, false, isNotifyClientAsync);
    }
    if (IsSameClient(client, clientGroup->GetInactiveClient())) {
        clientGroup->SetInactiveClient(nullptr);
        StopClientInput(clientInfo, isInactiveClient);
    }
    clientGroup->RemoveClientInfo(client->AsObject());
    return ErrorCode::NO_ERROR;
}

bool PerUserSession::IsProxyImeEnable()
{
    auto data = GetReadyImeData(ImeType::PROXY_IME);
    return data != nullptr && data->core != nullptr && data->core->IsEnable();
}

int32_t PerUserSession::OnStartInput(
    const InputClientInfo &clientInfo, sptr<IRemoteObject> &agent, std::pair<int64_t, std::string> &imeInfo)
{
    auto clientGroup = GetClientGroup(clientInfo.displayId);
    if (clientGroup == nullptr) {
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    const sptr<IInputClient> &client = clientInfo.client;
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto tempClientInfo = clientGroup->GetClientInfo(client->AsObject());
    if (tempClientInfo == nullptr) {
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    InputClientInfo infoTemp = *tempClientInfo;
    IMSA_HILOGD("start input with keyboard[%{public}d].", clientInfo.isShowKeyboard);
    infoTemp.isNotifyInputStart = clientInfo.isNotifyInputStart;
    ImeType imeType;
    if (GetDisplayGroupId(clientInfo.displayId) == DEFAULT_DISPLAY_ID) {
        HandleImeBindTypeChanged(infoTemp, clientGroup);
        imeType = IsProxyImeEnable() ? ImeType::PROXY_IME : ImeType::IME;
    } else if (clientInfo.displayId == aiAgentDisplayId_) {
        imeType = ImeType::AI_AGENT_IME;
    }
    infoTemp.isShowKeyboard = clientInfo.isShowKeyboard;
    infoTemp.needHide = clientInfo.needHide;
    infoTemp.requestKeyboardReason = clientInfo.requestKeyboardReason;
    int32_t ret = BindClientWithIme(std::make_shared<InputClientInfo>(infoTemp), imeType, true, clientInfo.displayId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("bind failed, ret: %{public}d!", ret);
        return ret;
    }
    auto data = GetReadyImeData(imeType);
    if (data == nullptr || data->agent == nullptr) {
        IMSA_HILOGE("data or agent is nullptr!");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    agent = data->agent;
    imeInfo = { data->pid, data->ime.first };
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::BindClientWithIme(const std::shared_ptr<InputClientInfo> &clientInfo,
    const std::shared_ptr<ClientGroup> clientGroup, ImeType type, bool isBindFromClient)
{
    if (clientInfo == nullptr || clientGroup == nullptr) {
        IMSA_HILOGE("clientInfo or clientGroup is nullptr!");
        return ErrorCode::ERROR_IMSA_NULLPTR;
    }
    IMSA_HILOGD("imeType: %{public}d, isShowKeyboard: %{public}d, isBindFromClient: %{public}d.", type,
        clientInfo->isShowKeyboard, isBindFromClient);
    auto data = GetValidIme(type);
    if (data == nullptr) {
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    auto ret = RequestIme(data, RequestType::START_INPUT,
        [&data, &clientInfo, isBindFromClient]() { return data->core->StartInput(*clientInfo, isBindFromClient); });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("start input failed, ret: %{public}d!", ret);
        return ErrorCode::ERROR_IME_START_INPUT_FAILED;
    }
    if (type == ImeType::IME) {
        InputMethodSysEvent::GetInstance().ReportImeState(
            ImeState::BIND, data->pid, ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName);
        Memory::MemMgrClient::GetInstance().SetCritical(getpid(), true, INPUT_METHOD_SYSTEM_ABILITY_ID);
    }
    if (!isBindFromClient) {
        ret = clientInfo->client->OnInputReady(data->agent, { data->pid, data->ime.first });
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("start client input failed, ret: %{public}d!", ret);
            return ErrorCode::ERROR_IMSA_CLIENT_INPUT_READY_FAILED;
        }
    }
    clientGroup->UpdateClientInfo(clientInfo->client->AsObject(),
        { { UpdateFlag::BINDIMETYPE, type }, { UpdateFlag::ISSHOWKEYBOARD, clientInfo->isShowKeyboard },
            { UpdateFlag::STATE, ClientState::ACTIVE } });
    ReplaceCurrentClient(clientInfo->client, clientGroup);
    if (clientInfo->isShowKeyboard) {
        NotifyInputStartToClients(
            clientInfo->config.windowId, clientGroup, static_cast<int32_t>(clientInfo->requestKeyboardReason));
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::BindClientWithIme(
    const std::shared_ptr<InputClientInfo> &clientInfo, ImeType type, bool isBindFromClient, uint64_t displayId)
{
    auto clientGroup = GetClientGroup(displayId);
    if (clientGroup == nullptr) {
        IMSA_HILOGE("not found group");
        return ErrorCode::ERROR_IMSA_NULLPTR;
    }
    return BindClientWithIme(clientInfo, clientGroup, type, isBindFromClient);
}

void PerUserSession::UnBindClientWithIme(
    const std::shared_ptr<InputClientInfo> &currentClientInfo, bool isUnbindFromClient, bool isNotifyClientAsync)
{
    if (currentClientInfo == nullptr) {
        return;
    }
    if (!isUnbindFromClient) {
        IMSA_HILOGD("unbind from service.");
        StopClientInput(currentClientInfo, isNotifyClientAsync);
    }
    StopImeInput(currentClientInfo->bindImeType, currentClientInfo->channel);
}

void PerUserSession::StopClientInput(
    const std::shared_ptr<InputClientInfo> &clientInfo, bool isStopInactiveClient, bool isAsync)
{
    if (clientInfo == nullptr || clientInfo->client == nullptr) {
        return;
    }
    auto ret = clientInfo->client->OnInputStop(isStopInactiveClient, isAsync);
    IMSA_HILOGI("isStopInactiveClient: %{public}d, client pid: %{public}d, ret: %{public}d.", isStopInactiveClient,
        clientInfo->pid, ret);
}

void PerUserSession::StopImeInput(ImeType currentType, const sptr<IRemoteObject> &currentChannel)
{
    auto data = GetReadyImeData(currentType);
    if (data == nullptr) {
        return;
    }
    auto ret = RequestIme(data, RequestType::STOP_INPUT,
        [&data, &currentChannel]() { return data->core->StopInput(currentChannel); });
    IMSA_HILOGI("stop ime input, ret: %{public}d.", ret);
    if (ret == ErrorCode::NO_ERROR && currentType == ImeType::IME) {
        InputMethodSysEvent::GetInstance().ReportImeState(ImeState::UNBIND, data->pid,
            ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName);
        Memory::MemMgrClient::GetInstance().SetCritical(getpid(), false, INPUT_METHOD_SYSTEM_ABILITY_ID);
    }
    if (currentType == ImeType::IME) {
        RestoreCurrentImeSubType();
    }
}

void PerUserSession::OnSecurityChange(int32_t security)
{
    auto data = GetReadyImeData(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist!", ImeType::IME);
        return;
    }
    auto ret =
        RequestIme(data, RequestType::NORMAL, [&data, security] { return data->core->OnSecurityChange(security); });
    IMSA_HILOGD("on security change, ret: %{public}d.", ret);
}

int32_t PerUserSession::OnSetCoreAndAgent(const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent)
{
    IMSA_HILOGI("start.");
    auto ret = UpdateImeData(core, agent, IPCSkeleton::GetCallingPid());
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    auto action = GetImeAction(ImeEvent::SET_CORE_AND_AGENT);
    if (action == ImeAction::DO_NOTHING) {
        return ErrorCode::NO_ERROR;
    }
    if (action != ImeAction::DO_SET_CORE_AND_AGENT) {
        return ErrorCode::ERROR_IME;
    }
    ret = InitInputControlChannel();
    IMSA_HILOGI("init input control channel ret: %{public}d.", ret);
    auto imeType = ImeType::IME;
    auto clientInfo = GetCurClientInfo();
    if (clientInfo != nullptr && IsImeStartInBind(clientInfo->bindImeType, imeType)) {
        BindClientWithIme(clientInfo, imeType);
        SetInputType();
    }
    bool isStarted = true;
    isImeStarted_.SetValue(isStarted);
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnRegisterProxyIme(const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent)
{
    IMSA_HILOGD("start.");
    auto imeType = ImeType::PROXY_IME;
    auto ret = AddImeData(imeType, core, agent, IPCSkeleton::GetCallingPid());
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    auto clientInfo = GetCurClientInfo();
    if (clientInfo != nullptr) {
        if (IsProxyImeStartInBind(clientInfo->bindImeType, imeType)) {
            BindClientWithIme(clientInfo, imeType);
        }
        if (IsProxyImeStartInImeBind(clientInfo->bindImeType, imeType)) {
            UnBindClientWithIme(clientInfo);
            BindClientWithIme(clientInfo, imeType);
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnRegisterProxyIme(
    uint64_t displayId, const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent)
{
    IMSA_HILOGD("start. displayId: %{public}d " PRIu64 "", displayId);
    auto imeType = ImeType::AI_AGENT_IME;
    auto ret = AddImeData(imeType, core, agent, IPCSkeleton::GetCallingPid());
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    virtualScreenDisplayId_.emplace(displayId);
    auto clientInfo = GetCurClientInfo();
    if (clientInfo != nullptr) {
        if (IsProxyImeStartInBind(clientInfo->bindImeType, imeType)) {
            BindClientWithIme(clientInfo, imeType);
        }
        if (IsProxyImeStartInImeBind(clientInfo->bindImeType, imeType)) {
            UnBindClientWithIme(clientInfo);
            BindClientWithIme(clientInfo, imeType);
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnUnregisterProxyIme(uint64_t displayId)
{
    IMSA_HILOGD("start. displayId: %{public}d " PRIu64 "", displayId);
    if (displayId != aiAgentDisplayId_) {
        return false; // 需要考虑一下多线程并发场景
    }
    RemoveImeData(ImeType::AI_AGENT_IME, false);
    auto clientInfo = GetCurClientInfo(displayId);
    if (clientInfo == nullptr) {
        IMSA_HILOGD("no current client");
        return ErrorCode::NO_ERROR;
    }
    UnBindClientWithIme(clientInfo);
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnUnRegisteredProxyIme(UnRegisteredType type, const sptr<IInputMethodCore> &core)
{
    IMSA_HILOGD("proxy unregister type: %{public}d.", type);
    // 0: stop proxy  1: switch to ima
    if (type == UnRegisteredType::REMOVE_PROXY_IME) {
        RemoveIme(core, ImeType::PROXY_IME);
        return ErrorCode::NO_ERROR;
    }
    if (type == UnRegisteredType::SWITCH_PROXY_IME_TO_IME) {
        auto clientInfo = GetCurClientInfo();
        if (clientInfo == nullptr) {
            IMSA_HILOGE("not found current client!");
            return ErrorCode::ERROR_CLIENT_NOT_BOUND;
        }
        if (clientInfo->bindImeType == ImeType::PROXY_IME) {
            UnBindClientWithIme(clientInfo);
        }
        InputClientInfo infoTemp = { .isShowKeyboard = true,
            .client = clientInfo->client,
            .channel = clientInfo->channel };
        return BindClientWithIme(std::make_shared<InputClientInfo>(infoTemp), ImeType::IME);
    }
    return ErrorCode::ERROR_BAD_PARAMETERS;
}

int32_t PerUserSession::InitInputControlChannel()
{
    IMSA_HILOGD("PerUserSession::InitInputControlChannel start.");
    sptr<IInputControlChannel> inputControlChannel = new InputControlChannelStub(userId_);
    auto data = GetReadyImeData(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist!", ImeType::IME);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    return RequestIme(data, RequestType::NORMAL,
        [&data, &inputControlChannel] { return data->core->InitInputControlChannel(inputControlChannel); });
}

void PerUserSession::StartImeInImeDied()
{
    IMSA_HILOGD("StartImeInImeDied.");
    {
        std::lock_guard<std::mutex> lock(resetLock);
        auto now = time(nullptr);
        if (difftime(now, manager.last) > IME_RESET_TIME_OUT) {
            manager = { 0, now };
        }
        ++manager.num;
        if (manager.num > MAX_RESTART_NUM) {
            return;
        }
    }
    if (!IsWmsReady()) {
        IMSA_HILOGW("not ready to start ime.");
        return;
    }
    StartImeIfInstalled();
}

void PerUserSession::StartImeIfInstalled()
{
    std::shared_ptr<ImeNativeCfg> imeToStart = nullptr;
    if (!GetInputTypeToStart(imeToStart)) {
        imeToStart = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    }
    if (imeToStart == nullptr || imeToStart->imeId.empty()) {
        IMSA_HILOGE("imeToStart is nullptr!");
        return;
    }
    if (!ImeInfoInquirer::GetInstance().IsImeInstalled(userId_, imeToStart->bundleName, imeToStart->extName)) {
        IMSA_HILOGE("imeToStart is not installed, imeId = %{public}s!", imeToStart->imeId.c_str());
        return;
    }
    StartCurrentIme();
}

void PerUserSession::SetCurrentClient(sptr<IInputClient> client)
{
    IMSA_HILOGD("set current client.");
    std::lock_guard<std::mutex> lock(clientLock_);
    currentClient_ = client;
}

sptr<IInputClient> PerUserSession::GetCurrentClient()
{
    IMSA_HILOGD("get current client.");
    std::lock_guard<std::mutex> lock(clientLock_);
    return currentClient_;
}

void PerUserSession::ReplaceCurrentClient(
    const sptr<IInputClient> &client, const std::shared_ptr<ClientGroup> &clientGroup)
{
    std::lock_guard<std::mutex> lock(focusedClientLock_);
    if (client == nullptr) {
        return;
    }
    auto clientInfo = clientGroup->GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        return;
    }
    auto replacedClient = clientGroup->GetCurrentClient();
    clientGroup->SetCurrentClient(client);
    if (replacedClient != nullptr) {
        auto replacedClientInfo = clientGroup->GetClientInfo(replacedClient->AsObject());
        if (replacedClientInfo != nullptr && replacedClientInfo->pid != clientInfo->pid) {
            IMSA_HILOGI("remove replaced client: [%{public}d]", replacedClientInfo->pid);
            RemoveClient(replacedClient, clientGroup);
        }
    }
    auto inactiveClient = clientGroup->GetInactiveClient();
    if (inactiveClient != nullptr) {
        auto inactiveClientInfo = clientGroup->GetClientInfo(inactiveClient->AsObject());
        if (inactiveClientInfo != nullptr && inactiveClientInfo->pid != clientInfo->pid) {
            IMSA_HILOGI("remove inactive client: [%{public}d]", inactiveClientInfo->pid);
            RemoveClient(inactiveClient, clientGroup, false);
        }
    }
}

sptr<IInputClient> PerUserSession::GetInactiveClient()
{
    std::lock_guard<std::mutex> lock(inactiveClientLock_);
    return inactiveClient_;
}

void PerUserSession::NotifyImeChangeToClients(const Property &property, const SubProperty &subProperty)
{
    IMSA_HILOGD("start.");
    auto clientMap = GetClientMap();
    for (const auto &client : clientMap) {
        auto clientInfo = client.second;
        if (clientInfo == nullptr || !EventStatusManager::IsImeChangeOn(clientInfo->eventFlag)) {
            IMSA_HILOGD("client nullptr or no need to notify.");
            continue;
        }
        IMSA_HILOGD("notify client: [%{public}d]", static_cast<int32_t>(clientInfo->pid));
        int32_t ret = clientInfo->client->OnSwitchInput(property, subProperty);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("notify failed, ret: %{public}d, uid: %{public}d!", ret, static_cast<int32_t>(clientInfo->uid));
            continue;
        }
    }
}

int32_t PerUserSession::AddImeData(ImeType type, sptr<IInputMethodCore> core, sptr<IRemoteObject> agent, pid_t pid)
{
    if (core == nullptr || agent == nullptr) {
        IMSA_HILOGE("core or agent is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    sptr<InputDeathRecipient> deathRecipient = new (std::nothrow) InputDeathRecipient();
    if (deathRecipient == nullptr) {
        IMSA_HILOGE("failed to new deathRecipient!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    deathRecipient->SetDeathRecipient([this, core, type](const wptr<IRemoteObject> &) { this->OnImeDied(core, type); });
    auto coreObject = core->AsObject();
    if (coreObject == nullptr || (coreObject->IsProxyObject() && !coreObject->AddDeathRecipient(deathRecipient))) {
        IMSA_HILOGE("failed to add death recipient!");
        return ErrorCode::ERROR_ADD_DEATH_RECIPIENT_FAILED;
    }
    std::lock_guard<std::mutex> lock(imeDataLock_);
    auto imeData = std::make_shared<ImeData>(core, agent, deathRecipient, pid);
    imeData->imeStatus = ImeStatus::READY;
    imeData->ime.first = "proxyIme";
    imeData_.insert_or_assign(type, imeData);
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<ImeData> PerUserSession::GetReadyImeData(ImeType type)
{
    std::lock_guard<std::mutex> lock(imeDataLock_);
    auto it = imeData_.find(type);
    if (it == imeData_.end()) {
        return nullptr;
    }
    if (it->second->imeStatus != ImeStatus::READY) {
        return nullptr;
    }
    return it->second;
}

std::shared_ptr<ImeData> PerUserSession::GetValidIme(ImeType type)
{
    auto data = GetReadyImeData(type);
    if (data != nullptr || type != ImeType::IME) {
        return data;
    }
    IMSA_HILOGI("current ime is empty, try to restart it.");
    StartCurrentIme();
    return GetReadyImeData(type);
}

void PerUserSession::RemoveImeData(ImeType type, bool isImeDied)
{
    std::lock_guard<std::mutex> lock(imeDataLock_);
    auto it = imeData_.find(type);
    if (it == imeData_.end()) {
        IMSA_HILOGD("imeData not found.");
        return;
    }
    auto data = it->second;
    if (isImeDied && data->core != nullptr && data->core->AsObject() != nullptr) {
        data->core->AsObject()->RemoveDeathRecipient(data->deathRecipient);
    }
    imeData_.erase(type);
}

void PerUserSession::OnFocused(uint64_t displayId, int32_t pid, int32_t uid)
{
    std::lock_guard<std::mutex> lock(focusedClientLock_);
    auto clientGroup = GetClientGroup(displayId);
    if (clientGroup == nullptr) {
        return;
    }
    auto client = clientGroup->GetCurrentClient();
    if (client == nullptr) {
        return;
    }
    if (clientGroup->IsCurClientFocused(pid, uid)) {
        IMSA_HILOGD("current client focused, focusedPid: %{public}d", pid);
        return;
    }
    if (!OHOS::Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        IMSA_HILOGI("focus shifts to pid: %{public}d, remove current client.", pid);
        RemoveClient(client, clientGroup);
        InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_UNFOCUSED);
        return;
    }
    IMSA_HILOGI("focus shifts to pid: %{public}d, deactivate current client.", pid);
    clientGroup->DeactivateClient(client);
}

void PerUserSession::OnUnfocused(uint64_t displayId, int32_t pid, int32_t uid)
{
    auto clientGroup = GetClientGroup(displayId);
    if (clientGroup == nullptr) {
        return;
    }
    if (clientGroup->GetCurrentClient() == nullptr) {
        return;
    }
    if (clientGroup->IsCurClientUnFocused(pid, uid)) {
        IMSA_HILOGD("current client Unfocused, unFocusedPid: %{public}d", pid);
        return;
    }
    auto clientInfo = clientGroup->GetClientInfo(pid);
    if (clientInfo == nullptr) {
        return;
    }
    RemoveClient(clientInfo->client, clientGroup);
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_UNFOCUSED);
}

void PerUserSession::OnUserUnlocked()
{
    isUserUnlocked_.store(true);
    ImeCfgManager::GetInstance().ModifyTempScreenLockImeCfg(userId_, "");
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    if (currentIme == nullptr) {
        IMSA_HILOGE("currentIme nullptr");
        return;
    }
    auto imeData = GetImeData(ImeType::IME);
    if (imeData != nullptr && imeData->ime.first == currentIme->bundleName) {
        IMSA_HILOGD("no need to switch");
        return;
    }
    IMSA_HILOGI("user %{public}d unlocked, start current ime", userId_);
#ifndef IMF_ON_DEMAND_START_STOP_SA_ENABLE
    AddRestartIme();
#endif
}

void PerUserSession::UpdateUserLockState()
{
    bool isUnlocked = false;
    if (OsAccountAdapter::IsOsAccountVerified(userId_, isUnlocked) != ErrorCode::NO_ERROR) {
        return;
    }
    IMSA_HILOGI("isUnlocked: %{public}d", isUnlocked);
    isUserUnlocked_.store(isUnlocked);
    if (isUnlocked) {
        OnUserUnlocked();
    }
}

std::shared_ptr<InputClientInfo> PerUserSession::GetCurClientInfo(uint64_t displayId)
{
    auto clientGroup = GetClientGroup(displayId);
    if (clientGroup == nullptr) {
        IMSA_HILOGD("client group: %{public} " PRIu64 "", displayId);
        return nullptr;
    }
    return clientGroup->GetCurrentClientInfo();
}

bool PerUserSession::IsSameClient(sptr<IInputClient> source, sptr<IInputClient> dest)
{
    return source != nullptr && dest != nullptr && source->AsObject() == dest->AsObject();
}

int32_t PerUserSession::StartCurrentIme(bool isStopCurrentIme)
{
    std::shared_ptr<ImeNativeCfg> imeToStart = nullptr;
    if (!GetInputTypeToStart(imeToStart)) {
        imeToStart = ImeInfoInquirer::GetInstance().GetImeToStart(userId_);
    }
    if (imeToStart == nullptr) {
        IMSA_HILOGE("imeToStart is nullptr!");
        return ErrorCode::ERROR_IMSA_IME_TO_START_NULLPTR;
    }
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    IMSA_HILOGD("currentIme: %{public}s, imeToStart: %{public}s.", currentIme->imeId.c_str(),
        imeToStart->imeId.c_str());
    auto ret = StartIme(imeToStart, isStopCurrentIme);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to start ime!");
        InputMethodSysEvent::GetInstance().InputmethodFaultReporter(ret,
            imeToStart->imeId, "start ime failed!");
        return ret;
    }
    currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    IMSA_HILOGI("current ime changed to %{public}s.", currentIme->imeId.c_str());
    auto currentImeInfo =
        ImeInfoInquirer::GetInstance().GetImeInfo(userId_, currentIme->bundleName, currentIme->subName);
    if (currentImeInfo == nullptr) {
        IMSA_HILOGD("currentImeInfo is nullptr!");
        return ErrorCode::NO_ERROR;
    }

    NotifyImeChangeToClients(currentImeInfo->prop, currentImeInfo->subProp);
    ret = SwitchSubtypeWithoutStartIme(currentImeInfo->subProp);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("SwitchSubtype failed!");
    }
    return ErrorCode::NO_ERROR;
}

bool PerUserSession::GetCurrentUsingImeId(ImeIdentification &imeId)
{
    if (InputTypeManager::GetInstance().IsStarted()) {
        IMSA_HILOGI("get right click on state current ime.");
        auto currentIme = InputTypeManager::GetInstance().GetCurrentIme();
        imeId = currentIme;
        return true;
    }
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    if (currentImeCfg == nullptr) {
        IMSA_HILOGE("currentImeCfg is nullptr!");
        return false;
    }
    imeId.bundleName = currentImeCfg->bundleName;
    imeId.subName = currentImeCfg->extName;
    return true;
}

bool PerUserSession::CanStartIme()
{
    return IsSaReady(MEMORY_MANAGER_SA_ID) && IsWmsReady() && runningIme_.empty();
}

int32_t PerUserSession::ChangeToDefaultImeIfFolded()
{
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    auto imeToStart = std::make_shared<ImeNativeCfg>();
    if (Rosen::DisplayManager::GetInstance().IsFoldable() &&
        Rosen::DisplayManager::GetInstance().GetFoldStatus() == Rosen::FoldStatus::FOLDED) {
        IMSA_HILOGI("FoldStatus is %{public}d", Rosen::DisplayManager::GetInstance().GetFoldStatus());
        auto defaultIme = ImeInfoInquirer::GetInstance().GetDefaultImeCfg();
        if (defaultIme == nullptr) {
            IMSA_HILOGE("failed to get default ime");
            return ErrorCode::ERROR_IMSA_DEFAULT_IME_NOT_FOUND;
        }
        if (defaultIme->bundleName == currentIme->bundleName) {
            IMSA_HILOGD("no need");
            imeToStart = currentIme;
            return ErrorCode::NO_ERROR;
        }
        imeToStart = defaultIme;
        ImeCfgManager::GetInstance().ModifyTempScreenLockImeCfg(userId_, imeToStart->imeId);
        RestoreCurrentIme();
        return ErrorCode::NO_ERROR;
    } else {
        IMSA_HILOGI("FoldStatus is %{public}d", Rosen::DisplayManager::GetInstance().GetFoldStatus());
        ImeCfgManager::GetInstance().ModifyTempScreenLockImeCfg(userId_, "");
        RestoreCurrentIme();
        return ErrorCode::NO_ERROR;
    }
}

int32_t PerUserSession::ChangeToDefaultImeIfNeed(
    const std::shared_ptr<ImeNativeCfg> &targetIme, std::shared_ptr<ImeNativeCfg> &imeToStart)
{
    if (isUserUnlocked_.load()) {
        IMSA_HILOGD("no need");
        imeToStart = targetIme;
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("Screen is locked, start default ime");
    auto defaultIme = ImeInfoInquirer::GetInstance().GetDefaultImeCfg();
    if (defaultIme == nullptr) {
        IMSA_HILOGE("failed to get default ime");
        return ErrorCode::ERROR_IMSA_DEFAULT_IME_NOT_FOUND;
    }
    if (defaultIme->bundleName == targetIme->bundleName) {
        IMSA_HILOGD("no need");
        imeToStart = targetIme;
        return ErrorCode::NO_ERROR;
    }
    imeToStart = defaultIme;
    ImeCfgManager::GetInstance().ModifyTempScreenLockImeCfg(userId_, imeToStart->imeId);
    return ErrorCode::NO_ERROR;
}

AAFwk::Want PerUserSession::GetWant(const std::shared_ptr<ImeNativeCfg> &ime)
{
    SecurityMode mode;
    bool isolatedSandBox = true;
    if (SecurityModeParser::GetInstance()->IsDefaultFullMode(ime->bundleName, userId_)) {
        mode = SecurityMode::FULL;
        isolatedSandBox = false;
    } else if (ImeInfoInquirer::GetInstance().IsEnableSecurityMode()) {
        mode = SecurityModeParser::GetInstance()->GetSecurityMode(ime->bundleName, userId_);
    } else {
        mode = SecurityMode::FULL;
    }
    AAFwk::Want want;
    want.SetElementName(ime->bundleName, ime->extName);
    want.SetParam(STRICT_MODE, !(mode == SecurityMode::FULL));
    want.SetParam(ISOLATED_SANDBOX, isolatedSandBox);
    IMSA_HILOGI("StartInputService userId: %{public}d, ime: %{public}s, mode: %{public}d, isolatedSandbox: %{public}d",
        userId_, ime->imeId.c_str(), static_cast<int32_t>(mode), isolatedSandBox);
    return want;
}

int32_t PerUserSession::StartInputService(const std::shared_ptr<ImeNativeCfg> &ime)
{
    if (ime == nullptr) {
        return ErrorCode::ERROR_IMSA_IME_TO_START_NULLPTR;
    }
    auto imeToStart = std::make_shared<ImeNativeCfg>();
    auto ret = ChangeToDefaultImeIfNeed(ime, imeToStart);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    InitImeData({ imeToStart->bundleName, imeToStart->extName });
    isImeStarted_.Clear(false);
    sptr<AAFwk::IAbilityConnection> connection = new (std::nothrow) ImeConnection();
    if (connection == nullptr) {
        IMSA_HILOGE("failed to create connection!");
        return ErrorCode::ERROR_IMSA_MALLOC_FAILED;
    }
    auto want = GetWant(imeToStart);
    ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectExtensionAbility(want, connection, userId_);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("connect %{public}s failed, ret: %{public}d!", imeToStart->imeId.c_str(), ret);
        InputMethodSysEvent::GetInstance().InputmethodFaultReporter(
            ErrorCode::ERROR_IMSA_IME_CONNECT_FAILED, imeToStart->imeId, "failed to start ability.");
        return ErrorCode::ERROR_IMSA_IME_CONNECT_FAILED;
    }
    if (!isImeStarted_.GetValue()) {
        IMSA_HILOGE("start %{public}s timeout!", imeToStart->imeId.c_str());
        return ErrorCode::ERROR_IMSA_IME_START_TIMEOUT;
    }
    IMSA_HILOGI("%{public}s started successfully.", imeToStart->imeId.c_str());
    InputMethodSysEvent::GetInstance().RecordEvent(IMEBehaviour::START_IME);
    return ErrorCode::NO_ERROR;
}

int64_t PerUserSession::GetCurrentClientPid(uint64_t displayId)
{
    auto clientGroup = GetClientGroup(displayId);
    if (clientGroup == nullptr) {
        return INVALID_PID;
    }
    auto clientInfo = clientGroup->GetCurrentClientInfo();
    if (clientInfo == nullptr) {
        return INVALID_PID;
    }
    return clientInfo->pid;
}

int64_t PerUserSession::GetInactiveClientPid(uint64_t displayId)
{
    auto clientGroup = GetClientGroup(displayId);
    if (clientGroup == nullptr) {
        return INVALID_PID;
    }
    auto client = clientGroup->GetInactiveClient();
    if (client == nullptr) {
        return INVALID_PID;
    }
    auto clientInfo = clientGroup->GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        return INVALID_PID;
    }
    return clientInfo->pid;
}

int32_t PerUserSession::OnPanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info)
{
    auto clientGroup = GetClientGroup(DEFAULT_DISPLAY_ID);
    if (clientGroup == nullptr) {
        IMSA_HILOGD("client nullptr");
        return ErrorCode::NO_ERROR;
    }
    auto clientMap = clientGroup->GetClientMap();
    for (const auto &client : clientMap) {
        auto clientInfo = client.second;
        if (clientInfo == nullptr) {
            IMSA_HILOGD("client nullptr or no need to notify.");
            continue;
        }
        if (status == InputWindowStatus::SHOW && !EventStatusManager::IsImeShowOn(clientInfo->eventFlag)) {
            IMSA_HILOGD("has not imeShow callback");
            continue;
        }
        if (status == InputWindowStatus::HIDE && !EventStatusManager::IsImeHideOn(clientInfo->eventFlag)) {
            IMSA_HILOGD("has not imeHide callback");
            continue;
        }
        int32_t ret = clientInfo->client->OnPanelStatusChange(status, info);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("failed to OnPanelStatusChange, ret: %{public}d", ret);
            continue;
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnUpdateListenEventFlag(const InputClientInfo &clientInfo)
{
    auto remoteClient = clientInfo.client->AsObject();
    auto ret = AddClientInfo(remoteClient, clientInfo, START_LISTENING);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to AddClientInfo!");
        return ret;
    }
    auto clientGroup = GetClientGroup(remoteClient);
    if (clientGroup == nullptr) {
        IMSA_HILOGE("group is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    auto info = clientGroup->GetClientInfo(remoteClient);
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    if (info->eventFlag == NO_EVENT_ON && info->bindImeType == ImeType::NONE) {
        clientGroup->RemoveClientInfo(remoteClient, false);
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnSetCallingWindow(uint32_t callingWindowId, sptr<IInputClient> client)
{
    IMSA_HILOGD("OnSetCallingWindow enter");
    auto clientGroup = GetClientGroup(client);
    if (clientGroup == nullptr) {
        IMSA_HILOGE("clientGroup nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    if (!IsSameClient(client, clientGroup->GetCurrentClient())) {
        IMSA_HILOGE("client is not current client!");
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    if (client == nullptr) {
        IMSA_HILOGE("nullptr client!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto clientInfo = clientGroup->GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        IMSA_HILOGE("nullptr clientInfo!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    // if windowId change, refresh windowId info and notify clients input start;
    if (clientInfo->config.windowId != callingWindowId) {
        IMSA_HILOGD("windowId changed, refresh windowId info and notify clients input start.");
        clientInfo->config.windowId = callingWindowId;
        return NotifyInputStartToClients(
            callingWindowId, clientGroup, static_cast<int32_t>(clientInfo->requestKeyboardReason));
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::GetInputStartInfo(
    uint64_t displayId, bool &isInputStart, uint32_t &callingWndId, int32_t &requestKeyboardReason)
{
    auto clientInfo = GetCurClientInfo(displayId);
    if (clientInfo == nullptr) {
        IMSA_HILOGE("nullptr clientInfo!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    isInputStart = true;
    callingWndId = clientInfo->config.windowId;
    requestKeyboardReason = static_cast<int32_t>(clientInfo->requestKeyboardReason);
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::NotifyInputStartToClients(
    uint32_t callingWndId, const std::shared_ptr<ClientGroup> &clientGroup, int32_t requestKeyboardReason)
{
    IMSA_HILOGD("NotifyInputStartToClients enter");
    if (clientGroup == nullptr) {
        IMSA_HILOGE("clientGroup is nullptr");
        return ErrorCode::ERROR_IMSA_NULLPTR;
    }
    auto clientMap = clientGroup->GetClientMap();
    for (const auto &client : clientMap) {
        auto clientInfo = client.second;
        if (clientInfo == nullptr || clientInfo->client == nullptr ||
            !EventStatusManager::IsInputStatusChangedOn(clientInfo->eventFlag)) {
            IMSA_HILOGE("nullptr clientInfo or no need to notify");
            continue;
        }
        int32_t ret = clientInfo->client->NotifyInputStart(callingWndId, requestKeyboardReason);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("failed to notify OnInputStart, errorCode: %{public}d", ret);
            continue;
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::NotifyInputStopToClients(const std::shared_ptr<ClientGroup> &clientGroup)
{
    IMSA_HILOGD("NotifyInputStopToClients enter");
    if (clientGroup == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto clientMap = clientGroup->GetClientMap();
    for (const auto &client : clientMap) {
        auto clientInfo = client.second;
        if (clientInfo == nullptr || clientInfo->client == nullptr ||
            !EventStatusManager::IsInputStatusChangedOn(clientInfo->eventFlag)) {
            IMSA_HILOGE("nullptr clientInfo or no need to notify");
            continue;
        }
        int32_t ret = clientInfo->client->NotifyInputStop();
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("failed to notify OnInputStop, errorCode: %{public}d", ret);
            continue;
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::NotifyInputStopToClients()
{
    IMSA_HILOGD("NotifyInputStopToClients enter");
    auto clientMap = GetClientMap();
    for (const auto &client : clientMap) {
        auto clientInfo = client.second;
        if (clientInfo == nullptr || clientInfo->client == nullptr ||
            !EventStatusManager::IsInputStatusChangedOn(clientInfo->eventFlag)) {
            IMSA_HILOGE("nullptr clientInfo or no need to notify");
            continue;
        }
        int32_t ret = clientInfo->client->NotifyInputStop();
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("failed to notify OnInputStop, errorCode: %{public}d", ret);
            continue;
        }
    }
    return ErrorCode::NO_ERROR;
}

bool PerUserSession::IsNotifyInputStop(
    const sptr<IInputClient> &client, const std::shared_ptr<ClientGroup> &clientGroup)
{
    if (clientGroup == nullptr) {
        IMSA_HILOGE("clientGroup is nullptr");
        return false;
    }
    if (IsSameClient(client, clientGroup->GetCurrentClient())) {
        return true;
    }
    if (IsSameClient(client, clientGroup->GetInactiveClient()) && clientGroup->GetCurrentClient() == nullptr) {
        return true;
    }
    return false;
}

bool PerUserSession::IsImeStartInBind(ImeType bindImeType, ImeType startImeType)
{
    return startImeType == ImeType::IME && bindImeType == ImeType::IME;
}

bool PerUserSession::IsProxyImeStartInBind(ImeType bindImeType, ImeType startImeType)
{
    return startImeType == ImeType::PROXY_IME && bindImeType == ImeType::PROXY_IME;
}

bool PerUserSession::IsProxyImeStartInImeBind(ImeType bindImeType, ImeType startImeType)
{
    return startImeType == ImeType::PROXY_IME && bindImeType == ImeType::IME;
}

bool PerUserSession::IsImeBindTypeChanged(ImeType bindImeType)
{
    return (bindImeType == ImeType::IME && IsProxyImeEnable()) ||
           (bindImeType == ImeType::PROXY_IME && !IsProxyImeEnable());
}

int32_t PerUserSession::SwitchSubtype(const SubProperty &subProperty)
{
    auto data = GetValidIme(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist!", ImeType::IME);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    return RequestIme(data, RequestType::NORMAL, [&data, &subProperty] { return data->core->SetSubtype(subProperty); });
}

int32_t PerUserSession::SwitchSubtypeWithoutStartIme(const SubProperty &subProperty)
{
    auto data = GetReadyImeData(ImeType::IME);
    if (data == nullptr || data->core == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist, or core is nullptr.", ImeType::IME);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    return RequestIme(data, RequestType::NORMAL,
        [&data, &subProperty] { return data->core->SetSubtype(subProperty); });
}

int32_t PerUserSession::SetInputType()
{
    InputType inputType = InputTypeManager::GetInstance().GetCurrentInputType();
    auto data = GetValidIme(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist!", ImeType::IME);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    return RequestIme(data, RequestType::NORMAL, [&data, &inputType] { return data->core->OnSetInputType(inputType); });
}

bool PerUserSession::IsBoundToClient()
{
    if (GetCurrentClient() == nullptr) {
        IMSA_HILOGE("not in bound state!");
        return false;
    }
    return true;
}

int32_t PerUserSession::RestoreCurrentImeSubType()
{
    if (!InputTypeManager::GetInstance().IsStarted()) {
        IMSA_HILOGD("already exit.");
        return ErrorCode::NO_ERROR;
    }
    auto typeIme = InputTypeManager::GetInstance().GetCurrentIme();
    auto cfgIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    if (cfgIme->bundleName != typeIme.bundleName) {
        IMSA_HILOGD("diff ime, not deal, restore ime when attach.");
        return ErrorCode::NO_ERROR;
    }
    auto imeData = GetReadyImeData(ImeType::IME);
    InputTypeManager::GetInstance().Set(false);
    if (imeData == nullptr || imeData->ime.first != cfgIme->bundleName || imeData->ime.second != cfgIme->extName) {
        return ErrorCode::NO_ERROR;
    }
    SubProperty subProp = { .name = cfgIme->bundleName, .id = cfgIme->subName };
    auto subPropTemp = ImeInfoInquirer::GetInstance().GetCurrentSubtype(userId_);
    if (subPropTemp != nullptr) {
        subProp = *subPropTemp;
    }
    IMSA_HILOGD("same ime, restore subtype: %{public}s.", cfgIme->subName.c_str());
    return SwitchSubtype(subProp);
}

bool PerUserSession::IsCurrentImeByPid(int32_t pid)
{
    auto imeData = GetImeData(ImeType::IME);
    if (imeData == nullptr) {
        IMSA_HILOGE("ime not started!");
        return false;
    }
    IMSA_HILOGD("userId: %{public}d, pid: %{public}d, current pid: %{public}d.", userId_, pid, imeData->pid);
    return imeData->pid == pid;
}

int32_t PerUserSession::IsPanelShown(const PanelInfo &panelInfo, bool &isShown)
{
    auto clientGroup = GetClientGroup(DEFAULT_DISPLAY_ID);
    if (clientGroup == nullptr || clientGroup->GetCurrentClient() == nullptr) {
        IMSA_HILOGD("not in bound state.");
        isShown = false;
        return ErrorCode::NO_ERROR;
    }
    auto ime = GetReadyImeData(ImeType::IME);
    if (ime == nullptr) {
        IMSA_HILOGE("ime not started!");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    return RequestIme(ime, RequestType::NORMAL,
        [&ime, &panelInfo, &isShown] { return ime->core->IsPanelShown(panelInfo, isShown); });
}

bool PerUserSession::CheckSecurityMode()
{
    auto clientInfo = GetCurClientInfo();
    if (clientInfo != nullptr) {
        return clientInfo->config.inputAttribute.GetSecurityFlag();
    }
    return false;
}

std::map<sptr<IRemoteObject>, std::shared_ptr<InputClientInfo>> PerUserSession::GetClientMap()
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    return mapClients_;
}

int32_t PerUserSession::RequestIme(const std::shared_ptr<ImeData> &data, RequestType type, const IpcExec &exec)
{
    if (IsProxyImeEnable()) {
        IMSA_HILOGD("proxy enable.");
        return exec();
    }
    if (data == nullptr || data->freezeMgr == nullptr) {
        IMSA_HILOGE("data is nullptr!");
        return ErrorCode::NO_ERROR;
    }
    if (!data->freezeMgr->IsIpcNeeded(type)) {
        IMSA_HILOGD("no need to request, type: %{public}d.", type);
        return ErrorCode::NO_ERROR;
    }
    data->freezeMgr->BeforeIpc(type);
    auto ret = exec();
    data->freezeMgr->AfterIpc(type, ret == ErrorCode::NO_ERROR);
    return ret;
}

int32_t PerUserSession::OnConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent)
{
    auto data = GetReadyImeData(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist!", ImeType::IME);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    auto ret = RequestIme(data, RequestType::NORMAL,
        [&data, &channel, &agent] { return data->core->OnConnectSystemCmd(channel, agent); });
    IMSA_HILOGD("on connect systemCmd, ret: %{public}d.", ret);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("bind failed, ret: %{public}d!", ret);
        return ret;
    }
    return ErrorCode::NO_ERROR;
}

bool PerUserSession::WaitForCurrentImeStop()
{
    IMSA_HILOGI("start.");
    std::unique_lock<std::mutex> lock(imeStopMutex_);
    isSwitching_.store(true);
    return imeStopCv_.wait_for(lock, std::chrono::milliseconds(STOP_IME_TIME), [this]() { return !isSwitching_; });
}

void PerUserSession::NotifyImeStopFinished()
{
    IMSA_HILOGI("start.");
    std::unique_lock<std::mutex> lock(imeStopMutex_);
    isSwitching_.store(false);
    imeStopCv_.notify_one();
}

int32_t PerUserSession::RemoveCurrentClient()
{
    auto currentClient = GetCurrentClient();
    if (currentClient == nullptr) {
        IMSA_HILOGE("currentClient is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    NotifyInputStopToClients();
    return RemoveClient(currentClient, false);
}

bool PerUserSession::IsWmsReady()
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        IMSA_HILOGD("scb enable");
        return WmsConnectionObserver::IsWmsConnected(userId_);
    }
    return IsSaReady(WINDOW_MANAGER_SERVICE_ID);
}

bool PerUserSession::IsSaReady(int32_t saId)
{
    auto saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgr == nullptr) {
        IMSA_HILOGE("get saMgr failed!");
        return false;
    }
    if (saMgr->CheckSystemAbility(saId) == nullptr) {
        IMSA_HILOGE("sa:%{public}d not ready!", saId);
        return false;
    }
    return true;
}

void PerUserSession::AddRestartIme()
{
    int32_t tasks = 0;
    {
        std::lock_guard<std::mutex> lock(restartMutex_);
        if (restartTasks_ >= MAX_RESTART_TASKS) {
            return;
        }
        restartTasks_ = std::max(restartTasks_, 0);
        tasks = ++restartTasks_;
    }
    if (tasks == 1 && !RestartIme()) {
        std::lock_guard<std::mutex> lock(restartMutex_);
        restartTasks_ = 0;
    }
}

bool PerUserSession::RestartIme()
{
    auto task = [this]() {
        if (CanStartIme()) {
            auto ret = StartCurrentIme(true);
            if (ret != ErrorCode::NO_ERROR) {
                IMSA_HILOGE("start ime failed!");
            }
        }
        int32_t tasks = 0;
        {
            std::lock_guard<std::mutex> lock(restartMutex_);
            tasks = --restartTasks_;
        }
        if (tasks > 0 && !RestartIme()) {
            std::lock_guard<std::mutex> lock(restartMutex_);
            restartTasks_ = 0;
        }
    };
    if (eventHandler_ == nullptr) {
        IMSA_HILOGE("eventHandler_ is nullptr!");
        return false;
    }
    return eventHandler_->PostTask(task, "RestartCurrentImeTask", 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

BlockQueue<SwitchInfo>& PerUserSession::GetSwitchQueue()
{
    return switchQueue_;
}

int32_t PerUserSession::InitImeData(const std::pair<std::string, std::string> &ime)
{
    std::lock_guard<std::mutex> lock(imeDataLock_);
    auto it = imeData_.find(ImeType::IME);
    if (it != imeData_.end()) {
        return ErrorCode::NO_ERROR;
    }
    auto imeData = std::make_shared<ImeData>(nullptr, nullptr, nullptr, -1);
    imeData->startTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    imeData->ime = ime;
    imeData_.insert({ ImeType::IME, imeData });
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::UpdateImeData(sptr<IInputMethodCore> core, sptr<IRemoteObject> agent, pid_t pid)
{
    if (core == nullptr || agent == nullptr) {
        IMSA_HILOGE("core or agent is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    std::lock_guard<std::mutex> lock(imeDataLock_);
    auto it = imeData_.find(ImeType::IME);
    if (it == imeData_.end()) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    it->second->core = core;
    it->second->agent = agent;
    it->second->pid = pid;
    it->second->freezeMgr = std::make_shared<FreezeManager>(pid);
    sptr<InputDeathRecipient> deathRecipient = new (std::nothrow) InputDeathRecipient();
    if (deathRecipient == nullptr) {
        IMSA_HILOGE("failed to new deathRecipient!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto type = ImeType::IME;
    deathRecipient->SetDeathRecipient([this, core, type](const wptr<IRemoteObject> &) { this->OnImeDied(core, type); });
    auto coreObject = core->AsObject();
    if (coreObject == nullptr || (coreObject->IsProxyObject() && !coreObject->AddDeathRecipient(deathRecipient))) {
        IMSA_HILOGE("failed to add death recipient!");
        return ErrorCode::ERROR_ADD_DEATH_RECIPIENT_FAILED;
    }
    it->second->deathRecipient = deathRecipient;
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::InitConnect(pid_t pid)
{
    std::lock_guard<std::mutex> lock(imeDataLock_);
    auto it = imeData_.find(ImeType::IME);
    if (it == imeData_.end()) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    it->second->pid = pid;
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<ImeData> PerUserSession::GetImeData(ImeType type)
{
    std::lock_guard<std::mutex> lock(imeDataLock_);
    auto it = imeData_.find(type);
    if (it == imeData_.end()) {
        return nullptr;
    }
    return it->second;
}

int32_t PerUserSession::StartIme(const std::shared_ptr<ImeNativeCfg> &ime, bool isStopCurrentIme)
{
    std::lock_guard<std::mutex> lock(imeStartLock_);
    if (ime == nullptr) {
        return ErrorCode::ERROR_IMSA_IME_TO_START_NULLPTR;
    }
    auto imeData = GetImeData(ImeType::IME);
    if (imeData == nullptr) {
        return HandleFirstStart(ime, isStopCurrentIme);
    }
    if (imeData->ime.first == ime->bundleName && imeData->ime.second == ime->extName) {
        if (isStopCurrentIme) {
            return StartNewIme(ime);
        }
        return StartCurrentIme(ime);
    }
    return StartNewIme(ime);
}

ImeAction PerUserSession::GetImeAction(ImeEvent action)
{
    std::lock_guard<std::mutex> lock(imeDataLock_);
    auto it = imeData_.find(ImeType::IME);
    if (it == imeData_.end()) {
        return ImeAction::DO_ACTION_IN_NULL_IME_DATA;
    }
    auto iter = imeEventConverter_.find({ it->second->imeStatus, action });
    if (iter == imeEventConverter_.end()) {
        IMSA_HILOGE("abnormal!");
        return ImeAction::DO_ACTION_IN_IME_EVENT_CONVERT_FAILED;
    }
    it->second->imeStatus = iter->second.first;
    return iter->second.second;
}

int32_t PerUserSession::StartCurrentIme(const std::shared_ptr<ImeNativeCfg> &ime)
{
    auto imeData = GetImeData(ImeType::IME);
    if (imeData == nullptr) {
        return StartInputService(ime);
    }
    auto action = GetImeAction(ImeEvent::START_IME);
    if (action == ImeAction::DO_ACTION_IN_IME_EVENT_CONVERT_FAILED) {
        return ErrorCode::ERROR_IMSA_IME_EVENT_CONVERT_FAILED;
    }
    if (action == ImeAction::DO_ACTION_IN_NULL_IME_DATA) {
        return StartInputService(ime);
    }
    if (action == ImeAction::DO_NOTHING) {
        return ErrorCode::NO_ERROR;
    }
    if (action == ImeAction::HANDLE_STARTING_IME) {
        int64_t time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        if (time - imeData->startTime > ImeData::START_TIME_OUT) {
            IMSA_HILOGE("[%{public}s, %{public}s] start abnormal, more than eight second!", imeData->ime.first.c_str(),
                imeData->ime.second.c_str());
            return HandleStartImeTimeout(ime);
        }
        return StartInputService(ime);
    }
    auto ret = ForceStopCurrentIme();
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    return StartInputService(ime);
}

int32_t PerUserSession::HandleStartImeTimeout(const std::shared_ptr<ImeNativeCfg> &ime)
{
    auto action = GetImeAction(ImeEvent::START_IME_TIMEOUT);
    if (action == ImeAction::DO_ACTION_IN_NULL_IME_DATA) {
        return StartInputService(ime);
    }
    if (action == ImeAction::DO_ACTION_IN_IME_EVENT_CONVERT_FAILED) {
        return ErrorCode::ERROR_IMSA_IME_EVENT_CONVERT_FAILED;
    }
    if (action == ImeAction::DO_NOTHING) {
        IMSA_HILOGW("ready when timeout");
        return ErrorCode::NO_ERROR;
    }
    auto ret = ForceStopCurrentIme(false);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    return StartInputService(ime);
}

int32_t PerUserSession::StartNewIme(const std::shared_ptr<ImeNativeCfg> &ime)
{
    auto ret = StopCurrentIme();
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    return StartInputService(ime);
}

int32_t PerUserSession::StopCurrentIme()
{
    auto action = GetImeAction(ImeEvent::STOP_IME);
    if (action == ImeAction::DO_ACTION_IN_NULL_IME_DATA) {
        return ErrorCode::NO_ERROR;
    }
    if (action == ImeAction::DO_ACTION_IN_IME_EVENT_CONVERT_FAILED) {
        return  ErrorCode::ERROR_IMSA_IME_EVENT_CONVERT_FAILED;
    }
    if (action == ImeAction::STOP_READY_IME) {
        return StopReadyCurrentIme();
    }
    return ForceStopCurrentIme();
}

int32_t PerUserSession::StopReadyCurrentIme()
{
    auto clientInfo = GetCurClientInfo();
    if (clientInfo != nullptr && clientInfo->bindImeType == ImeType::IME) {
        StopClientInput(clientInfo);
    }
    auto imeData = GetImeData(ImeType::IME);
    if (imeData == nullptr) {
        return ErrorCode::NO_ERROR;
    }
    if (imeData->core == nullptr) {
        IMSA_HILOGE("core is nullptr!");
        return ForceStopCurrentIme();
    }
    auto ret = RequestIme(imeData, RequestType::NORMAL, [&imeData] {
        // failed when register onInputStop after SetCoreAndAgent
        return imeData->core->StopInputService(true);
    });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("StopInputService failed.");
        return ForceStopCurrentIme();
    }
    if (!WaitForCurrentImeStop()) {
        IMSA_HILOGI("stop timeout.");
        return ForceStopCurrentIme();
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::ForceStopCurrentIme(bool isNeedWait)
{
    auto imeData = GetImeData(ImeType::IME);
    if (imeData == nullptr) {
        return ErrorCode::NO_ERROR;
    }
    if (!ImeInfoInquirer::GetInstance().IsRunningIme(userId_, imeData->ime.first)) {
        IMSA_HILOGW("[%{public}s, %{public}s] already stop.", imeData->ime.first.c_str(), imeData->ime.second.c_str());
        RemoveImeData(ImeType::IME, true);
        return ErrorCode::NO_ERROR;
    }
    auto clientInfo = GetCurClientInfo();
    if (clientInfo != nullptr && clientInfo->bindImeType == ImeType::IME) {
        StopClientInput(clientInfo);
    }

    AAFwk::Want want;
    want.SetElementName(imeData->ime.first, imeData->ime.second);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->StopExtensionAbility(
        want, nullptr, userId_, AppExecFwk::ExtensionAbilityType::INPUTMETHOD);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("StopExtensionAbility [%{public}s, %{public}s] failed, ret: %{public}d!",
            imeData->ime.first.c_str(), imeData->ime.second.c_str(), ret);
        return ErrorCode::ERROR_IMSA_IME_DISCONNECT_FAILED;
    }
    if (!isNeedWait) {
        return ErrorCode::ERROR_IMSA_IME_START_MORE_THAN_EIGHT_SECOND;
    }
    WaitForCurrentImeStop();
    if (ImeInfoInquirer::GetInstance().IsRunningIme(userId_, imeData->ime.first)) {
        IMSA_HILOGW("stop [%{public}s, %{public}s] timeout.", imeData->ime.first.c_str(), imeData->ime.second.c_str());
        return ErrorCode::ERROR_IMSA_FORCE_STOP_IME_TIMEOUT;
    }
    RemoveImeData(ImeType::IME, true);
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::HandleFirstStart(const std::shared_ptr<ImeNativeCfg> &ime, bool isStopCurrentIme)
{
    if (runningIme_.empty()) {
        return StartInputService(ime);
    }
    IMSA_HILOGW("imsa abnormal restore.");
    if (isStopCurrentIme) {
        return ErrorCode::NO_ERROR;
    }
    if (BlockRetry(CHECK_IME_RUNNING_RETRY_INTERVAL, CHECK_IME_RUNNING_RETRY_TIMES,
                   [this]() -> bool { return !ImeInfoInquirer::GetInstance().IsRunningIme(userId_, runningIme_); })) {
        IMSA_HILOGI("[%{public}d, %{public}s] stop completely", userId_, runningIme_.c_str());
        runningIme_.clear();
        return StartInputService(ime);
    }
    IMSA_HILOGW("[%{public}d, %{public}s] stop timeout", userId_, runningIme_.c_str());
    return ErrorCode::ERROR_IMSA_REBOOT_OLD_IME_NOT_STOP;
}

int32_t PerUserSession::RestoreCurrentIme()
{
    InputTypeManager::GetInstance().Set(false);
    auto cfgIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    auto imeData = GetReadyImeData(ImeType::IME);
    if (imeData != nullptr && cfgIme != nullptr && imeData->ime.first == cfgIme->bundleName
        && imeData->ime.second == cfgIme->extName) {
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGD("need restore!");
    auto ret = StartCurrentIme();
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("start ime failed!");
        return ret;  // ERROR_IME_START_FAILED
    }
    return ErrorCode::NO_ERROR;
}

bool PerUserSession::CheckPwdInputPatternConv(InputClientInfo &clientInfo, uint64_t displayId)
{
    auto clientGroup = GetClientGroup(displayId);
    if (clientGroup == nullptr) {
        IMSA_HILOGD("clientGroup not found");
        return false;
    }
    auto exClient = clientGroup->GetCurrentClient();
    if (exClient == nullptr) {
        exClient = clientGroup->GetInactiveClient();
    }
    auto exClientInfo = exClient != nullptr ? clientGroup->GetClientInfo(exClient->AsObject()) : nullptr;
    if (exClientInfo == nullptr) {
        IMSA_HILOGE("exClientInfo is nullptr!");
        return false;
    }
    // if current input pattern differ from previous in pwd and normal, need hide panel first.
    if (clientInfo.config.inputAttribute.GetSecurityFlag()) {
        IMSA_HILOGI("new input pattern is pwd.");
        return !exClientInfo->config.inputAttribute.GetSecurityFlag();
    }
    IMSA_HILOGI("new input pattern is normal.");
    return exClientInfo->config.inputAttribute.GetSecurityFlag();
}

std::shared_ptr<ImeNativeCfg> PerUserSession::GetImeNativeCfg(int32_t userId, const std::string &bundleName,
    const std::string &subName)
{
    auto targetImeProperty = ImeInfoInquirer::GetInstance().GetImeProperty(userId, bundleName);
    if (targetImeProperty == nullptr) {
        IMSA_HILOGE("GetImeProperty [%{public}d, %{public}s] failed!", userId, bundleName.c_str());
        return nullptr;
    }
    std::string targetName = bundleName + "/" + targetImeProperty->id;
    ImeNativeCfg targetIme = { targetName, bundleName, subName, targetImeProperty->id };
    return std::make_shared<ImeNativeCfg>(targetIme);
}

bool PerUserSession::GetInputTypeToStart(std::shared_ptr<ImeNativeCfg> &imeToStart)
{
    if (!InputTypeManager::GetInstance().IsStarted()) {
        return false;
    }
    auto currentInputTypeIme = InputTypeManager::GetInstance().GetCurrentIme();
    if (currentInputTypeIme.bundleName.empty()) {
        auto currentInputType = InputTypeManager::GetInstance().GetCurrentInputType();
        InputTypeManager::GetInstance().GetImeByInputType(currentInputType, currentInputTypeIme);
    }
    imeToStart = GetImeNativeCfg(userId_, currentInputTypeIme.bundleName, currentInputTypeIme.subName);
    return true;
}

void PerUserSession::HandleImeBindTypeChanged(InputClientInfo &newClientInfo, const std::shared_ptr<ClientGroup> &clientGroup)
{
    /* isClientInactive: true: represent the oldClientInfo is inactiveClient's
                         false: represent the oldClientInfo is currentClient's */
    std::shared_ptr<InputClientInfo> oldClientInfo = nullptr;
    bool isClientInactive = false;
    {
        std::lock_guard<std::mutex> lock(focusedClientLock_);
        oldClientInfo = clientGroup->GetCurrentClientInfo();
        if (oldClientInfo == nullptr) {
            auto inactiveClient = GetInactiveClient();
            if (inactiveClient != nullptr) {
                oldClientInfo = clientGroup->GetClientInfo(inactiveClient->AsObject());
                isClientInactive = true;
            }
        }
        if (oldClientInfo == nullptr) {
            return;
        }
        if (!IsImeBindTypeChanged(oldClientInfo->bindImeType)) {
            return;
        }
        // has current client, but new client is not current client
        if (!isClientInactive && !IsSameClient(newClientInfo.client, oldClientInfo->client)) {
            clientGroup->SetCurrentClient(nullptr);
            if (oldClientInfo->client != nullptr) {
                clientGroup->RemoveClientInfo(oldClientInfo->client->AsObject());
            }
        }
    }
    IMSA_HILOGD("isClientInactive: %{public}d!", isClientInactive);
    if (IsSameClient(newClientInfo.client, oldClientInfo->client)) {
        newClientInfo.isNotifyInputStart = true;
    }
    if (isClientInactive) {
        StopImeInput(oldClientInfo->bindImeType, oldClientInfo->channel);
        return;
    }
    UnBindClientWithIme(oldClientInfo);
}

void PerUserSession::TryUnloadSystemAbility()
{
    auto data = GetReadyImeData(ImeType::IME);
    if (data != nullptr && data->freezeMgr != nullptr) {
        if (data->freezeMgr->IsImeInUse()) {
            return;
        }
    }

    auto onDemandStartStopSa = std::make_shared<OnDemandStartStopSa>();
    onDemandStartStopSa->UnloadInputMethodSystemAbility();
}

ImeType PerUserSession::GetCurDisplayImeType(uint64_t displayId)
{
    displayId = GetDisplayGroupId(displayId);
    if (displayId == DEFAULT_DISPLAY_ID) {
        return ImeType::IME;
    }
    if (displayId == aiAgentDisplayId_) {
        return ImeType::AI_AGENT_IME;
    }
    return ImeType::NONE;
}
} // namespace MiscServices
} // namespace OHOS