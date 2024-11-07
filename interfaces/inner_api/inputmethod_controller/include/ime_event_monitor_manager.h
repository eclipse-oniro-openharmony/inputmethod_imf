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
#include <memory>
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
     * This function is used to Register Ime Event Listener, only IME_SHOW, IME_HIDE,
     * INPUT_STATUS_CHANGED supported at present
     *
     * @param eventFlag Indicates the flag of the ime event to be registered
     * @param listener Indicates the the listener to be registered.
     * @return Returns 0 for success, others for failure.
     * @since 12
    */
    IMF_API int32_t RegisterImeEventListener(uint32_t eventFlag, const std::shared_ptr<ImeEventListener> &listener);

    /**
     * @brief UnRegister Ime Event Listener.
     *
     * This function is used to UnRegister Ime Event Listener, only IME_SHOW and IME_HIDE,
     * INPUT_STATUS_CHANGED supported at present
     *
     * @param types Indicates the flag of the ime event to be unRegistered
     * @param listener Indicates the the listener to be unregistered.
     * @return Returns 0 for success, others for failure.
     * @since 12
    */
    IMF_API int32_t UnRegisterImeEventListener(uint32_t eventFlag, const std::shared_ptr<ImeEventListener> &listener);

private:
    static constexpr uint32_t ALL_EVENT_MASK = EVENT_IME_SHOW_MASK | EVENT_IME_HIDE_MASK |
        EVENT_INPUT_STATUS_CHANGED_MASK;
    ImeEventMonitorManager();
    ~ImeEventMonitorManager();
    bool IsParamValid(uint32_t eventFlag, const std::shared_ptr<ImeEventListener> &listener);
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_IME_EVENT_MONITOR_MANAGER_H