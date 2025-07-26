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
#include "js_get_input_method_textchange_listener.h"

#include "js_get_input_method_controller.h"
namespace OHOS {
namespace MiscServices {
std::mutex JsGetInputMethodTextChangedListener::listenerMutex_;
sptr<JsGetInputMethodTextChangedListener> JsGetInputMethodTextChangedListener::inputMethodListener_{ nullptr };
sptr<JsGetInputMethodTextChangedListener> JsGetInputMethodTextChangedListener::GetTextListener(
    const std::shared_ptr<AppExecFwk::EventHandler> &handler, bool newEditBox)
{
    IMSA_HILOGD("newEditBox is %{public}d.", newEditBox);
    if (newEditBox) {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        inputMethodListener_ = new (std::nothrow) JsGetInputMethodTextChangedListener(handler);
    } else {
        if (inputMethodListener_ == nullptr) {
            std::lock_guard<std::mutex> lock(listenerMutex_);
            if (inputMethodListener_ == nullptr) {
                inputMethodListener_ = new (std::nothrow) JsGetInputMethodTextChangedListener(handler);
            }
        }
    }
    return inputMethodListener_;
}

void JsGetInputMethodTextChangedListener::InsertText(const std::u16string &text)
{
    auto controller = JsGetInputMethodController::GetInstance();
    if (controller == nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->InsertText(text);
}

void JsGetInputMethodTextChangedListener::DeleteForward(int32_t length)
{
    auto controller = JsGetInputMethodController::GetInstance();
    if (controller == nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->DeleteRight(length);
}

void JsGetInputMethodTextChangedListener::DeleteBackward(int32_t length)
{
    auto controller = JsGetInputMethodController::GetInstance();
    if (controller == nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->DeleteLeft(length);
}

void JsGetInputMethodTextChangedListener::SendKeyboardStatus(const KeyboardStatus &status)
{
    auto controller = JsGetInputMethodController::GetInstance();
    if (controller == nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->SendKeyboardStatus(status);
}

void JsGetInputMethodTextChangedListener::SendFunctionKey(const FunctionKey &functionKey)
{
    auto controller = JsGetInputMethodController::GetInstance();
    if (controller == nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->SendFunctionKey(functionKey);
}

void JsGetInputMethodTextChangedListener::MoveCursor(const Direction direction)
{
    auto controller = JsGetInputMethodController::GetInstance();
    if (controller == nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->MoveCursor(direction);
}

void JsGetInputMethodTextChangedListener::HandleExtendAction(int32_t action)
{
    auto controller = JsGetInputMethodController::GetInstance();
    if (controller == nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    controller->HandleExtendAction(action);
}

std::u16string JsGetInputMethodTextChangedListener::GetLeftTextOfCursor(int32_t number)
{
    auto controller = JsGetInputMethodController::GetInstance();
    if (controller == nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return std::u16string();
    }
    return controller->GetText("getLeftTextOfCursor", number);
}

std::u16string JsGetInputMethodTextChangedListener::GetRightTextOfCursor(int32_t number)
{
    auto controller = JsGetInputMethodController::GetInstance();
    if (controller == nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return std::u16string();
    }
    return controller->GetText("getRightTextOfCursor", number);
}

int32_t JsGetInputMethodTextChangedListener::GetTextIndexAtCursor()
{
    auto controller = JsGetInputMethodController::GetInstance();
    if (controller == nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        int32_t index = -1;
        return index;
    }
    return controller->GetTextIndexAtCursor();
}

int32_t JsGetInputMethodTextChangedListener::ReceivePrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    return ErrorCode::NO_ERROR;
}

bool JsGetInputMethodTextChangedListener::IsFromTs()
{
    return true;
}

int32_t JsGetInputMethodTextChangedListener::SetPreviewText(const std::u16string &text, const Range &range)
{
    auto controller = JsGetInputMethodController::GetInstance();
    if (controller == nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return controller->SetPreviewText(text, range);
}

void JsGetInputMethodTextChangedListener::FinishTextPreview()
{
    auto controller = JsGetInputMethodController::GetInstance();
    if (controller == nullptr) {
        IMSA_HILOGE("controller is nullptr!");
        return;
    }
    return controller->FinishTextPreview();
}

std::shared_ptr<AppExecFwk::EventHandler> JsGetInputMethodTextChangedListener::GetEventHandler()
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    return handler_;
}

JsGetInputMethodTextChangedListener::JsGetInputMethodTextChangedListener(
    const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    handler_ = handler;
}
JsGetInputMethodTextChangedListener::~JsGetInputMethodTextChangedListener()
{
}
} // namespace MiscServices
} // namespace OHOS