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

#include <string>
#include <locale>
#include <codecvt>

#include "cj_input_method_controller.h"
#include "cj_input_method_textchanged_listener.h"
#include "cj_lambda.h"
#include "utils.h"
#include "input_method_utils.h"
#include "string_ex.h"

namespace OHOS {
namespace MiscServices {
std::mutex CjInputMethodController::controllerMutex_;
std::shared_ptr<CjInputMethodController> CjInputMethodController::controller_{ nullptr };
const int8_t INSERT_TEXT = 0;
const int8_t DELETE_LEFT = 1;
const int8_t DELETE_RIGHT = 2;
const int8_t SEND_KEYBOARD_STATUS = 3;
const int8_t SEND_FUNCTION_KEY = 4;
const int8_t MOVE_CURSOR = 5;
const int8_t HANDLE_EXTEND_ACTION = 6;
const int8_t GET_LEFT_TEXT = 7;
const int8_t GET_RIGHT_TEXT = 8;
const int8_t GET_TEXT_INDEX = 9;
const int8_t SELECT_BY_MOVEMENT = 10;

std::shared_ptr<CjInputMethodController> CjInputMethodController::GetInstance()
{
    if (controller_ == nullptr) {
        std::lock_guard<std::mutex> lock(controllerMutex_);
        if (controller_ == nullptr) {
            auto controller = std::make_shared<CjInputMethodController>();
            controller_ = controller;
            InputMethodController::GetInstance()->SetControllerListener(controller_);
        }
    }
    return controller_;
}

int32_t CjInputMethodController::Attach(const CTextConfig &txtCfg, bool showKeyboard)
{
    auto textListener = CjInputMethodTextChangedListener::GetInstance();
    if (!textListener) {
        IMSA_HILOGE("failed to create CjInputMethodTextChangedListener!");
        return ERR_NO_MEMORY;
    }
    TextConfig textCfg = {
        .inputAttribute = {
            .inputPattern = txtCfg.inputAttrbute.textInputType,
            .enterKeyType = txtCfg.inputAttrbute.enterKeyType
        },
        .cursorInfo = {
            .left = txtCfg.cursor.left,
            .top = txtCfg.cursor.top,
            .width = txtCfg.cursor.width,
            .height = txtCfg.cursor.height
        },
        .range = {
            .start = txtCfg.range.start,
            .end = txtCfg.range.end
        },
        .windowId = txtCfg.windowId
    };

    auto controller = InputMethodController::GetInstance();
    if (!controller) {
        return ERR_NO_MEMORY;
    }
    return controller->Attach(textListener, showKeyboard, textCfg);
}

int32_t CjInputMethodController::Detach()
{
    auto controller = InputMethodController::GetInstance();
    if (!controller) {
        return ERR_NO_MEMORY;
    }
    return controller->Close();
}

int32_t CjInputMethodController::ShowTextInput()
{
    auto controller = InputMethodController::GetInstance();
    if (!controller) {
        return ERR_NO_MEMORY;
    }
    return controller->ShowTextInput();
}

int32_t CjInputMethodController::HideTextInput()
{
    auto controller = InputMethodController::GetInstance();
    if (!controller) {
        return ERR_NO_MEMORY;
    }
    return controller->HideTextInput();
}

int32_t CjInputMethodController::SetCallingWindow(uint32_t windowId)
{
    auto controller = InputMethodController::GetInstance();
    if (!controller) {
        return ERR_NO_MEMORY;
    }
    return controller->SetCallingWindow(windowId);
}

int32_t CjInputMethodController::UpdateCursor(const CCursorInfo &cursor)
{
    auto controller = InputMethodController::GetInstance();
    if (!controller) {
        return ERR_NO_MEMORY;
    }
    CursorInfo cursorInfo = {
        .left = cursor.left,
        .top = cursor.top,
        .width = cursor.width,
        .height = cursor.height
    };
    return controller->OnCursorUpdate(cursorInfo);
}

int32_t CjInputMethodController::ChangeSelection(const std::string &text, int32_t start, int32_t end)
{
    auto controller = InputMethodController::GetInstance();
    if (!controller) {
        return ERR_NO_MEMORY;
    }
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    std::u16string txt = converter.from_bytes(text);
    return controller->OnSelectionChange(txt, start, end);
}

int32_t CjInputMethodController::UpdateAttribute(const CInputAttribute &inputAttribute)
{
    auto controller = InputMethodController::GetInstance();
    if (!controller) {
        return ERR_NO_MEMORY;
    }
    Configuration config = Configuration();
    config.SetTextInputType(static_cast<TextInputType>(inputAttribute.textInputType));
    config.SetEnterKeyType(static_cast<EnterKeyType>(inputAttribute.enterKeyType));
    return controller->OnConfigurationChange(config);
}

int32_t CjInputMethodController::ShowSoftKeyboard()
{
    auto controller = InputMethodController::GetInstance();
    if (!controller) {
        return ERR_NO_MEMORY;
    }
    return controller->ShowSoftKeyboard();
}

int32_t CjInputMethodController::HideSoftKeyboard()
{
    auto controller = InputMethodController::GetInstance();
    if (!controller) {
        return ERR_NO_MEMORY;
    }
    return controller->HideSoftKeyboard();
}

int32_t CjInputMethodController::StopInputSession()
{
    auto controller = InputMethodController::GetInstance();
    if (!controller) {
        return ERR_NO_MEMORY;
    }
    return controller->StopInputSession();
}

void CjInputMethodController::RegisterListener(int8_t type, int64_t id)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    switch (type) {
        case INSERT_TEXT: {
            auto callback = reinterpret_cast<void(*)(const char*)>(id);
            insertText = [lambda = CJLambda::Create(callback)](const char* text) -> void {
                lambda(text);
            };
            break;
        }
        case DELETE_LEFT: {
            auto callback = reinterpret_cast<void(*)(int32_t)>(id);
            deleteLeft = [lambda = CJLambda::Create(callback)](int32_t length) -> void {
                lambda(length);
            };
            break;
        }
        case DELETE_RIGHT: {
            auto callback = reinterpret_cast<void(*)(int32_t)>(id);
            deleteRight = [lambda = CJLambda::Create(callback)](int32_t length) -> void {
                lambda(length);
            };
            break;
        }
        case SEND_KEYBOARD_STATUS: {
            auto callback = reinterpret_cast<void(*)(int64_t)>(id);
            sendKeyboardStatus = [lambda = CJLambda::Create(callback)](int64_t status) -> void {
                lambda(status);
            };
            break;
        }
        case SEND_FUNCTION_KEY: {
            auto callback = reinterpret_cast<void(*)(int64_t)>(id);
            sendFunctionKey = [lambda = CJLambda::Create(callback)](int64_t functionKey) -> void {
                lambda(functionKey);
            };
            break;
        }
        case MOVE_CURSOR: {
            auto callback = reinterpret_cast<void(*)(int64_t)>(id);
            moveCursor = [lambda = CJLambda::Create(callback)](int64_t direction) -> void {
                lambda(direction);
            };
            break;
        }
        case HANDLE_EXTEND_ACTION: {
            auto callback = reinterpret_cast<void(*)(int32_t)>(id);
            handleExtendAction = [lambda = CJLambda::Create(callback)](int32_t action) -> void {
                lambda(action);
            };
            break;
        }
        case GET_LEFT_TEXT: {
            auto callback = reinterpret_cast<char*(*)(int32_t)>(id);
            getLeftText = [lambda = CJLambda::Create(callback)](int32_t number) -> char* {
                return lambda(number);
            };
            break;
        }
        case GET_RIGHT_TEXT: {
            auto callback = reinterpret_cast<char*(*)(int32_t)>(id);
            getRightText = [lambda = CJLambda::Create(callback)](int32_t number) -> char* {
                return lambda(number);
            };
            break;
        }
        case GET_TEXT_INDEX: {
            auto callback = reinterpret_cast<int32_t(*)()>(id);
            getTextIndexAtCursor = [lambda = CJLambda::Create(callback)](void) -> int32_t {
                return lambda();
            };
            break;
        }
        case SELECT_BY_MOVEMENT: {
            auto callback = reinterpret_cast<void(*)(int32_t)>(id);
            onSelectByMovement = [lambda = CJLambda::Create(callback)](int32_t direction) -> void {
                lambda(direction);
            };
            break;
        }
        default: {
            auto callback = reinterpret_cast<void(*)(int32_t, int32_t)>(id);
            onSelectByRange = [lambda = CJLambda::Create(callback)](int32_t start, int32_t end) -> void {
                lambda(start, end);
            };
            break;
        }
    }
}
void CjInputMethodController::UnRegisterListener(int8_t type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    switch (type) {
        case INSERT_TEXT:
            insertText = nullptr;
            break;
        case DELETE_LEFT:
            deleteLeft = nullptr;
            break;
        case DELETE_RIGHT:
            deleteRight = nullptr;
            break;
        case SEND_KEYBOARD_STATUS:
            sendKeyboardStatus = nullptr;
            break;
        case SEND_FUNCTION_KEY:
            sendFunctionKey = nullptr;
            break;
        case MOVE_CURSOR:
            moveCursor = nullptr;
            break;
        case HANDLE_EXTEND_ACTION:
            handleExtendAction = nullptr;
            break;
        case GET_LEFT_TEXT:
            getLeftText = nullptr;
            break;
        case GET_RIGHT_TEXT:
            getRightText = nullptr;
            break;
        case GET_TEXT_INDEX:
            getTextIndexAtCursor = nullptr;
            break;
        case SELECT_BY_MOVEMENT:
            onSelectByMovement = nullptr;
            break;
        default:
            onSelectByRange = nullptr;
            break;
    }
}

int32_t CjInputMethodController::Subscribe(int8_t type, int64_t id)
{
    auto controller = CjInputMethodController::GetInstance();
    if (!controller) {
        return ERR_NO_MEMORY;
    }
    controller->RegisterListener(type, id);
    return SUCCESS_CODE;
}

int32_t CjInputMethodController::Unsubscribe(int8_t type)
{
    auto controller = CjInputMethodController::GetInstance();
    if (!controller) {
        return ERR_NO_MEMORY;
    }
    controller->UnRegisterListener(type);
    return SUCCESS_CODE;
}

void CjInputMethodController::OnSelectByRange(int32_t start, int32_t end)
{
    return onSelectByRange(start, end);
}

void CjInputMethodController::OnSelectByMovement(int32_t direction)
{
    return onSelectByMovement(direction);
}

void CjInputMethodController::InsertText(const std::u16string &text)
{
    char *insertTxt = Utils::MallocCString(Str16ToStr8(text));
    if (insertTxt == nullptr) {
        IMSA_HILOGE("Failed to excute InsertText callback: out of memory.");
        return;
    }
    IMSA_HILOGI("insertText runs");
    insertText(insertTxt);
    free(insertTxt);
    return;
}

void CjInputMethodController::DeleteRight(int32_t length)
{
    IMSA_HILOGI("deleteRight runs");
    deleteRight(length);
    return;
}

void CjInputMethodController::DeleteLeft(int32_t length)
{
    IMSA_HILOGI("deleteLeft runs");
    deleteLeft(length);
    return;
}

void CjInputMethodController::SendKeyboardStatus(const KeyboardStatus &status)
{
    IMSA_HILOGI("sendKeyboardStatus runs");
    auto statusNum = static_cast<int64_t>(status);
    sendKeyboardStatus(statusNum);
    return;
}

void CjInputMethodController::SendFunctionKey(const FunctionKey &functionKey)
{
    IMSA_HILOGI("sendFunctionKey runs");
    auto type = static_cast<int64_t>(functionKey.GetEnterKeyType());
    sendFunctionKey(type);
    return;
}

void CjInputMethodController::MoveCursor(const Direction direction)
{
    IMSA_HILOGI("moveCursor runs");
    auto dir = static_cast<int64_t>(direction);
    moveCursor(dir);
    return;
}

void CjInputMethodController::HandleExtendAction(int32_t action)
{
    IMSA_HILOGI("handleExtendAction runs");
    handleExtendAction(action);
    return;
}

std::u16string CjInputMethodController::GetLeftText(int32_t number)
{
    IMSA_HILOGI("getLeftText runs");
    char *text = getLeftText(number);
    auto ret = Str8ToStr16(std::string(text));
    free(text);
    return ret;
}

std::u16string CjInputMethodController::GetRightText(int32_t number)
{
    IMSA_HILOGI("getRightText runs");
    char *text = getRightText(number);
    auto ret = Str8ToStr16(std::string(text));
    free(text);
    return ret;
}

int32_t CjInputMethodController::GetTextIndexAtCursor()
{
    IMSA_HILOGI("getTextIndexAtCursor runs");
    return getTextIndexAtCursor();
}
} // namespace MiscServices
} // namespace OHOS
