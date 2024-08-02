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
#include "input_method_utils.h"
#include "input_method_controller.h"

namespace OHOS {
namespace MiscServices {
std::mutex CjInputMethodController::controllerMutex_;
std::shared_ptr<CjInputMethodController> CjInputMethodController::controller_{ nullptr };
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

int32_t CjInputMethodController::Attach(bool showKeyboard, const CTextConfig &txtCfg)
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

void CjInputMethodController::OnSelectByRange(int32_t start, int32_t end)
{
    return;
}

void CjInputMethodController::OnSelectByMovement(int32_t direction)
{
    return;
}

void CjInputMethodController::InsertText(const std::u16string &text)
{
    return;
}

void CjInputMethodController::DeleteRight(int32_t length)
{
    return;
}

void CjInputMethodController::DeleteLeft(int32_t length)
{
    return;
}

void CjInputMethodController::SendKeyboardStatus(const KeyboardStatus &status)
{
    return;
}

void CjInputMethodController::SendFunctionKey(const FunctionKey &functionKey)
{
    return;
}

void CjInputMethodController::MoveCursor(const Direction direction)
{
    return;
}

void CjInputMethodController::HandleExtendAction(int32_t action)
{
    return;
}

std::u16string CjInputMethodController::GetText(const std::string &type, int32_t number)
{
    std::u16string str;
    str.push_back(u'1');
    return str;
}

int32_t CjInputMethodController::GetTextIndexAtCursor()
{
    return 0;
}
} // namespace MiscServices
} // namespace OHOS
