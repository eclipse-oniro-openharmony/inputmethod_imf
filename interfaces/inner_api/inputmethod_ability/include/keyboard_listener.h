/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_IMF_KEYBOARD_LISTENER_H
#define INPUTMETHOD_IMF_KEYBOARD_LISTENER_H

#include "input_attribute.h"
#include "key_event.h"
#include "keyevent_consumer_proxy.h"
namespace OHOS {
namespace MiscServices {
class KeyboardListener {
public:
    virtual ~KeyboardListener() = default;
    virtual bool OnKeyEvent(int32_t keyCode, int32_t keyStatus, sptr<KeyEventConsumerProxy> consumer) = 0;
    virtual bool OnKeyEvent(const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<KeyEventConsumerProxy> consumer) = 0;
    virtual void OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height) = 0;
    virtual void OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) = 0;
    virtual void OnTextChange(const std::string &text) = 0;
    virtual void OnEditorAttributeChange(const InputAttribute &inputAttribute) = 0;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_IMF_KEYBOARD_LISTENER_H
