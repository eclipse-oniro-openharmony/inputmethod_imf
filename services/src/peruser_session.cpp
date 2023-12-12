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

#include "ability_manager_client.h"
#include "element_name.h"
#include "ime_cfg_manager.h"
#include "ime_info_inquirer.h"
#include "input_client_proxy.h"
#include "input_control_channel_proxy.h"
#include "input_data_channel_proxy.h"
#include "input_method_agent_proxy.h"
#include "input_method_core_proxy.h"
#include "input_type_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "parcel.h"
#include "scene_board_judgement.h"
#include "sys/prctl.h"
#include "system_ability_definition.h"
#include "unistd.h"
#include "want.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
constexpr uint32_t IME_RESTART_TIMES = 5;
constexpr uint32_t IME_RESTART_INTERVAL = 300;
constexpr int64_t INVALID_PID = -1;
PerUserSession::PerUserSession(int32_t userId) : userId_(userId)
{
}

PerUserSession::~PerUserSession()
{
}

int PerUserSession::AddClientInfo(
    sptr<IRemoteObject> inputClient, const InputClientInfo &clientInfo, ClientAddEvent event)
{
    IMSA_HILOGD("PerUserSession, run in");
    auto cacheInfo = GetClientInfo(inputClient);
    if (cacheInfo != nullptr) {
        IMSA_HILOGD("info is existed");
        UpdateClientInfo(inputClient, { { UpdateFlag::CLIENT_INFO, clientInfo.config } });
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
        IMSA_HILOGE("client obj is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    if (!obj->AddDeathRecipient(info->deathRecipient)) {
        IMSA_HILOGE("failed to add client death recipient");
        return ErrorCode::ERROR_CLIENT_ADD_FAILED;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx);
    mapClients_.insert({ inputClient, info });
    IMSA_HILOGI("add client end");
    return ErrorCode::NO_ERROR;
}

void PerUserSession::RemoveClientInfo(const sptr<IRemoteObject> &client, bool isClientDied)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto clientInfo = GetClientInfo(client);
    if (clientInfo == nullptr) {
        IMSA_HILOGD("client already removed");
        return;
    }
    // if client is subscriber and the release is not because of the client died, do not remove
    if (clientInfo->eventFlag != EventStatusManager::NO_EVENT_ON && !isClientDied) {
        IMSA_HILOGD("is subscriber, do not remove");
        auto isShowKeyboard = false;
        auto bindImeType = ImeType::NONE;
        UpdateClientInfo(
            client, { { UpdateFlag::BINDIMETYPE, bindImeType }, { UpdateFlag::ISSHOWKEYBOARD, isShowKeyboard } });
        return;
    }
    if (clientInfo->deathRecipient != nullptr) {
        IMSA_HILOGD("deathRecipient remove");
        client->RemoveDeathRecipient(clientInfo->deathRecipient);
        clientInfo->deathRecipient = nullptr;
    }
    mapClients_.erase(client);
    IMSA_HILOGI("client[%{public}d] is removed", clientInfo->pid);
}

void PerUserSession::UpdateClientInfo(const sptr<IRemoteObject> &client,
    const std::unordered_map<UpdateFlag, std::variant<bool, uint32_t, ImeType, ClientState, TextTotalConfig>> &updateInfos)
{
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr.");
        return;
    }
    auto info = GetClientInfo(client);
    if (info == nullptr) {
        IMSA_HILOGE("client info is not exist.");
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
            }
            default:
                break;
        }
    }
}

