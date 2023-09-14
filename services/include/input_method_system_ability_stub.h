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

#include <errors.h>

#include "global.h"
#include "i_input_method_system_ability.h"
#include "inputmethod_service_ipc_interface_code.h"
#include "iremote_stub.h"
#include "message_parcel.h"
#include "refbase.h"

namespace OHOS ::MiscServices {
class InputMethodSystemAbilityStub : public IRemoteStub<IInputMethodSystemAbility> {
public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t PrepareInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t StartInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ShowCurrentInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t HideCurrentInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t StopInputSessionOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t StopInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ReleaseInputOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t GetCurrentInputMethodOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t GetCurrentInputMethodSubtypeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ListInputMethodOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t SwitchInputMethodOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t DisplayOptionalInputMethodOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t SetCoreAndAgentOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ListInputMethodSubtypeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ListCurrentInputMethodSubtypeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t PanelStatusChangeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t UpdateListenEventFlagOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t IsCurrentImeOnRemote(MessageParcel &data, MessageParcel &reply);

    // Deprecated because of no permission check, kept for compatibility
    int32_t DisplayInputOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply);

    int32_t HideCurrentInputOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply);

    int32_t ShowCurrentInputOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply);

    using RequestHandler = int32_t (InputMethodSystemAbilityStub::*)(MessageParcel &, MessageParcel &);
    static constexpr RequestHandler HANDLERS[static_cast<uint32_t>(InputMethodInterfaceCode::IMS_CMD_LAST)] = {
        [static_cast<uint32_t>(InputMethodInterfaceCode::PREPARE_INPUT)] =
            &InputMethodSystemAbilityStub::PrepareInputOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::START_INPUT)] =
            &InputMethodSystemAbilityStub::StartInputOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::SHOW_CURRENT_INPUT)] =
            &InputMethodSystemAbilityStub::ShowCurrentInputOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::HIDE_CURRENT_INPUT)] =
            &InputMethodSystemAbilityStub::HideCurrentInputOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::STOP_INPUT)] =
            &InputMethodSystemAbilityStub::StopInputOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::RELEASE_INPUT)] =
            &InputMethodSystemAbilityStub::ReleaseInputOnRemote,
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
        [static_cast<uint32_t>(InputMethodInterfaceCode::DISPLAY_OPTIONAL_INPUT_DEPRECATED)] =
            &InputMethodSystemAbilityStub::DisplayInputOnRemoteDeprecated,
        [static_cast<uint32_t>(InputMethodInterfaceCode::STOP_INPUT_SESSION)] =
            &InputMethodSystemAbilityStub::StopInputSessionOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::PANEL_STATUS_CHANGE)] =
            &InputMethodSystemAbilityStub::PanelStatusChangeOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::UPDATE_LISTEN_EVENT_FLAG)] =
            &InputMethodSystemAbilityStub::UpdateListenEventFlagOnRemote,
        [static_cast<uint32_t>(InputMethodInterfaceCode::IS_CURRENT_IME)] =
            &InputMethodSystemAbilityStub::IsCurrentImeOnRemote,
    };
};
} // namespace OHOS::MiscServices

#endif // SERVICES_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_STUB_H
