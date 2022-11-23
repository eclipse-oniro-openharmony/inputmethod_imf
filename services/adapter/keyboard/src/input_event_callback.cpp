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

#include "../include/input_event_callback.h"

#include "global.h"

namespace OHOS {
namespace MiscServices {
constexpr uint32_t SHIFT_LEFT_POS = 0X1;
constexpr uint32_t SHIFT_RIGHT_POS = 0X1 << 1;
constexpr uint32_t CTRL_LEFT_POS = 0X1 << 2;
constexpr uint32_t CTRL_RIGHT_POS = 0X1 << 3;
constexpr uint32_t CAPS_POS = 0X1 << 4;

uint32_t InputEventCallback::keyState = 0;

std::map<int32_t, uint32_t> POS_MAP{
    { MMI::KeyEvent::KEYCODE_SHIFT_LEFT, SHIFT_LEFT_POS },
    { MMI::KeyEvent::KEYCODE_SHIFT_RIGHT, SHIFT_RIGHT_POS },
    { MMI::KeyEvent::KEYCODE_CTRL_LEFT, CTRL_LEFT_POS },
    { MMI::KeyEvent::KEYCODE_CTRL_RIGHT, CTRL_RIGHT_POS },
    { MMI::KeyEvent::KEYCODE_CAPS_LOCK, CAPS_POS },
};

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    auto keyCode = keyEvent->GetKeyCode();
    auto keyAction = keyEvent->GetKeyAction();
    if (POS_MAP.find(keyCode) == POS_MAP.end() || keyAction == MMI::KeyEvent::KEY_ACTION_UNKNOWN) {
        IMSA_HILOGD("key event unknown");
        return;
    }
    IMSA_HILOGD("keyCode: %{public}d, keyAction: %{public}d", keyCode, keyAction);
    if (keyAction == MMI::KeyEvent::KEY_ACTION_DOWN) {
        IMSA_HILOGD("key %{public}d pressed down", keyCode);
        keyState = keyState | POS_MAP[keyCode];
        return;
    }

    CombinationKey key = FindCombinationKey(keyState);
    keyState = keyState & ~POS_MAP[keyCode];
    if (key == CombinationKey::UNKNOWN) {
        IMSA_HILOGE("combination key unknown");
        return;
    }
    if (keyHandler_ == nullptr) {
        IMSA_HILOGE("keyHandler_ is nullptr");
        return;
    }
    int32_t ret = keyHandler_(key);
    IMSA_HILOGI("handle key event ret: %{public}d", ret);
}

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
}

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
}

void InputEventCallback::SetKeyHandle(KeyHandle handle)
{
    keyHandler_ = std::move(handle);
}

CombinationKey InputEventCallback::FindCombinationKey(uint32_t state)
{
    if (state == CAPS_POS) {
        return CombinationKey::CAPS;
    }
    if (state == SHIFT_LEFT_POS || state == SHIFT_RIGHT_POS) {
        return CombinationKey::SHIFT;
    }
    if ((state == (CTRL_LEFT_POS | SHIFT_LEFT_POS)) || (state == (CTRL_LEFT_POS | SHIFT_RIGHT_POS))
        || (state == (CTRL_RIGHT_POS | SHIFT_LEFT_POS)) || (state == (CTRL_RIGHT_POS | SHIFT_RIGHT_POS))) {
        return CombinationKey::CTRL_SHIFT;
    }
    return CombinationKey::UNKNOWN;
}
} // namespace MiscServices
} // namespace OHOS