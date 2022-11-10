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

#include <input_manager.h>
#include <key_event.h>

#include "global.h"

namespace OHOS {
namespace MiscServices {
class InputEventCallback : public MMI::IInputEventConsumer {
    virtual void OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const;
    virtual void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const;
    virtual void OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const;
};

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    IMSA_HILOGI("OnInputEvent");
    auto pressKeys = keyEvent->GetPressedKeys();
    for (const auto key : pressKeys) {
        IMSA_HILOGI("OnInputEvent key code: %{public}d", key);
    }
}

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
}

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
}
} // namespace MiscServices
} // namespace OHOS

#endif //INPUTMETHOD_IMF_INPUT_EVENT_CALLBACK_H
