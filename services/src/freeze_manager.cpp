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

#include "freeze_manager.h"

#include "ability_manager_client.h"
#include "global.h"
#include "res_sched_client.h"
#include "res_type.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
const std::string INPUT_METHOD_SERVICE_SA_NAME = "inputmethod_service";
bool FreezeManager::IsIpcNeeded(RequestType type)
{
    // If ime is in use, no need to request hide.
    return !(type == RequestType::REQUEST_HIDE && !isImeInUse_);
}

void FreezeManager::BeforeIpc(RequestType type)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (type == RequestType::START_INPUT) {
        isImeInUse_ = true;
    }
    SetFrozen(false);
}

void FreezeManager::AfterIpc(RequestType type, bool isSuccess)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (type == RequestType::START_INPUT) {
        isImeInUse_ = isSuccess;
    }
    if (type == RequestType::REQUEST_HIDE && isImeInUse_) {
        isImeInUse_ = !isSuccess;
    }
    if (type == RequestType::STOP_INPUT) {
        isImeInUse_ = false;
    }
    SetFrozen(!isImeInUse_);
}

void FreezeManager::SetFrozen(bool isFrozen)
{
    if (isFrozen_ == isFrozen) {
        IMSA_HILOGD("frozen state %{public}d not changed", isFrozen);
        return;
    }
    uint32_t type = ResourceSchedule::ResType::RES_TYPE_SA_CONTROL_APP_EVENT;
    int64_t status = isFrozen ? ResourceSchedule::ResType::SaControlAppStatus::SA_STOP_APP
                              : ResourceSchedule::ResType::SaControlAppStatus::SA_START_APP;
    std::unordered_map<std::string, std::string> payload;
    payload["saId"] = std::to_string(INPUT_METHOD_SYSTEM_ABILITY_ID);
    payload["saName"] = INPUT_METHOD_SERVICE_SA_NAME;
    payload["extensionType"] = std::to_string(static_cast<int32_t>(AppExecFwk::ExtensionAbilityType::INPUTMETHOD));
    payload["pid"] = std::to_string(pid_);
    IMSA_HILOGI("report RSS freezable: %{public}d", isFrozen);
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, status, payload);
    isFrozen_ = isFrozen;
}
} // namespace MiscServices
} // namespace OHOS