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

#include <chrono>
#include <vector>

#include "ability_manager_client.h"
#include "app_mgr_client.h"
#include "element_name.h"
#include "identity_checker_impl.h"
#include "ime_cfg_manager.h"
#include "ime_connection.h"
#include "ime_info_inquirer.h"
#include "input_control_channel_stub.h"
#include "input_type_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "mem_mgr_client.h"
#include "message_parcel.h"
#include "os_account_adapter.h"
#include "parcel.h"
#include "running_process_info.h"
#include "scene_board_judgement.h"
#include "security_mode_parser.h"
#include "system_ability_definition.h"
#include "unistd.h"
#include "wms_connection_observer.h"

namespace OHOS {
namespace MiscServices {
using namespace std::chrono;
using namespace MessageID;
using namespace OHOS::AppExecFwk;
constexpr int64_t INVALID_PID = -1;
constexpr uint32_t STOP_IME_TIME = 600;
constexpr const char *STRICT_MODE = "strictMode";
constexpr const char *ISOLATED_SANDBOX = "isolatedSandbox";
constexpr uint32_t CHECK_IME_RUNNING_RETRY_INTERVAL = 60;
constexpr uint32_t CHECK_IME_RUNNING_RETRY_TIMES = 10;
PerUserSession::PerUserSession(int userId) : userId_(userId)
{
}

PerUserSession::PerUserSession(int32_t userId, const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler)
    : userId_(userId), eventHandler_(eventHandler)
{
    auto bundleNames = ImeInfoInquirer::GetInstance().GetRunningIme(userId_);
    if (!bundleNames.empty()) {
        runningIme_ = bundleNames[0]; // one user only has one ime at present
    }
}

PerUserSession::~PerUserSession()
{
}

int PerUserSession::AddClientInfo(sptr<IRemoteObject> inputClient, const InputClientInfo &clientInfo,
    ClientAddEvent event)
{
    IMSA_HILOGD("PerUserSession start.");
    auto cacheInfo = GetClientInfo(inputClient);
    if (cacheInfo != nullptr) {
        IMSA_HILOGD("info is existed.");
        if (cacheInfo->uiExtensionTokenId == IMF_INVALID_TOKENID &&
            clientInfo.uiExtensionTokenId != IMF_INVALID_TOKENID) {
            UpdateClientInfo(inputClient, { { UpdateFlag::UIEXTENSION_TOKENID, clientInfo.uiExtensionTokenId } });
        }
        UpdateClientInfo(inputClient, { { UpdateFlag::TEXT_CONFIG, clientInfo.config } });
        if (event == START_LISTENING) {
            UpdateClientInfo(inputClient, { { UpdateFlag::EVENTFLAG, clientInfo.eventFlag } });
        }
        return ErrorCode::NO_ERROR;
    }
    auto info = std::make_shared<InputClientInfo>(clientInfo);
    info->deathRecipient->SetDeathRecipient(
        [this, info](const wptr<IRemoteObject> &) { this->OnClientDied(info->client); });
    auto obj = info->client->AsObject();
    if (obj == nullptr) {
        IMSA_HILOGE("client obj is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    if (obj->IsProxyObject() && !obj->AddDeathRecipient(info->deathRecipient)) {
        IMSA_HILOGE("failed to add client death recipient!");
        return ErrorCode::ERROR_CLIENT_ADD_FAILED;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx);
    mapClients_.insert({ inputClient, info });
    IMSA_HILOGI("add client end.");
    return ErrorCode::NO_ERROR;
}

void PerUserSession::RemoveClientInfo(const sptr<IRemoteObject> &client, bool isClientDied)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto clientInfo = GetClientInfo(client);
    if (clientInfo == nullptr) {
        IMSA_HILOGD("client already removed.");
        return;
    }
    // if client is subscriber and the release is not because of the client died, do not remove
    if (clientInfo->eventFlag != NO_EVENT_ON && !isClientDied) {
        IMSA_HILOGD("is subscriber, do not remove.");
        auto isShowKeyboard = false;
        auto bindImeType = ImeType::NONE;
        UpdateClientInfo(client,
            { { UpdateFlag::BINDIMETYPE, bindImeType }, { UpdateFlag::ISSHOWKEYBOARD, isShowKeyboard } });
        return;
    }
    if (clientInfo->deathRecipient != nullptr) {
        IMSA_HILOGD("deathRecipient remove.");
        client->RemoveDeathRecipient(clientInfo->deathRecipient);
        clientInfo->deathRecipient = nullptr;
    }
    mapClients_.erase(client);
    IMSA_HILOGI("client[%{public}d] is removed.", clientInfo->pid);
}

void PerUserSession::UpdateClientInfo(const sptr<IRemoteObject> &client, const std::unordered_map<UpdateFlag,
    std::variant<bool, uint32_t, ImeType, ClientState, TextTotalConfig>> &updateInfos)
{
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr!");
        return;
    }
    auto info = GetClientInfo(client);
    if (info == nullptr) {
        IMSA_HILOGE("client info is not exist!");
        return;
    }
    for (const auto &updateInfo : updateInfos) {
        switch (updateInfo.first) {
            case UpdateFlag::EVENTFLAG: {
                info->eventFlag = std::get<uint32_t>(updateInfo.second);
                break;
            }
            case UpdateFlag::ISSHOWKEYBOARD: {
                info->isShowKeyboard = std::get<bool>(updateInfo.second);
                break;
            }
            case UpdateFlag::BINDIMETYPE: {
                info->bindImeType = std::get<ImeType>(updateInfo.second);
                break;
            }
            case UpdateFlag::STATE: {
                info->state = std::get<ClientState>(updateInfo.second);
                break;
            }
            case UpdateFlag::TEXT_CONFIG: {
                info->config = std::get<TextTotalConfig>(updateInfo.second);
                break;
            }
            case UpdateFlag::UIEXTENSION_TOKENID: {
                info->uiExtensionTokenId = std::get<uint32_t>(updateInfo.second);
                break;
            }
            default:
                break;
        }
    }
}

int32_t PerUserSession::HideKeyboard(const sptr<IInputClient> &currentClient)
{
    IMSA_HILOGD("PerUserSession::HideKeyboard start.");
    auto clientInfo = GetClientInfo(currentClient->AsObject());
    if (clientInfo == nullptr) {
        IMSA_HILOGE("client info is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    auto data = GetReadyImeData(clientInfo->bindImeType);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist!", clientInfo->bindImeType);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    auto ret = RequestIme(data, RequestType::NORMAL, [&data] { return data->core->HideKeyboard(false); });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to hide keyboard, ret: %{public}d!", ret);
        return ErrorCode::ERROR_KBD_HIDE_FAILED;
    }
    bool isShowKeyboard = false;
    UpdateClientInfo(currentClient->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, isShowKeyboard } });
    RestoreCurrentImeSubType();
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::ShowKeyboard(const sptr<IInputClient> &currentClient)
{
    IMSA_HILOGD("PerUserSession::ShowKeyboard start.");
    auto clientInfo = GetClientInfo(currentClient->AsObject());
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
    UpdateClientInfo(currentClient->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, isShowKeyboard } });
    return ErrorCode::NO_ERROR;
}

