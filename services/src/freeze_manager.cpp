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
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (type == RequestType::START_INPUT || type == RequestType::REQUEST_SHOW) {
            isImeInUse_ = true;
        }
        if (!isFrozen_) {
            IMSA_HILOGD("already not frozen");
            return;
        }
        isFrozen_ = false;
    }
    ControlIme(false);
}

void FreezeManager::AfterIpc(RequestType type, bool isSuccess)
{
    bool shouldFreeze = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (type == RequestType::START_INPUT || type == RequestType::REQUEST_SHOW) {
            isImeInUse_ = isSuccess;
        }
        if (type == RequestType::REQUEST_HIDE && isImeInUse_) {
            isImeInUse_ = !isSuccess;
        }
        if (type == RequestType::STOP_INPUT) {
            isImeInUse_ = false;
        }
        if (isFrozen_ == !isImeInUse_) {
            IMSA_HILOGD("frozen state already: %{public}d", isFrozen_);
            return;
        }
        isFrozen_ = !isImeInUse_;
        shouldFreeze = isFrozen_;
    }
    ControlIme(shouldFreeze);
}

void FreezeManager::ControlIme(bool shouldFreeze)
{
    auto type = ResourceSchedule::ResType::RES_TYPE_SA_CONTROL_APP_EVENT;
    auto status = shouldFreeze ? ResourceSchedule::ResType::SaControlAppStatus::SA_STOP_APP
                               : ResourceSchedule::ResType::SaControlAppStatus::SA_START_APP;
    std::unordered_map<std::string, std::string> payload = {
        { "saId", std::to_string(INPUT_METHOD_SYSTEM_ABILITY_ID) },
        { "saName", INPUT_METHOD_SERVICE_SA_NAME },
        { "extensionType", std::to_string(static_cast<int32_t>(AppExecFwk::ExtensionAbilityType::INPUTMETHOD)) },
        { "pid", std::to_string(pid_) } };
    IMSA_HILOGI("report RSS should freeze: %{public}d", shouldFreeze);
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, status, payload);
}
} // namespace MiscServices
} // namespace OHOS