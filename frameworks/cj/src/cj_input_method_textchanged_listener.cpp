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

#include "cj_input_method_textchanged_listener.h"
#include "textchange_listener.h"

namespace OHOS {
namespace MiscServices {
std::mutex CjInputMethodTextChangedListener::listenerMutex_;
sptr<CjInputMethodTextChangedListener> CjInputMethodTextChangedListener::inputMethodListener_{ nullptr };
sptr<CjInputMethodTextChangedListener> CjInputMethodTextChangedListener::GetInstance()
{
    if (inputMethodListener_ == nullptr) {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        if (inputMethodListener_ == nullptr) {
            inputMethodListener_ = new (std::nothrow) CjInputMethodTextChangedListener();
        }
    }
    return inputMethodListener_;
}

void CjInputMethodTextChangedListener::InsertText(const std::u16string &text)
{
    CjInputMethodController::GetInstance()->InsertText(text);
}

void CjInputMethodTextChangedListener::DeleteForward(int32_t length)
{
    CjInputMethodController::GetInstance()->DeleteRight(length);
}

void CjInputMethodTextChangedListener::DeleteBackward(int32_t length)
{
    CjInputMethodController::GetInstance()->DeleteLeft(length);
}

void CjInputMethodTextChangedListener::SendKeyboardStatus(const KeyboardStatus &status)
{
    CjInputMethodController::GetInstance()->SendKeyboardStatus(status);
}

void CjInputMethodTextChangedListener::SendFunctionKey(const FunctionKey &functionKey)
{
    CjInputMethodController::GetInstance()->SendFunctionKey(functionKey);
}

void CjInputMethodTextChangedListener::MoveCursor(const Direction direction)
{
    CjInputMethodController::GetInstance()->MoveCursor(direction);
}

void CjInputMethodTextChangedListener::HandleExtendAction(int32_t action)
{
    CjInputMethodController::GetInstance()->HandleExtendAction(action);
}

std::u16string CjInputMethodTextChangedListener::GetLeftTextOfCursor(int32_t number)
{
    return CjInputMethodController::GetInstance()->GetText("getLeftTextOfCursor", number);
}

std::u16string CjInputMethodTextChangedListener::GetRightTextOfCursor(int32_t number)
{
    return CjInputMethodController::GetInstance()->GetText("getRightTextOfCursor", number);
}

int32_t CjInputMethodTextChangedListener::GetTextIndexAtCursor()
{
    return CjInputMethodController::GetInstance()->GetTextIndexAtCursor();
}

int32_t CjInputMethodTextChangedListener::ReceivePrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS