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

    // Deprecated because of no permission check, kept for compatibility
    int32_t DisplayInputOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply);

    int32_t HideCurrentInputOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply);

    int32_t ShowCurrentInputOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply);

    int32_t SetCoreAndAgentOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply);

    bool CheckPermission(const std::string &permission);

    std::string GetBundleNameByTokenId(uint32_t tokenId);

    using RequestHandler = int32_t (InputMethodSystemAbilityStub::*)(MessageParcel &, MessageParcel &);
    static constexpr RequestHandler HANDLERS[INPUT_SERVICE_CMD_LAST] = {
        [PREPARE_INPUT] = &InputMethodSystemAbilityStub::PrepareInputOnRemote,
        [START_INPUT] = &InputMethodSystemAbilityStub::StartInputOnRemote,
        [SHOW_CURRENT_INPUT] = &InputMethodSystemAbilityStub::ShowCurrentInputOnRemote,
        [HIDE_CURRENT_INPUT] = &InputMethodSystemAbilityStub::HideCurrentInputOnRemote,
        [STOP_INPUT] = &InputMethodSystemAbilityStub::StopInputOnRemote,
        [RELEASE_INPUT] = &InputMethodSystemAbilityStub::ReleaseInputOnRemote,
        [GET_CURRENT_INPUT_METHOD] = &InputMethodSystemAbilityStub::GetCurrentInputMethodOnRemote,
        [GET_CURRENT_INPUT_METHOD_SUBTYPE] = &InputMethodSystemAbilityStub::GetCurrentInputMethodSubtypeOnRemote,
        [LIST_INPUT_METHOD] = &InputMethodSystemAbilityStub::ListInputMethodOnRemote,
        [LIST_INPUT_METHOD_SUBTYPE] = &InputMethodSystemAbilityStub::ListInputMethodSubtypeOnRemote,
        [LIST_CURRENT_INPUT_METHOD_SUBTYPE] = &InputMethodSystemAbilityStub::ListCurrentInputMethodSubtypeOnRemote,
        [SWITCH_INPUT_METHOD] = &InputMethodSystemAbilityStub::SwitchInputMethodOnRemote,
        [DISPLAY_OPTIONAL_INPUT_METHOD] = &InputMethodSystemAbilityStub::DisplayOptionalInputMethodOnRemote,
        [SET_CORE_AND_AGENT] = &InputMethodSystemAbilityStub::SetCoreAndAgentOnRemote,
        [SHOW_CURRENT_INPUT_DEPRECATED] = &InputMethodSystemAbilityStub::ShowCurrentInputOnRemoteDeprecated,
        [HIDE_CURRENT_INPUT_DEPRECATED] = &InputMethodSystemAbilityStub::HideCurrentInputOnRemoteDeprecated,
        [DISPLAY_OPTIONAL_INPUT_DEPRECATED] = &InputMethodSystemAbilityStub::DisplayInputOnRemoteDeprecated,
        [SET_CORE_AND_AGENT_DEPRECATED] = &InputMethodSystemAbilityStub::SetCoreAndAgentOnRemoteDeprecated,
        [STOP_INPUT_SESSION] = &InputMethodSystemAbilityStub::StopInputSessionOnRemote,
    };
};
} // namespace OHOS::MiscServices

#endif // SERVICES_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_STUB_H
