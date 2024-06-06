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

#include "keyboard_listener_test_impl.h"

#include "global.h"

namespace OHOS {
namespace MiscServices {
std::mutex KeyboardListenerTestImpl::kdListenerLock_;
std::condition_variable KeyboardListenerTestImpl::kdListenerCv_;
int32_t KeyboardListenerTestImpl::keyCode_{ -1 };
int32_t KeyboardListenerTestImpl::cursorHeight_{ -1 };
int32_t KeyboardListenerTestImpl::newBegin_{ -1 };
std::string KeyboardListenerTestImpl::text_;
InputAttribute KeyboardListenerTestImpl::inputAttribute_{ 0, 0, 0 };
bool KeyboardListenerTestImpl::OnKeyEvent(int32_t keyCode, int32_t keyStatus, sptr<KeyEventConsumerProxy> &consumer)
{
    keyCode_ = keyCode;
    if (consumer != nullptr) {
        consumer->OnKeyCodeConsumeResult(true);
    }
    return true;
}

bool KeyboardListenerTestImpl::OnDealKeyEvent(const std::shared_ptr<MMI::KeyEvent> &keyEvent,
    sptr<KeyEventConsumerProxy> &consumer)
{
    bool isKeyCodeConsume = OnKeyEvent(keyEvent->GetKeyCode(), keyEvent->GetKeyAction(), consumer);
    bool isKeyEventConsume = OnKeyEvent(keyEvent, consumer);
    if (consumer != nullptr) {
        consumer->OnKeyEventResult(isKeyEventConsume || isKeyCodeConsume);
    }
    return true;
}

void KeyboardListenerTestImpl::OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height)
{
    cursorHeight_ = height;
    kdListenerCv_.notify_one();
}
void KeyboardListenerTestImpl::OnSelectionChange(int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd)
{
    newBegin_ = newBegin;
    kdListenerCv_.notify_one();
}
void KeyboardListenerTestImpl::OnTextChange(const std::string &text)
{
    text_ = text;
    kdListenerCv_.notify_one();
}
void KeyboardListenerTestImpl::OnEditorAttributeChange(const InputAttribute &inputAttribute)
{
    inputAttribute_ = inputAttribute;
    kdListenerCv_.notify_one();
}
void KeyboardListenerTestImpl::ResetParam()
{
    keyCode_ = -1;
    cursorHeight_ = -1;
    newBegin_ = -1;
    text_ = "";
    inputAttribute_.inputPattern = 0;
    inputAttribute_.enterKeyType = 0;
    inputAttribute_.inputOption = 0;
}
bool KeyboardListenerTestImpl::WaitKeyEvent(int32_t keyCode)
{
    std::unique_lock<std::mutex> lock(kdListenerLock_);
    kdListenerCv_.wait_for(lock, std::chrono::seconds(1), [&keyCode]() { return keyCode == keyCode_; });
    return keyCode == keyCode_;
}
bool KeyboardListenerTestImpl::WaitCursorUpdate()
{
    std::unique_lock<std::mutex> lock(kdListenerLock_);
    kdListenerCv_.wait_for(lock, std::chrono::seconds(1), []() { return cursorHeight_ > 0; });
    return cursorHeight_ > 0;
}
bool KeyboardListenerTestImpl::WaitSelectionChange(int32_t newBegin)
{
    std::unique_lock<std::mutex> lock(kdListenerLock_);
    kdListenerCv_.wait_for(lock, std::chrono::seconds(1), [&newBegin]() { return newBegin == newBegin_; });
    return newBegin == newBegin_;
}
bool KeyboardListenerTestImpl::WaitTextChange(const std::string &text)
{
    std::unique_lock<std::mutex> lock(kdListenerLock_);
    kdListenerCv_.wait_for(lock, std::chrono::seconds(1), [&text]() { return text == text_; });
    return text == text_;
}
bool KeyboardListenerTestImpl::WaitEditorAttributeChange(const InputAttribute &inputAttribute)
{
    std::unique_lock<std::mutex> lock(kdListenerLock_);
    kdListenerCv_.wait_for(lock, std::chrono::seconds(1),
        [&inputAttribute]() { return inputAttribute == inputAttribute_; });
    return inputAttribute == inputAttribute_;
}
} // namespace MiscServices
} // namespace OHOS
