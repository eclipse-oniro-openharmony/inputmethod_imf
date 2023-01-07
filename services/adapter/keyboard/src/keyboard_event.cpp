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
    IMSA_HILOGI("KeyboardEvent::AddKeyEventMonitor");
    std::shared_ptr<InputEventCallback> callback = std::make_shared<InputEventCallback>();
    callback->SetKeyHandle(handle);
    int32_t monitorId =
        InputManager::GetInstance()->AddMonitor(std::static_pointer_cast<MMI::IInputEventConsumer>(callback));
    if (monitorId < 0) {
        IMSA_HILOGE("add monitor failed, id: %{public}d", monitorId);
        return ErrorCode::ERROR_SUBSCRIBE_KEYBOARD_EVENT;
    }
    IMSA_HILOGD("add monitor success, id: %{public}d", monitorId);
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS
