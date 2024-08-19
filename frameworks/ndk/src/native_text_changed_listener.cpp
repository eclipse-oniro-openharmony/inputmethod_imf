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
#include "input_method_utils.h"
#include "native_text_editor.h"

namespace OHOS {
namespace MiscServices {
void NativeTextChangedListener::InsertText(const std::u16string &text)
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return;
    }

    if (textEditor_->insertTextFunc == nullptr) {
        IMSA_HILOGE("insertTextFunc is nullptr");
        return;
    }

    textEditor_->insertTextFunc(textEditor_, text.c_str(), text.length());
}

void NativeTextChangedListener::DeleteForward(int32_t length)
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return;
    }

    if (textEditor_->deleteForwardFunc == nullptr) {
        IMSA_HILOGE("deleteForwardFunc is nullptr");
        return;
    }

    textEditor_->deleteForwardFunc(textEditor_, length);
}

void NativeTextChangedListener::DeleteBackward(int32_t length)
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return;
    }

    if (textEditor_->deleteBackwardFunc == nullptr) {
        IMSA_HILOGE("deleteBackwardFunc is nullptr");
        return;
    }

    textEditor_->deleteBackwardFunc(textEditor_, length);
}

void NativeTextChangedListener::SendKeyboardStatus(const OHOS::MiscServices::KeyboardStatus &status)
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return;
    }

    if (textEditor_->sendKeyboardStatusFunc == nullptr) {
        IMSA_HILOGE("sendKeyboardStatusFunc is nullptr");
        return;
    }

    textEditor_->sendKeyboardStatusFunc(textEditor_, ConvertToCKeyboardStatus(status));
}

void NativeTextChangedListener::SendFunctionKey(const OHOS::MiscServices::FunctionKey &functionKey)
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return;
    }

    if (textEditor_->sendEnterKeyFunc == nullptr) {
        IMSA_HILOGE("sendEnterKeyFunc is nullptr");
        return;
    }

    auto enterKeyType = ConvertToCEnterKeyType(functionKey.GetEnterKeyType());

    textEditor_->sendEnterKeyFunc(textEditor_, enterKeyType);
}

void NativeTextChangedListener::MoveCursor(const OHOS::MiscServices::Direction direction)
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return;
    }

    if (textEditor_->moveCursorFunc == nullptr) {
        IMSA_HILOGE("moveCursorFunc is nullptr");
        return;
    }

    textEditor_->moveCursorFunc(textEditor_, ConvertToCDirection(direction));
}

void NativeTextChangedListener::HandleSetSelection(int32_t start, int32_t end)
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return;
    }

    if (textEditor_->handleSetSelectionFunc == nullptr) {
        IMSA_HILOGE("handleSetSelectionFunc is nullptr");
        return;
    }

    textEditor_->handleSetSelectionFunc(textEditor_, start, end);
}

void NativeTextChangedListener::HandleExtendAction(int32_t action)
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return;
    }

    if (textEditor_->handleExtendActionFunc == nullptr) {
        IMSA_HILOGE("handleExtendActionFunc is nullptr");
        return;
    }

    textEditor_->handleExtendActionFunc(textEditor_, ConvertToCExtendAction(action));
}

std::u16string NativeTextChangedListener::GetLeftTextOfCursor(int32_t number)
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return u"";
    }

    if (textEditor_->getLeftTextOfCursorFunc == nullptr) {
        IMSA_HILOGE("getLeftTextOfCursorFunc is nullptr");
        return u"";
    }

    if (number <= 0 || number > MAX_TEXT_LENGTH) {
        IMSA_HILOGE("number is invalid");
        return u"";
    }

    size_t length = number + 1;
    char16_t *text = new char16_t[length];
    if (text == nullptr) {
        IMSA_HILOGE("text is nullptr");
        return u"";
    }

    textEditor_->getLeftTextOfCursorFunc(textEditor_, number, text, &length);

    std::u16string textStr(text, length);
    delete[] text;
    return textStr;
}

std::u16string NativeTextChangedListener::GetRightTextOfCursor(int32_t number)
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return u"";
    }

    if (textEditor_->getRightTextOfCursorFunc == nullptr) {
        IMSA_HILOGE("getRightTextOfCursorFunc is nullptr");
        return u"";
    }

    if (number <= 0 || number > MAX_TEXT_LENGTH) {
        IMSA_HILOGE("number is invalid");
        return u"";
    }

    size_t length = number + 1;
    char16_t *text = new char16_t[length];
    if (text == nullptr) {
        IMSA_HILOGE("text is nullptr");
        return u"";
    }

    textEditor_->getRightTextOfCursorFunc(textEditor_, number, text, &length);
    std::u16string textStr(text, length);
    delete[] text;
    return textStr;
}

int32_t NativeTextChangedListener::GetTextIndexAtCursor()
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return 0;
    }

    if (textEditor_->getTextIndexAtCursorFunc == nullptr) {
        IMSA_HILOGE("getTextIndexAtCursorFunc is nullptr");
        return 0;
    }

    return textEditor_->getTextIndexAtCursorFunc(textEditor_);
}

