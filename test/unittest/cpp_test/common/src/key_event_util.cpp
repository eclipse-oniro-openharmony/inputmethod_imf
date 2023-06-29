/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "key_event_util.h"

#include <algorithm>

#include "global.h"
#include "input_manager.h"
#include "input_method_controller.h"
#include "input_method_property.h"

namespace OHOS {
namespace MiscServices {
constexpr int32_t SEC_TO_NANOSEC = 1000000000;
constexpr int32_t NANOSECOND_TO_MILLISECOND = 1000000;
constexpr int32_t DEFAULT_DEVICE_ID = -1;
constexpr int32_t DEFAULT_UNICODE = 0x0000;
bool KeyEventUtil::SimulateKeyEvent(int32_t keyCode)
{
    auto keyDown = CreateKeyEvent(keyCode, MMI::KeyEvent::KEY_ACTION_DOWN);
    auto keyUp = CreateKeyEvent(keyCode, MMI::KeyEvent::KEY_ACTION_UP);
    if (keyDown == nullptr || keyUp == nullptr) {
        IMSA_HILOGE("failed to create key event: %{public}d", keyCode);
        return false;
    }
    MMI::InputManager::GetInstance()->SimulateInputEvent(keyDown);
    MMI::InputManager::GetInstance()->SimulateInputEvent(keyUp);
    return true;
}

bool KeyEventUtil::SimulateKeyEvents(const std::vector<int32_t> &keys)
{
    if (keys.empty()) {
        IMSA_HILOGE("keys is empty");
        return false;
    }
    std::vector<std::shared_ptr<MMI::KeyEvent>> downKeys_;
    std::vector<std::shared_ptr<MMI::KeyEvent>> upKeys_;
    for (auto &key : keys) {
        auto keyDown = CreateKeyEvent(key, MMI::KeyEvent::KEY_ACTION_DOWN);
        auto keyUp = CreateKeyEvent(key, MMI::KeyEvent::KEY_ACTION_UP);
        if (keyDown == nullptr || keyUp == nullptr) {
            IMSA_HILOGE("failed to create key event: %{public}d", key);
            return false;
        }
        downKeys_.push_back(keyDown);
        upKeys_.push_back(keyUp);
    }
    // first pressed last lift.
    std::reverse(upKeys_.begin(), upKeys_.end());
    for (auto &downKey : downKeys_) {
        MMI::InputManager::GetInstance()->SimulateInputEvent(downKey);
    }
    for (auto &upkey : upKeys_) {
        MMI::InputManager::GetInstance()->SimulateInputEvent(upkey);
    }
    return true;
}

std::shared_ptr<MMI::KeyEvent> KeyEventUtil::CreateKeyEvent(int32_t keyCode, int32_t keyAction)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    int64_t downTime = GetNanoTime() / NANOSECOND_TO_MILLISECOND;
    MMI::KeyEvent::KeyItem keyItem;
    keyItem.SetKeyCode(keyCode);
    keyItem.SetPressed(keyAction == MMI::KeyEvent::KEY_ACTION_DOWN);
    keyItem.SetDownTime(downTime);
    keyItem.SetDeviceId(DEFAULT_DEVICE_ID);
    keyItem.SetUnicode(DEFAULT_UNICODE);
    if (keyEvent != nullptr) {
        keyEvent->SetKeyCode(keyCode);
        keyEvent->SetKeyAction(keyAction);
        keyEvent->AddPressedKeyItems(keyItem);
    }
    return keyEvent;
}

int64_t KeyEventUtil::GetNanoTime()
{
    struct timespec time = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &time);
    return static_cast<int64_t>(time.tv_sec) * SEC_TO_NANOSEC + time.tv_nsec;
}
} // namespace MiscServices
} // namespace OHOS
