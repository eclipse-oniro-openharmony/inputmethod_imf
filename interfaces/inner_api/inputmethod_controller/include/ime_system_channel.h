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

#ifndef INPUTMETHOD_CONTROLLER_IME_SYSTEM_CHANNEL_H
#define INPUTMETHOD_CONTROLLER_IME_SYSTEM_CHANNEL_H

#include "bundle_mgr_client.h"
#include "i_input_method_agent.h"
#include "i_input_method_system_ability.h"
#include "input_method_utils.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "private_command_interface.h"
#include "refbase.h"
#include "visibility.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::AppExecFwk;
class OnSystemCmdListener : public virtual RefBase {
public:
    virtual void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
    {
    }
    virtual void NotifyPanelStatus(const SysPanelStatus &sysPanelStatus)
    {
    }
};
using PrivateDataValue = std::variant<std::string, bool, int32_t>;
class ImeSystemCmdChannel : public RefBase, public PrivateCommandInterface {
public:
    /**
     * @brief Get the instance of ImeSystemCmdChannel.
     *
     * This function is used to get the instance of ImeSystemCmdChannel.
     *
     * @return The instance of ImeSystemCmdChannel.
     * @since 12
     */
    IMF_API static sptr<ImeSystemCmdChannel> GetInstance();

    /**
     * @brief Connect system channel, set listener and bind IMSA.
     *
     * This function is used to connect the system app and  input method.
     *
     * @param listener Indicates the listener in order to receive private command.
     * @return Returns 0 for success, others for failure.
     * @since 12
     */
    IMF_API int32_t ConnectSystemCmd(const sptr<OnSystemCmdListener> &listener);

    /**
     * @brief Send private command to ime.
     *
     * This function is used to send private command to ime.
     *
     * @param privateCommand Indicates the private command which will be send.
     * @return Returns 0 for success, others for failure.
     * @since 12
     */
    IMF_API int32_t SendPrivateCommand(
        const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;

    /**
     * @brief Get smart menu config from default input method.
     *
     * This function is used to get smart menu config from default input method.
     *
     * @return string.
     * @since 12
     */
    IMF_API std::string GetSmartMenuCfg();
    int32_t ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;
    int32_t NotifyPanelStatus(const SysPanelStatus &sysPanelStatus);
    void OnConnectCmdReady(const sptr<IRemoteObject> &agentObject);
    IMF_API int32_t GetDefaultImeCfg(std::shared_ptr<Property> &property);

private:
    ImeSystemCmdChannel();
    ~ImeSystemCmdChannel();
    int32_t RunConnectSystemCmd();
    sptr<IInputMethodSystemAbility> GetSystemAbilityProxy();
    void OnRemoteSaDied(const wptr<IRemoteObject> &object);
    void SetSystemCmdListener(const sptr<OnSystemCmdListener> &listener);
    sptr<IInputMethodAgent> GetSystemCmdAgent();
    sptr<OnSystemCmdListener> GetSystemCmdListener();
    void ClearSystemCmdAgent();
    void GetExtensionInfo(std::vector<ExtensionAbilityInfo> extensionInfos, ExtensionAbilityInfo &extInfo);
    void OnSystemCmdAgentDied(const wptr<IRemoteObject> &remote);

    static std::mutex instanceLock_;
    static sptr<ImeSystemCmdChannel> instance_;

    std::mutex abilityLock_;
    sptr<IInputMethodSystemAbility> systemAbility_ = nullptr;
    sptr<InputDeathRecipient> deathRecipient_;

    std::mutex systemCmdListenerLock_;
    sptr<OnSystemCmdListener> systemCmdListener_ = nullptr;

    std::mutex systemAgentLock_;
    sptr<IInputMethodAgent> systemAgent_ = nullptr;
    sptr<InputDeathRecipient> agentDeathRecipient_;
    std::atomic_bool isSystemCmdConnect_{ false };

    std::mutex systemChannelMutex_;
    sptr<ISystemCmdChannel> systemChannelStub_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_CONTROLLER_IME_SYSTEM_CHANNEL_H