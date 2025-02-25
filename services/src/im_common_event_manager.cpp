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

#include "im_common_event_manager.h"
#include "full_ime_info_manager.h"
#include "ime_info_inquirer.h"
#include "iservice_registry.h"
#include "itypes_util.h"
#include "inputmethod_message_handler.h"
#include "os_account_adapter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
sptr<ImCommonEventManager> ImCommonEventManager::instance_;
std::mutex ImCommonEventManager::instanceLock_;
using namespace OHOS::EventFwk;
constexpr const char *COMMON_EVENT_INPUT_PANEL_STATUS_CHANGED = "usual.event.imf.input_panel_status_changed";
constexpr const char *COMMON_EVENT_PARAM_USER_ID = "userId";
constexpr const char *COMMON_EVENT_PARAM_PANEL_STATE = "panelState";
constexpr const char *COMMON_EVENT_PARAM_PANEL_RECT = "panelRect";
ImCommonEventManager::ImCommonEventManager()
{
}

ImCommonEventManager::~ImCommonEventManager()
{
}

sptr<ImCommonEventManager> ImCommonEventManager::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            IMSA_HILOGI("instance_ is nullptr.");
            instance_ = new ImCommonEventManager();
        }
    }
    return instance_;
}

bool ImCommonEventManager::SubscribeEvent()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_USER_REMOVED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_DATA_SHARE_READY);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_USER_STOPPED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);

    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<EventSubscriber> subscriber = std::make_shared<EventSubscriber>(subscriberInfo);
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        IMSA_HILOGE("SubscribeEvent abilityManager is nullptr!");
        return false;
    }
    sptr<ISystemAbilityStatusChange> listener = new (std::nothrow) SystemAbilityStatusChangeListener([subscriber]() {
        bool subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
        IMSA_HILOGI("SubscribeCommonEvent ret: %{public}d", subscribeResult);
    });
    if (listener == nullptr) {
        IMSA_HILOGE("SubscribeEvent listener is nullptr!");
        return false;
    }
    int32_t ret = abilityManager->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, listener);
    if (ret != ERR_OK) {
        IMSA_HILOGE("SubscribeEvent SubscribeSystemAbility failed. ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool ImCommonEventManager::SubscribeKeyboardEvent(const Handler &handler)
{
    IMSA_HILOGI("ImCommonEventManager::SubscribeKeyboardEvent start.");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        IMSA_HILOGE("SubscribeKeyboardEvent abilityManager is nullptr!");
        return false;
    }
    sptr<ISystemAbilityStatusChange> listener = new (std::nothrow) SystemAbilityStatusChangeListener([handler]() {
        if (handler != nullptr) {
            handler();
        }
    });
    if (listener == nullptr) {
        IMSA_HILOGE("listener is nullptr!");
        return false;
    }
    int32_t ret = abilityManager->SubscribeSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, listener);
    if (ret != ERR_OK) {
        IMSA_HILOGE("failed to SubscribeSystemAbility, ret: %{public}d!", ret);
        return false;
    }
    return true;
}

bool ImCommonEventManager::SubscribeWindowManagerService(const Handler &handler)
{
    IMSA_HILOGI("start.");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        IMSA_HILOGE("abilityManager is nullptr!");
        return false;
    }
    sptr<ISystemAbilityStatusChange> listener = new (std::nothrow) SystemAbilityStatusChangeListener([handler]() {
        if (handler != nullptr) {
            handler();
        }
    });
    if (listener == nullptr) {
        IMSA_HILOGE("failed to create listener!");
        return false;
    }
    int32_t ret = abilityManager->SubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, listener);
    if (ret != ERR_OK) {
        IMSA_HILOGE("subscribe system ability failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool ImCommonEventManager::SubscribeMemMgrService(const Handler &handler)
{
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        IMSA_HILOGE("abilityManager is nullptr!");
        return false;
    }
    sptr<ISystemAbilityStatusChange> listener = new (std::nothrow) SystemAbilityStatusChangeListener([handler]() {
        if (handler != nullptr) {
            handler();
        }
    });
    if (listener == nullptr) {
        IMSA_HILOGE("failed to create listener!");
        return false;
    }
    int32_t ret = abilityManager->SubscribeSystemAbility(MEMORY_MANAGER_SA_ID, listener);
    if (ret != ERR_OK) {
        IMSA_HILOGE("subscribe system ability failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool ImCommonEventManager::SubscribeAccountManagerService(Handler handler)
{
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        IMSA_HILOGE("abilityManager is nullptr!");
        return false;
    }
    sptr<ISystemAbilityStatusChange> listener = new (std::nothrow) SystemAbilityStatusChangeListener([handler]() {
        if (handler != nullptr) {
            handler();
        }
    });
    if (listener == nullptr) {
        IMSA_HILOGE("failed to create listener!");
        return false;
    }
    int32_t ret = abilityManager->SubscribeSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, listener);
    if (ret != ERR_OK) {
        IMSA_HILOGE("subscribe system ability failed, ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool ImCommonEventManager::UnsubscribeEvent()
{
    return true;
}

ImCommonEventManager::EventSubscriber::EventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo)
{
    EventManagerFunc_[CommonEventSupport::COMMON_EVENT_USER_SWITCHED] =
        [] (EventSubscriber *that, const EventFwk::CommonEventData &data) {
            return that->StartUser(data);
        };
    EventManagerFunc_[CommonEventSupport::COMMON_EVENT_USER_STOPPED] =
        [] (EventSubscriber *that, const EventFwk::CommonEventData &data) {
            return that->StopUser(data);
        };
    EventManagerFunc_[CommonEventSupport::COMMON_EVENT_USER_REMOVED] =
        [] (EventSubscriber *that, const EventFwk::CommonEventData &data) {
            return that->RemoveUser(data);
        };
    EventManagerFunc_[CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED] =
        [] (EventSubscriber *that, const EventFwk::CommonEventData &data) {
            return that->RemovePackage(data);
        };
    EventManagerFunc_[CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED] =
        [] (EventSubscriber *that, const EventFwk::CommonEventData &data) {
            return that->OnBundleScanFinished(data);
        };
    EventManagerFunc_[CommonEventSupport::COMMON_EVENT_DATA_SHARE_READY] =
        [] (EventSubscriber *that, const EventFwk::CommonEventData &data) {
            return that->OnDataShareReady(data);
        };
    EventManagerFunc_[CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED] =
        [] (EventSubscriber *that, const EventFwk::CommonEventData &data) {
            return that->AddPackage(data);
        };
    EventManagerFunc_[CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED] =
        [] (EventSubscriber *that, const EventFwk::CommonEventData &data) {
            return that->ChangePackage(data);
        };
    EventManagerFunc_[CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED] =
        [] (EventSubscriber *that, const EventFwk::CommonEventData &data) {
            return that->HandleBootCompleted(data);
        };
    EventManagerFunc_[CommonEventSupport::COMMON_EVENT_USER_UNLOCKED] =
        [] (EventSubscriber *that, const EventFwk::CommonEventData &data) {
            return that->OnUserUnlocked(data);
        };
}

void ImCommonEventManager::EventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    auto const &want = data.GetWant();
    std::string action = want.GetAction();
    IMSA_HILOGI("ImCommonEventManager::action: %{public}s!", action.c_str());
    auto iter = EventManagerFunc_.find(action);
    if (iter != EventManagerFunc_.end()) {
        EventManagerFunc_[action] (this, data);
    }
}

void ImCommonEventManager::EventSubscriber::StopUser(const CommonEventData &data)
{
    HandleUserEvent(MessageID::MSG_ID_USER_STOP, data);
}

void ImCommonEventManager::EventSubscriber::StartUser(const CommonEventData &data)
{
    HandleUserEvent(MessageID::MSG_ID_USER_START, data);
}

void ImCommonEventManager::EventSubscriber::RemoveUser(const CommonEventData &data)
{
    HandleUserEvent(MessageID::MSG_ID_USER_REMOVED, data);
}

void ImCommonEventManager::EventSubscriber::HandleUserEvent(int32_t messageId, const EventFwk::CommonEventData &data)
{
    auto userId = data.GetCode();
    MessageParcel *parcel = new (std::nothrow) MessageParcel();
    if (parcel == nullptr) {
        return;
    }
    IMSA_HILOGD("userId:%{public}d, messageId:%{public}d", userId, messageId);
    parcel->WriteInt32(userId);
    Message *msg = new (std::nothrow) Message(messageId, parcel);
    if (msg == nullptr) {
        delete parcel;
        return;
    }
    MessageHandler::Instance()->SendMessage(msg);
}

void ImCommonEventManager::EventSubscriber::OnBundleScanFinished(const EventFwk::CommonEventData &data)
{
    IMSA_HILOGI("ImCommonEventManager start.");
    auto parcel = new (std::nothrow) MessageParcel();
    if (parcel == nullptr) {
        IMSA_HILOGE("failed to create MessageParcel!");
        return;
    }
    auto msg = new (std::nothrow) Message(MessageID::MSG_ID_BUNDLE_SCAN_FINISHED, parcel);
    if (msg == nullptr) {
        IMSA_HILOGE("failed to create Message!");
        delete parcel;
        return;
    }
    MessageHandler::Instance()->SendMessage(msg);
}

void ImCommonEventManager::EventSubscriber::OnDataShareReady(const EventFwk::CommonEventData &data)
{
    IMSA_HILOGI("ImCommonEventManager start.");
    auto parcel = new (std::nothrow) MessageParcel();
    if (parcel == nullptr) {
        IMSA_HILOGE("failed to create MessageParcel!");
        return;
    }
    auto msg = new (std::nothrow) Message(MessageID::MSG_ID_DATA_SHARE_READY, parcel);
    if (msg == nullptr) {
        IMSA_HILOGE("failed to create Message!");
        delete parcel;
        return;
    }
    MessageHandler::Instance()->SendMessage(msg);
}

void ImCommonEventManager::EventSubscriber::RemovePackage(const CommonEventData &data)
{
    HandlePackageEvent(MessageID::MSG_ID_PACKAGE_REMOVED, data);
}

void ImCommonEventManager::EventSubscriber::AddPackage(const EventFwk::CommonEventData &data)
{
    HandlePackageEvent(MessageID::MSG_ID_PACKAGE_ADDED, data);
}

void ImCommonEventManager::EventSubscriber::ChangePackage(const EventFwk::CommonEventData &data)
{
    HandlePackageEvent(MessageID::MSG_ID_PACKAGE_CHANGED, data);
}

void ImCommonEventManager::EventSubscriber::HandlePackageEvent(int32_t messageId, const EventFwk::CommonEventData &data)
{
    auto const &want = data.GetWant();
    auto element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    int32_t userId = want.GetIntParam("userId", OsAccountAdapter::INVALID_USER_ID);
    if (userId == OsAccountAdapter::INVALID_USER_ID) {
        IMSA_HILOGE("invalid user id, messageId:%{public}d", messageId);
        return;
    }
    if (messageId == MessageID::MSG_ID_PACKAGE_REMOVED) {
        if (!FullImeInfoManager::GetInstance().Has(userId, bundleName)) {
            return;
        }
    } else {
        if (!ImeInfoInquirer::GetInstance().IsInputMethod(userId, bundleName)) {
            return;
        }
    }
    MessageParcel *parcel = new (std::nothrow) MessageParcel();
    if (parcel == nullptr) {
        IMSA_HILOGE("parcel is nullptr!");
        return;
    }
    if (!ITypesUtil::Marshal(*parcel, userId, bundleName)) {
        IMSA_HILOGE("Failed to write message parcel!");
        delete parcel;
        return;
    }
    Message *msg = new (std::nothrow) Message(messageId, parcel);
    if (msg == nullptr) {
        IMSA_HILOGE("failed to create Message!");
        delete parcel;
        return;
    }
    MessageHandler::Instance()->SendMessage(msg);
}

void ImCommonEventManager::EventSubscriber::HandleBootCompleted(const EventFwk::CommonEventData &data)
{
    Message *msg = new (std::nothrow) Message(MessageID::MSG_ID_BOOT_COMPLETED, nullptr);
    if (msg == nullptr) {
        return;
    }
    MessageHandler::Instance()->SendMessage(msg);
}

void ImCommonEventManager::EventSubscriber::OnUserUnlocked(const EventFwk::CommonEventData &data)
{
    MessageParcel *parcel = new (std::nothrow) MessageParcel();
    if (parcel == nullptr) {
        IMSA_HILOGE("parcel is nullptr!");
        return;
    }
    int32_t userId = data.GetCode();
    if (!ITypesUtil::Marshal(*parcel, userId)) {
        IMSA_HILOGE("Failed to write message parcel!");
        delete parcel;
        return;
    }
    Message *msg = new (std::nothrow) Message(MessageID::MSG_ID_USER_UNLOCKED, parcel);
    if (msg == nullptr) {
        IMSA_HILOGE("failed to create Message!");
        delete parcel;
        return;
    }
    MessageHandler::Instance()->SendMessage(msg);
}

ImCommonEventManager::SystemAbilityStatusChangeListener::SystemAbilityStatusChangeListener(std::function<void()> func)
    : func_(std::move(func))
{
}

void ImCommonEventManager::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    IMSA_HILOGD("systemAbilityId: %{public}d.", systemAbilityId);
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID && systemAbilityId != MULTIMODAL_INPUT_SERVICE_ID &&
        systemAbilityId != WINDOW_MANAGER_SERVICE_ID && systemAbilityId != SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN &&
        systemAbilityId != MEMORY_MANAGER_SA_ID) {
        return;
    }
    if (func_ != nullptr) {
        func_();
    }
}

void ImCommonEventManager::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
}

int32_t ImCommonEventManager::PublishPanelStatusChangeEvent(
    int32_t userId, const InputWindowStatus &status, const ImeWindowInfo &info)
{
    EventFwk::CommonEventPublishInfo publicInfo;
    publicInfo.SetOrdered(false);
    AAFwk::Want want;
    want.SetAction(COMMON_EVENT_INPUT_PANEL_STATUS_CHANGED);
    bool visible = (status == InputWindowStatus::SHOW);
    std::vector<int32_t> panelRect = { info.windowInfo.left, info.windowInfo.top,
        static_cast<int32_t>(info.windowInfo.width), static_cast<int32_t>(info.windowInfo.height) };
    want.SetParam(COMMON_EVENT_PARAM_USER_ID, userId);
    want.SetParam(COMMON_EVENT_PARAM_PANEL_STATE, visible);
    want.SetParam(COMMON_EVENT_PARAM_PANEL_RECT, panelRect);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    return EventFwk::CommonEventManager::NewPublishCommonEvent(data, publicInfo);
}
} // namespace MiscServices
} // namespace OHOS