/** Handle the situation a remote input client died.
 * It's called when a remote input client died
 * @param the remote object handler of the input client died.
 */
void PerUserSession::OnClientDied(sptr<IInputClient> remote)
{
    if (remote == nullptr) {
        return;
    }
    IMSA_HILOGI("userId: %{public}d.", userId_);
    if (IsSameClient(remote, GetCurrentClient())) {
        auto clientInfo = GetClientInfo(remote->AsObject());
        if (clientInfo != nullptr) {
            StopImeInput(clientInfo->bindImeType, clientInfo->channel);
        }
        SetCurrentClient(nullptr);
        RestoreCurrentImeSubType();
    }
    RemoveClientInfo(remote->AsObject(), true);
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
        NotifyImeStopFinished();
        IMSA_HILOGI("%{public}d not current imeData.", type);
        return;
    }
    RemoveImeData(type, true);
    InputTypeManager::GetInstance().Set(false);
    if (!OsAccountAdapter::IsOsAccountForeground(userId_)) {
        IMSA_HILOGW("userId:%{public}d in background, no need to restart ime.", userId_);
        return;
    }
    auto client = GetCurrentClient();
    auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
    if (clientInfo != nullptr && clientInfo->bindImeType == type) {
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

    auto client = GetCurrentClient();
    auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
    if (clientInfo != nullptr && clientInfo->bindImeType == type) {
        UnBindClientWithIme(clientInfo);
    }
    RemoveImeData(type, true);
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnHideCurrentInput()
{
    sptr<IInputClient> client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGE("current client is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    return HideKeyboard(client);
}

int32_t PerUserSession::OnShowCurrentInput()
{
    IMSA_HILOGD("PerUserSession::OnShowCurrentInput start.");
    sptr<IInputClient> client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGE("current client is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    return ShowKeyboard(client);
}

int32_t PerUserSession::OnHideInput(sptr<IInputClient> client)
{
    IMSA_HILOGD("PerUserSession::OnHideInput start.");
    if (!IsSameClient(client, GetCurrentClient())) {
        IMSA_HILOGE("client is not current client!");
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    return HideKeyboard(client);
}

int32_t PerUserSession::OnShowInput(sptr<IInputClient> client)
{
    IMSA_HILOGD("PerUserSession::OnShowInput start.");
    if (!IsSameClient(client, GetCurrentClient())) {
        IMSA_HILOGE("client is not current client!");
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    return ShowKeyboard(client);
}

void PerUserSession::OnHideSoftKeyBoardSelf()
{
    IMSA_HILOGD("PerUserSession::OnHideSoftKeyBoardSel start.");
    sptr<IInputClient> client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGE("current client is nullptr!");
        return;
    }
    UpdateClientInfo(client->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, false } });
    RestoreCurrentImeSubType();
}

int32_t PerUserSession::OnRequestShowInput()
{
    IMSA_HILOGD("PerUserSession::OnRequestShowInput start.");
    auto data = GetReadyImeData(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d doesn't exist!", ImeType::IME);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    auto ret = RequestIme(data, RequestType::REQUEST_SHOW, [&data] { return data->core->ShowKeyboard(); });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to show keyboard, ret: %{public}d!", ret);
        return ErrorCode::ERROR_KBD_SHOW_FAILED;
    }
    InputMethodSysEvent::GetInstance().ReportImeState(ImeState::BIND, data->pid,
        ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName);
    Memory::MemMgrClient::GetInstance().SetCritical(getpid(), true, INPUT_METHOD_SYSTEM_ABILITY_ID);
    auto currentClient = GetCurrentClient();
    if (currentClient != nullptr) {
        UpdateClientInfo(currentClient->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, true } });
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnRequestHideInput()
{
    IMSA_HILOGD("PerUserSession::OnRequestHideInput start.");
    auto data = GetReadyImeData(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d doesn't exist!", ImeType::IME);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }

    bool isForce = false;
    if (!data->freezeMgr->IsIpcNeeded(RequestType::REQUEST_HIDE)) {
        IMSA_HILOGD("need to force hide");
        isForce = true;
    }
    auto ret = RequestIme(data, RequestType::REQUEST_HIDE,
        [&data, isForce] { return data->core->HideKeyboard(isForce); });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to hide keyboard, ret: %{public}d!", ret);
        return ErrorCode::ERROR_KBD_HIDE_FAILED;
    }
    auto currentClient = GetCurrentClient();
    if (currentClient != nullptr) {
        UpdateClientInfo(currentClient->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, false } });
    }
    auto inactiveClient = GetInactiveClient();
    if (inactiveClient != nullptr) {
        RemoveClient(inactiveClient, false);
    }
    RestoreCurrentImeSubType();
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
    if (inputClient == nullptr) {
        IMSA_HILOGE("inputClient is nullptr!");
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto it = mapClients_.find(inputClient);
    if (it == mapClients_.end()) {
        IMSA_HILOGD("client not found.");
        return nullptr;
    }
    return it->second;
}

std::shared_ptr<InputClientInfo> PerUserSession::GetClientInfo(pid_t pid)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto iter = std::find_if(mapClients_.begin(), mapClients_.end(),
        [pid](const auto &mapClient) { return mapClient.second->pid == pid; });
    if (iter == mapClients_.end()) {
        IMSA_HILOGD("not found.");
        return nullptr;
    }
    return iter->second;
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
    return RemoveClient(client, true);
}

