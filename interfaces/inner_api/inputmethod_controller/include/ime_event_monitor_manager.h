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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_IME_EVENT_MONITOR_MANAGER_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_IME_EVENT_MONITOR_MANAGER_H
#include <map>
#include <set>

#include "ime_event_listener.h"
#include "visibility.h"

namespace OHOS {
namespace MiscServices {
class ImeEventMonitorManager {
public:
    /**
     * @brief Get the instance of ImeEventMonitorManager.
     *
     * This function is used to get the instance of ImeEventMonitorManager.
     *
     * @return The instance of ImeEventMonitorManager.
     * @since 12
    */
    IMF_API static ImeEventMonitorManager &GetInstance();

    /**
     * @brief Register Ime Event Listener.
     *
     * This function is used to Register Ime Event Listener, only IME_SHOW and IME_HIDE supported at present
     *
     * @param types Indicates the event type.
     * @param listener Indicates the the listener to be registered.
     * @return Returns 0 for success, others for failure.
     * @since 12
    */
    IMF_API int32_t RegisterImeEventListener(
        const std::set<EventType> &types, const std::shared_ptr<ImeEventListener> &listener);

    /**
     * @brief UnRegister Ime Event Listener.
     *
     * This function is used to UnRegister Ime Event Listener, only IME_SHOW and IME_HIDE supported at present
     *
     * @param types Indicates the event type.
     * @param listener Indicates the the listener to be unregistered.
     * @return Returns 0 for success, others for failure.
     * @since 12
    */
    IMF_API int32_t UnRegisterImeEventListener(
        const std::set<EventType> &types, const std::shared_ptr<ImeEventListener> &listener);

private:
    static constexpr uint32_t EVENT_NUM = 2;
    static const std::set<EventType> EVENT_TYPE;
    ImeEventMonitorManager();
    ~ImeEventMonitorManager();
    bool IsParamValid(const std::set<EventType> &types, const std::shared_ptr<ImeEventListener> &listener);
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_IME_EVENT_MONITOR_MANAGER_H