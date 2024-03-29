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

#include "ime_event_monitor_manager.h"

#include "input_method_controller.h"

namespace OHOS {
namespace MiscServices {
;
ImeEventMonitorManager::ImeEventMonitorManager()
{
}

ImeEventMonitorManager::~ImeEventMonitorManager()
{
}

ImeEventMonitorManager &ImeEventMonitorManager::GetInstance()
{
    static ImeEventMonitorManager manager;
    return manager;
}

int32_t ImeEventMonitorManager::RegisterImeEventListener(
    const std::set<EventType> &types, const std::shared_ptr<ImeEventListener> &listener)
{
    if (!IsParamValid(types, listener)) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    std::lock_guard<std::mutex> lock(lock_);
    for (const auto &type : types) {
        auto it = listeners_.find(type);
        if (it == listeners_.end()) {
            auto ret = InputMethodController::GetInstance()->UpdateListenEventFlag(type, true);
            if (ret != ErrorCode::NO_ERROR) {
                IMSA_HILOGI("UpdateListenEventFlag failed: %{public}d", ret);
                return ret;
            }
            listeners_.insert({ type, { listener } });
        } else {
            it->second.insert(listener);
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEventMonitorManager::UnRegisterImeEventListener(
    const std::set<EventType> &types, const std::shared_ptr<ImeEventListener> &listener)
{
    if (!IsParamValid(types, listener)) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    bool isContainInvalidParam = false;
    std::lock_guard<std::mutex> lock(lock_);
    for (const auto &type : types) {
        auto it = listeners_.find(type);
        if (it == listeners_.end()) {
            isContainInvalidParam = true;
            continue;
        }
        auto iter = it->second.find(listener);
        if (iter == it->second.end()) {
            isContainInvalidParam = true;
            continue;
        }
        it->second.erase(iter);
        if (it->second.empty()) {
            auto ret = InputMethodController::GetInstance()->UpdateListenEventFlag(type, false);
            if (ret != ErrorCode::NO_ERROR) {
                IMSA_HILOGI("UpdateListenEventFlag failed: %{public}d", ret);
            }
            listeners_.erase(it);
        }
    }
    return isContainInvalidParam ? ErrorCode::ERROR_BAD_PARAMETERS : ErrorCode::NO_ERROR;
}

bool ImeEventMonitorManager::IsParamValid(
    const std::set<EventType> &types, const std::shared_ptr<ImeEventListener> &listener)
{
    if (listener == nullptr) {
        IMSA_HILOGI("listener is nullptr");
        return false;
    }
    if (types.size() > EventType::IME_NONE) {
        IMSA_HILOGI("over the max num");
        return false;
    }
    for (const auto &type : types) {
        if (type >= EventType::IME_NONE) {
            IMSA_HILOGI("eventType is error");
            return false;
        }
    }
    return true;
}

int32_t ImeEventMonitorManager::OnImeChange(const Property &property, const SubProperty &subProperty)
{
    auto listeners = GetListeners(EventType::IME_CHANGE);
    if (listeners.empty()) {
        IMSA_HILOGD("not has IME_CHANGE listeners");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    for (const auto &listener : listeners) {
        listener->OnImeChange(property, subProperty);
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEventMonitorManager::OnPanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info)
{
    if (status != InputWindowStatus::HIDE && status != InputWindowStatus::SHOW) {
        IMSA_HILOGE("status:%{public}d is invalid", status);
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto type = status == InputWindowStatus::HIDE ? EventType::IME_HIDE : EventType::IME_SHOW;
    auto listeners = GetListeners(type);
    if (listeners.empty()) {
        IMSA_HILOGD("not has %{public}d listeners", type);
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    for (const auto &listener : listeners) {
        if (type == EventType::IME_HIDE) {
            listener->OnImeHide(info);
        } else {
            listener->OnImeShow(info);
        }
    }
    return ErrorCode::NO_ERROR;
}

std::set<std::shared_ptr<ImeEventListener>> ImeEventMonitorManager::GetListeners(EventType type)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = listeners_.find(type);
    if (it == listeners_.end()) {
        return {};
    }
    return it->second;
}
} // namespace MiscServices
} // namespace OHOS