int32_t PerUserSession::RemoveClient(const sptr<IInputClient> &client, bool isUnbindFromClient)
{
    if (client == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    // if client is current client, unbind firstly
    auto clientInfo = GetClientInfo(client->AsObject());
    if (IsSameClient(client, GetCurrentClient())) {
        UnBindClientWithIme(clientInfo, isUnbindFromClient);
        SetCurrentClient(nullptr);
        RestoreCurrentImeSubType();
    }
    if (IsSameClient(client, GetInactiveClient())) {
        SetInactiveClient(nullptr);
    }
    StopClientInput(clientInfo);
    RemoveClientInfo(client->AsObject());
    return ErrorCode::NO_ERROR;
}

void PerUserSession::DeactivateClient(const sptr<IInputClient> &client)
{
    if (client == nullptr) {
        IMSA_HILOGD("client is nullptr.");
        return;
    }
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        return;
    }
    IMSA_HILOGI("deactivate client[%{public}d].", clientInfo->pid);
    UpdateClientInfo(client->AsObject(), { { UpdateFlag::STATE, ClientState::INACTIVE } });
    if (IsSameClient(client, GetCurrentClient())) {
        SetCurrentClient(nullptr);
    }
    SetInactiveClient(client);
    client->DeactivateClient();
    if (InputTypeManager::GetInstance().IsStarted()) {
        RestoreCurrentImeSubType();
        return;
    }
    auto data = GetReadyImeData(clientInfo->bindImeType);
    if (data == nullptr) {
        IMSA_HILOGE("ime %{public}d doesn't exist!", clientInfo->bindImeType);
        return;
    }
    RequestIme(data, RequestType::NORMAL, [&data, &clientInfo] {
        data->core->OnClientInactive(clientInfo->channel);
        return ErrorCode::NO_ERROR;
    });
    InputMethodSysEvent::GetInstance().ReportImeState(ImeState::UNBIND, data->pid,
        ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName);
    Memory::MemMgrClient::GetInstance().SetCritical(getpid(), false, INPUT_METHOD_SYSTEM_ABILITY_ID);
}

