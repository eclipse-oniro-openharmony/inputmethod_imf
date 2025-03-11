/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef SERVICES_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_STUB_H
#define SERVICES_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_STUB_H

#include "i_input_method_system_ability.h"
#include "inputmethod_service_ipc_interface_code.h"
#include "iremote_stub.h"

namespace OHOS ::MiscServices {
class InputMethodSystemAbilityStub : public IRemoteStub<IInputMethodSystemAbility> {
public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t InvalidRequest(MessageParcel &data, MessageParcel &reply)
    {
        return ERR_UNKNOWN_TRANSACTION;
    };

    int32_t StartInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ShowCurrentInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t HideCurrentInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t StopInputSessionOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ShowInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t HideInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ReleaseInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t RequestShowInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t RequestHideInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t GetCurrentInputMethodOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t GetCurrentInputMethodSubtypeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ListInputMethodOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t SwitchInputMethodOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t DisplayOptionalInputMethodOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t SetCoreAndAgentOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t UnRegisteredProxyImeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ListInputMethodSubtypeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ListCurrentInputMethodSubtypeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t PanelStatusChangeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t UpdateListenEventFlagOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t IsCurrentImeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t IsInputTypeSupportedOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t StartInputTypeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ExitCurrentInputTypeOnRemote(MessageParcel &data, MessageParcel &reply);

    // Deprecated because of no permission check, kept for compatibility
    int32_t HideCurrentInputOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply);

