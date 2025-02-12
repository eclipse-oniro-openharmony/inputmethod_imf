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

#include "element_name.h"
#include "event_status_manager.h"
#include "global.h"
#include "i_input_client.h"
#include "i_input_method_core.h"
#include "i_system_cmd_channel.h"
#include "input_attribute.h"
#include "input_client_info.h"
#include "input_method_property.h"
#include "input_method_status.h"
#include "input_window_info.h"
#include "iremote_broker.h"
#include "message_parcel.h"
#include "panel_info.h"
#include "input_method_types.h"

namespace OHOS {
namespace MiscServices {
class IInputMethodSystemAbility : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.IInputMethodSystemAbility");

    virtual int32_t StartInput(InputClientInfo &inputClientInfo, sptr<IRemoteObject> &agent) = 0;
    virtual int32_t ShowCurrentInput() = 0;
    virtual int32_t HideCurrentInput() = 0;
    virtual int32_t StopInputSession() = 0;
    virtual int32_t ShowInput(sptr<IInputClient> client) = 0;
    virtual int32_t HideInput(sptr<IInputClient> client) = 0;
    virtual int32_t ReleaseInput(sptr<IInputClient> client) = 0;
    virtual int32_t RequestShowInput() = 0;
    virtual int32_t RequestHideInput() = 0;
    virtual int32_t GetDefaultInputMethod(std::shared_ptr<Property> &prop, bool isBrief) = 0;
    virtual int32_t GetInputMethodConfig(AppExecFwk::ElementName &inputMethodConfig) = 0;
    virtual std::shared_ptr<Property> GetCurrentInputMethod() = 0;
    virtual std::shared_ptr<SubProperty> GetCurrentInputMethodSubtype() = 0;
    virtual int32_t ListInputMethod(InputMethodStatus status, std::vector<Property> &props) = 0;
    virtual int32_t DisplayOptionalInputMethod() = 0;
    virtual int32_t SetCoreAndAgent(const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent) = 0;
    virtual int32_t InitConnect() = 0;
    virtual int32_t UnRegisteredProxyIme(UnRegisteredType type, const sptr<IInputMethodCore> &core) = 0;
    virtual int32_t ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProps) = 0;
    virtual int32_t ListInputMethodSubtype(const std::string &name, std::vector<SubProperty> &subProps) = 0;
    virtual int32_t SwitchInputMethod(
        const std::string &bundleName, const std::string &name, SwitchTrigger trigger) = 0;
    virtual int32_t PanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info) = 0;
    virtual int32_t UpdateListenEventFlag(InputClientInfo &clientInfo, uint32_t eventFlag) = 0;
    virtual bool IsCurrentIme() = 0;
    virtual bool IsInputTypeSupported(InputType type) = 0;
    virtual bool IsCurrentImeByPid(int32_t pid) = 0;
    virtual int32_t StartInputType(InputType type) = 0;
    virtual int32_t ExitCurrentInputType() = 0;
    virtual int32_t IsPanelShown(const PanelInfo &panelInfo, bool &isShown) = 0;
    virtual int32_t GetSecurityMode(int32_t &security) = 0;
    virtual int32_t IsDefaultIme() = 0;
    virtual bool IsDefaultImeSet() = 0;
    virtual bool EnableIme(const std::string &bundleName) = 0;
    virtual int32_t ConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent) = 0;
    virtual int32_t GetInputMethodState(EnabledStatus &status) = 0;

    // Deprecated because of no permission check, and keep for compatibility
    virtual int32_t HideCurrentInputDeprecated() = 0;
    virtual int32_t ShowCurrentInputDeprecated() = 0;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_I_INPUT_METHOD_SYSTEM_ABILITY_H