bool PerUserSession::IsProxyImeEnable()
{
    auto data = GetReadyImeData(ImeType::PROXY_IME);
    return data != nullptr && data->core != nullptr && data->core->IsEnable();
}

int32_t PerUserSession::OnStartInput(const InputClientInfo &inputClientInfo, sptr<IRemoteObject> &agent)
{
    const sptr<IInputClient> &client = inputClientInfo.client;
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    IMSA_HILOGD("start input with keyboard[%{public}d].", inputClientInfo.isShowKeyboard);
    if (IsSameClient(client, GetCurrentClient()) && IsImeBindChanged(clientInfo->bindImeType)) {
        UnBindClientWithIme(clientInfo);
    }
    InputClientInfo infoTemp = *clientInfo;
    infoTemp.isShowKeyboard = inputClientInfo.isShowKeyboard;
    infoTemp.isNotifyInputStart = inputClientInfo.isNotifyInputStart;
    infoTemp.needHide = inputClientInfo.needHide;
    auto imeType = IsProxyImeEnable() ? ImeType::PROXY_IME : ImeType::IME;
    int32_t ret = BindClientWithIme(std::make_shared<InputClientInfo>(infoTemp), imeType, true);
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
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::BindClientWithIme(const std::shared_ptr<InputClientInfo> &clientInfo, ImeType type,
    bool isBindFromClient)
{
    if (clientInfo == nullptr) {
        IMSA_HILOGE("clientInfo is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
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
        InputMethodSysEvent::GetInstance().ReportImeState(ImeState::BIND, data->pid,
            ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName);
        Memory::MemMgrClient::GetInstance().SetCritical(getpid(), true, INPUT_METHOD_SYSTEM_ABILITY_ID);
    }
    if (!isBindFromClient && clientInfo->client->OnInputReady(data->agent) != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("start client input failed, ret: %{public}d!", ret);
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    UpdateClientInfo(clientInfo->client->AsObject(),
        { { UpdateFlag::BINDIMETYPE, type }, { UpdateFlag::ISSHOWKEYBOARD, clientInfo->isShowKeyboard },
            { UpdateFlag::STATE, ClientState::ACTIVE } });
    ReplaceCurrentClient(clientInfo->client);
    return ErrorCode::NO_ERROR;
}

void PerUserSession::UnBindClientWithIme(const std::shared_ptr<InputClientInfo> &currentClientInfo,
    bool isUnbindFromClient)
{
    if (currentClientInfo == nullptr) {
        return;
    }
    if (!isUnbindFromClient) {
        IMSA_HILOGD("unbind from service.");
        StopClientInput(currentClientInfo);
    }
    StopImeInput(currentClientInfo->bindImeType, currentClientInfo->channel);
}

void PerUserSession::StopClientInput(const std::shared_ptr<InputClientInfo> &clientInfo)
{
    if (clientInfo == nullptr || clientInfo->client == nullptr) {
        return;
    }
    auto ret = clientInfo->client->OnInputStop();
    IMSA_HILOGI("stop client input, client pid: %{public}d, ret: %{public}d.", ret, clientInfo->pid);
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
    auto client = GetCurrentClient();
    auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
    if (clientInfo != nullptr && IsImeStartInBind(clientInfo->bindImeType, imeType)) {
        BindClientWithIme(clientInfo, imeType);
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
    auto client = GetCurrentClient();
    auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
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

int32_t PerUserSession::OnUnRegisteredProxyIme(UnRegisteredType type, const sptr<IInputMethodCore> &core)
{
    IMSA_HILOGD("proxy unregister type: %{public}d.", type);
    // 0: stop proxy  1: switch to ima
    if (type == UnRegisteredType::REMOVE_PROXY_IME) {
        RemoveIme(core, ImeType::PROXY_IME);
        return ErrorCode::NO_ERROR;
    }
    if (type == UnRegisteredType::SWITCH_PROXY_IME_TO_IME) {
        auto client = GetCurrentClient();
        auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
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

void PerUserSession::ReplaceCurrentClient(const sptr<IInputClient> &client)
{
    std::lock_guard<std::mutex> lock(focusedClientLock_);
    if (client == nullptr) {
        return;
    }
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        return;
    }
    auto replacedClient = GetCurrentClient();
    SetCurrentClient(client);
    if (replacedClient != nullptr) {
        auto replacedClientInfo = GetClientInfo(replacedClient->AsObject());
        if (replacedClientInfo != nullptr && replacedClientInfo->pid != clientInfo->pid) {
            IMSA_HILOGI("remove replaced client: [%{public}d]", replacedClientInfo->pid);
            RemoveClient(replacedClient);
        }
    }
    auto inactiveClient = GetInactiveClient();
    if (inactiveClient != nullptr) {
        auto inactiveClientInfo = GetClientInfo(inactiveClient->AsObject());
        if (inactiveClientInfo != nullptr && inactiveClientInfo->pid != clientInfo->pid) {
            IMSA_HILOGI("remove inactive client: [%{public}d]", inactiveClientInfo->pid);
            RemoveClient(inactiveClient, false);
        }
    }
}

void PerUserSession::SetInactiveClient(sptr<IInputClient> client)
{
    IMSA_HILOGD("set inactive client.");
    std::lock_guard<std::mutex> lock(inactiveClientLock_);
    inactiveClient_ = client;
}

sptr<IInputClient> PerUserSession::GetInactiveClient()
{
    std::lock_guard<std::mutex> lock(inactiveClientLock_);
    return inactiveClient_;
}

void PerUserSession::NotifyImeChangeToClients(const Property &property, const SubProperty &subProperty)
{
    IMSA_HILOGD("start.");
    std::lock_guard<std::recursive_mutex> lock(mtx);
    for (const auto &client : mapClients_) {
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
        data->deathRecipient = nullptr;
    }
    imeData_.erase(type);
}

void PerUserSession::OnFocused(int32_t pid, int32_t uid)
{
    std::lock_guard<std::mutex> lock(focusedClientLock_);
    auto client = GetCurrentClient();
    if (client == nullptr) {
        return;
    }
    if (IsCurClientFocused(pid, uid)) {
        IMSA_HILOGD("current client focused, focusedPid: %{public}d", pid);
        return;
    }
    if (!OHOS::Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        IMSA_HILOGI("focus shifts to pid: %{public}d, remove current client.", pid);
        RemoveClient(client);
        InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_UNFOCUSED);
        return;
    }
    IMSA_HILOGI("focus shifts to pid: %{public}d, deactivate current client.", pid);
    DeactivateClient(client);
}

void PerUserSession::OnUnfocused(int32_t pid, int32_t uid)
{
    if (GetCurrentClient() == nullptr) {
        return;
    }
    if (IsCurClientUnFocused(pid, uid)) {
        IMSA_HILOGD("current client Unfocused, unFocusedPid: %{public}d", pid);
        return;
    }
    auto clientInfo = GetClientInfo(pid);
    if (clientInfo == nullptr) {
        return;
    }
    RemoveClient(clientInfo->client);
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_UNFOCUSED);
}

std::shared_ptr<InputClientInfo> PerUserSession::GetCurClientInfo()
{
    auto client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGD("no client in bound state.");
        return nullptr;
    }
    return GetClientInfo(client->AsObject());
}

bool PerUserSession::IsCurClientFocused(int32_t pid, int32_t uid)
{
    auto clientInfo = GetCurClientInfo();
    if (clientInfo == nullptr) {
        IMSA_HILOGE("failed to get cur client info!");
        return false;
    }
    auto identityChecker = std::make_shared<IdentityCheckerImpl>();
    if (clientInfo->uiExtensionTokenId != IMF_INVALID_TOKENID &&
        identityChecker->IsFocusedUIExtension(clientInfo->uiExtensionTokenId)) {
        IMSA_HILOGI("UIExtension focused");
        return true;
    }
    return clientInfo->pid == pid && clientInfo->uid == uid;
}

bool PerUserSession::IsCurClientUnFocused(int32_t pid, int32_t uid)
{
    auto clientInfo = GetCurClientInfo();
    if (clientInfo == nullptr) {
        IMSA_HILOGE("failed to get cur client info!");
        return false;
    }
    auto identityChecker = std::make_shared<IdentityCheckerImpl>();
    if (clientInfo->uiExtensionTokenId != IMF_INVALID_TOKENID &&
        !identityChecker->IsFocusedUIExtension(clientInfo->uiExtensionTokenId)) {
        IMSA_HILOGI("UIExtension UnFocused.");
        return true;
    }
    return clientInfo->pid == pid && clientInfo->uid == uid;
}

bool PerUserSession::IsSameClient(sptr<IInputClient> source, sptr<IInputClient> dest)
{
    return source != nullptr && dest != nullptr && source->AsObject() == dest->AsObject();
}

bool PerUserSession::StartCurrentIme(bool isStopCurrentIme)
{
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    auto imeToStart = ImeInfoInquirer::GetInstance().GetImeToStart(userId_);
    IMSA_HILOGD("currentIme: %{public}s, imeToStart: %{public}s.", currentIme->imeId.c_str(),
        imeToStart->imeId.c_str());
    if (!StartIme(imeToStart, isStopCurrentIme)) {
        IMSA_HILOGE("failed to start ime!");
        InputMethodSysEvent::GetInstance().InputmethodFaultReporter(ErrorCode::ERROR_IME_START_FAILED,
            imeToStart->imeId, "start ime failed!");
        return false;
    }
    IMSA_HILOGI("current ime changed to %{public}s.", imeToStart->imeId.c_str());
    auto currentImeInfo =
        ImeInfoInquirer::GetInstance().GetImeInfo(userId_, imeToStart->bundleName, imeToStart->subName);
    if (currentImeInfo != nullptr) {
        NotifyImeChangeToClients(currentImeInfo->prop, currentImeInfo->subProp);
    }
    return true;
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
        IMSA_HILOGE("currentImeCfg is nullptr");
        return false;
    }
    imeId.bundleName = currentImeCfg->bundleName;
    imeId.subName = currentImeCfg->extName;
    return true;
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
    IMSA_HILOGI("userId: %{public}d, ime: %{public}s, mode: %{public}d, isolatedSandbox: %{public}d", userId_,
        ime->imeId.c_str(), static_cast<int32_t>(mode), isolatedSandBox);
    return want;
}

bool PerUserSession::StartInputService(const std::shared_ptr<ImeNativeCfg> &ime)
{
    if (ime == nullptr) {
        return false;
    }
    isImeStarted_.Clear(false);
    sptr<AAFwk::IAbilityConnection> connection = new (std::nothrow) ImeConnection();
    if (connection == nullptr) {
        IMSA_HILOGE("failed to create connection!");
        return false;
    }
    auto want = GetWant(ime);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectExtensionAbility(want, connection, userId_);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("connect %{public}s failed, ret: %{public}d!", ime->imeId.c_str(), ret);
        InputMethodSysEvent::GetInstance().InputmethodFaultReporter(ErrorCode::ERROR_IME_START_FAILED, ime->imeId,
            "failed to start ability.");
        return false;
    }
    InitImeData({ ime->bundleName, ime->extName });
    if (!isImeStarted_.GetValue()) {
        IMSA_HILOGE("start %{public}s timeout!", ime->imeId.c_str());
        return false;
    }
    IMSA_HILOGI("%{public}s started successfully.", ime->imeId.c_str());
    InputMethodSysEvent::GetInstance().RecordEvent(IMEBehaviour::START_IME);
    auto info = ImeInfoInquirer::GetInstance().GetImeInfo(userId_, ime->bundleName, ime->subName);
    if (info == nullptr) {
        IMSA_HILOGW("ime doesn't exist!");
        return true;
    }
    auto subProp = info->subProp;
    auto data = GetReadyImeData(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGW("ime doesn't exist!");
        return true;
    }
    RequestIme(data, RequestType::NORMAL, [&data, &subProp] { return data->core->SetSubtype(subProp); });
    return true;
}

int64_t PerUserSession::GetCurrentClientPid()
{
    auto client = GetCurrentClient();
    if (client == nullptr) {
        return INVALID_PID;
    }
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        return INVALID_PID;
    }
    return clientInfo->pid;
}