int32_t PerUserSession::HideKeyboard(const sptr<IInputClient> &currentClient)
{
    IMSA_HILOGD("PerUserSession::HideKeyboard");
    auto clientInfo = GetClientInfo(currentClient->AsObject());
    if (clientInfo == nullptr) {
        IMSA_HILOGE("client info is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    auto data = GetImeData(clientInfo->bindImeType);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist", clientInfo->bindImeType);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    auto ret = data->core->HideKeyboard();
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to hide keyboard, ret: %{public}d", ret);
        return ErrorCode::ERROR_KBD_HIDE_FAILED;
    }
    bool isShowKeyboard = false;
    UpdateClientInfo(currentClient->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, isShowKeyboard } });
    ExitCurrentInputType();
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::ShowKeyboard(const sptr<IInputClient> &currentClient)
{
    IMSA_HILOGD("PerUserSession::ShowKeyboard");
    auto clientInfo = GetClientInfo(currentClient->AsObject());
    if (clientInfo == nullptr) {
        IMSA_HILOGE("client info is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    auto data = GetImeData(clientInfo->bindImeType);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist", clientInfo->bindImeType);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    auto ret = data->core->ShowKeyboard();
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to show keyboard, ret: %{public}d", ret);
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
    IMSA_HILOGI("userId: %{public}d", userId_);
    if (IsCurrentClient(remote)) {
        auto clientInfo = GetClientInfo(remote->AsObject());
        StopImeInput(clientInfo->bindImeType, clientInfo->channel);
        SetCurrentClient(nullptr);
        ExitCurrentInputType();
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
    IMSA_HILOGI("type: %{public}d", type);
    RemoveImeData(type);
    auto client = GetCurrentClient();
    auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
    if (clientInfo != nullptr && clientInfo->bindImeType == type) {
        StopClientInput(client);
    }
    if (type == ImeType::IME) {
        InputTypeManager::GetInstance().Set(false);
        RestartIme();
    }
}

int32_t PerUserSession::RemoveIme(const sptr<IInputMethodCore> &core, ImeType type)
{
    if (core == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto data = GetImeData(type);
    if (data == nullptr || data->core->AsObject() != core->AsObject()) {
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }

    auto client = GetCurrentClient();
    auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
    if (clientInfo != nullptr && clientInfo->bindImeType == type) {
        UnBindClientWithIme(clientInfo);
    }
    RemoveImeData(type);
    return ErrorCode::NO_ERROR;
}

void PerUserSession::UpdateCurrentUserId(int32_t userId)
{
    userId_ = userId;
}

int32_t PerUserSession::OnHideCurrentInput()
{
    sptr<IInputClient> client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGE("current client is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    IMSA_HILOGI("PerUserSession");
    return HideKeyboard(client);
}

int32_t PerUserSession::OnShowCurrentInput()
{
    IMSA_HILOGD("PerUserSession::OnShowCurrentInput");
    sptr<IInputClient> client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGE("current client is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    return ShowKeyboard(client);
}

int32_t PerUserSession::OnHideInput(sptr<IInputClient> client)
{
    IMSA_HILOGD("PerUserSession::OnHideInput");
    if (!IsCurrentClient(client)) {
        IMSA_HILOGE("client is not current client");
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    return HideKeyboard(client);
}

int32_t PerUserSession::OnShowInput(sptr<IInputClient> client)
{
    IMSA_HILOGD("PerUserSession::OnShowInput");
    if (!IsCurrentClient(client)) {
        IMSA_HILOGE("client is not current client");
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    return ShowKeyboard(client);
}

void PerUserSession::OnHideSoftKeyBoardSelf()
{
    IMSA_HILOGD("run in");
    sptr<IInputClient> client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGE("current client is nullptr");
        return;
    }
    UpdateClientInfo(client->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, false } });
    ExitCurrentInputType();
}

int32_t PerUserSession::OnRequestShowInput()
{
    IMSA_HILOGD("run in");
    auto data = GetImeData(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d doesn't exist", ImeType::IME);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    auto ret = data->core->ShowKeyboard();
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to show keyboard, ret: %{public}d", ret);
        return ErrorCode::ERROR_KBD_SHOW_FAILED;
    }
    auto currentClient = GetCurrentClient();
    if (currentClient != nullptr) {
        UpdateClientInfo(currentClient->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, true } });
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnRequestHideInput()
{
    IMSA_HILOGD("run in");
    auto data = GetImeData(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d doesn't exist", ImeType::IME);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    auto ret = data->core->HideKeyboard();
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to hide keyboard, ret: %{public}d", ret);
        return ErrorCode::ERROR_KBD_HIDE_FAILED;
    }
    auto currentClient = GetCurrentClient();
    if (currentClient != nullptr) {
        UpdateClientInfo(currentClient->AsObject(), { { UpdateFlag::ISSHOWKEYBOARD, false } });
    }
    ExitCurrentInputType();
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

std::shared_ptr<InputClientInfo> PerUserSession::GetClientInfo(pid_t pid)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto iter = std::find_if(
        mapClients_.begin(), mapClients_.end(), [pid](const auto &mapClient) { return mapClient.second->pid == pid; });
    if (iter == mapClients_.end()) {
        IMSA_HILOGD("not found");
        return nullptr;
    }
    return iter->second;
}

int32_t PerUserSession::OnPrepareInput(const InputClientInfo &clientInfo)
{
    IMSA_HILOGD("PerUserSession::OnPrepareInput Start");
    return AddClientInfo(clientInfo.client->AsObject(), clientInfo, PREPARE_INPUT);
}

/** Release input. Called by an input client.Run in work thread of this user
 * @param the parameters from remote client
 * @return ErrorCode
 */
int32_t PerUserSession::OnReleaseInput(const sptr<IInputClient> &client)
{
    IMSA_HILOGD("PerUserSession::Start");
    return RemoveClient(client, true);
}

int32_t PerUserSession::RemoveClient(const sptr<IInputClient> &client, bool isUnbindFromClient)
{
    if (client == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    // if client is current client, unbind firstly
    if (IsCurrentClient(client)) {
        UnBindClientWithIme(GetClientInfo(client->AsObject()), isUnbindFromClient);
        SetCurrentClient(nullptr);
        ExitCurrentInputType();
    }
    StopClientInput(client);
    RemoveClientInfo(client->AsObject());
    return ErrorCode::NO_ERROR;
}

void PerUserSession::DeactivateClient(const sptr<IInputClient> &client)
{
    if (client == nullptr) {
        IMSA_HILOGD("client is nullptr");
        return;
    }
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        return;
    }
    IMSA_HILOGI("deactivate client[%{public}d]", clientInfo->pid);
    UpdateClientInfo(client->AsObject(), { { UpdateFlag::STATE, ClientState::INACTIVE } });
    if (IsCurrentClient(client)) {
        SetCurrentClient(nullptr);
    }
    SetInactiveClient(client);
    client->DeactivateClient();
    if (InputTypeManager::GetInstance().IsStarted()) {
        ExitCurrentInputType();
        return;
    }
    auto data = GetImeData(clientInfo->bindImeType);
    if (data == nullptr) {
        IMSA_HILOGE("ime %{public}d doesn't exist", clientInfo->bindImeType);
        return;
    }
    data->core->OnClientInactive(clientInfo->channel);
}

bool PerUserSession::IsProxyImeEnable()
{
    auto data = GetImeData(ImeType::PROXY_IME);
    return data != nullptr && data->core != nullptr && data->core->IsEnable();
}

int32_t PerUserSession::OnStartInput(const sptr<IInputClient> &client, bool isShowKeyboard, sptr<IRemoteObject> &agent)
{
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    IMSA_HILOGD("start input with keyboard[%{public}d]", isShowKeyboard);
    if (IsCurrentClient(client) && IsImeBindChanged(clientInfo->bindImeType)) {
        UnBindClientWithIme(clientInfo);
    }
    InputClientInfo infoTemp = *clientInfo;
    infoTemp.isShowKeyboard = isShowKeyboard;
    auto imeType = IsProxyImeEnable() ? ImeType::PROXY_IME : ImeType::IME;
    int32_t ret = BindClientWithIme(std::make_shared<InputClientInfo>(infoTemp), imeType, true);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("bind failed, ret: %{public}d", ret);
        return ret;
    }
    auto data = GetImeData(imeType);
    if (data == nullptr || data->agent == nullptr) {
        IMSA_HILOGE("data or agent is nullptr.");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    agent = data->agent->AsObject();
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::BindClientWithIme(
    const std::shared_ptr<InputClientInfo> &clientInfo, ImeType type, bool isBindFromClient)
{
    if (clientInfo == nullptr) {
        IMSA_HILOGE("clientInfo is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    IMSA_HILOGD("imeType: %{public}d, isShowKeyboard: %{public}d, isBindFromClient: %{public}d", type,
        clientInfo->isShowKeyboard, isBindFromClient);
    auto data = GetValidIme(type);
    if (data == nullptr) {
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    auto ret = InputTypeManager::GetInstance().IsStarted() ? data->core->OnTextConfigChange(*clientInfo)
                                                           : data->core->StartInput(*clientInfo, isBindFromClient);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("start input failed, ret: %{public}d", ret);
        return ErrorCode::ERROR_IME_START_INPUT_FAILED;
    }
    if (!isBindFromClient && clientInfo->client->OnInputReady(data->agent) != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("start client input failed, ret: %{public}d", ret);
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    UpdateClientInfo(clientInfo->client->AsObject(),
        { { UpdateFlag::BINDIMETYPE, type }, { UpdateFlag::ISSHOWKEYBOARD, clientInfo->isShowKeyboard },
            { UpdateFlag::STATE, ClientState::ACTIVE } });
    ReplaceCurrentClient(clientInfo->client);
    return ErrorCode::NO_ERROR;
}

void PerUserSession::UnBindClientWithIme(
    const std::shared_ptr<InputClientInfo> &currentClientInfo, bool isUnbindFromClient)
{
    if (currentClientInfo == nullptr) {
        return;
    }
    if (!isUnbindFromClient) {
        IMSA_HILOGD("Unbind from service.");
        StopClientInput(currentClientInfo->client);
    }
    StopImeInput(currentClientInfo->bindImeType, currentClientInfo->channel);
}

void PerUserSession::StopClientInput(const sptr<IInputClient> &currentClient)
{
    if (currentClient == nullptr) {
        return;
    }
    auto ret = currentClient->OnInputStop();
    IMSA_HILOGE("stop client input, ret: %{public}d", ret);
}

void PerUserSession::StopImeInput(ImeType currentType, const sptr<IInputDataChannel> &currentChannel)
{
    auto data = GetImeData(currentType);
    if (data == nullptr) {
        return;
    }
    auto ret = data->core->StopInput(currentChannel);
    IMSA_HILOGE("stop ime input, ret: %{public}d", ret);
}

void PerUserSession::OnSecurityChange(int32_t &security)
{
    auto data = GetImeData(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist", ImeType::IME);
        return;
    }
    auto ret = data->core->OnSecurityChange(security);
    IMSA_HILOGD("on security change, ret: %{public}d", ret);
}

int32_t PerUserSession::OnSetCoreAndAgent(const sptr<IInputMethodCore> &core, const sptr<IInputMethodAgent> &agent)
{
    IMSA_HILOGI("run in");
    auto imeType = ImeType::IME;
    auto ret = AddImeData(imeType, core, agent);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    ret = InitInputControlChannel();
    IMSA_HILOGI("init input control channel ret: %{public}d", ret);

    auto client = GetCurrentClient();
    auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
    if (clientInfo != nullptr && IsImeStartInBind(clientInfo->bindImeType, imeType)) {
        BindClientWithIme(clientInfo, imeType);
    }
    bool isStarted = true;
    isImeStarted_.SetValue(isStarted);
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnRegisterProxyIme(const sptr<IInputMethodCore> &core, const sptr<IInputMethodAgent> &agent)
{
    IMSA_HILOGD("run in");
    auto imeType = ImeType::PROXY_IME;
    auto ret = AddImeData(imeType, core, agent);
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
    IMSA_HILOGD("proxy unregister type: %{public}d", type);
    // 0: stop proxy  1: switch to ima
    if (type == UnRegisteredType::REMOVE_PROXY_IME) {
        RemoveIme(core, ImeType::PROXY_IME);
        return ErrorCode::NO_ERROR;
    }
    if (type == UnRegisteredType::SWITCH_PROXY_IME_TO_IME) {
        auto client = GetCurrentClient();
        auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
        if (clientInfo == nullptr) {
            IMSA_HILOGE("not find current client");
            return ErrorCode::ERROR_CLIENT_NOT_BOUND;
        }
        if (clientInfo->bindImeType == ImeType::PROXY_IME) {
            UnBindClientWithIme(clientInfo);
        }
        InputClientInfo infoTemp = {
            .isShowKeyboard = true, .client = clientInfo->client, .channel = clientInfo->channel
        };
        return BindClientWithIme(std::make_shared<InputClientInfo>(infoTemp), ImeType::IME);
    }
    return ErrorCode::ERROR_BAD_PARAMETERS;
}

int32_t PerUserSession::InitInputControlChannel()
{
    IMSA_HILOGD("PerUserSession::InitInputControlChannel");
    sptr<IInputControlChannel> inputControlChannel = new InputControlChannelStub(userId_);
    auto data = GetImeData(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist", ImeType::IME);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    return data->core->InitInputControlChannel(inputControlChannel);
}

void PerUserSession::StopInputService()
{
    auto data = GetImeData(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist", ImeType::IME);
        return;
    }
    IMSA_HILOGI("PerUserSession");
    RemoveImeData(ImeType::IME);
    auto client = GetCurrentClient();
    auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
    if (clientInfo != nullptr && clientInfo->bindImeType == ImeType::IME) {
        StopClientInput(client);
    }
    data->core->StopInputService();
}

bool PerUserSession::IsRestartIme()
{
    IMSA_HILOGD("PerUserSession::IsRestartIme");
    std::lock_guard<std::mutex> lock(resetLock);
    auto now = time(nullptr);
    if (difftime(now, manager.last) > IME_RESET_TIME_OUT) {
        manager = { 0, now };
    }
    ++manager.num;
    return manager.num <= MAX_RESTART_NUM;
}

void PerUserSession::RestartIme()
{
    if (!IsRestartIme()) {
        IMSA_HILOGI("ime deaths over max num");
        return;
    }
    if (!IsReadyToStartIme()) {
        return;
    }
    IMSA_HILOGI("user %{public}d ime died, restart!", userId_);
    StartInputService(ImeInfoInquirer::GetInstance().GetImeToBeStarted(userId_), true);
}

void PerUserSession::SetCurrentClient(sptr<IInputClient> client)
{
    IMSA_HILOGD("set current client");
    std::lock_guard<std::mutex> lock(clientLock_);
    currentClient_ = client;
}

sptr<IInputClient> PerUserSession::GetCurrentClient()
{
    std::lock_guard<std::mutex> lock(clientLock_);
    return currentClient_;
}

void PerUserSession::ReplaceCurrentClient(const sptr<IInputClient> &client)
{
    std::lock_guard<std::mutex> lock(focusedClientLock_);
    if (client == nullptr) {
        return;
    }
    auto replacedClient = GetCurrentClient();
    SetCurrentClient(client);
    if (replacedClient != nullptr && replacedClient != client) {
        IMSA_HILOGD("remove replaced client");
        RemoveClient(replacedClient);
    }
    auto inactiveClient = GetInactiveClient();
    if (inactiveClient != nullptr && inactiveClient != client) {
        IMSA_HILOGD("remove inactive client");
        RemoveClientInfo(inactiveClient->AsObject());
        SetInactiveClient(nullptr);
    }
}

void PerUserSession::SetInactiveClient(sptr<IInputClient> client)
{
    IMSA_HILOGD("set inactive client");
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
    IMSA_HILOGD("PerUserSession::NotifyImeChangeToClients");
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
}

int32_t PerUserSession::AddImeData(ImeType type, sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent)
{
    if (core == nullptr || agent == nullptr) {
        IMSA_HILOGE("core or agent is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    sptr<InputDeathRecipient> deathRecipient = new (std::nothrow) InputDeathRecipient();
    if (deathRecipient == nullptr) {
        IMSA_HILOGE("failed to new deathRecipient");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    deathRecipient->SetDeathRecipient([this, core, type](const wptr<IRemoteObject> &) { this->OnImeDied(core, type); });
    auto coreObject = core->AsObject();
    if (coreObject == nullptr || !coreObject->AddDeathRecipient(deathRecipient)) {
        IMSA_HILOGE("failed to add death recipient");
        return ErrorCode::ERROR_ADD_DEATH_RECIPIENT_FAILED;
    }
    std::lock_guard<std::mutex> lock(imeDataLock_);
    imeData_.insert_or_assign(type, std::make_shared<ImeData>(core, agent, deathRecipient));
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

std::shared_ptr<ImeData> PerUserSession::GetValidIme(ImeType type)
{
    auto data = GetImeData(type);
    if (data != nullptr || type != ImeType::IME) {
        return data;
    }
    IMSA_HILOGI("current ime is empty, try to restart it");
    if (!StartInputService(ImeInfoInquirer::GetInstance().GetImeToBeStarted(userId_), true)) {
        return nullptr;
    }
    data = GetImeData(type);
    return data;
}

void PerUserSession::RemoveImeData(ImeType type)
{
    std::lock_guard<std::mutex> lock(imeDataLock_);
    auto it = imeData_.find(type);
    if (it == imeData_.end()) {
        IMSA_HILOGD("imeData not found");
        return;
    }
    auto data = it->second;
    if (data->core != nullptr && data->core->AsObject() != nullptr) {
        data->core->AsObject()->RemoveDeathRecipient(data->deathRecipient);
    }
    data->deathRecipient = nullptr;
    imeData_.erase(type);
}

void PerUserSession::OnFocused(int32_t pid, int32_t uid)
{
    std::lock_guard<std::mutex> lock(focusedClientLock_);
    auto client = GetCurrentClient();
    if (client == nullptr) {
        return;
    }
    if (IsCurrentClient(pid, uid)) {
        IMSA_HILOGD("pid[%{public}d] same as current client", pid);
        return;
    }
    if (!OHOS::Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        IMSA_HILOGI("focus shifts to pid: %{public}d, remove current client", pid);
        RemoveClient(client);
        InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_UNFOCUSED);
        return;
    }
    IMSA_HILOGI("focus shifts to pid: %{public}d, deactivate current client", pid);
    DeactivateClient(client);
}

void PerUserSession::OnUnfocused(int32_t pid, int32_t uid)
{
    if (GetCurrentClient() == nullptr) {
        return;
    }
    if (IsCurrentClient(pid, uid)) {
        IMSA_HILOGD("pid[%{public}d] same as current client", pid);
        return;
    }
    auto clientInfo = GetClientInfo(pid);
    if (clientInfo == nullptr) {
        return;
    }
    RemoveClient(clientInfo->client);
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_UNFOCUSED);
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

bool PerUserSession::IsCurrentClient(sptr<IInputClient> client)
{
    auto currentClient = GetCurrentClient();
    return currentClient != nullptr && client != nullptr && client->AsObject() == currentClient->AsObject();
}

bool PerUserSession::StartInputService(const std::string &imeName, bool isRetry)
{
    std::string::size_type pos = imeName.find('/');
    if (pos == std::string::npos) {
        IMSA_HILOGE("invalid ime name");
        return false;
    }
    IMSA_HILOGI("start ime: %{public}s with isRetry: %{public}d", imeName.c_str(), isRetry);
    AAFwk::Want want;
    want.SetElementName(imeName.substr(0, pos), imeName.substr(pos + 1));
    isImeStarted_.Clear(false);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, nullptr, userId_, AppExecFwk::ExtensionAbilityType::INPUTMETHOD);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to start ability");
        InputMethodSysEvent::GetInstance().InputmethodFaultReporter(
            ErrorCode::ERROR_IME_START_FAILED, imeName, "StartInputService, failed to start ability.");
    } else if (isImeStarted_.GetValue()) {
        IMSA_HILOGI("ime started successfully");
        InputMethodSysEvent::GetInstance().RecordEvent(IMEBehaviour::START_IME);
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
    auto ret = AddClientInfo(remoteClient, clientInfo, START_LISTENING);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("AddClientInfo failed");
        return ret;
    }
    auto info = GetClientInfo(remoteClient);
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    if (info->eventFlag == EventStatusManager::NO_EVENT_ON && info->bindImeType == ImeType::NONE) {
        RemoveClientInfo(remoteClient, false);
    }
    return ErrorCode::NO_ERROR;
}

bool PerUserSession::IsReadyToStartIme()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGI("system ability manager is nullptr");
        return false;
    }
    auto systemAbility = systemAbilityManager->GetSystemAbility(WINDOW_MANAGER_SERVICE_ID, "");
    if (systemAbility == nullptr) {
        IMSA_HILOGI("window manager service not found");
        return false;
    }
    return true;
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

bool PerUserSession::IsBindProxyImeInImeBind(ImeType bindImeType)
{
    return bindImeType == ImeType::IME && IsProxyImeEnable();
}

bool PerUserSession::IsBindImeInProxyImeBind(ImeType bindImeType)
{
    return bindImeType == ImeType::PROXY_IME && !IsProxyImeEnable();
}

bool PerUserSession::IsImeBindChanged(ImeType bindImeType)
{
    return (bindImeType == ImeType::IME && IsProxyImeEnable())
           || (bindImeType == ImeType::PROXY_IME && !IsProxyImeEnable());
}

int32_t PerUserSession::SwitchSubtype(const SubProperty &subProperty)
{
    auto data = GetImeData(ImeType::IME);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist", ImeType::IME);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    return data->core->SetSubtype(subProperty);
}

bool PerUserSession::IsBoundToClient()
{
    if (GetCurrentClient() == nullptr) {
        IMSA_HILOGE("not in bound state");
        return false;
    }
    return true;
}

int32_t PerUserSession::ExitCurrentInputType()
{
    if (!InputTypeManager::GetInstance().IsStarted()) {
        IMSA_HILOGD("already exit");
        return ErrorCode::NO_ERROR;
    }
    auto typeIme = InputTypeManager::GetInstance().GetCurrentIme();
    auto cfgIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    if (cfgIme->bundleName == typeIme.bundleName) {
        IMSA_HILOGI("only need to switch subtype: %{public}s", cfgIme->subName.c_str());
        int32_t ret = SwitchSubtype({ .name = cfgIme->bundleName, .id = cfgIme->subName });
        if (ret == ErrorCode::NO_ERROR) {
            InputTypeManager::GetInstance().Set(false);
        }
        return ret;
    }
    IMSA_HILOGI("need switch ime to: %{public}s/%{public}s", cfgIme->bundleName.c_str(), cfgIme->subName.c_str());
    StopInputService();
    InputTypeManager::GetInstance().Set(false);
    if (!StartInputService(cfgIme->imeId, true)) {
        IMSA_HILOGE("failed to start ime");
        return ErrorCode::ERROR_IME_START_FAILED;
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::IsPanelShown(const PanelInfo &panelInfo, bool &isShown)
{
    auto ime = GetImeData(ImeType::IME);
    if (ime == nullptr) {
        IMSA_HILOGE("ime not started");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    return ime->core->IsPanelShown(panelInfo, isShown);
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
} // namespace MiscServices
} // namespace OHOS
