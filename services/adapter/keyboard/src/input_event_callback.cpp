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

#include "input_event_callback.h"

#include "global.h"

namespace OHOS {
namespace MiscServices {
uint32_t InputEventCallback::keyState_ = static_cast<uint32_t>(0);

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    auto keyCode = keyEvent->GetKeyCode();
    auto keyAction = keyEvent->GetKeyAction();
    auto currKey = MASK_MAP.find(keyCode);
    if (currKey == MASK_MAP.end() || keyAction == MMI::KeyEvent::KEY_ACTION_UNKNOWN) {
        IMSA_HILOGD("key event unknown");
        return;
    }
    IMSA_HILOGD("keyCode: %{public}d, keyAction: %{public}d", keyCode, keyAction);
    if (keyAction == MMI::KeyEvent::KEY_ACTION_DOWN) {
        IMSA_HILOGD("key %{public}d pressed down", keyCode);
        keyState_ = static_cast<uint32_t>(keyState_ | currKey->second);
        return;
    }

    if (keyHandler_ == nullptr) {
        IMSA_HILOGE("keyHandler_ is nullptr");
        keyState_ = static_cast<uint32_t>(keyState_ & ~currKey->second);
        return;
    }
    int32_t ret = keyHandler_(keyState_);
    keyState_ = static_cast<uint32_t>(keyState_ & ~currKey->second);
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
} // namespace MiscServices
} // namespace OHOS