int64_t PerUserSession::GetInactiveClientPid()
{
    auto client = GetInactiveClient();
    if (client == nullptr) {
        return INVALID_PID;
    }
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        return INVALID_PID;
    }
    return clientInfo->pid;
}

int32_t PerUserSession::OnPanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info)
{
    auto clientMap = GetClientMap();
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
        IMSA_HILOGE("failed to AddClientInfo");
        return ret;
    }
    auto info = GetClientInfo(remoteClient);
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    if (info->eventFlag == NO_EVENT_ON && info->bindImeType == ImeType::NONE) {
        RemoveClientInfo(remoteClient, false);
    }
    return ErrorCode::NO_ERROR;
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

bool PerUserSession::IsImeBindChanged(ImeType bindImeType)
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

int32_t PerUserSession::IsPanelShown(const PanelInfo &panelInfo, bool &isShown)
{
    if (GetCurrentClient() == nullptr) {
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
    auto client = GetCurrentClient();
    auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
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
        IMSA_HILOGE("data is nullptr");
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
        IMSA_HILOGE("currentClient is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return RemoveClient(currentClient, false);
}

bool PerUserSession::IsWmsReady()
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        IMSA_HILOGD("scb enable");
        return WmsConnectionObserver::IsWmsConnected(userId_);
    }
    return IsReady(WINDOW_MANAGER_SERVICE_ID);
}

