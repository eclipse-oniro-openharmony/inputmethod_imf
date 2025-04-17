/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_IMF_IME_EVENT_LISTENER_MANAGER_H
#define INPUTMETHOD_IMF_IME_EVENT_LISTENER_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "global.h"
#include "iinput_client.h"
#include "input_death_recipient.h"

namespace OHOS {
namespace MiscServices {
struct ImeEventListenerInfo {
    static constexpr uint32_t NO_EVENT_ON = 0;
    uint32_t eventFlag{ NO_EVENT_ON };    // the flag of the all listen event
    sptr<IInputClient> client{ nullptr }; // the remote object handler for service to callback input client
    int64_t pid = 0;
    sptr<InputDeathRecipient> deathRecipient { nullptr }; // death recipient of client
};
class ImeEventListenerManager {
public:
    static ImeEventListenerManager &GetInstance();
    int32_t UpdateListenerInfo(int32_t userId, const ImeEventListenerInfo &info);
    int32_t NotifyInputStart(int32_t userId, int32_t callingWndId, int32_t requestKeyboardReason = 0);
    int32_t NotifyInputStop(int32_t userId);
    int32_t NotifyPanelStatusChange(int32_t userId, const InputWindowStatus &status, const ImeWindowInfo &info);
    int32_t NotifyImeChange(int32_t userId, const Property &property, const SubProperty &subProperty);

private:
    ImeEventListenerManager() = default;
    ~ImeEventListenerManager() = default;
    std::mutex imeEventListenersLock_;
    std::map<int32_t, std::vector<ImeEventListenerInfo>> imeEventListeners_;
    void OnListenerDied(int32_t userId, const wptr<IRemoteObject> &object);
    int32_t GenerateListenerDeath(int32_t userId, ImeEventListenerInfo &listenerInfo);
    std::vector<ImeEventListenerInfo> GetListenerInfo(int32_t userId);
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_IMF_IME_EVENT_LISTENER_MANAGER_H
