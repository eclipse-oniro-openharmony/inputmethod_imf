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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_IME_LISTEN_EVENT_MANAGER_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_IME_LISTEN_EVENT_MANAGER_H
#include <map>
#include <set>

#include "event_status_manager.h"
#include "ime_event_listener.h"
#include "input_client_stub.h"
#include "visibility.h"

namespace OHOS {
namespace MiscServices {
enum EventType : uint32_t { IME_CHANGE = 0, IME_SHOW = 1, IME_HIDE = 2, IME_NONE };

class ImeEventMonitorManager {
public:
    /**
     * @brief Get the instance of TmeEventMonitorManager.
     *
     * This function is used to get the instance of TmeEventMonitorManager.
     *
     * @return The instance of TmeEventMonitorManager.
     * @since 11
    */
    IMF_API static ImeEventMonitorManager &GetInstance();

    /**
     * @brief Register Ime Event Listener.
     *
     * This function is used to Register Ime Event Listener.
     *
     * @param types Indicates the event type.
     * @param listener Indicates the the listener to be registered.
     * @return Returns 0 for success, others for failure.
     * @since 11
    */
    IMF_API int32_t RegisterImeEventListener(
        const std::set<EventType> &types, const std::shared_ptr<ImeEventListener> &listener);

    /**
     * @brief UnRegister Ime Event Listener.
     *
     * This function is used to UnRegister Ime Event Listener.
     *
     * @param types Indicates the event type.
     * @param listener Indicates the the listener to be unregistered.
     * @return Returns 0 for success, others for failure.
     * @since 11
    */
    IMF_API int32_t UnRegisterImeEventListener(
        const std::set<EventType> &types, const std::shared_ptr<ImeEventListener> &listener);
private:
    friend class InputClientStub;
    ImeEventMonitorManager();
    ~ImeEventMonitorManager();
    std::set<std::shared_ptr<ImeEventListener>>GetListeners(EventType type);
    bool IsParamValid(const std::set<EventType> &types, const std::shared_ptr<ImeEventListener> &listener);
    int32_t OnImeChange(const Property &property, const SubProperty &subProperty);
    int32_t OnPanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info);

    std::mutex lock_;
    std::map<EventType, std::set<std::shared_ptr<ImeEventListener>>> listeners_{};
};
} // namespace MiscServices
} // namespace OHOS
#endif // INTERFACE_KITS_JS_GETINPUT_METHOD_SETTING_H