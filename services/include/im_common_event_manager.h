/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef SERVICES_INCLUDE_IM_COMMON_EVENT_MANAGER_H
#define SERVICES_INCLUDE_IM_COMMON_EVENT_MANAGER_H
#include "common_event_manager.h"
#include "common_event_support.h"
#include "focus_monitor_manager.h"
#include "input_window_info.h"
#include "keyboard_event.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace MiscServices {
using Handler = std::function<void()>;
class ImCommonEventManager : public RefBase {
public:
    ImCommonEventManager();
    ~ImCommonEventManager();
    static sptr<ImCommonEventManager> GetInstance();
    bool SubscribeEvent();
    bool SubscribeKeyboardEvent(const Handler &handler);
    bool SubscribeWindowManagerService(const Handler &handler);
    bool SubscribeMemMgrService(const Handler &handler);
    bool SubscribeAccountManagerService(Handler handle);
    bool UnsubscribeEvent();
    // only public the status change of softKeyboard in FLG_FIXED or FLG_FLOATING
    int32_t PublishPanelStatusChangeEvent(int32_t userId, const InputWindowStatus &status, const ImeWindowInfo &info);
    class EventSubscriber : public EventFwk::CommonEventSubscriber {
    public:
        EventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo);
        void OnReceiveEvent(const EventFwk::CommonEventData &data);
        void RemovePackage(const EventFwk::CommonEventData &data);
        void StartUser(const EventFwk::CommonEventData &data);
        void RemoveUser(const EventFwk::CommonEventData &data);
        void StopUser(const EventFwk::CommonEventData &data);
        void OnBundleScanFinished(const EventFwk::CommonEventData &data);
        void OnDataShareReady(const EventFwk::CommonEventData &data);
        void AddPackage(const EventFwk::CommonEventData &data);
        void ChangePackage(const EventFwk::CommonEventData &data);
        void HandleBootCompleted(const EventFwk::CommonEventData &data);
        void OnUserUnlocked(const EventFwk::CommonEventData &data);

    private:
        using EventListenerFunc = std::function<void(EventSubscriber *that, const EventFwk::CommonEventData &data)>;
        std::map<std::string, EventListenerFunc> EventManagerFunc_;
        void HandlePackageEvent(int32_t messageId, const EventFwk::CommonEventData &data);
        void HandleUserEvent(int32_t messageId, const EventFwk::CommonEventData &data);
    };

private:
    bool SubscribeManagerServiceCommon(const Handler &handler, int32_t saId);
    class SystemAbilityStatusChangeListener : public SystemAbilityStatusChangeStub {
    public:
        explicit SystemAbilityStatusChangeListener(std::function<void()>);
        ~SystemAbilityStatusChangeListener() = default;
        virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
        virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

    private:
        std::function<void()> func_ = nullptr;
    };

private:
    static std::mutex instanceLock_;
    static sptr<ImCommonEventManager> instance_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_IM_COMMON_EVENT_MANAGER_H
