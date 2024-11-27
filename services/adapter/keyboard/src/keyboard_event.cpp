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

#include "keyboard_event.h"

#include <global.h>
#include <memory>

#include "global.h"
#include "input_event_callback.h"
#include "inputmethod_sysevent.h"
#include "key_event.h"

namespace OHOS {
namespace MiscServices {
using namespace MMI;
KeyboardEvent &KeyboardEvent::GetInstance()
{
    static KeyboardEvent keyboardEvent;
    return keyboardEvent;
}

int32_t KeyboardEvent::AddKeyEventMonitor(KeyHandle handle)
{
    IMSA_HILOGI("KeyboardEvent::AddKeyEventMonitor start.");
    std::shared_ptr<InputEventCallback> callback = std::make_shared<InputEventCallback>();
    callback->SetKeyHandle(handle);
    int32_t monitorId = InputManager::GetInstance()->AddMonitor([callback](std::shared_ptr<MMI::KeyEvent> keyEvent) {
        if (callback == nullptr) {
            IMSA_HILOGE("callback is nullptr!");
            return;
        }
        callback->OnInputEvent(keyEvent);
    });
    if (monitorId < 0) {
        IMSA_HILOGE("add monitor failed, id: %{public}d!", monitorId);
        return ErrorCode::ERROR_SUBSCRIBE_KEYBOARD_EVENT;
    }
    IMSA_HILOGD("add monitor success, id: %{public}d.", monitorId);

    CombinationKeyCallBack combinationKeyCallBack = [callback](std::shared_ptr<MMI::KeyEvent> keyEvent) {
        InputMethodSysEvent::GetInstance().ReportSystemShortCut("usual.event.WIN_SPACE");
        if (callback == nullptr) {
            IMSA_HILOGE("callback is nullptr!");
            return;
        }
        callback->TriggerSwitch();
    };
    SubscribeCombinationKey(
        MMI::KeyEvent::KEYCODE_META_LEFT, MMI::KeyEvent::KEYCODE_SPACE, combinationKeyCallBack, true);
    SubscribeCombinationKey(
        MMI::KeyEvent::KEYCODE_META_RIGHT, MMI::KeyEvent::KEYCODE_SPACE, combinationKeyCallBack, true);

    CombinationKeyCallBack ctrlShiftCallBack = [callback](std::shared_ptr<MMI::KeyEvent> keyEvent) {
        InputMethodSysEvent::GetInstance().ReportSystemShortCut("usual.event.CTRL_SHIFT");
        if (callback == nullptr) {
            IMSA_HILOGE("callback is nullptr!");
            return;
        }
        callback->TriggerSwitch();
    };

    SubscribeCombinationKey(MMI::KeyEvent::KEYCODE_CTRL_LEFT, MMI::KeyEvent::KEYCODE_SHIFT_LEFT, ctrlShiftCallBack);
    SubscribeCombinationKey(MMI::KeyEvent::KEYCODE_CTRL_LEFT, MMI::KeyEvent::KEYCODE_SHIFT_RIGHT, ctrlShiftCallBack);
    SubscribeCombinationKey(MMI::KeyEvent::KEYCODE_CTRL_RIGHT, MMI::KeyEvent::KEYCODE_SHIFT_LEFT, ctrlShiftCallBack);
    SubscribeCombinationKey(MMI::KeyEvent::KEYCODE_CTRL_RIGHT, MMI::KeyEvent::KEYCODE_SHIFT_RIGHT, ctrlShiftCallBack);
    SubscribeCombinationKey(MMI::KeyEvent::KEYCODE_SHIFT_LEFT, MMI::KeyEvent::KEYCODE_CTRL_LEFT, ctrlShiftCallBack);
    SubscribeCombinationKey(MMI::KeyEvent::KEYCODE_SHIFT_RIGHT, MMI::KeyEvent::KEYCODE_CTRL_LEFT, ctrlShiftCallBack);
    SubscribeCombinationKey(MMI::KeyEvent::KEYCODE_SHIFT_LEFT, MMI::KeyEvent::KEYCODE_CTRL_RIGHT, ctrlShiftCallBack);
    SubscribeCombinationKey(MMI::KeyEvent::KEYCODE_SHIFT_RIGHT, MMI::KeyEvent::KEYCODE_CTRL_RIGHT, ctrlShiftCallBack);
    return ErrorCode::NO_ERROR;
}

void KeyboardEvent::SubscribeCombinationKey(
    int32_t preKey, int32_t finalKey, CombinationKeyCallBack callback, bool setFinalKeyDown)
{
    std::shared_ptr<KeyOption> keyOption = std::make_shared<KeyOption>();
    std::set<int32_t> preKeys;
    preKeys.insert(preKey);
    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(finalKey);
    keyOption->SetFinalKeyDown(setFinalKeyDown);
    // 0 means press delay 0 ms
    keyOption->SetFinalKeyDownDuration(0);
    int32_t subscribeId = InputManager::GetInstance()->SubscribeKeyEvent(keyOption, callback);
    if (subscribeId < 0) {
        IMSA_HILOGE("failed to SubscribeKeyEvent, id: %{public}d preKey: %{public}d.", subscribeId, preKey);
    }
}
} // namespace MiscServices
} // namespace OHOS