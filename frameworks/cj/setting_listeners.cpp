/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "setting_listeners.h"

#include "ime_event_monitor_manager_impl.h"
#include "cj_lambda.h"
#include "utils.h"

namespace OHOS::MiscServices {

std::mutex CJGetInputMethodSetting::msMutex_;
std::shared_ptr<AppExecFwk::EventHandler> CJGetInputMethodSetting::handler_{ nullptr };
std::mutex CJGetInputMethodSetting::eventHandlerMutex_;
std::shared_ptr<CJGetInputMethodSetting> CJGetInputMethodSetting::inputMethod_{ nullptr };

std::shared_ptr<CJGetInputMethodSetting> CJGetInputMethodSetting::GetIMSInstance()
{
    if (inputMethod_ == nullptr) {
        std::lock_guard<std::mutex> lock(msMutex_);
        if (inputMethod_ == nullptr) {
            auto engine = std::make_shared<CJGetInputMethodSetting>();
            if (engine == nullptr) {
                IMSA_HILOGE("engine is nullptr!");
                return nullptr;
            }
            inputMethod_ = engine;
        }
    }
    return inputMethod_;
}

int32_t CJGetInputMethodSetting::Subscribe(uint32_t type, void (*func)(CInputMethodProperty, CInputMethodSubtype))
{
    auto engine = CJGetInputMethodSetting::GetIMSInstance();
    if (engine == nullptr) {
        return 0;
    }

    auto ret = ImeEventMonitorManagerImpl::GetInstance().RegisterImeEventListener(type, inputMethod_);
    if (ret == ErrorCode::NO_ERROR) {
        callback = CJLambda::Create(func);
    }
    return ret;
}

int32_t CJGetInputMethodSetting::UnSubscribe(uint32_t type)
{
    auto engine = CJGetInputMethodSetting::GetIMSInstance();
    if (engine == nullptr) {
        return 0;
    }
    auto ret = ImeEventMonitorManagerImpl::GetInstance().UnRegisterImeEventListener(type, inputMethod_);
    IMSA_HILOGI("UpdateListenEventFlag, ret: %{public}d, type: %{public}d.", ret, type);
    return ret;
}

void CJGetInputMethodSetting::OnImeChange(const Property &property, const SubProperty &subProperty)
{
    std::string type = "imeChange";
    auto eventHandler = CJGetInputMethodSetting::GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGI("start");
    auto task = [property, subProperty, this]() {
        CInputMethodProperty prop;
        CInputMethodSubtype subProp;
        Utils::InputMethodSubProperty2C(&subProp, subProperty);
        Utils::InputMethodProperty2C(&prop, property);
        callback(prop, subProp);
    };
    eventHandler->PostTask(task, type);
}

std::shared_ptr<AppExecFwk::EventHandler> CJGetInputMethodSetting::GetEventHandler()
{
    if (handler_ == nullptr) {
        std::lock_guard<std::mutex> lock(eventHandlerMutex_);
        handler_ = AppExecFwk::EventHandler::Current();
    }
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    return handler_;
}
}