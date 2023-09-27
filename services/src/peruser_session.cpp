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
constexpr int64_t INVALID_PID = -1;
PerUserSession::PerUserSession(int32_t userId) : userId_(userId)
{
}

PerUserSession::~PerUserSession()
{
}

int PerUserSession::AddClientInfo(sptr<IRemoteObject> inputClient, const InputClientInfo &clientInfo, ClientAddEvent event)
{
    IMSA_HILOGD("PerUserSession, run in");
    auto cacheInfo = GetClientInfo(inputClient);
    if (cacheInfo != nullptr) {
        IMSA_HILOGI("client info is exist, not need add.");
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
        IMSA_HILOGI("failed to add client death recipient");
        return ErrorCode::ERROR_CLIENT_ADD_FAILED;
    }

    std::lock_guard<std::recursive_mutex> lock(mtx);
    mapClients_.insert({ inputClient, info });
    IMSA_HILOGD("add client end");
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
        IMSA_HILOGI("deathRecipient remove");
        client->RemoveDeathRecipient(clientInfo->deathRecipient);
        clientInfo->deathRecipient = nullptr;
    }
    mapClients_.erase(client);
    IMSA_HILOGD("client[%{public}d] is removed successfully", clientInfo->pid);
}

void PerUserSession::UpdateClientInfo(const sptr<IRemoteObject> &client,
    const std::unordered_map<UpdateFlag, std::variant<bool, uint32_t, ImeType>> &updateInfos)
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
    IMSA_HILOGI("userId: %{public}d", userId_);
    if (remote == nullptr) {
        return;
    }
    RemoveClient(remote, UnBindCause::CLIENT_DIED);
}

/** Handle the situation that an ime died
 * It's called when an ime died
 * @param the remote object handler of the ime who died.
 */
void PerUserSession::OnImsDied(const sptr<IInputMethodCore> &remote)
{
    IMSA_HILOGI("run in");
    RemoveIme(remote, ImeType::IMA, UnBindCause::IME_DIED);
    if (!IsRestartIme()) {
        IMSA_HILOGI("ime deaths over max num");
        return;
    }
    IMSA_HILOGI("user %{public}d ime died, restart!", userId_);
    if (!IsReadyToStartIme()) {
        return;
    }
    StartInputService(ImeInfoInquirer::GetInstance().GetStartedIme(userId_), true);
}

void PerUserSession::OnProxyDied(const sptr<IInputMethodCore> &remote)
{
    IMSA_HILOGI("run in");
    RemoveIme(remote, ImeType::PROXY, UnBindCause::IME_DIED);
}

int32_t PerUserSession::RemoveIme(const sptr<IInputMethodCore> &core, ImeType type, UnBindCause cause)
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
        UnBindClientWithIme(clientInfo, cause);
    }
    RemoveImeData(type);
    return ErrorCode::NO_ERROR;
}

void PerUserSession::UpdateCurrentUserId(int32_t userId)
{
    userId_ = userId;
}

