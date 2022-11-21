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

#include <map>
#include <mutex>
#include <utility>

#include "global.h"
#include "keyboard_event.h"

namespace OHOS {
namespace MiscServices {
class InputEventCallback : public MMI::IInputEventConsumer {
public:
    virtual void OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const;
    virtual void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const;
    virtual void OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const;
    void SetKeyHandle(KeyHandle handle);
    void InitStatusMap();

private:
    KeyHandle keyHandler_ = nullptr;
    std::mutex statusMapLock_;
    static std::map<int32_t, bool> keyStatusMap_;
    std::map<std::vector<int32_t>, CombineKeyCode> combinedKeyMap_ = {
        { { MMI::KeyEvent::KEYCODE_CAPS_LOCK }, CombineKeyCode::COMBINE_KEYCODE_CAPS },
        { { MMI::KeyEvent::KEYCODE_SHIFT_LEFT }, CombineKeyCode::COMBINE_KEYCODE_SHIFT },
        { { MMI::KeyEvent::KEYCODE_SHIFT_RIGHT }, CombineKeyCode::COMBINE_KEYCODE_SHIFT },
        { { MMI::KeyEvent::KEYCODE_CTRL_LEFT, MMI::KeyEvent::KEYCODE_SHIFT_LEFT },
            CombineKeyCode::COMBINE_KEYCODE_CTRL_SHIFT },
        { { MMI::KeyEvent::KEYCODE_CTRL_LEFT, MMI::KeyEvent::KEYCODE_SHIFT_RIGHT },
            CombineKeyCode::COMBINE_KEYCODE_CTRL_SHIFT },
        { { MMI::KeyEvent::KEYCODE_CTRL_RIGHT, MMI::KeyEvent::KEYCODE_SHIFT_LEFT },
            CombineKeyCode::COMBINE_KEYCODE_CTRL_SHIFT },
        { { MMI::KeyEvent::KEYCODE_CTRL_RIGHT, MMI::KeyEvent::KEYCODE_SHIFT_RIGHT },
            CombineKeyCode::COMBINE_KEYCODE_CTRL_SHIFT },
    };
};

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    IMSA_HILOGI("OnInputEvent");
    std::lock_guard<std::mutex> lock(statusMapLock_);
    auto keyCode = keyEvent->GetKeyCode();
    auto keyAction = keyEvent->GetKeyAction();

    if (keyStatusMap_.find(keyCode) == keyStatusMap_.end() || keyAction == MMI::KeyEvent::KEY_ACTION_UNKNOWN) {
        IMSA_HILOGD("keycode undefined");
        return;
    }
    if (keyAction == MMI::KeyEvent::KEY_ACTION_DOWN) {
        IMSA_HILOGD("key %{public}d pressed down", keyCode);
        keyStatusMap_[keyCode] = true;
        return;
    }

    // keyUp event occurs
    std::vector<int32_t> pressedKeys;
    for (auto &key : keyStatusMap_) {
        if (key.second) {
            pressedKeys.push_back(key.first);
        }
    }
    auto combinedKey = combinedKeyMap_.find(pressedKeys);
    if (combinedKey == combinedKeyMap_.end()) {
        IMSA_HILOGD("undefined combinedkey");
        keyStatusMap_[keyCode] = false;
        return;
    }
    if (keyHandler_ == nullptr) {
        IMSA_HILOGE("keyHandler_ is nullptr");
        return;
    }
    int32_t ret = keyHandler_(combinedKey->second);
    IMSA_HILOGI("handle keyevent ret: %{public}d", ret);
}

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
}

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
}

void InputEventCallback::SetKeyHandle(KeyHandle handle)
{
    IMSA_HILOGI("set key handle");
    keyHandler_ = std::move(handle);
}

void InputEventCallback::InitStatusMap()
{
    std::lock_guard<std::mutex> lock(statusMapLock_);
    keyStatusMap_ = {
        { MMI::KeyEvent::KEYCODE_CAPS_LOCK, false },
        { MMI::KeyEvent::KEYCODE_SHIFT_LEFT, false },
        { MMI::KeyEvent::KEYCODE_SHIFT_RIGHT, false },
        { MMI::KeyEvent::KEYCODE_CTRL_LEFT, false },
        { MMI::KeyEvent::KEYCODE_CTRL_RIGHT, false },
    };
}
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_INPUT_EVENT_CALLBACK_H
