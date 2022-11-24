/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_IMF_INPUT_EVENT_CALLBACK_H
#define INPUTMETHOD_IMF_INPUT_EVENT_CALLBACK_H

#include <map>
#include <mutex>
#include <utility>

#include "input_manager.h"
#include "key_event.h"
#include "keyboard_event.h"

namespace OHOS {
namespace MiscServices {
constexpr uint8_t SHIFT_LEFT_MASK = 0X1;
constexpr uint8_t SHIFT_RIGHT_MASK = 0X1 << 1;
constexpr uint8_t CTRL_LEFT_MASK = 0X1 << 2;
constexpr uint8_t CTRL_RIGHT_MASK = 0X1 << 3;
constexpr uint8_t CAPS_MASK = 0X1 << 4;

constexpr bool IS_KEYS_DOWN(uint32_t state, uint8_t mask)
{
    return (state & mask) == state;
}

class InputEventCallback : public MMI::IInputEventConsumer {
public:
    virtual void OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const;
    virtual void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const;
    virtual void OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const;
    void SetKeyHandle(KeyHandle handle);

private:
    KeyHandle keyHandler_ = nullptr;
    static uint32_t keyState_;
    static uint32_t FindCombinationKey(uint32_t state);
};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_INPUT_EVENT_CALLBACK_H