int PerUserSession::OnHideCurrentInput()
{
    IMSA_HILOGI("PerUserSession::OnHideCurrentInput");
    sptr<IInputClient> client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGE("current client is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    return HideKeyboard(client);
}

int PerUserSession::OnShowCurrentInput()
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

int32_t PerUserSession::OnPrepareInput(const InputClientInfo &clientInfo)
{
    IMSA_HILOGD("PerUserSession::OnPrepareInput Start\n");
    return AddClientInfo(clientInfo.client->AsObject(), clientInfo, PREPARE_INPUT);
}

/** Release input. Called by an input client.Run in work thread of this user
 * @param the parameters from remote client
 * @return ErrorCode
 */
int32_t PerUserSession::OnReleaseInput(const sptr<IInputClient> &client)
{
    IMSA_HILOGI("PerUserSession::Start");
    return RemoveClient(client, UnBindCause::CLIENT_CLOSE_SELF);
}

int32_t PerUserSession::RemoveClient(const sptr<IInputClient> &client, UnBindCause cause)
{
    if (client == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    // if client is current client, unbind firstly
    if (IsCurrentClient(client)) {
        UnBindClientWithIme(GetClientInfo(client->AsObject()), cause);
        SetCurrentClient(nullptr);
    }
    RemoveClientInfo(client->AsObject(), cause == UnBindCause::CLIENT_DIED);
    return ErrorCode::NO_ERROR;
}

bool PerUserSession::IsCurrentClient(sptr<IInputClient> client)
{
    auto currentClient = GetCurrentClient();
    return currentClient != nullptr && client != nullptr && client->AsObject() == currentClient->AsObject();
}

bool PerUserSession::IsProxyEnable()
{
    auto data = GetImeData(ImeType::PROXY);
    return data != nullptr && data->core != nullptr && data->core->IsEnable();
}

int32_t PerUserSession::OnStartInput(const sptr<IInputClient> &client, bool isShowKeyboard)
{
    IMSA_HILOGD("start input with keyboard[%{public}d]", isShowKeyboard);
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        IMSA_HILOGE("client not find");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    if (IsCurrentClient(client)) {
        HandleSpecialScene(clientInfo);
    }
    InputClientInfo infoTemp = { .client = client, .channel = clientInfo->channel, .isShowKeyboard = isShowKeyboard };
    auto imeType = IsProxyEnable() ? ImeType::PROXY : ImeType::IMA;
    return BindClientWithIme(std::make_shared<InputClientInfo>(infoTemp), imeType);
}

void PerUserSession::HandleSpecialScene(const std::shared_ptr<InputClientInfo> &currentClientInfo, ImeType startImeType)
{
    if (currentClientInfo == nullptr) {
        return;
    }
    auto scene = GetSpecialScene(currentClientInfo->bindImeType, startImeType);
    switch (scene) {
        case SpecialScene::IMA_ATTACH_IN_PROXY_BIND:
        case SpecialScene::PROXY_ATTACH_IN_IMA_BIND: {
            UnBindClientWithIme(currentClientInfo, UnBindCause::IME_SWITCH);
            break;
        }
        case SpecialScene::IMA_START_IN_BIND: {
            BindClientWithIme(currentClientInfo, ImeType::IMA);
            break;
        }
        case SpecialScene::PROXY_START_IN_BIND: {
            BindClientWithIme(currentClientInfo, ImeType::PROXY);
            break;
        }
        case SpecialScene::PROXY_START_IN_IMA_BIND: {
            UnBindClientWithIme(currentClientInfo, UnBindCause::IME_SWITCH);
            BindClientWithIme(currentClientInfo, ImeType::PROXY);
            break;
        }
        default: {
            break;
        }
    }
}

SpecialScene PerUserSession::GetSpecialScene(ImeType bindImeType, ImeType startImeType)
{
    if (startImeType == ImeType::IMA && bindImeType == ImeType::IMA) {
        IMSA_HILOGD("IMA_START_IN_BIND");
        return SpecialScene::IMA_START_IN_BIND;
    }
    if (startImeType == ImeType::PROXY && bindImeType == ImeType::PROXY) {
        IMSA_HILOGD("PROXY_START_IN_BIND");
        return SpecialScene::PROXY_START_IN_BIND;
    }
    if (startImeType == ImeType::PROXY && bindImeType == ImeType::IMA) {
        IMSA_HILOGD("PROXY_START_IN_IMA_BIND");
        return SpecialScene::PROXY_START_IN_IMA_BIND;
    }
    if (startImeType == ImeType::NONE && bindImeType == ImeType::IMA && IsProxyEnable()) {
        IMSA_HILOGD("PROXY_ATTACH_IN_IMA_BIND");
        return SpecialScene::PROXY_ATTACH_IN_IMA_BIND;
    }
    if (startImeType == ImeType::NONE && bindImeType == ImeType::PROXY && !IsProxyEnable()) {
        IMSA_HILOGD("IMA_ATTACH_IN_PROXY_BIND");
        return SpecialScene::IMA_ATTACH_IN_PROXY_BIND;
    }
    return SpecialScene::NONE;
}

int32_t PerUserSession::BindClientWithIme(const std::shared_ptr<InputClientInfo> &clientInfo, ImeType type)
{
    if (clientInfo == nullptr) {
        IMSA_HILOGE("clientInfo is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    IMSA_HILOGI("imeType: %{public}d, isShowKeyboard: %{public}d", type, clientInfo->isShowKeyboard);
    auto data = GetImeData(type);
    if (data == nullptr && type == ImeType::IMA) {
        IMSA_HILOGI("current ime is empty, try to restart it");
        if (!StartInputService(ImeInfoInquirer::GetInstance().GetStartedIme(userId_), true)) {
            IMSA_HILOGE("failed to restart ime");
            return ErrorCode::ERROR_IME_START_FAILED;
        }
        data = GetImeData(type);
    }
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is abnormal", type);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    auto ret = clientInfo->client->OnInputReady(data->agent);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("start client input failed, ret: %{public}d", ret);
        return ret;
    }
    ret = data->core->StartInput(clientInfo->channel, clientInfo->isShowKeyboard);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("stop client input failed, ret: %{public}d", ret);
        return ErrorCode::ERROR_IME_START_INPUT_FAILED;
    }
    UpdateClientInfo(clientInfo->client->AsObject(),
        { { UpdateFlag::BINDIMETYPE, type }, { UpdateFlag::ISSHOWKEYBOARD, clientInfo->isShowKeyboard } });
    SetCurrentClient(clientInfo->client);
    return ErrorCode::NO_ERROR;
}

void PerUserSession::UnBindClientWithIme(const std::shared_ptr<InputClientInfo> &currentClientInfo, UnBindCause cause)
{
    if (currentClientInfo == nullptr) {
        IMSA_HILOGE("clientInfo is nullptr");
        return;
    }
    IMSA_HILOGI("imeType: %{public}d, UnBindCause: %{public}d", currentClientInfo->bindImeType, cause);
    if (cause != UnBindCause::CLIENT_DIED) {
        auto ret = currentClientInfo->client->OnInputStop(cause);
        IMSA_HILOGE("stop client input, ret: %{public}d", ret);
    }
    auto data = GetImeData(currentClientInfo->bindImeType);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is abnormal", currentClientInfo->bindImeType);
        return;
    }
    if (cause != UnBindCause::IME_DIED) {
        auto ret = data->core->StopInput(currentClientInfo->channel);
        IMSA_HILOGE("stop ime input, ret: %{public}d", ret);
    }
    /* CLIENT_DIED, CLIENT_UNFOCUSED, CLIENT_CLOSE_SELF:the client will be removed, has no point in updating
    IME_DIED: The update will causes the client can't be rebind after ime or proxyIme reboot */
    if (cause == UnBindCause::IME_SWITCH) {
        UpdateClientInfo(currentClientInfo->client->AsObject(), { { UpdateFlag::BINDIMETYPE, ImeType::NONE } });
    }
}

int32_t PerUserSession::OnSetCoreAndAgent(
    const sptr<IInputMethodCore> &core, const sptr<IInputMethodAgent> &agent, ImeType type)
{
    IMSA_HILOGD("PerUserSession::SetCoreAndAgent Start");
    auto ret = AddImeData(type, core, agent);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    if (type == ImeType::IMA) {
        ret = InitInputControlChannel();
        IMSA_HILOGI("init input control channel ret: %{public}d", ret);
        bool isStarted = true;
        isImeStarted_.SetValue(isStarted);
    }
    auto client = GetCurrentClient();
    auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
    if (clientInfo != nullptr) {
        HandleSpecialScene(clientInfo, type);
    }
    return ErrorCode::NO_ERROR;
}

int32_t PerUserSession::OnClearCoreAndAgent(int32_t type, const sptr<IInputMethodCore> &core)
{
    IMSA_HILOGD("proxy unregister type: %{public}d", type);
    // 0: stop proxy  1: switch to ima
    if (type == 0) {
        RemoveIme(core, ImeType::PROXY, UnBindCause::IME_CLEAR_SELF);
        return ErrorCode::NO_ERROR;
    }
    if (type == 1) {
        auto client = GetCurrentClient();
        auto clientInfo = client != nullptr ? GetClientInfo(client->AsObject()) : nullptr;
        if (clientInfo == nullptr) {
            IMSA_HILOGE("not find current client");
            return ErrorCode::ERROR_CLIENT_NOT_BOUND;
        }
        if (clientInfo->bindImeType == ImeType::PROXY) {
            UnBindClientWithIme(clientInfo, UnBindCause::IME_SWITCH);
        }
        InputClientInfo infoTemp = {
            .client = clientInfo->client, .channel = clientInfo->channel, .isShowKeyboard = true
        };
        return BindClientWithIme(std::make_shared<InputClientInfo>(infoTemp), ImeType::IMA);
    }
    return ErrorCode::ERROR_BAD_PARAMETERS;
}

int32_t PerUserSession::InitInputControlChannel()
{
    IMSA_HILOGD("PerUserSession::InitInputControlChannel");
    sptr<IInputControlChannel> inputControlChannel = new InputControlChannelStub(userId_);
    auto data = GetImeData(ImeType::IMA);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist", ImeType::IMA);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    return data->core->InitInputControlChannel(
        inputControlChannel, ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->imeId);
}

