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

#ifndef FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_AGENT_STUB_H
#define FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_AGENT_STUB_H

#include "i_input_method_agent.h"
#include "iremote_stub.h"
#include "message_handler.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
class InputMethodAgentStub : public IRemoteStub<IInputMethodAgent> {
public:
    explicit InputMethodAgentStub();
    virtual ~InputMethodAgentStub();
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    int32_t DispatchKeyEvent(
        const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<IKeyEventConsumer> &consumer) override;
    void OnCursorUpdate(int32_t positionX, int32_t positionY, int height) override;
    void OnSelectionChange(
        std::u16string text, int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) override;
    void SetCallingWindow(uint32_t windowId) override;
    void OnAttributeChange(const InputAttribute &attribute) override;
    void SetMessageHandler(MessageHandler *msgHandler);
    int32_t SendPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;

private:
    int32_t DispatchKeyEventOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t SendPrivateCommandOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t OnAttributeChangeOnRemote(MessageParcel &data, MessageParcel &reply);
    MessageHandler *msgHandler_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_AGENT_STUB_H
