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

private:
    KeyHandle keyHandler_ = nullptr;
    static std::mutex statusMapLock_;
    static std::map<int32_t, bool> keyStatusMap_;
    std::map<std::vector<int32_t>, CombineKeyCode> combinedKeyMap_ = {
        { { MMI::KeyEvent::KEYCODE_CAPS_LOCK }, CombineKeyCode::COMBINE_KEYCODE_CAPS },
        { { MMI::KeyEvent::KEYCODE_SHIFT_LEFT }, CombineKeyCode::COMBINE_KEYCODE_SHIFT },
        { { MMI::KeyEvent::KEYCODE_SHIFT_RIGHT }, CombineKeyCode::COMBINE_KEYCODE_SHIFT },
        { { MMI::KeyEvent::KEYCODE_SHIFT_LEFT, MMI::KeyEvent::KEYCODE_CTRL_LEFT },
            CombineKeyCode::COMBINE_KEYCODE_CTRL_SHIFT },
        { { MMI::KeyEvent::KEYCODE_SHIFT_LEFT, MMI::KeyEvent::KEYCODE_CTRL_RIGHT },
            CombineKeyCode::COMBINE_KEYCODE_CTRL_SHIFT },
        { { MMI::KeyEvent::KEYCODE_SHIFT_RIGHT, MMI::KeyEvent::KEYCODE_CTRL_LEFT },
            CombineKeyCode::COMBINE_KEYCODE_CTRL_SHIFT },
        { { MMI::KeyEvent::KEYCODE_SHIFT_RIGHT, MMI::KeyEvent::KEYCODE_CTRL_RIGHT },
            CombineKeyCode::COMBINE_KEYCODE_CTRL_SHIFT },
    };
};

std::mutex InputEventCallback::statusMapLock_;
std::map<int32_t, bool> InputEventCallback::keyStatusMap_ = {
    { MMI::KeyEvent::KEYCODE_CAPS_LOCK, false },
    { MMI::KeyEvent::KEYCODE_CTRL_LEFT, false },
    { MMI::KeyEvent::KEYCODE_CTRL_RIGHT, false },
    { MMI::KeyEvent::KEYCODE_SHIFT_LEFT, false },
    { MMI::KeyEvent::KEYCODE_SHIFT_RIGHT, false },
};

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    auto keyCode = keyEvent->GetKeyCode();
    auto keyAction = keyEvent->GetKeyAction();
    IMSA_HILOGD("keyCode: %{public}d, keyAction: %{public}d", keyCode, keyAction);

    std::lock_guard<std::mutex> lock(statusMapLock_);
    if (keyStatusMap_.find(keyCode) == keyStatusMap_.end() || keyAction == MMI::KeyEvent::KEY_ACTION_UNKNOWN) {
        IMSA_HILOGD("keyevent undefined");
        return;
    }
    if (keyAction == MMI::KeyEvent::KEY_ACTION_DOWN) {
        IMSA_HILOGD("key %{public}d pressed down", keyCode);
        keyStatusMap_[keyCode] = true;
        return;
    }

    IMSA_HILOGI("key %{public}d pressed up", keyCode);
    std::vector<int32_t> pressedKeys;
    for (auto &key : keyStatusMap_) {
        if (key.second) {
            pressedKeys.push_back(key.first);
            key.second = false;
        }
    }
    auto combinedKey = combinedKeyMap_.find(pressedKeys);
    if (combinedKey == combinedKeyMap_.end()) {
        IMSA_HILOGD("undefined combinedkey");
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
    IMSA_HILOGD("set key handle");
    keyHandler_ = std::move(handle);
}
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_INPUT_EVENT_CALLBACK_H