void PerUserSession::StopInputService(std::string imeId)
{
    IMSA_HILOGI("PerUserSession::StopInputService");
    auto data = GetImeData(ImeType::IMA);
    if (data == nullptr) {
        IMSA_HILOGE("ime: %{public}d is not exist", ImeType::IMA);
        return;
    }
    RemoveIme(data->core, ImeType::IMA, UnBindCause::IME_DIED);
    data->core->StopInputService(imeId);
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
        auto data = GetImeData(ImeType::IMA);
        if (data == nullptr) {
            IMSA_HILOGE("ime: %{public}d is not exist", ImeType::IMA);
            return ErrorCode::ERROR_IME_NOT_STARTED;
        }
        int32_t ret = data->core->SetSubtype(subProperty);
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

int32_t PerUserSession::AddImeData(ImeType type, sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent)
{
    if (core == nullptr || agent == nullptr) {
        IMSA_HILOGE("core or agent is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    sptr<InputDeathRecipient> deathRecipient = new (std::nothrow) InputDeathRecipient();
    if (deathRecipient == nullptr) {
        IMSA_HILOGE("failed to new deathRecipient");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    deathRecipient->SetDeathRecipient([this, core, type](const wptr<IRemoteObject> &) {
        type == ImeType::IMA ? this->OnImsDied(core) : this->OnProxyDied(core);
    });
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
    if (IsCurrentClient(pid, uid)) {
        IMSA_HILOGD("pid[%{public}d] same as current client", pid);
        return;
    }
    auto client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGD("no client in bound state");
        return;
    }
    IMSA_HILOGI("focus shifts to pid: %{public}d, start clear unfocused client info", pid);
    RemoveClient(client, UnBindCause::CLIENT_UNFOCUSED);
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_UNFOCUSED);
}

void PerUserSession::OnUnfocused(int32_t pid, int32_t uid)
{
    if (IsCurrentClient(pid, uid)) {
        IMSA_HILOGD("pid[%{public}d] same as current client", pid);
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx);
    for (const auto &mapClient : mapClients_) {
        if (mapClient.second->pid == pid) {
            IMSA_HILOGI("clear unfocused client info: %{public}d", pid);
            RemoveClient(mapClient.second->client, UnBindCause::CLIENT_UNFOCUSED);
            InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_UNFOCUSED);
            break;
        }
    }
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

bool PerUserSession::StartInputService(const std::string &imeName, bool isRetry)
{
    IMSA_HILOGI("start ime: %{public}s with isRetry: %{public}d", imeName.c_str(), isRetry);
    std::string::size_type pos = imeName.find('/');
    if (pos == std::string::npos) {
        IMSA_HILOGE("invalid ime name");
        return false;
    }
    IMSA_HILOGI("ime: %{public}s", imeName.c_str());
    AAFwk::Want want;
    want.SetElementName(imeName.substr(0, pos), imeName.substr(pos + 1));
    isImeStarted_.Clear(false);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want);
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
} // namespace MiscServices
} // namespace OHOS
