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

#include <utility>

#include "global.h"
#include "input_method_system_ability_stub.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "itypes_util.h"
#include "message_handler.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
sptr<ImCommonEventManager> ImCommonEventManager::instance_;
std::mutex ImCommonEventManager::instanceLock_;
using namespace OHOS::EventFwk;
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
            IMSA_HILOGI("ImCommonEventManager::GetInstance instance_ is nullptr");
            instance_ = new ImCommonEventManager();
        }
    }
    return instance_;
}

bool ImCommonEventManager::SubscribeEvent(const std::string &event)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(event);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_USER_REMOVED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);

    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<EventSubscriber> subscriber = std::make_shared<EventSubscriber>(subscriberInfo);
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        IMSA_HILOGE("SubscribeEvent abilityManager is nullptr");
        return false;
    }
    sptr<ISystemAbilityStatusChange> listener = new (std::nothrow) SystemAbilityStatusChangeListener([subscriber]() {
        bool subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
        IMSA_HILOGI("ImCommonEventManager::OnAddSystemAbility subscribeResult = %{public}d", subscribeResult);
    });
    if (listener == nullptr) {
        IMSA_HILOGE("SubscribeEvent listener is nullptr");
        return false;
    }
    int32_t ret = abilityManager->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, listener);
    if (ret != ERR_OK) {
        IMSA_HILOGE("SubscribeEvent SubscribeSystemAbility failed. ret = %{public}d", ret);
        return false;
    }
    statusChangeListener_ = listener;
    return true;
}

bool ImCommonEventManager::SubscribeKeyboardEvent(KeyHandle handle)
{
    IMSA_HILOGI("ImCommonEventManager::SubscribeKeyboardEvent");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        IMSA_HILOGE("SubscribeKeyboardEvent abilityManager is nullptr");
        return false;
    }
    sptr<ISystemAbilityStatusChange> listener = new (std::nothrow) SystemAbilityStatusChangeListener([handle]() {
        int32_t ret = KeyboardEvent::GetInstance().AddKeyEventMonitor(handle);
        IMSA_HILOGI("SubscribeKeyboardEvent add monitor %{public}s", ret == ErrorCode::NO_ERROR ? "success" : "failed");
    });
    if (listener == nullptr) {
        IMSA_HILOGE("SubscribeKeyboardEvent listener is nullptr");
        return false;
    }
    int32_t ret = abilityManager->SubscribeSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, listener);
    if (ret != ERR_OK) {
        IMSA_HILOGE("SubscribeKeyboardEvent SubscribeSystemAbility failed. ret = %{public}d", ret);
        return false;
    }
    keyboardEventListener_ = listener;
    return true;
}

bool ImCommonEventManager::UnsubscribeEvent()
{
    return true;
}

ImCommonEventManager::EventSubscriber::EventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo)
{
    EventManagerFunc_[CommonEventSupport::COMMON_EVENT_USER_SWITCHED] = &EventSubscriber::StartUser;
    EventManagerFunc_[CommonEventSupport::COMMON_EVENT_USER_REMOVED] = &EventSubscriber::RemoveUser;
    EventManagerFunc_[CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED] = &EventSubscriber::RemovePackage;
}

void ImCommonEventManager::EventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    auto const &want = data.GetWant();
    std::string action = want.GetAction();
    IMSA_HILOGI("ImCommonEventManager::action = %{public}s!", action.c_str());
    auto iter = EventManagerFunc_.find(action);
    if (iter == EventManagerFunc_.end()) {
        return;
    }
    auto EventListenerFunc = iter->second;
    if (EventListenerFunc != nullptr) {
        (this->*EventListenerFunc)(data);
    }
}

void ImCommonEventManager::EventSubscriber::StartUser(const CommonEventData &data)
{
    auto newUserId = data.GetCode();
    IMSA_HILOGI("ImCommonEventManager::StartUser, userId = %{public}d", newUserId);
    MessageParcel *parcel = new MessageParcel();
    parcel->WriteInt32(newUserId);
    Message *msg = new Message(MessageID::MSG_ID_USER_START, parcel);
    MessageHandler::Instance()->SendMessage(msg);
}

void ImCommonEventManager::EventSubscriber::RemoveUser(const CommonEventData &data)
{
    auto userId = data.GetCode();
    IMSA_HILOGI("ImCommonEventManager::RemoveUser, userId = %{public}d", userId);
    MessageParcel *parcel = new MessageParcel();
    parcel->WriteInt32(userId);
    Message *msg = new Message(MessageID::MSG_ID_USER_REMOVED, parcel);
    MessageHandler::Instance()->SendMessage(msg);
}

void ImCommonEventManager::EventSubscriber::RemovePackage(const CommonEventData &data)
{
    auto const &want = data.GetWant();
    auto element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    int32_t userId = want.GetIntParam("userId", 0);
    IMSA_HILOGD("ImCommonEventManager::RemovePackage, bundleName = %{public}s, userId = %{public}d",
        bundleName.c_str(), userId);
    MessageParcel *parcel = new MessageParcel();
    if (!ITypesUtil::Marshal(*parcel, userId, bundleName)) {
        IMSA_HILOGE("Failed to write message parcel");
        return;
    }
    Message *msg = new Message(MessageID::MSG_ID_PACKAGE_REMOVED, parcel);
    MessageHandler::Instance()->SendMessage(msg);
}

ImCommonEventManager::SystemAbilityStatusChangeListener::SystemAbilityStatusChangeListener(std::function<void()> func)
    : func_(std::move(func))
{
}

void ImCommonEventManager::SystemAbilityStatusChangeListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string &deviceId)
{
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID && systemAbilityId != MULTIMODAL_INPUT_SERVICE_ID) {
        IMSA_HILOGE("ImCommonEventManager::OnAddSystemAbility systemAbilityId %{public}d", systemAbilityId);
        return;
    }
    if (func_ != nullptr) {
        func_();
    }
}

void ImCommonEventManager::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string &deviceId)
{
}
} // namespace MiscServices
} // namespace OHOS
