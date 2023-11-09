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

#ifndef INPUTMETHOD_IMF_KEYBOARD_LISTENER_TEST_IMPL_H
#define INPUTMETHOD_IMF_KEYBOARD_LISTENER_TEST_IMPL_H

#include <condition_variable>
#include <mutex>

#include "keyboard_listener.h"

namespace OHOS {
namespace MiscServices {
class KeyboardListenerTestImpl : public KeyboardListener {
public:
    KeyboardListenerTestImpl(){};
    ~KeyboardListenerTestImpl(){};
    bool OnKeyEvent(int32_t keyCode, int32_t keyStatus) override;
    bool OnKeyEvent(const std::shared_ptr<MMI::KeyEvent> &keyEvent) override
    {
        return false;
    }
    void OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height) override;
    void OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd) override;
    void OnTextChange(const std::string &text) override;
    void OnEditorAttributeChange(const InputAttribute &inputAttribute) override;
    static void ResetParam();
    static bool WaitKeyEvent(int32_t keyCode);
    static bool WaitCursorUpdate();
    static bool WaitSelectionChange(int32_t newBegin);
    static bool WaitTextChange(const std::string &text);
    static bool WaitEditorAttributeChange(const InputAttribute &inputAttribute);

private:
    static std::mutex kdListenerLock_;
    static std::condition_variable kdListenerCv_;
    static int32_t keyCode_;
    static int32_t cursorHeight_;
    static int32_t newBegin_;
    static std::string text_;
    static InputAttribute inputAttribute_;
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUTMETHOD_IMF_KEYBOARD_LISTENER_TEST_IMPL_H
