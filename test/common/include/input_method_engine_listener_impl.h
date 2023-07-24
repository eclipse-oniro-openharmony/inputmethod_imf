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

#include "input_method_engine_listener.h"

namespace OHOS {
namespace MiscServices {
class InputMethodEngineListenerImpl : public InputMethodEngineListener {
public:
    InputMethodEngineListenerImpl(){};
    ~InputMethodEngineListenerImpl(){};
    static bool keyboardState_;
    static bool isInputStart_;
    static uint32_t windowId_;
    static std::mutex imeListenerMutex_;
    static std::condition_variable imeListenerCv_;
    void OnKeyboardStatus(bool isShow) override;
    void OnInputStart() override;
    void OnInputStop(const std::string &imeId) override;
    void OnSetCallingWindow(uint32_t windowId) override;
    void OnSetSubtype(const SubProperty &property) override;
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUTMETHOD_IMF_INPUT_METHOD_ENGINE_LISTENER_IMPL_H
