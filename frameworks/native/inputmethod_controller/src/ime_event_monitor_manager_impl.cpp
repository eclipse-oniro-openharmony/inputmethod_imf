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

#include "ime_event_monitor_manager_impl.h"

#include <algorithm>

#include "input_method_controller.h"

namespace OHOS {
namespace MiscServices {
ImeEventMonitorManagerImpl::ImeEventMonitorManagerImpl()
{
}

ImeEventMonitorManagerImpl::~ImeEventMonitorManagerImpl()
{
}

ImeEventMonitorManagerImpl &ImeEventMonitorManagerImpl::GetInstance()
{
    static ImeEventMonitorManagerImpl manager;
    return manager;
}

int32_t ImeEventMonitorManagerImpl::RegisterImeEventListener(
    const std::set<EventType> &types, const std::shared_ptr<ImeEventListener> &listener)
{
    std::lock_guard<std::mutex> lock(lock_);
    for (const auto &type : types) {
        auto it = listeners_.find(type);
        if (it == listeners_.end()) {
            auto ret = InputMethodController::GetInstance()->UpdateListenEventFlag(type, true);
            if (ret != ErrorCode::NO_ERROR) {
                IMSA_HILOGE("UpdateListenEventFlag failed: %{public}d", ret);
                return ret;
            }
            listeners_.insert({ type, { listener } });
        } else {
            it->second.insert(listener);
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEventMonitorManagerImpl::UnRegisterImeEventListener(
    const std::set<EventType> &types, const std::shared_ptr<ImeEventListener> &listener)
{
    bool isAbsentParam = false;
    std::lock_guard<std::mutex> lock(lock_);
    for (const auto &type : types) {
        auto it = listeners_.find(type);
        if (it == listeners_.end()) {
            isAbsentParam = true;
            continue;
        }
        auto iter = it->second.find(listener);
        if (iter == it->second.end()) {
            isAbsentParam = true;
            continue;
        }
        it->second.erase(iter);
        if (it->second.empty()) {
            auto ret = InputMethodController::GetInstance()->UpdateListenEventFlag(type, false);
            if (ret != ErrorCode::NO_ERROR) {
                IMSA_HILOGE("UpdateListenEventFlag failed: %{public}d", ret);
            }
            listeners_.erase(it);
        }
    }
    return isAbsentParam ? ErrorCode::ERROR_BAD_PARAMETERS : ErrorCode::NO_ERROR;
}

int32_t ImeEventMonitorManagerImpl::OnImeChange(const Property &property, const SubProperty &subProperty)
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

int32_t ImeEventMonitorManagerImpl::OnPanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info)
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

std::set<std::shared_ptr<ImeEventListener>> ImeEventMonitorManagerImpl::GetListeners(EventType type)
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