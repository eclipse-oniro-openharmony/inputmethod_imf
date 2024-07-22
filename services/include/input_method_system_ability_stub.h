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

    int32_t GetInputMethodConfigOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t IsPanelShownOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t GetSecurityModeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t IsDefaultImeOnRemote(MessageParcel &data, MessageParcel &reply);

    int32_t ConnectSystemCmdOnRemote(MessageParcel &data, MessageParcel &reply);

    using RequestHandler = int32_t (InputMethodSystemAbilityStub::*)(MessageParcel &, MessageParcel &);
    const RequestHandler HANDLERS[static_cast<uint32_t>(InputMethodInterfaceCode::IMS_CMD_LAST)] = {
        &InputMethodSystemAbilityStub::InvalidRequest,
        &InputMethodSystemAbilityStub::StartInputOnRemote,
        &InputMethodSystemAbilityStub::ShowCurrentInputOnRemote,
        &InputMethodSystemAbilityStub::HideCurrentInputOnRemote,
        &InputMethodSystemAbilityStub::ShowInputOnRemote,
        &InputMethodSystemAbilityStub::HideInputOnRemote,
        &InputMethodSystemAbilityStub::StopInputSessionOnRemote,
        &InputMethodSystemAbilityStub::ReleaseInputOnRemote,
        &InputMethodSystemAbilityStub::RequestShowInputOnRemote,
        &InputMethodSystemAbilityStub::RequestHideInputOnRemote,
        &InputMethodSystemAbilityStub::GetCurrentInputMethodOnRemote,
        &InputMethodSystemAbilityStub::GetCurrentInputMethodSubtypeOnRemote,
        &InputMethodSystemAbilityStub::ListInputMethodOnRemote,
        &InputMethodSystemAbilityStub::ListInputMethodSubtypeOnRemote,
        &InputMethodSystemAbilityStub::ListCurrentInputMethodSubtypeOnRemote,
        &InputMethodSystemAbilityStub::SwitchInputMethodOnRemote,
        &InputMethodSystemAbilityStub::DisplayOptionalInputMethodOnRemote,
        &InputMethodSystemAbilityStub::SetCoreAndAgentOnRemote,
        &InputMethodSystemAbilityStub::ShowCurrentInputOnRemoteDeprecated,
        &InputMethodSystemAbilityStub::HideCurrentInputOnRemoteDeprecated,
        &InputMethodSystemAbilityStub::PanelStatusChangeOnRemote,
        &InputMethodSystemAbilityStub::UpdateListenEventFlagOnRemote,
        &InputMethodSystemAbilityStub::IsCurrentImeOnRemote,
        &InputMethodSystemAbilityStub::UnRegisteredProxyImeOnRemote,
        &InputMethodSystemAbilityStub::IsInputTypeSupportedOnRemote,
        &InputMethodSystemAbilityStub::StartInputTypeOnRemote,
        &InputMethodSystemAbilityStub::ExitCurrentInputTypeOnRemote,
        &InputMethodSystemAbilityStub::GetDefaultInputMethodOnRemote,
        &InputMethodSystemAbilityStub::GetInputMethodConfigOnRemote,
        &InputMethodSystemAbilityStub::IsPanelShownOnRemote,
        &InputMethodSystemAbilityStub::GetSecurityModeOnRemote,
        &InputMethodSystemAbilityStub::IsDefaultImeOnRemote,
        &InputMethodSystemAbilityStub::ConnectSystemCmdOnRemote,
    };
};
} // namespace OHOS::MiscServices

#endif // SERVICES_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_STUB_H
