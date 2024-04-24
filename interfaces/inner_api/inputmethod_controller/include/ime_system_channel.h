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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_IME_SYSTEM_CHANNEL_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_IME_SYSTEM_CHANNEL_H

#include "global.h"
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
class OnSystemCmdListener : public virtual RefBase {
public:
    virtual int32_t ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
    {
        return ErrorCode::NO_ERROR;
    }
    virtual void NotifyIsShowSysPanel(bool shouldSysPanelShow)
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

    int32_t ReceivePrivateCommand(
        const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;

    int32_t ShowSysPanel(bool shouldSysPanelShow);

    void OnConnectCmdReady(const sptr<IRemoteObject> &agentObject);

    int32_t RunConnectSystemCmd();

private:
    ImeSystemCmdChannel();
    ~ImeSystemCmdChannel();
    sptr<IInputMethodSystemAbility> GetSystemAbilityProxy();
    void OnRemoteSaDied(const wptr<IRemoteObject> &object);

    void SetSystemCmdListener(const sptr<OnSystemCmdListener> &listener);
    sptr<IInputMethodAgent> GetSystemCmdAgent();
    sptr<OnSystemCmdListener> GetSystemCmdListener();
    void ClearSystemCmdAgent();

    static std::mutex instanceLock_;
    static sptr<ImeSystemCmdChannel> instance_;

    std::mutex abilityLock_;
    sptr<IInputMethodSystemAbility> abilityManager_ = nullptr;
    sptr<InputDeathRecipient> deathRecipient_;
    
    std::mutex systemCmdListenerLock_;
    sptr<OnSystemCmdListener> systemCmdListener_ = nullptr;

    std::mutex systemAgentLock_;
    sptr<IInputMethodAgent> systemAgent_ = nullptr;
    std::atomic_bool isSystemCmdConnect_{ false };
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_IME_SYSTEM_CHANNEL_H