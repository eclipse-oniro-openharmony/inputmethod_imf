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

#include "i_input_control_channel.h"
#include "i_input_data_channel.h"
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

    int32_t showKeyboard(
        const sptr<IInputDataChannel> &inputDataChannel, bool isShowKeyboard, const SubProperty &subProperty) override;
    bool hideKeyboard(int32_t flags) override;
    int32_t InitInputControlChannel(sptr<IInputControlChannel> &inputControlChannel, const std::string &imeId) override;
    void StopInputService(std::string imeId) override;
    int32_t SetSubtype(const SubProperty &property) override;

private:
    static inline BrokerDelegator<InputMethodCoreProxy> delegator_;
    using ParcelHandler = std::function<bool(MessageParcel &)>;
    int32_t SendRequest(int code, ParcelHandler input = nullptr, ParcelHandler output = nullptr);
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_CORE_PROXY_H
