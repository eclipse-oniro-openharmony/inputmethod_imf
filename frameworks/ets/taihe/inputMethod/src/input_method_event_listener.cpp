/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "input_method_event_listener.h"

#include "input_method_setting_impl.h"
namespace OHOS {
namespace MiscServices {
std::mutex InputMethodEventListener::listenerMutex_;
std::shared_ptr<InputMethodEventListener> InputMethodEventListener::inputMethodListener_{ nullptr };
std::shared_ptr<InputMethodEventListener> InputMethodEventListener::GetInstance()
{
    if (inputMethodListener_ == nullptr) {
        std::lock_guard<std::mutex> lock(listenerMutex_);
        if (inputMethodListener_ == nullptr) {
            inputMethodListener_ = std::make_shared<InputMethodEventListener>();
        }
    }
    return inputMethodListener_;
}
void InputMethodEventListener::OnImeChange(const Property &property, const SubProperty &subProperty)
{
    InputMethodSettingImpl::GetInstance().OnImeChangeCallback(property, subProperty);
}

void InputMethodEventListener::OnImeShow(const ImeWindowInfo &info)
{
    InputMethodSettingImpl::GetInstance().OnImeShowCallback(info);
}

void InputMethodEventListener::OnImeHide(const ImeWindowInfo &info)
{
    InputMethodSettingImpl::GetInstance().OnImeHideCallback(info);
}
} // namespace MiscServices
} // namespace OHOS