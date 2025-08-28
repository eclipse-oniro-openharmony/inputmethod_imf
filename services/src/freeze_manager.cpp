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
#ifndef FREEZE_MANAGER_H
#define FREEZE_MANAGER_H
#include "freeze_manager.h"

#include "ability_manager_client.h"
#include "global.h"
#include "res_sched_client.h"
#include "system_ability_definition.h"
namespace OHOS {
namespace MiscServices {
constexpr const char *INPUT_METHOD_SERVICE_SA_NAME = "inputmethod_service";
constexpr const char *STOP_TASK_NAME = "ReportStop";
constexpr int32_t DELAY_TIME = 3000; // 3s
void FreezeManager::ControlIme(bool shouldApply)
{
    if (eventHandler_ == nullptr) {
        IMSA_HILOGW("eventHandler_ is nullptr.");
        ReportRss(shouldApply, pid_);
        return;
    }
    if (shouldApply) {
        // Delay the FREEZE report by 3s.
        eventHandler_->PostTask(
            [shouldApply, pid = pid_]() {
                ReportRss(shouldApply, pid);
            },
            STOP_TASK_NAME, DELAY_TIME);
    } else {
        // Cancel the unexecuted FREEZE task.
        eventHandler_->RemoveTask(STOP_TASK_NAME);
        ReportRss(shouldApply, pid_);
    }
}

void FreezeManager::ReportRss(bool shouldFreeze, pid_t pid)
{
    auto type = ResourceSchedule::ResType::RES_TYPE_SA_CONTROL_APP_EVENT;
    auto status = shouldFreeze ? ResourceSchedule::ResType::SaControlAppStatus::SA_STOP_APP :
                                 ResourceSchedule::ResType::SaControlAppStatus::SA_START_APP;
    std::unordered_map<std::string, std::string> payload = {
        { "saId",          std::to_string(INPUT_METHOD_SYSTEM_ABILITY_ID)                                      },
        { "saName",        std::string(INPUT_METHOD_SERVICE_SA_NAME)                                           },
        { "extensionType", std::to_string(static_cast<int32_t>(AppExecFwk::ExtensionAbilityType::INPUTMETHOD)) },
        { "pid",           std::to_string(pid)                                                                 },
    };
    IMSA_HILOGD("report RSS should freeze: %{public}d.", shouldFreeze);
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, status, payload);
}
// LCOV_EXCL_START
void FreezeManager::TemporaryActiveIme()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!isFrozen_) {
        IMSA_HILOGI("IME is not frozen, no need active");
        return;
    }

    IMSA_HILOGI("temporary active IME");
    ReportRss(false, pid_);
    ReportRss(true, pid_);
}
// LCOV_EXCL_STOP
} // namespace MiscServices
} // namespace OHOS
#endif // FREEZE_MANAGER_H