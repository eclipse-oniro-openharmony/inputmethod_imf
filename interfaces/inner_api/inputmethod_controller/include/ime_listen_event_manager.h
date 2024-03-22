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

#include "input_method_setting_listener.h"
#include "event_status_manager.h"
#include "visibility.h"
#include "input_client_stub.h"

namespace OHOS {
namespace MiscServices {
enum EventType : uint32_t {
    IME_CHANGE = 0,
    IME_SHOW = 1,
    IME_HIDE = 2,
    IME_NONE
};

class ImeListenEventManager {
friend class InputClientStub;
public:
    IMF_API static sptr<ImeListenEventManager> GetInstance();
    IMF_API int32_t RegisterImeEventListener(EventType type, std::shared_ptr<ImeEventListener> listener);
    IMF_API void RegisterImeEventListener(const std::vector<EventType> types, std::shared_ptr<ImeEventListener> listener);
    IMF_API int32_t UnRegisterImeEventListener(EventType type, std::shared_ptr<ImeEventListener> listener);
    IMF_API void UnRegisterImeEventListener(const std::vector<EventType> types, std::shared_ptr<ImeEventListener> listener);
    /**
     * @brief Inform the change of ime to client.
     *
     * This function is used to inform the change of ime to client.
     *
     * @param subProperty Indicates the sub property of ime.
     * @param property Indicates the property of ime.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    IMF_API int32_t OnImeChange(const Property &property, const SubProperty &subProperty);

    /**
     * @brief Inform the change panel status.
     *
     * This function is used to inform the change panel status.
     *
     * @param status Indicates the status of panel.
     * @param windowInfo Indicates the detailed info of window.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    IMF_API int32_t OnPanelStatusChange(const InputWindowStatus &status, const PanelTotalInfo &info);

private:
    ImeListenEventManager();
    ~ImeListenEventManager();

    static std::mutex instanceLock_;
    static sptr<ImeListenEventManager> instance_;
    std::map<EventType, std::vector<std::shared_ptr<ImeEventListener>>> imeEventListeners_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INTERFACE_KITS_JS_GETINPUT_METHOD_SETTING_H