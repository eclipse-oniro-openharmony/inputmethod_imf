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

#include "event_status_manager.h"
#include "global.h"
#include "i_input_client.h"
#include "i_input_data_channel.h"
#include "i_input_method_core.h"
#include "input_attribute.h"
#include "input_client_info.h"
#include "input_method_property.h"
#include "input_method_status.h"
#include "input_window_info.h"
#include "iremote_broker.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
class IInputMethodSystemAbility : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.IInputMethodSystemAbility");

    virtual int32_t PrepareInput(InputClientInfo &clientInfo) = 0;
    virtual int32_t StartInput(sptr<IInputClient> client, bool isShowKeyboard, bool attachFlag) = 0;
    virtual int32_t ShowCurrentInput() = 0;
    virtual int32_t HideCurrentInput() = 0;
    virtual int32_t StopInputSession() = 0;
    virtual int32_t StopInput(sptr<IInputClient> client) = 0;
    virtual int32_t ReleaseInput(sptr<IInputClient> client) = 0;
    virtual std::shared_ptr<Property> GetCurrentInputMethod() = 0;
    virtual std::shared_ptr<SubProperty> GetCurrentInputMethodSubtype() = 0;
    virtual int32_t ListInputMethod(InputMethodStatus status, std::vector<Property> &props) = 0;
    virtual int32_t DisplayOptionalInputMethod() = 0;
    virtual int32_t SetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent) = 0;
    virtual int32_t ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProps) = 0;
    virtual int32_t ListInputMethodSubtype(const std::string &name, std::vector<SubProperty> &subProps) = 0;
    virtual int32_t SwitchInputMethod(const std::string &bundleName, const std::string &name) = 0;
    virtual int32_t PanelStatusChange(const InputWindowStatus &status, const InputWindowInfo &windowInfo) = 0;
    virtual int32_t UpdateListenEventFlag(InputClientInfo &clientInfo, EventType eventType) = 0;

    // Deprecated because of no permission check, and keep for compatibility
    virtual int32_t HideCurrentInputDeprecated() = 0;
    virtual int32_t ShowCurrentInputDeprecated() = 0;
    virtual int32_t DisplayOptionalInputMethodDeprecated() = 0;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_I_INPUT_METHOD_SYSTEM_ABILITY_H
