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

#include <algorithm>

#include "global.h"
#include "ime_event_monitor_manager_impl.h"

namespace OHOS {
namespace MiscServices {
const std::set<EventType> ImeEventMonitorManager::EVENT_TYPE{ IME_SHOW, IME_HIDE };
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
    return ImeEventMonitorManagerImpl::GetInstance().RegisterImeEventListener(types, listener);
}

int32_t ImeEventMonitorManager::UnRegisterImeEventListener(
    const std::set<EventType> &types, const std::shared_ptr<ImeEventListener> &listener)
{
    if (!IsParamValid(types, listener)) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    return ImeEventMonitorManagerImpl::GetInstance().UnRegisterImeEventListener(types, listener);
}

bool ImeEventMonitorManager::IsParamValid(
    const std::set<EventType> &types, const std::shared_ptr<ImeEventListener> &listener)
{
    if (listener == nullptr) {
        IMSA_HILOGE("listener is nullptr");
        return false;
    }
    if (types.empty()) {
        IMSA_HILOGE("no eventType");
        return false;
    }
    if (types.size() > EVENT_NUM) {
        IMSA_HILOGE("over eventNum");
        return false;
    }
    auto it = std::find_if(
        types.begin(), types.end(), [](EventType type) { return EVENT_TYPE.find(type) == EVENT_TYPE.end(); });
    return it == types.end();
}
} // namespace MiscServices
} // namespace OHOS