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

#include "sa_action_wait.h"

#include "sa_task_manager.h"

namespace OHOS {
namespace MiscServices {
RunningState SaActionWait::Execute(int32_t &ret, ServiceResponseData &responseData)
{
    state_ = RUNNING_STATE_PAUSED;

    auto task = std::make_shared<SaTask>(SaTaskCode::RESUME_TIMEOUT, timeoutId_);
    ret = SaTaskManager::GetInstance().PostTask(task, timeoutMs_);
    return state_;
}

RunningState SaActionWait::Execute(int32_t &ret, ServiceResponseData &responseData, ActionInnerData &innerData)
{
    return Execute(ret, responseData);
}

RunningState SaActionWait::Resume(uint64_t resumedId, int32_t &ret, ServiceResponseData &data)
{
    ActionInnerData innerData;
    return Resume(resumedId, ret, data, innerData);
}

RunningState SaActionWait::Resume(
    uint64_t resumedId, int32_t &ret, ServiceResponseData &data, ActionInnerData &innerData)
{
    if (state_ != RUNNING_STATE_PAUSED) {
        return RUNNING_STATE_ERROR;
    }

    if (resumedId == completeId_) {
        if (onComplete_ != nullptr) {
            state_ = RUNNING_STATE_RUNNING;
            onComplete_(data, innerData);
        }
        state_ = RUNNING_STATE_COMPLETED;
        return state_;
    }

    if (resumedId == timeoutId_) {
        if (onTimeout_ != nullptr) {
            state_ = RUNNING_STATE_RUNNING;
            onTimeout_(data, innerData);
        }
        state_ = RUNNING_STATE_COMPLETED;
        return state_;
    }

    return state_;
}

PauseInfo SaActionWait::GetPausedInfo()
{
    return pauseInfo_;
}

RunningState SaActionWait::GetState()
{
    return state_;
}

bool SaActionWait::IsWaitAction()
{
    return true;
}
} // namespace MiscServices
} // namespace OHOS