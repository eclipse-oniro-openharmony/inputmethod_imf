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

#include "global.h"
#include "input_method_system_ability_stub.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "message_handler.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
    using namespace MessageID;
    sptr<ImCommonEventManager> ImCommonEventManager::instance_;
    std::mutex ImCommonEventManager::instanceLock_;

    /*! Constructor
    */
    ImCommonEventManager::ImCommonEventManager()
    {}

    /*! Destructor
    */
    ImCommonEventManager::~ImCommonEventManager()
    {}

    sptr<ImCommonEventManager> ImCommonEventManager::GetInstance()
    {
        if (!instance_) {
            std::lock_guard<std::mutex> autoLock(instanceLock_);
            if (!instance_) {
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

        EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);

        std::shared_ptr<EventSubscriber> subscriber = std::make_shared<EventSubscriber>(subscriberInfo);
        if (!subscriber) {
            IMSA_HILOGI("ImCommonEventManager::SubscribeEvent subscriber is nullptr");
            return false;
        }
        subscriber_ = subscriber;
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        sptr<ISystemAbilityStatusChange> status = new (std::nothrow) SystemAbilityStatusChangeListener(subscriber_);
        if (samgrProxy == nullptr || statusChangeListener_ == nullptr) {
            IMSA_HILOGE("SubscribeEvent samgrProxy or statusChangeListener_ is nullptr");
            return false;
        }
        int32_t ret = samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, status);
        if (ret != ERR_OK) {
            IMSA_HILOGE("SubscribeEvent SubscribeSystemAbility failed. ret = %{public}d", ret);
            return false;
        }
        statusChangeListener_ = status;
        IMSA_HILOGE("SubscribeEvent SubscribeSystemAbility result:%{public}d", ret);
        return true;
    }

    bool ImCommonEventManager::UnsubscribeEvent()
    {
        if (subscriber_ == nullptr) {
            IMSA_HILOGE("ImCommonEventManager::UnsubscribeEvent subscriber_ is nullptr");
            return false;
        }
        bool subscribeResult = EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
        IMSA_HILOGI("ImCommonEventManager::UnsubscribeEvent subscribeResult = %{public}d", subscribeResult);
        return true;
    }

    void ImCommonEventManager::EventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
    {
        auto want = data.GetWant();
        std::string action = want.GetAction();
        IMSA_HILOGI("ImCommonEventManager::OnReceiveEvent data.GetCode = %{public}u", data.GetCode());
        if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
            IMSA_HILOGI("ImCommonEventManager::OnReceiveEvent user switched!!!");
            startUser(data.GetCode());
        }
    }

    void ImCommonEventManager::EventSubscriber::startUser(int newUserId)
    {
        IMSA_HILOGI("ImCommonEventManager::startUser 1");

        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(newUserId);

        IMSA_HILOGI("ImCommonEventManager::startUser 2");
        Message *msg = new Message(MessageID::MSG_ID_USER_START, parcel);
        MessageHandler::Instance()->SendMessage(msg);
        IMSA_HILOGI("ImCommonEventManager::startUser 3");
    }

    ImCommonEventManager::SystemAbilityStatusChangeListener::SystemAbilityStatusChangeListener(
        std::shared_ptr<EventSubscriber> &sub)
        : sub_(sub)
    {
    }

    void ImCommonEventManager::SystemAbilityStatusChangeListener::OnAddSystemAbility(
        int32_t systemAbilityId, const std::string& deviceId)
    {
        if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
            IMSA_HILOGE("ImCommonEventManager::OnAddSystemAbility systemAbilityId is not COMMON_EVENT_SERVICE_ID");
            return;
        }
        if (sub_ == nullptr) {
            IMSA_HILOGE("ImCommonEventManager::OnAddSystemAbility COMMON_EVENT_SERVICE_ID sub_ is nullptr");
            return;
        }
        bool subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(sub_);
        IMSA_HILOGI("ImCommonEventManager::OnAddSystemAbility subscribeResult = %{public}d", subscribeResult);
    }
    
    void ImCommonEventManager::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(
        int32_t systemAbilityId, const std::string& deviceId)
    {
    }
} // namespace MiscServices
} // namespace OHOS
