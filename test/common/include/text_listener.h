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

#ifndef INPUTMETHOD_TEST_TEXT_LISTENER_H
#define INPUTMETHOD_TEST_TEXT_LISTENER_H

#include <unistd.h>

#include <condition_variable>

#include "input_method_controller.h"
#include "input_method_utils.h"
#include "key_event.h"
#include "string_ex.h"

namespace OHOS {
namespace MiscServices {
class TextListener : public OnTextChangedListener {
public:
    TextListener();
    ~TextListener();
    void InsertText(const std::u16string &text) override;
    void DeleteForward(int32_t length) override;
    void DeleteBackward(int32_t length) override;
    void SendKeyEventFromInputMethod(const KeyEvent &event) override;
    void SendKeyboardStatus(const KeyboardStatus &keyboardStatus) override;
    void SendFunctionKey(const FunctionKey &functionKey) override;
    void SetKeyboardStatus(bool status) override;
    void MoveCursor(const Direction direction) override;
    void HandleSetSelection(int32_t start, int32_t end) override;
    void HandleExtendAction(int32_t action) override;
    void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip) override;
    void NotifyPanelStatusInfo(const PanelStatusInfo &info) override;
    void NotifyKeyboardHeight(uint32_t height) override;
    std::u16string GetLeftTextOfCursor(int32_t number) override;
    std::u16string GetRightTextOfCursor(int32_t number) override;
    int32_t GetTextIndexAtCursor() override;
    int32_t OnSendPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;
    static void ResetParam();
    static bool WaitSendKeyboardStatusCallback(const KeyboardStatus &keyboardStatus);
    static bool WaitNotifyPanelStatusInfoCallback(const PanelStatusInfo &info);
    static bool WaitNotifyKeyboardHeightCallback(uint32_t height);
    static bool WaitSendPrivateCommandCallback(std::unordered_map<std::string, PrivateDataValue> &privateCommand);
    static std::mutex textListenerCallbackLock_;
    static std::condition_variable textListenerCv_;
    static int32_t direction_;
    static int32_t deleteForwardLength_;
    static int32_t deleteBackwardLength_;
    static std::u16string insertText_;
    static int32_t key_;
    static bool status_;
    static int32_t selectionStart_;
    static int32_t selectionEnd_;
    static int32_t selectionDirection_;
    static int32_t selectionSkip_;
    static int32_t action_;
    static uint32_t height_;
    static KeyboardStatus keyboardStatus_;
    static PanelStatusInfo info_;
    std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    static constexpr int32_t TEXT_INDEX = 455;
    static constexpr const char *TEXT_BEFORE_CURSOR = "before";
    static constexpr const char *TEXT_AFTER_CURSOR = "after";
    static std::unordered_map<std::string, PrivateDataValue> privateCommand_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_TEST_TEXT_LISTENER_H
