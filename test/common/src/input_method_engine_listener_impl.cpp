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

namespace OHOS {
namespace MiscServices {
bool InputMethodEngineListenerImpl::keyboardState_ = false;
bool InputMethodEngineListenerImpl::isInputStart_ = false;
uint32_t InputMethodEngineListenerImpl::windowId_ = 0;
std::mutex InputMethodEngineListenerImpl::imeListenerMutex_;
std::condition_variable InputMethodEngineListenerImpl::imeListenerCv_;
void InputMethodEngineListenerImpl::OnKeyboardStatus(bool isShow)
{
    IMSA_HILOGI("InputMethodEngineListenerImpl::OnKeyboardStatus %{public}s", isShow ? "show" : "hide");
    keyboardState_ = isShow;
}
void InputMethodEngineListenerImpl::OnInputStart()
{
    IMSA_HILOGI("InputMethodEngineListenerImpl::OnInputStart");
    isInputStart_ = true;
}
void InputMethodEngineListenerImpl::OnInputStop(const std::string &imeId)
{
    IMSA_HILOGI("InputMethodEngineListenerImpl::OnInputStop %{public}s", imeId.c_str());
    isInputStart_ = false;
}
void InputMethodEngineListenerImpl::OnSetCallingWindow(uint32_t windowId)
{
    IMSA_HILOGI("InputMethodEngineListenerImpl::OnSetCallingWindow %{public}d", windowId);
    windowId_ = windowId;
}
void InputMethodEngineListenerImpl::OnSetSubtype(const SubProperty &property)
{
    IMSA_HILOGD("InputMethodEngineListenerImpl::OnSetSubtype");
}
} // namespace MiscServices
} // namespace OHOS
