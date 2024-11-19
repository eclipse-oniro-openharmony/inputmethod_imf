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

#include "text_listener.h"

#include "input_method_utils.h"

namespace OHOS {
namespace MiscServices {
constexpr uint32_t KEYBOARD_STATUS_WAIT_TIME_OUT = 3;
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
int32_t TextListener::selectionSkip_ = -1;
int32_t TextListener::action_ = -1;
uint32_t TextListener::height_ = 0;
KeyboardStatus TextListener::keyboardStatus_ = { KeyboardStatus::NONE };
PanelStatusInfo TextListener::info_ {};
std::unordered_map<std::string, PrivateDataValue> TextListener::privateCommand_ {};
std::string TextListener::previewText_;
Range TextListener::previewRange_ {};
bool TextListener::isFinishTextPreviewCalled_ { false };
TextListener::TextListener()
{
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("TextListenerNotifier");
    serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
}

TextListener::~TextListener() { }

void TextListener::InsertText(const std::u16string &text)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    insertText_ = text;
    textListenerCv_.notify_one();
    IMSA_HILOGI("TextListener text: %{public}s", Str16ToStr8(text).c_str());
}

void TextListener::DeleteForward(int32_t length)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    deleteForwardLength_ = length;
    textListenerCv_.notify_one();
    IMSA_HILOGI("TextListener: DeleteForward, length is: %{public}d", length);
}

void TextListener::DeleteBackward(int32_t length)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    deleteBackwardLength_ = length;
    textListenerCv_.notify_one();
    IMSA_HILOGI("TextListener: DeleteBackward, direction is: %{public}d", length);
}

void TextListener::SendKeyEventFromInputMethod(const KeyEvent &event) { }

void TextListener::SendKeyboardStatus(const KeyboardStatus &keyboardStatus)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    IMSA_HILOGI("TextListener::SendKeyboardStatus %{public}d", static_cast<int>(keyboardStatus));
    keyboardStatus_ = keyboardStatus;
    textListenerCv_.notify_one();
}

void TextListener::SendFunctionKey(const FunctionKey &functionKey)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    EnterKeyType enterKeyType = functionKey.GetEnterKeyType();
    key_ = static_cast<int32_t>(enterKeyType);
    IMSA_HILOGI("TextListener functionKey: %{public}d", key_);
    textListenerCv_.notify_one();
}

void TextListener::SetKeyboardStatus(bool status)
{
    status_ = status;
    IMSA_HILOGI("TextListener status: %{public}d", status);
}

void TextListener::MoveCursor(const Direction direction)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    direction_ = static_cast<int32_t>(direction);
    textListenerCv_.notify_one();
    IMSA_HILOGI("TextListener: MoveCursor, direction is: %{public}d", direction);
}

void TextListener::HandleSetSelection(int32_t start, int32_t end)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    selectionStart_ = start;
    selectionEnd_ = end;
    textListenerCv_.notify_one();
    IMSA_HILOGI("TextListener, selectionStart_: %{public}d, selectionEnd_: %{public}d", selectionStart_, selectionEnd_);
}

void TextListener::HandleExtendAction(int32_t action)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    action_ = action;
    textListenerCv_.notify_one();
    IMSA_HILOGI("HandleExtendAction, action_: %{public}d", action_);
}

void TextListener::HandleSelect(int32_t keyCode, int32_t cursorMoveSkip)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    selectionDirection_ = keyCode;
    selectionSkip_ = cursorMoveSkip;
    textListenerCv_.notify_one();
    IMSA_HILOGI("TextListener, selectionDirection_: %{public}d", selectionDirection_);
}

std::u16string TextListener::GetLeftTextOfCursor(int32_t number)
{
    IMSA_HILOGI("TextListener number: %{public}d", number);
    return Str8ToStr16(TEXT_BEFORE_CURSOR);
}

std::u16string TextListener::GetRightTextOfCursor(int32_t number)
{
    IMSA_HILOGI("TextListener number: %{public}d", number);
    return Str8ToStr16(TEXT_AFTER_CURSOR);
}

int32_t TextListener::GetTextIndexAtCursor()
{
    return TEXT_INDEX;
}

int32_t TextListener::ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    privateCommand_ = privateCommand;
    return 0;
}

