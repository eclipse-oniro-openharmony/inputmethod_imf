/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "ime_system_channel.h"

#include "global.h"
#include "input_method_agent_proxy.h"
#include "input_method_controller.h"
#include "input_method_system_ability_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "system_cmd_channel_stub.h"

namespace OHOS {
namespace MiscServices {
std::mutex ImeSystemChannel::instanceLock_;
sptr<ImeSystemChannel> ImeSystemChannel::instance_;
ImeSystemChannel::ImeSystemChannel()
{
    IMSA_HILOGD("IMC structure");
}

ImeSystemChannel::~ImeSystemChannel()
{
}
sptr<ImeSystemChannel> ImeSystemChannel::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            IMSA_HILOGD("System IMC instance_ is nullptr");
            instance_ = new (std::nothrow) ImeSystemChannel();
            if (instance_ == nullptr) {
                IMSA_HILOGE("failed to create ImeSystemChannel");
                return instance_;
            }
        }
    }
    return instance_;
}

sptr<IInputMethodSystemAbility> ImeSystemChannel::GetSystemAbilityProxy()
{
    std::lock_guard<std::mutex> lock(abilityLock_);
    if (abilityManager_ != nullptr) {
        return abilityManager_;
    }
    IMSA_HILOGI("get input method service proxy");
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGE("system ability manager is nullptr");
        return nullptr;
    }
    auto systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
    if (systemAbility == nullptr) {
        IMSA_HILOGE("system ability is nullptr");
        return nullptr;
    }
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new (std::nothrow) InputDeathRecipient();
        if (deathRecipient_ == nullptr) {
            IMSA_HILOGE("new death recipient failed");
            return nullptr;
        }
    }
    deathRecipient_->SetDeathRecipient([this](const wptr<IRemoteObject> &remote) { OnRemoteSaDied(remote); });
    if ((systemAbility->IsProxyObject()) && (!systemAbility->AddDeathRecipient(deathRecipient_))) {
        IMSA_HILOGE("failed to add death recipient.");
        return nullptr;
    }
    abilityManager_ = iface_cast<IInputMethodSystemAbility>(systemAbility);
    return abilityManager_;
}

void ImeSystemChannel::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
{
    IMSA_HILOGI("input method service death");
    {
        std::lock_guard<std::mutex> lock(abilityLock_);
        abilityManager_ = nullptr;
    }
    ClearSystemCmdAgent();
}

int32_t ImeSystemChannel::ConnectSystemCmd(const sptr<OnSystemCmdListener> &listener)
{
    IMSA_HILOGD("in");
    SetSystemCmdListener(listener);
    if (isSystemCmdConnect_.load()) {
        IMSA_HILOGD("in connected state");
        return ErrorCode::NO_ERROR;
    }
    auto channel = new (std::nothrow) SystemCmdChannelStub();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    sptr<IRemoteObject> agent = nullptr;
    int32_t ret =  proxy->ConnectSystemCmd(channel, agent);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to start input, ret:%{public}d", ret);
        return ret;
    }
    OnConnectCmdReady(agent);
    IMSA_HILOGD("connect imf successfully");
    return ErrorCode::NO_ERROR;
}

void ImeSystemChannel::OnConnectCmdReady(const sptr<IRemoteObject> &agentObject)
{
    if (agentObject == nullptr) {
        IMSA_HILOGE("agentObject is nullptr");
        return;
    }
    isSystemCmdConnect_.store(true);
    std::lock_guard<std::mutex> autoLock(systemAgentLock_);
    if (systemAgent_ != nullptr && systemAgentObject_.GetRefPtr() == agentObject.GetRefPtr()) {
        IMSA_HILOGD("agent has already been set");
        return;
    }
    systemAgent_ = std::make_shared<InputMethodAgentProxy>(agentObject);
    systemAgentObject_ = agentObject;
}

std::shared_ptr<IInputMethodAgent> ImeSystemChannel::GetSystemCmdAgent()
{
    IMSA_HILOGD("GetSystemCmdAgent");
    std::lock_guard<std::mutex> autoLock(systemAgentLock_);
    return systemAgent_;
}

void ImeSystemChannel::SetSystemCmdListener(const sptr<OnSystemCmdListener> &listener)
{
    std::lock_guard<std::mutex> lock(systemCmdListenerLock_);
    systemCmdListener_ = listener;
}

sptr<OnSystemCmdListener> ImeSystemChannel::GetSystemCmdListener()
{
    std::lock_guard<std::mutex> lock(systemCmdListenerLock_);
    return systemCmdListener_;
}

void ImeSystemChannel::ClearSystemCmdAgent()
{
    {
        std::lock_guard<std::mutex> autoLock(systemAgentLock_);
        systemAgent_ = nullptr;
        systemAgentObject_ = nullptr;
    }
    isSystemCmdConnect_.store(false);
}

int32_t ImeSystemChannel::ReceivePrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    auto cmdlistener = GetSystemCmdListener();
    if (cmdlistener == nullptr) {
        IMSA_HILOGE("cmdlistener is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret = cmdlistener->ReceivePrivateCommand(privateCommand);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("ReceivePrivateCommand err, ret %{public}d", ret);
        return ErrorCode::ERROR_CMD_LISTENER_ERROR;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeSystemChannel::SendPrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    IMSA_HILOGD("in");
    if (isSystemCmdConnect_.load() && TextConfig::IsSystemPrivateCommand(privateCommand)) {
        if (!TextConfig::IsPrivateCommandValid(privateCommand)) {
            IMSA_HILOGE("invalid private command size.");
            return ErrorCode::ERROR_INVALID_PRIVATE_COMMAND_SIZE;
        }
        auto agent = GetSystemCmdAgent();
        if (agent == nullptr) {
            IMSA_HILOGE("agent is nullptr");
            return ErrorCode::ERROR_CLIENT_NOT_BOUND;
        }
        return agent->SendPrivateCommand(privateCommand);
    }
    return ErrorCode::ERROR_BAD_PARAMETERS;
}

int32_t ImeSystemChannel::NotifyIsShowSysPanel(bool isShow)
{
    auto listener = GetSystemCmdListener();
    if (listener == nullptr) {
        IMSA_HILOGE("listener is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    listener->OnNotifyIsShowSysPanel(isShow);
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS