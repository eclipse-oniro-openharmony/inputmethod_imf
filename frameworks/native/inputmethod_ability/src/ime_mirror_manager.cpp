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
#include "ime_mirror_manager.h"

#include "global.h"
#include "system_ability_definition.h"
namespace OHOS {
namespace MiscServices {
bool ImeMirrorManager::IsImeMirrorEnable()
{
    return isEnable_;
}

void ImeMirrorManager::SetImeMirrorEnable(bool isRegistered)
{
    isEnable_ = isRegistered;
}

bool ImeMirrorManager::SubscribeSaStart(std::function<void()> handler, int32_t saId)
{
    if (handler == nullptr) {
        IMSA_HILOGE("handler is nullptr");
        return false;
    }
    {
        std::lock_guard<std::mutex> lockGuard(listenerMapMutex_);
        if (saMgrListenerMap_.find(saId) != saMgrListenerMap_.end()) {
            IMSA_HILOGW("[ImeMirrorTag]saId:%{public}d is already registered", saId);
            return true;
        }
    }

    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        IMSA_HILOGE("[ImeMirrorTag]abilityManager is nullptr, saId: %{public}d", saId);
        return false;
    }
    sptr<ISystemAbilityStatusChange> listener = new (std::nothrow) SaMgrListener([handler]() {
        if (handler != nullptr) {
            handler();
        }
    });
    if (listener == nullptr) {
        IMSA_HILOGE("[ImeMirrorTag]failed to create listener, saId: %{public}d", saId);
        return false;
    }
    int32_t ret = abilityManager->SubscribeSystemAbility(saId, listener);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("[ImeMirrorTag]subscribe system ability failed, ret: %{public}d, saId: %{public}d", ret, saId);
        return false;
    }
    {
        std::lock_guard<std::mutex> lockGuard(listenerMapMutex_);
        saMgrListenerMap_[saId] = listener;
    }
    IMSA_HILOGD("[ImeMirrorTag]subscribe system ability success, saId: %{public}d", saId);
    return true;
}

bool ImeMirrorManager::UnSubscribeSaStart(int32_t saId)
{
    sptr<ISystemAbilityStatusChange> listener = nullptr;
    {
        std::lock_guard<std::mutex> lockGuard(listenerMapMutex_);
        auto itr = saMgrListenerMap_.find(saId);
        if (itr == saMgrListenerMap_.end()) {
            IMSA_HILOGE("[ImeMirrorTag]saId:%{public}d is not registered", saId);
            return false;
        }

        listener = itr->second;
    }
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        IMSA_HILOGE("[ImeMirrorTag]abilityManager is nullptr, saId: %{public}d", saId);
        return false;
    }

    int32_t ret = abilityManager->UnSubscribeSystemAbility(saId, listener);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("[ImeMirrorTag]UnSubscribe system ability failed, ret: %{public}d, saId: %{public}d", ret, saId);
        return false;
    }
    {
        std::lock_guard<std::mutex> lockGuard(listenerMapMutex_);
        auto itr = saMgrListenerMap_.find(saId);
        if (itr == saMgrListenerMap_.end()) {
            IMSA_HILOGW("[ImeMirrorTag]saId:%{public}d is not found", saId);
            return true;
        }
        saMgrListenerMap_.erase(itr);
    }
    IMSA_HILOGD("[ImeMirrorTag]unsubscribe system ability success, saId: %{public}d", saId);
    return true;
}

void ImeMirrorManager::SaMgrListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    IMSA_HILOGD("[ImeMirrorTag]systemAbilityId: %{public}d.", systemAbilityId);
    if (systemAbilityId != INPUT_METHOD_SYSTEM_ABILITY_ID) {
        return;
    }
    if (func_ != nullptr) {
        func_();
    }
}
} // namespace MiscServices
} // namespace OHOS
