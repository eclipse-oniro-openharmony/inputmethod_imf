/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "cj_input_method_controller.h"
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
    auto controller = CjInputMethodController::GetInstance();
    if (controller ==  nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->InsertText(text);
}

void CjInputMethodTextChangedListener::DeleteForward(int32_t length)
{
    auto controller = CjInputMethodController::GetInstance();
    if (controller ==  nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->DeleteRight(length);
}

void CjInputMethodTextChangedListener::DeleteBackward(int32_t length)
{
    auto controller = CjInputMethodController::GetInstance();
    if (controller ==  nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->DeleteLeft(length);
}

void CjInputMethodTextChangedListener::SendKeyboardStatus(const KeyboardStatus &status)
{
    auto controller = CjInputMethodController::GetInstance();
    if (controller ==  nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->SendKeyboardStatus(status);
}

void CjInputMethodTextChangedListener::SendFunctionKey(const FunctionKey &functionKey)
{
    auto controller = CjInputMethodController::GetInstance();
    if (controller ==  nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->SendFunctionKey(functionKey);
}

void CjInputMethodTextChangedListener::MoveCursor(const Direction direction)
{
    auto controller = CjInputMethodController::GetInstance();
    if (controller ==  nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->MoveCursor(direction);
}

void CjInputMethodTextChangedListener::HandleExtendAction(int32_t action)
{
    auto controller = CjInputMethodController::GetInstance();
    if (controller ==  nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->HandleExtendAction(action);
}

std::u16string CjInputMethodTextChangedListener::GetLeftTextOfCursor(int32_t number)
{
    auto controller = CjInputMethodController::GetInstance();
    if (controller ==  nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return std::u16string();
    }
    return controller->GetLeftText(number);
}

std::u16string CjInputMethodTextChangedListener::GetRightTextOfCursor(int32_t number)
{
    auto controller = CjInputMethodController::GetInstance();
    if (controller ==  nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return std::u16string();
    }
    return controller->GetRightText(number);
}

int32_t CjInputMethodTextChangedListener::GetTextIndexAtCursor()
{
    auto controller = CjInputMethodController::GetInstance();
    if (controller ==  nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        int32_t index = -1;
        return index;
    }
    return controller->GetTextIndexAtCursor();
}

int32_t CjInputMethodTextChangedListener::ReceivePrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS