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

int32_t ImeEventMonitorManager::RegisterImeEventListener(uint32_t eventFlag,
    const std::shared_ptr<ImeEventListener> &listener)
{
    if (!IsParamValid(eventFlag, listener)) {
        IMSA_HILOGE("param is invalid");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    return ImeEventMonitorManagerImpl::GetInstance().RegisterImeEventListener(eventFlag & ALL_EVENT_MASK, listener);
}

int32_t ImeEventMonitorManager::UnRegisterImeEventListener(uint32_t eventFlag,
    const std::shared_ptr<ImeEventListener> &listener)
{
    if (!IsParamValid(eventFlag, listener)) {
        IMSA_HILOGE("param is invalid");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    return ImeEventMonitorManagerImpl::GetInstance().UnRegisterImeEventListener(eventFlag & ALL_EVENT_MASK, listener);
}
bool ImeEventMonitorManager::IsParamValid(uint32_t eventFlag, const std::shared_ptr<ImeEventListener> &listener)
{
    if (eventFlag == 0) {
        IMSA_HILOGE("eventFlag is 0");
        return false;
    }
    return listener != nullptr;
}
} // namespace MiscServices
} // namespace OHOS