    int32_t ShowCurrentInputOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply);

    int32_t GetDefaultInputMethodOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t IsDefaultImeSetOnRemote(MessageParcel &data, MessageParcel &reply);
 
    int32_t EnableImeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t GetInputMethodConfigOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t IsPanelShownOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t GetSecurityModeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t IsDefaultImeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ConnectSystemCmdOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t IsCurrentImeByPidOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t InitConnectOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t SetCallingWindowOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t GetInputStartInfoOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t GetInputMethodStateOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t IsSystemAppOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t RegisterProxyOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t UnregisterProxyOnRemote(MessageParcel &data, MessageParcel &reply);

    using RequestHandler = int32_t (InputMethodSystemAbilityStub::*)(MessageParcel &, MessageParcel &);
    static inline constexpr RequestHandler HANDLERS[static_cast<uint32_t>(InputMethodInterfaceCode::IMS_CMD_END)] = {
        &InputMethodSystemAbilityStub::InvalidRequest,
        [static_cast<uint32_t>(InputMethodInterfaceCode::START_INPUT)] =
            &InputMethodSystemAbilityStub::StartInputOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::SHOW_CURRENT_INPUT)] =
            &InputMethodSystemAbilityStub::ShowCurrentInputOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::HIDE_CURRENT_INPUT)] =
            &InputMethodSystemAbilityStub::HideCurrentInputOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::SHOW_INPUT)] =
            &InputMethodSystemAbilityStub::ShowInputOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::HIDE_INPUT)] =
            &InputMethodSystemAbilityStub::HideInputOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::STOP_INPUT_SESSION)] =
            &InputMethodSystemAbilityStub::StopInputSessionOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::RELEASE_INPUT)] =
            &InputMethodSystemAbilityStub::ReleaseInputOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::REQUEST_SHOW_INPUT)] =
            &InputMethodSystemAbilityStub::RequestShowInputOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::REQUEST_HIDE_INPUT)] =
            &InputMethodSystemAbilityStub::RequestHideInputOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::GET_CURRENT_INPUT_METHOD)] =
            &InputMethodSystemAbilityStub::GetCurrentInputMethodOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::GET_CURRENT_INPUT_METHOD_SUBTYPE)] =
            &InputMethodSystemAbilityStub::GetCurrentInputMethodSubtypeOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::LIST_INPUT_METHOD)] =
            &InputMethodSystemAbilityStub::ListInputMethodOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::LIST_INPUT_METHOD_SUBTYPE)] =
            &InputMethodSystemAbilityStub::ListInputMethodSubtypeOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::LIST_CURRENT_INPUT_METHOD_SUBTYPE)] =
            &InputMethodSystemAbilityStub::ListCurrentInputMethodSubtypeOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::SWITCH_INPUT_METHOD)] =
            &InputMethodSystemAbilityStub::SwitchInputMethodOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::DISPLAY_OPTIONAL_INPUT_METHOD)] =
            &InputMethodSystemAbilityStub::DisplayOptionalInputMethodOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::SET_CORE_AND_AGENT)] =
            &InputMethodSystemAbilityStub::SetCoreAndAgentOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::SHOW_CURRENT_INPUT_DEPRECATED)] =
            &InputMethodSystemAbilityStub::ShowCurrentInputOnRemoteDeprecated,
        [static_cast<uint32_t>(InputMethodInterfaceCode::HIDE_CURRENT_INPUT_DEPRECATED)] =
            &InputMethodSystemAbilityStub::HideCurrentInputOnRemoteDeprecated,
        [static_cast<uint32_t>(InputMethodInterfaceCode::PANEL_STATUS_CHANGE)] =
            &InputMethodSystemAbilityStub::PanelStatusChangeOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::UPDATE_LISTEN_EVENT_FLAG)] =
            &InputMethodSystemAbilityStub::UpdateListenEventFlagOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::IS_CURRENT_IME)] =
            &InputMethodSystemAbilityStub::IsCurrentImeOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::UNREGISTERED_PROXY_IME)] =
            &InputMethodSystemAbilityStub::UnRegisteredProxyImeOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::IS_INPUT_TYPE_SUPPORTED)] =
            &InputMethodSystemAbilityStub::IsInputTypeSupportedOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::START_INPUT_TYPE)] =
            &InputMethodSystemAbilityStub::StartInputTypeOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::EXIT_CURRENT_INPUT_TYPE)] =
            &InputMethodSystemAbilityStub::ExitCurrentInputTypeOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::GET_DEFAULT_INPUT_METHOD)] =
            &InputMethodSystemAbilityStub::GetDefaultInputMethodOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::GET_INPUT_METHOD_SETTINGS)] =
            &InputMethodSystemAbilityStub::GetInputMethodConfigOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::IS_PANEL_SHOWN)] =
            &InputMethodSystemAbilityStub::IsPanelShownOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::GET_SECURITY_MODE)] =
            &InputMethodSystemAbilityStub::GetSecurityModeOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::IS_DEFAULT_IME)] =
            &InputMethodSystemAbilityStub::IsDefaultImeOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::CONNECT_SYSTEM_CMD)] =
            &InputMethodSystemAbilityStub::ConnectSystemCmdOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::IS_CURRENT_IME_BY_PID)] =
            &InputMethodSystemAbilityStub::IsCurrentImeByPidOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::INIT_CONNECT)] =
            &InputMethodSystemAbilityStub::InitConnectOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::IS_DEFAULT_IME_SET)] =
            &InputMethodSystemAbilityStub::IsDefaultImeSetOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::ENABLE_IME)] =
            &InputMethodSystemAbilityStub::EnableImeOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::SET_CALLING_WINDOW)] =
            &InputMethodSystemAbilityStub::SetCallingWindowOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::GET_INPUT_START_INFO)] =
            &InputMethodSystemAbilityStub::GetInputStartInfoOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::GET_IME_STATE)] =
            &InputMethodSystemAbilityStub::GetInputMethodStateOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::IS_SYSTEM_APP)] =
            &InputMethodSystemAbilityStub::IsSystemAppOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::REGISTER_PROXY_IME)] =
            &InputMethodSystemAbilityStub::RegisterProxyOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::UNREGISTER_PROXY_IME)] =
            &InputMethodSystemAbilityStub::UnregisterProxyOnRemote,
    };
};
} // namespace OHOS::MiscServices

#endif // SERVICES_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_STUB_H
