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

#include "sa_task.h"
#include "requester_manager.h"

#include <utility>

namespace OHOS {
namespace MiscServices {
SaTask::~SaTask()
{
    if (!hasResponded_) {
        failRet_ = ErrorCode::ERROR_IMSA_TASK_TIMEOUT;
        InvokeResponse();
    }
    RequesterManager::GetInstance().TaskOut(callerInfo_.pid);
}

RunningState SaTask::Execute()
{
    if (state_ != RUNNING_STATE_IDLE) {
        IMSA_HILOGE("Task not runnable, state=%{public}u", state_);
        return RUNNING_STATE_ERROR;
    }
    return ExecuteInner();
}

RunningState SaTask::Resume(uint64_t resumeId)
{
    if (action_ == nullptr) {
        IMSA_HILOGE("curAction_ is nullptr");
        return RUNNING_STATE_ERROR;
    }

    if (state_ != RUNNING_STATE_PAUSED) {
        IMSA_HILOGE("state_ is %{public}u, do not need to resume", state_);
        return RUNNING_STATE_ERROR;
    }

    int32_t ret = 0;
    state_ = action_->Resume(resumeId, ret, responseData_);
    if (state_ == RUNNING_STATE_PAUSED) {
        return state_;
    }
    if (state_ == RUNNING_STATE_COMPLETED) {
        IMSA_HILOGE("curAction_ complete!");
        return RUNNING_STATE_COMPLETED;
    }
    IMSA_HILOGE("curAction_ resume return %{public}u, error!", state_);
    return RUNNING_STATE_ERROR;
}

RunningState SaTask::OnTask(const std::shared_ptr<SaTask> &task)
{
    if (task == nullptr) {
        IMSA_HILOGE("task is nullptr");
        return state_;
    }
    auto type = task->GetType();
    if (type == SaTaskType::TYPE_INNER) {
        return Resume(task->GetSeqId());
    }
    return state_;
}

int32_t SaTask::PendWaitResult(std::unique_ptr<SaAction> action)
{
    if (action == nullptr || action->GetState() != RUNNING_STATE_PAUSED) {
        IMSA_HILOGE("action is nullptr or not paused, pend failed");
        return ErrorCode::ERROR_SA_TASK_MANAGER_PEND_ACTION_FAILED;
    }
    return action_->Pend(std::move(action));
}

int32_t SaTask::Pend(std::unique_ptr<SaAction> action)
{
    if (action_ == nullptr || action_->GetState() != RUNNING_STATE_RUNNING) {
        IMSA_HILOGE("curAction_ is NULL or not running, pend failed!");
        return ErrorCode::ERROR_SA_TASK_MANAGER_PEND_ACTION_FAILED;
    }
    return action_->Pend(std::move(action));
}

int32_t SaTask::Pend(SaActionFunc func)
{
    if (action_ == nullptr || action_->GetState() != RUNNING_STATE_RUNNING) {
        IMSA_HILOGE("curAction_ is NULL or not running, pend failed!");
        return ErrorCode::ERROR_SA_TASK_MANAGER_PEND_ACTION_FAILED;
    }
    return action_->Pend(std::make_unique<SaAction>(func));
}

SaTaskType SaTask::GetType() const
{
    auto type = static_cast<uint32_t>(code_);
    if (type >= static_cast<uint32_t>(SaTaskCode::TASK_CRITICAL_CHANGE_BEGIN)
        && type <= static_cast<uint32_t>(SaTaskCode::TASK_CRITICAL_CHANGE_END)) {
        return SaTaskType::TYPE_CRITICAL_CHANGE;
    }
    if (type >= static_cast<uint32_t>(SaTaskCode::TASK_SWITCH_IME_BEGIN)
        && type <= static_cast<uint32_t>(SaTaskCode::TASK_SWITCH_IME_END)) {
        return SaTaskType::TYPE_SWITCH_IME;
    }
    if (type >= static_cast<uint32_t>(SaTaskCode::TASK_HIGHER_REQUEST_BEGIN)
        && type <= static_cast<uint32_t>(SaTaskCode::TASK_HIGHER_REQUEST_END)) {
        return SaTaskType::TYPE_HIGHER_REQUEST;
    }
    if (type >= static_cast<uint32_t>(SaTaskCode::TASK_NORMAL_REQUEST_BEGIN)
        && type <= static_cast<uint32_t>(SaTaskCode::TASK_NORMAL_REQUEST_END)) {
        return SaTaskType::TYPE_NORMAL_REQUEST;
    }
    if (type >= static_cast<uint32_t>(SaTaskCode::TASK_QUERY_BEGIN)
        && type <= static_cast<uint32_t>(SaTaskCode::TASK_QUERY_END)) {
        return SaTaskType::TYPE_QUERY;
    }
    if (type >= static_cast<uint32_t>(SaTaskCode::TASK_RESUME_BEGIN)
        && type <= static_cast<uint32_t>(SaTaskCode::TASK_RESUME_END)) {
        return SaTaskType::TYPE_RESUME;
    }
    if (type >= static_cast<uint32_t>(SaTaskCode::TASK_INNER_BEGIN)
        && type <= static_cast<uint32_t>(SaTaskCode::TASK_INNER_END)) {
        return SaTaskType::TYPE_INNER;
    }
    return SaTaskType::TYPE_INVALID;
}

SaTaskCode SaTask::GetCode() const
{
    return code_;
}

uint64_t SaTask::GetSeqId() const
{
    return seqId_;
}

CallerInfo SaTask::GetCallerInfo() const
{
    return callerInfo_;
}

bool SaTask::IsRunning() const
{
    if (action_ == nullptr) {
        return state_ == RUNNING_STATE_RUNNING;
    }
    return action_->GetState() == RUNNING_STATE_RUNNING;
}

bool SaTask::IsPaused() const
{
    return state_ == RUNNING_STATE_PAUSED;
}

uint64_t SaTask::GetNextSeqId()
{
    static std::atomic<uint64_t> seqId{ 1 };
    return seqId.fetch_add(1, std::memory_order_seq_cst);
}

void SaTask::SetHiSysReporter(const ReportFunc &func)
{
    hiSysReporter_ = std::make_unique<SaActionReport>(func);
}

RunningState SaTask::ExecuteInner()
{
    state_ = RUNNING_STATE_RUNNING;
    if (action_ == nullptr) {
        IMSA_HILOGW("action_ is nullptr");
        state_ = RUNNING_STATE_COMPLETED;
        return state_;
    }

    RunningState state = action_->Execute(retCode_, responseData_);
    if (state == RUNNING_STATE_COMPLETED) {
        InvokeResponse();
        state_ = RUNNING_STATE_COMPLETED;
        return state_;
    }

    state_ = RUNNING_STATE_PAUSED;
    return state_;
}

void SaTask::OnResponse(int32_t retCode)
{
    retCode_ = retCode;
    InvokeResponse();
}

void SaTask::InvokeResponse()
{
    hasResponded_ = true;

    if (retCode_ != ErrorCode::NO_ERROR) {
        // When failRect_ is set validly, return failRet_.
        if (failRet_ != INVALID_FAIL_CODE) {
            retCode_ = failRet_;
        }
    }

    if (hiSysReporter_ != nullptr) {
        hiSysReporter_->Execute(retCode_, responseData_);
    }

    if (imaResponseChannel_ != nullptr) {
        ServiceResponseDataInner inner;
        if (responseData_.valueless_by_exception()) {
            inner.data = std::monostate();
        } else {
            inner.data = responseData_;
        }
        imaResponseChannel_->OnResponse(callerInfo_.requestId, retCode_, inner);
    }
    if (imcResponseChannel_ != nullptr) {
        ServiceResponseDataInner inner;
        if (responseData_.valueless_by_exception()) {
            inner.data = std::monostate();
        } else {
            inner.data = responseData_;
        }
        imcResponseChannel_->OnResponse(callerInfo_.requestId, retCode_, inner);
    }

    action_ = nullptr;
}

PauseInfo SaTask::GetPauseInfo()
{
    if (action_ == nullptr || action_->GetState() != RUNNING_STATE_PAUSED) {
        IMSA_HILOGE("curAction_ nullptr or not paused");
        return {};
    }
    return action_->GetPausedInfo();
}
} // namespace MiscServices
} // namespace OHOS