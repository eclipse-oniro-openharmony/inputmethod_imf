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

#ifndef INPUTMETHOD_IMF_INPUT_METHOD_ENGINE_LISTENER_IMPL_H
#define INPUTMETHOD_IMF_INPUT_METHOD_ENGINE_LISTENER_IMPL_H

#include <condition_variable>

#include "event_handler.h"
#include "input_method_engine_listener.h"

namespace OHOS {
namespace MiscServices {
class InputMethodEngineListenerImpl : public InputMethodEngineListener {
public:
    InputMethodEngineListenerImpl() {};
    ~InputMethodEngineListenerImpl() {};
    explicit InputMethodEngineListenerImpl(std::shared_ptr<AppExecFwk::EventHandler> handler) : eventHandler_(handler)
    {
    }
    static bool keyboardState_;
    static bool isInputStart_;
    static uint32_t windowId_;
    static std::mutex imeListenerMutex_;
    static std::condition_variable imeListenerCv_;
    static bool isEnable_;
    static bool isInputFinish_;
    static std::unordered_map<std::string, PrivateDataValue> privateCommand_;
    static ArrayBuffer arrayBuffer_;
    static ArrayBuffer timingArrayBuffer_;
    static void ResetParam();
    static bool WaitInputStart();
    static bool WaitInputFinish();
    static bool WaitSetCallingWindow(uint32_t windowId);
    static bool WaitSendPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand);
    static bool WaitSendMessage(const ArrayBuffer &arrayBuffer);
    static bool WaitKeyboardStatus(bool state);
    static ArrayBuffer GetTimingArrayBuffer();
    void OnKeyboardStatus(bool isShow) override;
    void OnInputStart() override;
    int32_t OnInputStop() override;
    void OnSecurityChange(int32_t security) override;
    void OnSetCallingWindow(uint32_t windowId) override;
    void OnSetSubtype(const SubProperty &property) override;
    void OnInputFinish() override;
    void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;
    bool IsEnable() override;
    bool PostTaskToEventHandler(std::function<void()> task, const std::string &taskName) override;
    int32_t OnMessage(const ArrayBuffer &arrayBuffer) override;

private:
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    static bool isArrayBufferCallback_ ;
};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_INPUT_METHOD_ENGINE_LISTENER_IMPL_H
