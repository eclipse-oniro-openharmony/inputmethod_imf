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
#include <key_event.h>

#include <memory>

#include "global.h"
#include "input_event_callback.h"

namespace OHOS {
namespace MiscServices {
using namespace MMI;

KeyboardEvent &KeyboardEvent::GetInstance()
{
    static KeyboardEvent keyboardEvent;
    return keyboardEvent;
}

int32_t KeyboardEvent::InitKeyEventMonitor()
{
    IMSA_HILOGI("KeyboardEvent::InitKeyMonitor");
    std::shared_ptr<InputEventCallback> callback = std::make_shared<InputEventCallback>();
    int32_t monitorId =
        InputManager::GetInstance()->AddMonitor(std::static_pointer_cast<MMI::IInputEventConsumer>(callback));
    if (monitorId < 0) {
        IMSA_HILOGE("Add monitor failed, id: %{public}d", monitorId);
        return ErrorCode::ERROR_STATUS_UNKNOWN_ERROR;
    }
    return ErrorCode::NO_ERROR;
}

int32_t KeyboardEvent::SubscribeKeyboardEvent(const CombineKey &combine, KeyHandle handle)
{
    std::shared_ptr<KeyOption> keyOption = std::make_shared<KeyOption>();
    keyOption->SetPreKeys(combine.preKeys);
    keyOption->SetFinalKey(combine.finalKey);
    keyOption->SetFinalKeyDown(true);
    keyOption->SetFinalKeyDownDuration(0);
    auto ret =
        InputManager::GetInstance()->SubscribeKeyEvent(keyOption, [this, handle](std::shared_ptr<KeyEvent> keyEvent) {
            auto pressKeys = keyEvent->GetPressedKeys();
            for (auto &key : pressKeys) {
                IMSA_HILOGI("key code: %{public}d", key);
            }
            if (handle != nullptr) {
                handle();
            }
        });
    IMSA_HILOGI("subscribe %{public}d key event %{public}s", combine.finalKey, ret >= 0 ? "OK" : "ERROR");
    return ret >= 0 ? ErrorCode::NO_ERROR : ErrorCode::ERROR_SUBSCRIBE_KEYBOARD_EVENT;
}
} // namespace MiscServices
} // namespace OHOS