bool PerUserSession::IsReady(int32_t saId)
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
        if (IsReady(MEMORY_MANAGER_SA_ID) && IsWmsReady()) {
            auto ret = StartCurrentIme(true);
            if (!ret) {
                IMSA_HILOGE("start ime failed");
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

std::shared_ptr<ImeData> PerUserSession::GetImeData(ImeType type)
{
    std::lock_guard<std::mutex> lock(imeDataLock_);
    auto it = imeData_.find(type);
    if (it == imeData_.end()) {
        return nullptr;
    }
    return it->second;
}

bool PerUserSession::StartIme(const std::shared_ptr<ImeNativeCfg> &ime, bool isStopCurrentIme)
{
    std::lock_guard<std::mutex> lock(imeStartLock_);
    if (ime == nullptr) {
        return false;
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

bool PerUserSession::StartCurrentIme(const std::shared_ptr<ImeNativeCfg> &ime)
{
    auto imeData = GetImeData(ImeType::IME);
    if (imeData == nullptr) {
        return StartInputService(ime);
    }
    auto action = GetImeAction(ImeEvent::START_IME);
    if (action == ImeAction::DO_ACTION_IN_IME_EVENT_CONVERT_FAILED) {
        return false;
    }
    if (action == ImeAction::DO_ACTION_IN_NULL_IME_DATA) {
        return StartInputService(ime);
    }
    if (action == ImeAction::DO_NOTHING) {
        return true;
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
    if (!StopExitingCurrentIme()) {
        return false;
    }
    return StartInputService(ime);
}

bool PerUserSession::HandleStartImeTimeout(const std::shared_ptr<ImeNativeCfg> &ime)
{
    auto action = GetImeAction(ImeEvent::START_IME_TIMEOUT);
    if (action == ImeAction::DO_ACTION_IN_NULL_IME_DATA) {
        return StartInputService(ime);
    }
    if (action == ImeAction::DO_ACTION_IN_IME_EVENT_CONVERT_FAILED) {
        return false;
    }
    if (action == ImeAction::DO_NOTHING) {
        IMSA_HILOGW("ready when timeout");
        return true;
    }
    ForceStopCurrentIme(false);
    return false;
}

bool PerUserSession::StartNewIme(const std::shared_ptr<ImeNativeCfg> &ime)
{
    if (!StopCurrentIme()) {
        return false;
    }
    return StartInputService(ime);
}

bool PerUserSession::StopCurrentIme()
{
    auto action = GetImeAction(ImeEvent::STOP_IME);
    if (action == ImeAction::DO_ACTION_IN_NULL_IME_DATA) {
        return true;
    }
    if (action == ImeAction::DO_ACTION_IN_IME_EVENT_CONVERT_FAILED) {
        return false;
    }
    if (action == ImeAction::STOP_READY_IME) {
        return StopReadyCurrentIme();
    }
    if (action == ImeAction::STOP_STARTING_IME) {
        return ForceStopCurrentIme();
    }
    return StopExitingCurrentIme();
}

bool PerUserSession::StopReadyCurrentIme()
{
    auto client = GetCurrentClient();
    auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
    if (clientInfo != nullptr && clientInfo->bindImeType == ImeType::IME) {
        StopClientInput(clientInfo);
    }
    auto imeData = GetImeData(ImeType::IME);
    if (imeData == nullptr) {
        return true;
    }
    if (imeData->core == nullptr) {
        IMSA_HILOGE("core is nullptr.");
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
        return false;
    }
    return true;
}

bool PerUserSession::StopExitingCurrentIme()
{
    auto imeData = GetImeData(ImeType::IME);
    if (imeData == nullptr) {
        return true;
    }
    if (!ImeInfoInquirer::GetInstance().IsRunningIme(userId_, imeData->ime.first)) {
        IMSA_HILOGD("already stop!");
        RemoveImeData(ImeType::IME, true);
        return true;
    }
    return ForceStopCurrentIme();
}

bool PerUserSession::ForceStopCurrentIme(bool isNeedWait)
{
    auto client = GetCurrentClient();
    auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
    if (clientInfo != nullptr && clientInfo->bindImeType == ImeType::IME) {
        StopClientInput(clientInfo);
    }
    auto imeData = GetImeData(ImeType::IME);
    if (imeData == nullptr) {
        return true;
    }
    AAFwk::Want want;
    want.SetElementName(imeData->ime.first, imeData->ime.second);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->StopExtensionAbility(
        want, nullptr, userId_, AppExecFwk::ExtensionAbilityType::INPUTMETHOD);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("StopExtensionAbility [%{public}s, %{public}s] failed, ret: %{public}d!",
            imeData->ime.first.c_str(), imeData->ime.second.c_str(), ret);
        return false;
    }
    if (!isNeedWait) {
        return true;
    }
    WaitForCurrentImeStop();
    if (ImeInfoInquirer::GetInstance().IsRunningIme(userId_, imeData->ime.first)) {
        IMSA_HILOGW("stop [%{public}s, %{public}s] timeout.", imeData->ime.first.c_str(), imeData->ime.second.c_str());
        return false;
    }
    RemoveImeData(ImeType::IME, true);
    return true;
}

bool PerUserSession::HandleFirstStart(const std::shared_ptr<ImeNativeCfg> &ime, bool isStopCurrentIme)
{
    if (runningIme_.empty()) {
        return StartInputService(ime);
    }
    IMSA_HILOGW("imsa abnormal restore.");
    if (isStopCurrentIme) {
        return true;
    }
    if (BlockRetry(CHECK_IME_RUNNING_RETRY_INTERVAL, CHECK_IME_RUNNING_RETRY_TIMES,
                   [this]() -> bool { return !ImeInfoInquirer::GetInstance().IsRunningIme(userId_, runningIme_); })) {
        IMSA_HILOGI("[%{public}d, %{public}s] stop completely", userId_, runningIme_.c_str());
        runningIme_.clear();
        return StartInputService(ime);
    }
    IMSA_HILOGW("[%{public}d, %{public}s] stop timeout", userId_, runningIme_.c_str());
    return false;
}

int32_t PerUserSession::RestoreCurrentIme()
{
    InputTypeManager::GetInstance().Set(false);
    auto cfgIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    auto imeData = GetReadyImeData(ImeType::IME);
    if (imeData != nullptr && imeData->ime.first == cfgIme->bundleName && imeData->ime.second == cfgIme->extName) {
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGD("need restore!");
    if (!StartIme(cfgIme)) {
        IMSA_HILOGE("start ime failed!");
        return ErrorCode::ERROR_IME_START_FAILED;
    }
    SubProperty subProp = { .name = cfgIme->bundleName, .id = cfgIme->subName };
    auto subPropTemp = ImeInfoInquirer::GetInstance().GetCurrentSubtype(userId_);
    if (subPropTemp != nullptr) {
        subProp = *subPropTemp;
    }
    SwitchSubtype(subProp);
    return ErrorCode::NO_ERROR;
}

bool PerUserSession::CheckPwdInputPatternConv(InputClientInfo &newClientInfo)
{
    auto exClient = GetCurrentClient();
    if (exClient == nullptr) {
        exClient = GetInactiveClient();
    }
    auto exClientInfo = exClient != nullptr ? GetClientInfo(exClient->AsObject()) : nullptr;
    if (exClientInfo == nullptr) {
        IMSA_HILOGE("exClientInfo is nullptr!");
        return false;
    }
    // if current input pattern differ from previous in pwd and normal, need hide panel first.
    if (newClientInfo.config.inputAttribute.GetSecurityFlag()) {
        IMSA_HILOGI("new input pattern is pwd.");
        return !exClientInfo->config.inputAttribute.GetSecurityFlag();
    }
    IMSA_HILOGI("new input pattern is normal.");
    return exClientInfo->config.inputAttribute.GetSecurityFlag();
}
} // namespace MiscServices
} // namespace OHOS