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

#ifndef SERVICES_INCLUDE_I_INPUT_METHOD_SYSTEM_ABILITY_H
#define SERVICES_INCLUDE_I_INPUT_METHOD_SYSTEM_ABILITY_H

#include <errors.h>

#include <memory>
#include <vector>

#include "global.h"
#include "i_input_client.h"
#include "i_input_data_channel.h"
#include "i_input_method_core.h"
#include "input_attribute.h"
#include "input_method_property.h"
#include "input_method_status.h"
#include "iremote_broker.h"
#include "keyboard_type.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
class IInputMethodSystemAbility : public IRemoteBroker {
public:
    enum CommandId : int32_t {
        PREPARE_INPUT = 0,
        START_INPUT,
        SHOW_CURRENT_INPUT,
        HIDE_CURRENT_INPUT,
        STOP_INPUT,
        RELEASE_INPUT,
        GET_KEYBOARD_WINDOW_HEIGHT,
        GET_CURRENT_INPUT_METHOD,
        LIST_INPUT_METHOD,
        LIST_INPUT_METHOD_SUBTYPE,
        LIST_CURRENT_INPUT_METHOD_SUBTYPE,
        SWITCH_INPUT_METHOD,
        DISPLAY_OPTIONAL_INPUT_METHOD,
        SET_CORE_AND_AGENT,
        SHOW_CURRENT_INPUT_DEPRECATED,
        HIDE_CURRENT_INPUT_DEPRECATED,
        DISPLAY_OPTIONAL_INPUT_DEPRECATED,
        SET_CORE_AND_AGENT_DEPRECATED,
        INPUT_SERVICE_CMD_LAST
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.IInputMethodSystemAbility");

    virtual int32_t PrepareInput(
        int32_t displayId, sptr<IInputClient> client, sptr<IInputDataChannel> channel, InputAttribute &attribute) = 0;
    virtual int32_t StartInput(sptr<IInputClient> client, bool isShowKeyboard) = 0;
    virtual int32_t ShowCurrentInput() = 0;
    virtual int32_t HideCurrentInput() = 0;
    virtual int32_t StopInput(sptr<IInputClient> client) = 0;
    virtual int32_t ReleaseInput(sptr<IInputClient> client) = 0;
    virtual int32_t GetKeyboardWindowHeight(int32_t &retHeight) = 0;
    virtual std::shared_ptr<Property> GetCurrentInputMethod() = 0;
    virtual std::vector<Property> ListInputMethod(InputMethodStatus status) = 0;
    virtual int32_t DisplayOptionalInputMethod() = 0;
    virtual int32_t SetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent) = 0;
    virtual std::vector<SubProperty> ListCurrentInputMethodSubtype() = 0;
    virtual std::vector<SubProperty> ListInputMethodSubtype(const std::string &name) = 0;
    virtual int32_t SwitchInputMethod(const std::string &bundleName, const std::string &name) = 0;

    // Deprecated because of no permission check, and keep for compatibility
    virtual int32_t SetCoreAndAgentDeprecated(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent) = 0;
    virtual int32_t HideCurrentInputDeprecated() = 0;
    virtual int32_t ShowCurrentInputDeprecated() = 0;
    virtual int32_t DisplayOptionalInputMethodDeprecated() = 0;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_I_INPUT_METHOD_SYSTEM_ABILITY_H
