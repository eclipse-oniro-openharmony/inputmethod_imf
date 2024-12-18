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

#include "input_method_engine_listener_impl.h"

#include "global.h"
#include "input_method_utils.h"

namespace OHOS {
namespace MiscServices {
bool InputMethodEngineListenerImpl::keyboardState_ = false;
bool InputMethodEngineListenerImpl::isInputStart_ = false;
uint32_t InputMethodEngineListenerImpl::windowId_ = 0;
std::mutex InputMethodEngineListenerImpl::imeListenerMutex_;
std::condition_variable InputMethodEngineListenerImpl::imeListenerCv_;
bool InputMethodEngineListenerImpl::isEnable_ { false };
bool InputMethodEngineListenerImpl::isInputFinish_ { false };
std::unordered_map<std::string, PrivateDataValue> InputMethodEngineListenerImpl::privateCommand_ {};
ArrayBuffer InputMethodEngineListenerImpl::arrayBuffer_;
bool InputMethodEngineListenerImpl::isArrayBufferCallback_ = false;
constexpr int32_t TIMEOUT_SECONDS = 2;

void InputMethodEngineListenerImpl::OnKeyboardStatus(bool isShow)
{
    IMSA_HILOGI("InputMethodEngineListenerImpl::OnKeyboardStatus %{public}s", isShow ? "show" : "hide");
    keyboardState_ = isShow;
}

void InputMethodEngineListenerImpl::OnSecurityChange(int32_t security)
{
    IMSA_HILOGI("InputMethodEngineListenerImpl::OnSecurityChange %{public}d", security);
}

void InputMethodEngineListenerImpl::OnInputStart()
{
    IMSA_HILOGI("InputMethodEngineListenerImpl::OnInputStart");
    isInputStart_ = true;
    imeListenerCv_.notify_one();
}

int32_t InputMethodEngineListenerImpl::OnInputStop()
{
    IMSA_HILOGI("InputMethodEngineListenerImpl::OnInputStop");
    return ErrorCode::NO_ERROR;
}

void InputMethodEngineListenerImpl::OnSetCallingWindow(uint32_t windowId)
{
    IMSA_HILOGI("InputMethodEngineListenerImpl::OnSetCallingWindow %{public}d", windowId);
    windowId_ = windowId;
    imeListenerCv_.notify_one();
}

void InputMethodEngineListenerImpl::OnSetSubtype(const SubProperty &property)
{
    IMSA_HILOGI("InputMethodEngineListenerImpl::OnSetSubtype");
}

void InputMethodEngineListenerImpl::OnInputFinish()
{
    IMSA_HILOGI("InputMethodEngineListenerImpl");
    isInputFinish_ = true;
    imeListenerCv_.notify_one();
}

void InputMethodEngineListenerImpl::ReceivePrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    IMSA_HILOGI("InputMethodEngineListenerImpl");
    privateCommand_ = privateCommand;
    imeListenerCv_.notify_one();
}

bool InputMethodEngineListenerImpl::IsEnable()
{
    IMSA_HILOGI("InputMethodEngineListenerImpl isEnable: %{public}d", isEnable_);
    return isEnable_;
}

void InputMethodEngineListenerImpl::ResetParam()
{
    isInputStart_ = false;
    isInputFinish_ = false;
    windowId_ = 0;
    privateCommand_.clear();
    arrayBuffer_.msgId.clear();
    arrayBuffer_.msgParam.clear();
}

bool InputMethodEngineListenerImpl::WaitInputStart()
{
    std::unique_lock<std::mutex> lock(imeListenerMutex_);
    imeListenerCv_.wait_for(lock, std::chrono::seconds(TIMEOUT_SECONDS), []() {
        return isInputStart_;
    });
    return isInputStart_;
}

bool InputMethodEngineListenerImpl::WaitInputFinish()
{
    std::unique_lock<std::mutex> lock(imeListenerMutex_);
    imeListenerCv_.wait_for(lock, std::chrono::seconds(TIMEOUT_SECONDS), []() {
        return isInputFinish_;
    });
    return isInputFinish_;
}

bool InputMethodEngineListenerImpl::WaitSetCallingWindow(uint32_t windowId)
{
    std::unique_lock<std::mutex> lock(imeListenerMutex_);
    imeListenerCv_.wait_for(lock, std::chrono::seconds(1), [&windowId]() {
        return windowId_ == windowId;
    });
    return windowId_ == windowId;
}

bool InputMethodEngineListenerImpl::WaitSendPrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    std::unique_lock<std::mutex> lock(imeListenerMutex_);
    imeListenerCv_.wait_for(lock, std::chrono::seconds(1), [&privateCommand]() {
        return privateCommand_ == privateCommand;
    });

    return privateCommand_ == privateCommand;
}

bool InputMethodEngineListenerImpl::WaitSendMessage(const ArrayBuffer &arrayBuffer)
{
    std::string msgParam(arrayBuffer_.msgParam.begin(), arrayBuffer_.msgParam.end());
    std::string msgParam1(arrayBuffer.msgParam.begin(), arrayBuffer.msgParam.end());
    IMSA_HILOGE("arrayBuffer_ msgId: %{public}s, msgParam: %{publid}s", arrayBuffer_.msgId.c_str(), msgParam.c_str());
    IMSA_HILOGE("arrayBuffer msgId: %{public}s, msgParam: %{publid}s", arrayBuffer.msgId.c_str(), msgParam1.c_str());
    std::unique_lock<std::mutex> lock(imeListenerMutex_);
    if (isArrayBufferCallback_ && arrayBuffer_ == arrayBuffer) {
        isArrayBufferCallback_ = false;
        return true;
    }
    imeListenerCv_.wait_for(lock, std::chrono::seconds(1), [&arrayBuffer]() {
        return arrayBuffer_ == arrayBuffer;
    });
    isArrayBufferCallback_ = false;
    return arrayBuffer_ == arrayBuffer;
}

bool InputMethodEngineListenerImpl::WaitKeyboardStatus(bool state)
{
    std::unique_lock<std::mutex> lock(imeListenerMutex_);
    imeListenerCv_.wait_for(lock, std::chrono::seconds(1), [state]() {
        return keyboardState_ == state;
    });
    return keyboardState_ == state;
}

bool InputMethodEngineListenerImpl::PostTaskToEventHandler(std::function<void()> task, const std::string &taskName)
{
    if (eventHandler_ == nullptr) {
        return false;
    }
    eventHandler_->PostTask(
        [task]() {
            task();
        },
        taskName, 0);
    return true;
}

int32_t InputMethodEngineListenerImpl::OnMessage(const ArrayBuffer &arrayBuffer)
{
    std::unique_lock<std::mutex> lock(imeListenerMutex_);
    IMSA_HILOGI("OnMessage");
    arrayBuffer_ = arrayBuffer;
    isArrayBufferCallback_ = true;
    imeListenerCv_.notify_one();
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS
