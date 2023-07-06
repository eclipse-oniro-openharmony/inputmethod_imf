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

#include <condition_variable>

#include "input_method_controller.h"
#include "input_method_utils.h"
#include "key_event.h"

namespace OHOS {
namespace MiscServices {
class TextListener : public OnTextChangedListener {
public:
    TextListener()
    {
        std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("TextListenerNotifier");
        serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    ~TextListener()
    {
    }
    void InsertText(const std::u16string &text) override
    {
        insertText_ = text;
        textListenerCv_.notify_one();
    }

    void DeleteForward(int32_t length) override
    {
        deleteForwardLength_ = length;
        textListenerCv_.notify_one();
        IMSA_HILOGI("TextChangeListener: DeleteForward, length is: %{public}d", length);
    }

    void DeleteBackward(int32_t length) override
    {
        deleteBackwardLength_ = length;
        textListenerCv_.notify_one();
        IMSA_HILOGI("TextChangeListener: DeleteBackward, direction is: %{public}d", length);
    }

    void SendKeyEventFromInputMethod(const KeyEvent &event) override
    {
    }

    void SendKeyboardStatus(const KeyboardStatus &keyboardStatus) override
    {
        IMSA_HILOGD("TextListener::SendKeyboardStatus %{public}d", static_cast<int>(keyboardStatus));
        constexpr int32_t interval = 20;
        {
            std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
            IMSA_HILOGD("TextListener::SendKeyboardStatus lock");
            keyboardStatus_ = keyboardStatus;
        }
        serviceHandler_->PostTask([this]() { textListenerCv_.notify_all(); }, interval);
        IMSA_HILOGD("TextListener::SendKeyboardStatus notify_all");
    }

    void SendFunctionKey(const FunctionKey &functionKey) override
    {
        EnterKeyType enterKeyType = functionKey.GetEnterKeyType();
        key_ = static_cast<int32_t>(enterKeyType);
        textListenerCv_.notify_one();
    }

    void SetKeyboardStatus(bool status) override
    {
        status_ = status;
    }

    void MoveCursor(const Direction direction) override
    {
        direction_ = static_cast<int32_t>(direction);
        textListenerCv_.notify_one();
        IMSA_HILOGI("TextChangeListener: MoveCursor, direction is: %{public}d", direction);
    }

    void HandleSetSelection(int32_t start, int32_t end) override
    {
        selectionStart_ = start;
        selectionEnd_ = end;
        textListenerCv_.notify_one();
        IMSA_HILOGI("TextChangeListener, selectionStart_: %{public}d, selectionEnd_: %{public}d", selectionStart_,
            selectionEnd_);
    }

    void HandleExtendAction(int32_t action) override
    {
        action_ = action;
        textListenerCv_.notify_one();
        IMSA_HILOGI("HandleExtendAction, action_: %{public}d", action_);
    }

    void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip) override
    {
        selectionDirection_ = keyCode;
        textListenerCv_.notify_one();
        IMSA_HILOGI("TextChangeListener, selectionDirection_: %{public}d", selectionDirection_);
    }

    static void ResetParam()
    {
        direction_ = -1;
        deleteForwardLength_ = -1;
        deleteBackwardLength_ = -1;
        insertText_ = u"";
        key_ = -1;
        status_ = false;
        selectionStart_ = -1;
        selectionEnd_ = -1;
        selectionDirection_ = -1;
        action_ = -1;
        keyboardStatus_ = KeyboardStatus::NONE;
    }
    static bool WaitIMACallback()
    {
        std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
        return TextListener::textListenerCv_.wait_for(lock, std::chrono::seconds(1)) != std::cv_status::timeout;
    }
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
    static int32_t action_;
    static KeyboardStatus keyboardStatus_;
    std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
};
std::mutex TextListener::textListenerCallbackLock_;
std::condition_variable TextListener::textListenerCv_;
int32_t TextListener::direction_ = -1;
int32_t TextListener::deleteForwardLength_ = -1;
int32_t TextListener::deleteBackwardLength_ = -1;
std::u16string TextListener::insertText_;
int32_t TextListener::key_ = -1;
bool TextListener::status_ = false;
int32_t TextListener::selectionStart_ = -1;
int32_t TextListener::selectionEnd_ = -1;
int32_t TextListener::selectionDirection_ = -1;
int32_t TextListener::action_ = -1;
KeyboardStatus TextListener::keyboardStatus_ = { KeyboardStatus::NONE };
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_TEST_TEXT_LISTENER_H
