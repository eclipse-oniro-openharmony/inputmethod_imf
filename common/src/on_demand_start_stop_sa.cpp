/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "on_demand_start_stop_sa.h"

#include "global.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
sptr<IRemoteObject> OnDemandStartStopSa::LoadInputMethodSystemAbility()
{
    std::unique_lock<std::mutex> lock(loadSaMtx_);
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGE("get system ability manager fail");
        return nullptr;
    }

    auto remoteObject = systemAbilityManager->CheckSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID);
    if (remoteObject != nullptr) {
        return remoteObject;
    }

    auto sharedThis = shared_from_this();
    sptr<SaLoadCallback> callback = new (std::nothrow) SaLoadCallback(sharedThis);
    if (callback == nullptr) {
        IMSA_HILOGE("LoadCallback new fail.");
        return nullptr;
    }

    int32_t ret = systemAbilityManager->LoadSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, callback);
    if (ret != ERR_OK) {
        IMSA_HILOGE("load input method system ability fail, ret: %{public}d", ret);
        return nullptr;
    }

    loadSaCv_.wait_for(lock, std::chrono::seconds(LOAD_SA_MAX_WAIT_TIME), [&sharedThis]() {
        return sharedThis->remoteObj_ != nullptr;
    });

    return remoteObj_;
}

void OnDemandStartStopSa::UnloadInputMethodSystemAbility()
{
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGE("get system ability manager fail");
        return;
    }

    int32_t ret = systemAbilityManager->UnloadSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID);
    if (ret != ERR_OK) {
        IMSA_HILOGE("unload input method system ability fail, ret: %{public}d", ret);
        return;
    }
    IMSA_HILOGI("unload input method system ability success");
}

void OnDemandStartStopSa::SaLoadCallback::OnLoadSystemAbilitySuccess(int32_t said, const sptr<IRemoteObject> &object)
{
    IMSA_HILOGI("load inputmethod sa success");
    if (onDemandObj_ == nullptr) {
        IMSA_HILOGE("onDemandObj is null");
        return;
    }
    std::unique_lock<std::mutex> lock(onDemandObj_->loadSaMtx_);
    onDemandObj_->remoteObj_ = object;
    onDemandObj_->loadSaCv_.notify_all();
}

void OnDemandStartStopSa::SaLoadCallback::OnLoadSystemAbilityFail(int32_t said)
{
    IMSA_HILOGE("load inputmethod sa fail");
    if (onDemandObj_ == nullptr) {
        IMSA_HILOGE("onDemandObj is null");
        return;
    }
    std::unique_lock<std::mutex> lock(onDemandObj_->loadSaMtx_);
    onDemandObj_->remoteObj_ = nullptr;
    onDemandObj_->loadSaCv_.notify_all();
}

sptr<IRemoteObject> OnDemandStartStopSa::GetInputMethodSystemAbility(bool ifRetry)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGE("system ability manager is nullptr!");
        return nullptr;
    }

    sptr<IRemoteObject> systemAbility = nullptr;
    if (!ifRetry) {
        systemAbility = systemAbilityManager->CheckSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID);
        if (systemAbility == nullptr) {
            IMSA_HILOGE("check system ability is nullptr!");
            return nullptr;
        }
        return systemAbility;
    }

#ifdef IMF_ON_DEMAND_START_STOP_SA_ENABLE
    auto onDemandStartStopSa = std::make_shared<OnDemandStartStopSa>();
    if (onDemandStartStopSa == nullptr) {
        IMSA_HILOGE("onDemandStartStopSa is nullptr!");
        return nullptr;
    }

    systemAbility = onDemandStartStopSa->LoadInputMethodSystemAbility();
    if (systemAbility == nullptr) {
        IMSA_HILOGE("load system ability fail");
        return nullptr;
    }
#else
    systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
    if (systemAbility == nullptr) {
        IMSA_HILOGE("get system ability is nullptr!");
        return nullptr;
    }
#endif
    return systemAbility;
}
} // namespace MiscServices
} // namespace OHOS