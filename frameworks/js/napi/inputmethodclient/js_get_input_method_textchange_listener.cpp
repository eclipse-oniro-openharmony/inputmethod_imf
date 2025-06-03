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
#include "event_handler.h"
#include "js_get_input_method_controller.h"
#include "event_handler.h"
namespace OHOS {
namespace MiscServices {
std::mutex JsGetInputMethodTextChangedListener::listenerMutex_;
sptr<JsGetInputMethodTextChangedListener> JsGetInputMethodTextChangedListener::inputMethodListener_{ nullptr };
sptr<JsGetInputMethodTextChangedListener> JsGetInputMethodTextChangedListener::GetTextListener(bool newEditBox)
{
    IMSA_HILOGD("newEditBox is %{public}d.", newEditBox);
    if (newEditBox) {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        inputMethodListener_ = new (std::nothrow) JsGetInputMethodTextChangedListener();
    } else {
        if (inputMethodListener_ == nullptr) {
            std::lock_guard<std::mutex> lock(listenerMutex_);
            if (inputMethodListener_ == nullptr) {
                inputMethodListener_ = new (std::nothrow) JsGetInputMethodTextChangedListener();
            }
        }
    }
    inputMethodListener_->handler_ = AppExecFwk::EventHandler::Current();
    return inputMethodListener_;
}

void JsGetInputMethodTextChangedListener::InsertText(const std::u16string &text)
{
    JsGetInputMethodController::GetInstance()->InsertText(text);
}

void JsGetInputMethodTextChangedListener::DeleteForward(int32_t length)
{
    JsGetInputMethodController::GetInstance()->DeleteRight(length);
}

void JsGetInputMethodTextChangedListener::DeleteBackward(int32_t length)
{
    JsGetInputMethodController::GetInstance()->DeleteLeft(length);
}

void JsGetInputMethodTextChangedListener::SendKeyboardStatus(const KeyboardStatus &status)
{
    JsGetInputMethodController::GetInstance()->SendKeyboardStatus(status);
}

void JsGetInputMethodTextChangedListener::SendFunctionKey(const FunctionKey &functionKey)
{
    JsGetInputMethodController::GetInstance()->SendFunctionKey(functionKey);
}

void JsGetInputMethodTextChangedListener::MoveCursor(const Direction direction)
{
    JsGetInputMethodController::GetInstance()->MoveCursor(direction);
}

void JsGetInputMethodTextChangedListener::HandleExtendAction(int32_t action)
{
    JsGetInputMethodController::GetInstance()->HandleExtendAction(action);
}

std::u16string JsGetInputMethodTextChangedListener::GetLeftTextOfCursor(int32_t number)
{
    return JsGetInputMethodController::GetInstance()->GetText("getLeftTextOfCursor", number);
}

std::u16string JsGetInputMethodTextChangedListener::GetRightTextOfCursor(int32_t number)
{
    return JsGetInputMethodController::GetInstance()->GetText("getRightTextOfCursor", number);
}

int32_t JsGetInputMethodTextChangedListener::GetTextIndexAtCursor()
{
    return JsGetInputMethodController::GetInstance()->GetTextIndexAtCursor();
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
    return JsGetInputMethodController::GetInstance()->SetPreviewText(text, range);
}

void JsGetInputMethodTextChangedListener::FinishTextPreview()
{
    return JsGetInputMethodController::GetInstance()->FinishTextPreview();
}

std::shared_ptr<AppExecFwk::EventHandler> JsGetInputMethodTextChangedListener::GetEventHandler() const
{
    return handler_;
}
} // namespace MiscServices
} // namespace OHOS