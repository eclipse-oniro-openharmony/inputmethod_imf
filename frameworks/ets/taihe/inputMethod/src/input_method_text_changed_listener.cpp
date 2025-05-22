/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "input_method_text_changed_listener.h"

#include "input_method_controller_impl.h"

namespace OHOS {
namespace MiscServices {
std::mutex InputMethodTextChangedListener::listenerMutex_;
sptr<InputMethodTextChangedListener> InputMethodTextChangedListener::inputMethodListener_{ nullptr };
sptr<InputMethodTextChangedListener> InputMethodTextChangedListener::GetInstance()
{
    if (inputMethodListener_ == nullptr) {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        if (inputMethodListener_ == nullptr) {
            inputMethodListener_ = new (std::nothrow) InputMethodTextChangedListener();
        }
    }
    return inputMethodListener_;
}

void InputMethodTextChangedListener::InsertText(const std::u16string &text)
{
    InputMethodControllerImpl::GetInstance()->InsertTextCallback(text);
}

void InputMethodTextChangedListener::DeleteForward(int32_t length)
{
    InputMethodControllerImpl::GetInstance()->DeleteRightCallback(length);
}

void InputMethodTextChangedListener::DeleteBackward(int32_t length)
{
    InputMethodControllerImpl::GetInstance()->DeleteLeftCallback(length);
}

void InputMethodTextChangedListener::SendKeyboardStatus(const KeyboardStatus &status)
{
    InputMethodControllerImpl::GetInstance()->SendKeyboardStatusCallback(status);
}

void InputMethodTextChangedListener::SendFunctionKey(const FunctionKey &functionKey)
{
    InputMethodControllerImpl::GetInstance()->SendFunctionKeyCallback(functionKey);
}

void InputMethodTextChangedListener::MoveCursor(const Direction direction)
{
    InputMethodControllerImpl::GetInstance()->MoveCursorCallback(direction);
}

void InputMethodTextChangedListener::HandleExtendAction(int32_t action)
{
    InputMethodControllerImpl::GetInstance()->HandleExtendActionCallback(action);
}

std::u16string InputMethodTextChangedListener::GetLeftTextOfCursor(int32_t number)
{
    return InputMethodControllerImpl::GetInstance()->GetLeftTextOfCursorCallback(number);
}

std::u16string InputMethodTextChangedListener::GetRightTextOfCursor(int32_t number)
{
    return InputMethodControllerImpl::GetInstance()->GetRightTextOfCursorCallback(number);
}

int32_t InputMethodTextChangedListener::GetTextIndexAtCursor()
{
    return InputMethodControllerImpl::GetInstance()->GetTextIndexAtCursorCallback();
}
} // namespace MiscServices
} // namespace OHOS