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

#ifndef FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_CORE_STUB_H
#define FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_CORE_STUB_H

#include <condition_variable>
#include <cstdint>
#include <mutex>

#include "i_input_control_channel.h"
#include "i_input_data_channel.h"
#include "i_input_method_agent.h"
#include "i_input_method_core.h"
#include "input_attribute.h"
#include "input_channel.h"
#include "iremote_broker.h"
#include "iremote_stub.h"
#include "message_handler.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
class InputMethodCoreStub : public IRemoteStub<IInputMethodCore> {
public:
    DISALLOW_COPY_AND_MOVE(InputMethodCoreStub);
    explicit InputMethodCoreStub(int userId);
    virtual ~InputMethodCoreStub();
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    int32_t ShowKeyboard(
        const sptr<IInputDataChannel> &inputDataChannel, bool isShowKeyboard, const SubProperty &subProperty) override;
    bool HideKeyboard(int32_t flags) override;
    int32_t InitInputControlChannel(sptr<IInputControlChannel> &inputControlChannel, const std::string &imeId) override;
    void StopInputService(std::string imeId) override;
    int32_t SetSubtype(const SubProperty &property) override;
    int32_t ClearDataChannel(const sptr<IInputDataChannel> &channel) override;
    void SetMessageHandler(MessageHandler *msgHandler);

private:
    int userId_;
    MessageHandler *msgHandler_;
    void SetSubtypeOnRemote(MessageParcel &data, MessageParcel &reply);
    void ShowKeyboardOnRemote(MessageParcel &data, MessageParcel &reply);
    void ClearDataChannelOnRemote(MessageParcel &data, MessageParcel &reply);
    using ParcelHandler = std::function<bool(MessageParcel &)>;
    int32_t SendMessage(int code, ParcelHandler input = nullptr);
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_CORE_STUB_H