void TextListener::NotifyPanelStatusInfo(const PanelStatusInfo &info)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    IMSA_HILOGI("TextListener::type: %{public}d, flag: %{public}d, visible: %{public}d, trigger: %{public}d.",
        static_cast<PanelType>(info.panelInfo.panelType), static_cast<PanelFlag>(info.panelInfo.panelFlag),
        info.visible, static_cast<Trigger>(info.trigger));
    info_ = info;
    textListenerCv_.notify_one();
}

void TextListener::NotifyKeyboardHeight(uint32_t height)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    IMSA_HILOGI("keyboard height: %{public}u", height);
    height_ = height;
    textListenerCv_.notify_one();
}

int32_t TextListener::SetPreviewText(const std::u16string &text, const Range &range)
{
    IMSA_HILOGI("TextListener, text: %{public}s, range[start, end]: [%{public}d, %{public}d]",
        Str16ToStr8(text).c_str(), range.start, range.end);
    previewText_ = Str16ToStr8(text);
    previewRange_ = range;
    return ErrorCode::NO_ERROR;
}

void TextListener::FinishTextPreview()
{
    IMSA_HILOGI("TextListener in");
    isFinishTextPreviewCalled_ = true;
}

void TextListener::ResetParam()
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
    selectionSkip_ = -1;
    action_ = -1;
    keyboardStatus_ = KeyboardStatus::NONE;
    info_ = {};
    height_ = 0;
    previewText_ = "";
    previewRange_ = {};
    isFinishTextPreviewCalled_ = false;
}

bool TextListener::WaitSendKeyboardStatusCallback(const KeyboardStatus &keyboardStatus)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(lock, std::chrono::seconds(KEYBOARD_STATUS_WAIT_TIME_OUT), [&keyboardStatus]() {
        return keyboardStatus == keyboardStatus_;
    });
    return keyboardStatus == keyboardStatus_;
}

bool TextListener::WaitNotifyPanelStatusInfoCallback(const PanelStatusInfo &info)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(lock, std::chrono::seconds(1), [info]() {
        return info == info_;
    });
    return info == info_;
}

bool TextListener::WaitNotifyKeyboardHeightCallback(uint32_t height)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(lock, std::chrono::seconds(1), [height]() {
        return height_ == height;
    });
    return height_ == height;
}

bool TextListener::WaitSendPrivateCommandCallback(std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(lock, std::chrono::seconds(1), [privateCommand]() {
        return privateCommand_ == privateCommand;
    });
    return privateCommand_ == privateCommand;
}

bool TextListener::WaitInsertText(const std::u16string &insertText)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(lock, std::chrono::seconds(1), [insertText]() {
        return insertText_ == insertText;
    });
    return insertText_ == insertText;
}

bool TextListener::WaitMoveCursor(int32_t direction)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(lock, std::chrono::seconds(1), [direction]() {
        return direction_ == direction;
    });
    return direction_ == direction;
}

bool TextListener::WaitDeleteForward(int32_t length)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(lock, std::chrono::seconds(1), [length]() {
        return deleteForwardLength_ == length;
    });
    return deleteForwardLength_ == length;
}

bool TextListener::WaitDeleteBackward(int32_t length)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(lock, std::chrono::seconds(1), [length]() { return deleteBackwardLength_ == length; });
    return deleteBackwardLength_ == length;
}

bool TextListener::WaitSendFunctionKey(int32_t functionKey)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(lock, std::chrono::seconds(1), [functionKey]() {
        return key_ == functionKey;
    });
    return key_ == functionKey;
}

bool TextListener::WaitHandleExtendAction(int32_t action)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(lock, std::chrono::seconds(1), [action]() {
        return action_ == action;
    });
    return action_ == action;
}

bool TextListener::WaitHandleSetSelection(int32_t start, int32_t end)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(lock, std::chrono::seconds(1), [start, end]() {
        return selectionStart_ == start && selectionEnd_ == end;
    });
    return selectionStart_ == start && selectionEnd_ == end;
}

bool TextListener::WaitHandleSelect(int32_t keyCode, int32_t cursorMoveSkip)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(lock, std::chrono::seconds(1), [keyCode]() {
        return selectionDirection_ == keyCode;
    });
    return selectionDirection_ == keyCode;
}
} // namespace MiscServices
} // namespace OHOS