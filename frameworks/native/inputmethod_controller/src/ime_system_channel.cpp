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
constexpr const char *SMART_MENU_METADATA_NAME = "ohos.extension.smart_menu";
std::mutex ImeSystemCmdChannel::instanceLock_;
sptr<ImeSystemCmdChannel> ImeSystemCmdChannel::instance_;
ImeSystemCmdChannel::ImeSystemCmdChannel()
{
}

ImeSystemCmdChannel::~ImeSystemCmdChannel()
{
}

sptr<ImeSystemCmdChannel> ImeSystemCmdChannel::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            IMSA_HILOGD("System IMC instance_ is nullptr");
            instance_ = new (std::nothrow) ImeSystemCmdChannel();
            if (instance_ == nullptr) {
                IMSA_HILOGE("failed to create ImeSystemCmdChannel");
                return instance_;
            }
        }
    }
    return instance_;
}

sptr<IInputMethodSystemAbility> ImeSystemCmdChannel::GetSystemAbilityProxy()
{
    std::lock_guard<std::mutex> lock(abilityLock_);
    if (systemAbility_ != nullptr) {
        return systemAbility_;
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
    systemAbility_ = iface_cast<IInputMethodSystemAbility>(systemAbility);
    return systemAbility_;
}

void ImeSystemCmdChannel::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
{
    IMSA_HILOGI("input method service death");
    {
        std::lock_guard<std::mutex> lock(abilityLock_);
        systemAbility_ = nullptr;
    }
    ClearSystemCmdAgent();
}

int32_t ImeSystemCmdChannel::ConnectSystemCmd(const sptr<OnSystemCmdListener> &listener)
{
    IMSA_HILOGD("in");
    SetSystemCmdListener(listener);
    if (isSystemCmdConnect_.load()) {
        IMSA_HILOGD("in connected state");
        return ErrorCode::NO_ERROR;
    }
    return RunConnectSystemCmd();
}

int32_t ImeSystemCmdChannel::RunConnectSystemCmd()
{
    if (systemChannelStub_ == nullptr) {
        std::lock_guard<decltype(systemChannelMutex_)> lock(systemChannelMutex_);
        if (systemChannelStub_ == nullptr) {
            systemChannelStub_ = new (std::nothrow) SystemCmdChannelStub();
        }
        if (systemChannelStub_ == nullptr) {
            IMSA_HILOGE("channel is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
    }

    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    sptr<IRemoteObject> agent = nullptr;
    static constexpr uint32_t RETRY_INTERVAL = 100;
    static constexpr uint32_t BLOCK_RETRY_TIMES = 5;
    if (!BlockRetry(RETRY_INTERVAL, BLOCK_RETRY_TIMES, [&agent, this, proxy]() -> bool {
            int32_t ret = proxy->ConnectSystemCmd(systemChannelStub_->AsObject(), agent);
            return ret == ErrorCode::NO_ERROR;
        })) {
        IMSA_HILOGE("failed to connect system cmd");
        return ErrorCode::ERROR_SYSTEM_CMD_CHANNEL_ERROR;
    }
    OnConnectCmdReady(agent);
    IMSA_HILOGI("connect system cmd success");
    return ErrorCode::NO_ERROR;
}

void ImeSystemCmdChannel::OnConnectCmdReady(const sptr<IRemoteObject> &agentObject)
{
    if (agentObject == nullptr) {
        IMSA_HILOGE("agentObject is nullptr");
        return;
    }
    isSystemCmdConnect_.store(true);
    std::lock_guard<std::mutex> autoLock(systemAgentLock_);
    if (systemAgent_ != nullptr) {
        IMSA_HILOGD("agent has already been set");
        return;
    }
    systemAgent_ = new (std::nothrow) InputMethodAgentProxy(agentObject);
    if (agentDeathRecipient_ == nullptr) {
        agentDeathRecipient_ = new (std::nothrow) InputDeathRecipient();
        if (agentDeathRecipient_ == nullptr) {
            IMSA_HILOGE("new death recipient failed");
            return;
        }
    }
    agentDeathRecipient_->SetDeathRecipient(
        [this](const wptr<IRemoteObject> &remote) { OnSystemCmdAgentDied(remote); });
    if (!agentObject->AddDeathRecipient(agentDeathRecipient_)) {
        IMSA_HILOGE("failed to add death recipient.");
        return;
    }
}

void ImeSystemCmdChannel::OnSystemCmdAgentDied(const wptr<IRemoteObject> &remote)
{
    IMSA_HILOGI("input method death");
    ClearSystemCmdAgent();
    RunConnectSystemCmd();
}

sptr<IInputMethodAgent> ImeSystemCmdChannel::GetSystemCmdAgent()
{
    IMSA_HILOGD("GetSystemCmdAgent");
    std::lock_guard<std::mutex> autoLock(systemAgentLock_);
    return systemAgent_;
}

void ImeSystemCmdChannel::SetSystemCmdListener(const sptr<OnSystemCmdListener> &listener)
{
    std::lock_guard<std::mutex> lock(systemCmdListenerLock_);
    systemCmdListener_ = std::move(listener);
}

sptr<OnSystemCmdListener> ImeSystemCmdChannel::GetSystemCmdListener()
{
    std::lock_guard<std::mutex> lock(systemCmdListenerLock_);
    return systemCmdListener_;
}

void ImeSystemCmdChannel::ClearSystemCmdAgent()
{
    {
        std::lock_guard<std::mutex> autoLock(systemAgentLock_);
        systemAgent_ = nullptr;
    }
    isSystemCmdConnect_.store(false);
}

int32_t ImeSystemCmdChannel::ReceivePrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    auto cmdlistener = GetSystemCmdListener();
    if (cmdlistener == nullptr) {
        IMSA_HILOGE("cmdlistener is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    cmdlistener->ReceivePrivateCommand(privateCommand);
    return ErrorCode::NO_ERROR;
}

int32_t ImeSystemCmdChannel::SendPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    IMSA_HILOGD("in");
    if (TextConfig::IsSystemPrivateCommand(privateCommand)) {
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
    return ErrorCode::ERROR_INVALID_PRIVATE_COMMAND;
}

int32_t ImeSystemCmdChannel::ShowSysPanel(bool shouldSysPanelShow)
{
    auto listener = GetSystemCmdListener();
    if (listener == nullptr) {
        IMSA_HILOGE("listener is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    listener->NotifyIsShowSysPanel(shouldSysPanelShow);
    return ErrorCode::NO_ERROR;
}

std::string ImeSystemCmdChannel::GetSmartMenuCfg()
{
    std::shared_ptr<Property> defaultIme = nullptr;
    int32_t ret = GetDefaultImeCfg(defaultIme);
    if (ret != ErrorCode::NO_ERROR || defaultIme == nullptr) {
        IMSA_HILOGE("GetDefaultInputMethod failed");
        return "";
    }
    BundleMgrClient client;
    BundleInfo bundleInfo;
    if (!client.GetBundleInfo(defaultIme->name, BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO, bundleInfo)) {
        IMSA_HILOGE("GetBundleInfo failed");
        return "";
    }
    ExtensionAbilityInfo extInfo;
    GetExtensionInfo(bundleInfo.extensionInfos, extInfo);
    std::vector<std::string> profiles;
    if (!client.GetResConfigFile(extInfo, SMART_MENU_METADATA_NAME, profiles) || profiles.empty()) {
        IMSA_HILOGE("GetResConfigFile failed");
        return "";
    }
    return profiles[0];
}

void ImeSystemCmdChannel::GetExtensionInfo(std::vector<ExtensionAbilityInfo> extensionInfos,
    ExtensionAbilityInfo &extInfo)
{
    for (size_t i = 0; i < extensionInfos.size(); i++) {
        auto metadata = extensionInfos[i].metadata;
        for (size_t j = 0; j < metadata.size(); j++) {
            if (metadata[j].name == SMART_MENU_METADATA_NAME) {
                extInfo = extensionInfos[i];
                return;
            }
        }
    }
}

int32_t ImeSystemCmdChannel::GetDefaultImeCfg(std::shared_ptr<Property> &property)
{
    IMSA_HILOGD("InputMethodAbility::GetDefaultImeCfg");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("imsa proxy is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return proxy->GetDefaultInputMethod(property, true);
}
} // namespace MiscServices
} // namespace OHOS