int32_t NativeTextChangedListener::ReceivePrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }

    if (textEditor_->receivePrivateCommandFunc == nullptr) {
        IMSA_HILOGE("receivePrivateCommandFunc is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }

    InputMethod_PrivateCommand **command = new InputMethod_PrivateCommand *[privateCommand.size()];
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }

    size_t index = 0;
    for (auto &item : privateCommand) {
        command[index] = new InputMethod_PrivateCommand();
        command[index]->key = item.first;
        command[index]->value = item.second;
        ++index;
    }

    auto errCode = textEditor_->receivePrivateCommandFunc(textEditor_, command, privateCommand.size());

    for (size_t i = 0; i < index; ++i) {
        delete command[i];
    }
    delete[] command;
    return errCode;
}

int32_t NativeTextChangedListener::SetPreviewText(const std::u16string &text, const OHOS::MiscServices::Range &range)
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }

    if (textEditor_->setPreviewTextFunc == nullptr) {
        IMSA_HILOGE("setPreviewTextFunc is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }

    return textEditor_->setPreviewTextFunc(textEditor_, text.c_str(), text.length(), range.start, range.end);
}

void NativeTextChangedListener::FinishTextPreview()
{
    if (textEditor_ == nullptr) {
        IMSA_HILOGE("textEditor_ is nullptr");
        return;
    }

    if (textEditor_->finishTextPreviewFunc == nullptr) {
        IMSA_HILOGE("finishTextPreviewFunc is nullptr");
        return;
    }

    textEditor_->finishTextPreviewFunc(textEditor_);
}

InputMethod_KeyboardStatus NativeTextChangedListener::ConvertToCKeyboardStatus(
    OHOS::MiscServices::KeyboardStatus status)
{
    switch (status) {
        case OHOS::MiscServices::KeyboardStatus::HIDE:
            return IME_KEYBOARD_STATUS_HIDE;
        case OHOS::MiscServices::KeyboardStatus::SHOW:
            return IME_KEYBOARD_STATUS_SHOW;
        default:
            return IME_KEYBOARD_STATUS_NONE;
    }
}

InputMethod_EnterKeyType NativeTextChangedListener::ConvertToCEnterKeyType(
    OHOS::MiscServices::EnterKeyType enterKeyType)
{
    switch (enterKeyType) {
        case OHOS::MiscServices::EnterKeyType::NONE:
            return IME_ENTER_KEY_NONE;
        case OHOS::MiscServices::EnterKeyType::GO:
            return IME_ENTER_KEY_GO;
        case OHOS::MiscServices::EnterKeyType::SEARCH:
            return IME_ENTER_KEY_SEARCH;
        case OHOS::MiscServices::EnterKeyType::SEND:
            return IME_ENTER_KEY_SEND;
        case OHOS::MiscServices::EnterKeyType::NEXT:
            return IME_ENTER_KEY_NEXT;
        case OHOS::MiscServices::EnterKeyType::DONE:
            return IME_ENTER_KEY_DONE;
        case OHOS::MiscServices::EnterKeyType::PREVIOUS:
            return IME_ENTER_KEY_PREVIOUS;
        case OHOS::MiscServices::EnterKeyType::NEW_LINE:
            return IME_ENTER_KEY_NEWLINE;
        default:
            return IME_ENTER_KEY_UNSPECIFIED;
    }
}

InputMethod_Direction NativeTextChangedListener::ConvertToCDirection(OHOS::MiscServices::Direction direction)
{
    switch (direction) {
        case OHOS::MiscServices::Direction::NONE:
            return IME_DIRECTION_NONE;
        case OHOS::MiscServices::Direction::UP:
            return IME_DIRECTION_UP;
        case OHOS::MiscServices::Direction::DOWN:
            return IME_DIRECTION_DOWN;
        case OHOS::MiscServices::Direction::LEFT:
            return IME_DIRECTION_LEFT;
        case OHOS::MiscServices::Direction::RIGHT:
            return IME_DIRECTION_RIGHT;
        default:
            return IME_DIRECTION_NONE;
    }
}

InputMethod_ExtendAction NativeTextChangedListener::ConvertToCExtendAction(int32_t action)
{
    switch (action) {
        case static_cast<int32_t>(OHOS::MiscServices::ExtendAction::SELECT_ALL):
            return IME_EXTEND_ACTION_SELECT_ALL;
        case static_cast<int32_t>(OHOS::MiscServices::ExtendAction::CUT):
            return IME_EXTEND_ACTION_CUT;
        case static_cast<int32_t>(OHOS::MiscServices::ExtendAction::COPY):
            return IME_EXTEND_ACTION_COPY;
        case static_cast<int32_t>(OHOS::MiscServices::ExtendAction::PASTE):
            return IME_EXTEND_ACTION_PASTE;
        default:
            IMSA_HILOGE("invalid action:%{public}d", action);
            return IME_EXTEND_ACTION_SELECT_ALL;
    }
}
} // namespace MiscServices
} // namespace OHOS