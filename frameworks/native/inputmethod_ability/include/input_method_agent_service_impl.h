/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef SERVICE_INCLUDE_INPUT_CONTROL_CHANNEL_SERVICE_IMPL_H
#define SERVICE_INCLUDE_INPUT_CONTROL_CHANNEL_SERVICE_IMPL_H

#include "iinput_method_agent.h"
#include "input_method_agent_stub.h"
#include "iremote_object.h"

namespace OHOS {
namespace MiscServices {
class InputMethodAgentServiceImpl final : public InputMethodAgentStub,
    public std::enable_shared_from_this<InputMethodAgentServiceImpl> {
    DISALLOW_COPY_AND_MOVE(InputMethodAgentServiceImpl);

public:
    InputMethodAgentServiceImpl();
    ~InputMethodAgentServiceImpl();
    ErrCode DispatchKeyEvent(
        const MiscServices::KeyEventValue &keyEvent, const sptr<IKeyEventConsumer> &consumer) override;
    ErrCode OnCursorUpdate(int32_t positionX, int32_t positionY, int height) override;
    ErrCode OnSelectionChange(
        const std::string& text, int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) override;
    ErrCode SetCallingWindow(uint32_t windowId) override;
    ErrCode OnAttributeChange(const InputAttributeInner &attributeInner) override;
    ErrCode SendPrivateCommand(const Value &value) override;
    ErrCode SendMessage(const ArrayBuffer &arraybuffer) override;
    ErrCode DiscardTypingText() override;
    ErrCode ResponseDataChannel(uint64_t msgId, int code, const ResponseDataInner &msg) override;
};
}  // namespace MiscServices
}  // namespace OHOS
#endif // SERVICE_INCLUDE_INPUT_CONTROL_CHANNEL_SERVICE_IMPL_H