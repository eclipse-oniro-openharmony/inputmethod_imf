/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "ime_listen_event_manager.h"

#include "input_method_controller.h"

namespace OHOS {
namespace MiscServices {
std::mutex ImeListenEventManager::instanceLock_;
sptr<InputMethodController> ImeListenEventManager::instance_;
ImeListenEventManager::ImeListenEventManager()
{
}

ImeListenEventManager::~ImeListenEventManager()
{
}

sptr<ImeListenEventManager> ImeListenEventManager::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            IMSA_HILOGD("IMC instance_ is nullptr");
            instance_ = new (std::nothrow) ImeListenEventManager();
            if (instance_ == nullptr) {
                IMSA_HILOGE("failed to create ImeListenEventManager");
                return instance_;
            }
        }
    }
    return instance_;
}

int32_t ImeListenEventManager::RegisterImeEventListener(EventType eventType, std::shared_ptr<ImeEventListener> listener)
{
    if (!imeEventListeners_.empty()) {
        auto it = imeEventListeners_.find(eventType);
        if (it != imeEventListeners_.end()) {
            if (std::find(it->second.begin(), it->second.end(), listener) == it->second.end()) {
                imeEventListeners_[eventType].push_back(std::move(listener));
            } else {
                //return ErrorCode;
            }
        } else {
            imeEventListeners_[eventType].push_back(std::move(listener));
        }
    } else {
        imeEventListeners_[eventType].push_back(std::move(listener));
    }
    return InputMethodController::GetInstance()->UpdateListenEventFlag(eventType, true);
}

int32_t ImeListenEventManager::UnRegisterImeEventListener(EventType eventType, std::shared_ptr<ImeEventListener> listener)
{
    auto it = std::find_if(imeEventListeners_[eventType].begin(), imeEventListeners_[eventType].end(),
        [&listener](const std::shared_ptr<ImeEventListener>& l) {
            return l.get() == listener.get();
        });
    if (it != imeEventListeners_[eventType].end()) {
        imeEventListeners_[eventType].erase(it);
    }
    if (imeEventListeners_[eventType].empty()) {
        imeEventListeners_.erase(eventType);
    }
    return InputMethodController::GetInstance()->UpdateListenEventFlag(eventType, false);
}

int32_t ImeListenEventManager::RegisterImeEventListener(const std::vector<EventType> types, std::shared_ptr<ImeEventListener> listener)
{

}

int32_t ImeListenEventManager::UnRegisterImeEventListener(const std::vector<EventType> types, std::shared_ptr<ImeEventListener> listener)
{

}

int32_t ImeListenEventManager::OnImeChange(const Property &property, const SubProperty &subProperty)
{
    if (imeEventListeners_ == nullptr) {
        IMSA_HILOGE("imeEventListeners_ is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    imeEventListeners_->OnImeChange(property, subProperty);
    return ErrorCode::NO_ERROR;
}

int32_t ImeListenEventManager::OnPanelStatusChange(const InputWindowStatus &status, const PanelTotalInfo &info)
{
    if (imeEventListeners_ == nullptr) {
        IMSA_HILOGE("imeEventListeners_ is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (status ==  InputWindowStatus::HIDE){
        imeEventListeners_->OnImeHide(info);
    } else {
        imeEventListeners_->OnImeShow(info);
    }
    return ErrorCode::NO_ERROR;
}

} // namespace MiscServices
} // namespace OHOS