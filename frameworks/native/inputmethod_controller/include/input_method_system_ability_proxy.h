/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_PROXY_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_PROXY_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "global.h"
#include "element_name.h"
#include "i_input_method_system_ability.h"
#include "input_attribute.h"
#include "input_client_stub.h"
#include "input_method_info.h"
#include "input_window_info.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "itypes_util.h"
#include "message_parcel.h"
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
namespace MiscServices {
class InputMethodSystemAbilityProxy : public IRemoteProxy<IInputMethodSystemAbility> {
public:
    explicit InputMethodSystemAbilityProxy(const sptr<IRemoteObject> &object);
    ~InputMethodSystemAbilityProxy() = default;
    DISALLOW_COPY_AND_MOVE(InputMethodSystemAbilityProxy);

    int32_t StartInput(InputClientInfo &inputClientInfo, sptr<IRemoteObject> &agent) override;
    int32_t ShowCurrentInput() override;
    int32_t HideCurrentInput() override;
    int32_t StopInputSession() override;
    int32_t ShowInput(sptr<IInputClient> client) override;
    int32_t HideInput(sptr<IInputClient> client) override;
    int32_t ReleaseInput(sptr<IInputClient> client) override;
    int32_t RequestShowInput() override;
    int32_t RequestHideInput() override;
    std::shared_ptr<Property> GetCurrentInputMethod() override;
    std::shared_ptr<SubProperty> GetCurrentInputMethodSubtype() override;
    int32_t GetDefaultInputMethod(std::shared_ptr<Property> &prop, bool isBrief) override;
    int32_t GetInputMethodConfig(OHOS::AppExecFwk::ElementName &inputMethodConfig) override;
    int32_t ListInputMethod(InputMethodStatus status, std::vector<Property> &props) override;
    int32_t SwitchInputMethod(const std::string &name, const std::string &subName, SwitchTrigger trigger) override;
    int32_t DisplayOptionalInputMethod() override;
    int32_t SetCoreAndAgent(const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent) override;
    int32_t InitConnect() override;
    int32_t UnRegisteredProxyIme(UnRegisteredType type, const sptr<IInputMethodCore> &core) override;
    int32_t ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProps) override;
    int32_t ListInputMethodSubtype(const std::string &name, std::vector<SubProperty> &subProps) override;
    int32_t PanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info) override;
    int32_t UpdateListenEventFlag(InputClientInfo &clientInfo, uint32_t eventFlag) override;
    bool IsCurrentIme() override;
    bool IsInputTypeSupported(InputType type) override;
    bool IsCurrentImeByPid(int32_t pid) override;
    int32_t StartInputType(InputType type) override;
    int32_t ExitCurrentInputType() override;
    int32_t IsPanelShown(const PanelInfo &panelInfo, bool &isShown) override;
    int32_t GetSecurityMode(int32_t &security) override;
    int32_t IsDefaultIme() override;
    int32_t ConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent) override;
    // Deprecated because of no permission check, kept for compatibility
    int32_t HideCurrentInputDeprecated() override;
    int32_t ShowCurrentInputDeprecated() override;
    bool IsDefaultImeSet() override;
    bool EnableIme(const std::string &bundleName) override;
    int32_t GetInputMethodState(EnabledStatus &status) override;

private:
    static inline BrokerDelegator<InputMethodSystemAbilityProxy> delegator_;
    using ParcelHandler = std::function<bool(MessageParcel &)>;
    int32_t SendRequest(int code, ParcelHandler input = nullptr, ParcelHandler output = nullptr);
    void GetMessageOption(int32_t code, MessageOption &option);
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_PROXY_H
