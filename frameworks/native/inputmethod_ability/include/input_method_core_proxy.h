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

#ifndef FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_CORE_PROXY_H
#define FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_CORE_PROXY_H

#include "i_input_method_core.h"
#include "input_attribute.h"
#include "input_method_property.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
class InputMethodCoreProxy : public IRemoteProxy<IInputMethodCore> {
public:
    explicit InputMethodCoreProxy(const sptr<IRemoteObject> &object);
    ~InputMethodCoreProxy();

    DISALLOW_COPY_AND_MOVE(InputMethodCoreProxy);

    int32_t StartInput(const InputClientInfo &clientInfo, bool isBindFromClient) override;
    int32_t StopInput(const sptr<IRemoteObject> &channel, uint32_t sessionId) override;
    int32_t ShowKeyboard() override;
    int32_t HideKeyboard() override;
    int32_t InitInputControlChannel(const sptr<IInputControlChannel> &inputControlChannel) override;
    int32_t StopInputService(bool isTerminateIme) override;
    int32_t SetSubtype(const SubProperty &property) override;
    bool IsEnable() override;
    int32_t IsPanelShown(const PanelInfo &panelInfo, bool &isShown) override;
    int32_t OnSecurityChange(int32_t security) override;
    int32_t OnConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent) override;
    void OnClientInactive(const sptr<IRemoteObject> &channel) override;
    int32_t OnSetInputType(InputType inputType) override;
    void OnCallingDisplayIdChange(uint64_t dispalyId) override;

private:
    static inline BrokerDelegator<InputMethodCoreProxy> delegator_;
    using ParcelHandler = std::function<bool(MessageParcel &)>;
    int32_t SendRequest(int code, ParcelHandler input = nullptr, ParcelHandler output = nullptr,
        MessageOption option = MessageOption::TF_SYNC);
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_CORE_PROXY_H
