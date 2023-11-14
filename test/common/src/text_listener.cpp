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

namespace OHOS {
namespace MiscServices {
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
KeyboardStatus TextListener::keyboardStatus_ = { KeyboardStatus::NONE };
bool TextListener::isTimeout_ = { false };
PanelStatusInfo TextListener::info_{};

TextListener::TextListener()
{
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("TextListenerNotifier");
    serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
}

TextListener::~TextListener() {}

void TextListener::InsertText(const std::u16string &text)
{
    insertText_ = text;
    textListenerCv_.notify_one();
}

void TextListener::DeleteForward(int32_t length)
{
    deleteForwardLength_ = length;
    textListenerCv_.notify_one();
    IMSA_HILOGI("TextChangeListener: DeleteForward, length is: %{public}d", length);
}

void TextListener::DeleteBackward(int32_t length)
{
    deleteBackwardLength_ = length;
    textListenerCv_.notify_one();
    IMSA_HILOGI("TextChangeListener: DeleteBackward, direction is: %{public}d", length);
}

void TextListener::SendKeyEventFromInputMethod(const KeyEvent &event) {}

void TextListener::SendKeyboardStatus(const KeyboardStatus &keyboardStatus)
{
    IMSA_HILOGD("TextListener::SendKeyboardStatus %{public}d", static_cast<int>(keyboardStatus));
    keyboardStatus_ = keyboardStatus;
    textListenerCv_.notify_one();
}

void TextListener::SendFunctionKey(const FunctionKey &functionKey)
{
    EnterKeyType enterKeyType = functionKey.GetEnterKeyType();
    key_ = static_cast<int32_t>(enterKeyType);
    textListenerCv_.notify_one();
}

void TextListener::SetKeyboardStatus(bool status)
{
    status_ = status;
}

void TextListener::MoveCursor(const Direction direction)
{
    direction_ = static_cast<int32_t>(direction);
    textListenerCv_.notify_one();
    IMSA_HILOGI("TextChangeListener: MoveCursor, direction is: %{public}d", direction);
}

void TextListener::HandleSetSelection(int32_t start, int32_t end)
{
    selectionStart_ = start;
    selectionEnd_ = end;
    textListenerCv_.notify_one();
    IMSA_HILOGI(
        "TextChangeListener, selectionStart_: %{public}d, selectionEnd_: %{public}d", selectionStart_, selectionEnd_);
}

void TextListener::HandleExtendAction(int32_t action)
{
    action_ = action;
    textListenerCv_.notify_one();
    IMSA_HILOGI("HandleExtendAction, action_: %{public}d", action_);
}

void TextListener::HandleSelect(int32_t keyCode, int32_t cursorMoveSkip)
{
    selectionDirection_ = keyCode;
    selectionSkip_ = cursorMoveSkip;
    textListenerCv_.notify_one();
    IMSA_HILOGI("TextChangeListener, selectionDirection_: %{public}d", selectionDirection_);
}
std::u16string TextListener::GetLeftTextOfCursor(int32_t number)
{
    return Str8ToStr16(TEXT_BEFORE_CURSOR);
}
std::u16string TextListener::GetRightTextOfCursor(int32_t number)
{
    return Str8ToStr16(TEXT_AFTER_CURSOR);
}
int32_t TextListener::GetTextIndexAtCursor()
{
    return TEXT_INDEX;
}
void TextListener::NotifyPanelStatusInfo(const PanelStatusInfo &info)
{
    IMSA_HILOGD("TextListener::type: %{public}d, flag: %{public}d, visible: %{public}d, trigger: %{public}d.",
        static_cast<PanelType>(info.panelInfo.panelType), static_cast<PanelFlag>(info.panelInfo.panelFlag),
        info.visible, static_cast<Trigger>(info.trigger));
    info_ = info;
    textListenerCv_.notify_one();
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
    isTimeout_ = false;
}
bool TextListener::WaitSendKeyboardStatusCallback(const KeyboardStatus &keyboardStatus)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(
        lock, std::chrono::seconds(1), [&keyboardStatus]() { return keyboardStatus == keyboardStatus_; });
    return keyboardStatus == keyboardStatus_;
}
bool TextListener::WaitNotifyPanelStatusInfoCallback(const PanelStatusInfo &info)
{
    std::unique_lock<std::mutex> lock(textListenerCallbackLock_);
    textListenerCv_.wait_for(lock, std::chrono::seconds(1), [info]() { return info == info_; });
    return info == info_;
}
} // namespace MiscServices
} // namespace OHOS