/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
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

#include "tasks/task.h"

#include "global.h"

namespace OHOS {
namespace MiscServices {

Task::Task(TaskType type) : type_(type), seqId_(GetNextSeqId()) { }

Task::Task(TaskType type, uint64_t seqId) : type_(type), seqId_(seqId) { }

RunningState Task::Execute()
{
    if (state_ != RUNNING_STATE_IDLE) {
        IMSA_HILOGE("Task not runnable, state=%{public}u", state_);
        return RUNNING_STATE_ERROR;
    }
    return ExecuteInner();
}

RunningState Task::Resume(uint64_t seqId)
{
    if (!curAction_) {
        IMSA_HILOGE("curAction_ is NULL, error!");
        return RUNNING_STATE_ERROR;
    }

    if (state_ != RUNNING_STATE_PAUSED) {
        IMSA_HILOGE("state_ is %{public}u, do not need to resume", state_);
        return RUNNING_STATE_ERROR;
    }

    auto ret = curAction_->Resume(seqId);
    if (ret == RUNNING_STATE_PAUSED) { // resume failed, return
        return state_;
    }
    if (ret == RUNNING_STATE_COMPLETED) { // resume success, continue to execute
        curAction_.reset();
        return ExecuteInner();
    }

    // unreachable
    IMSA_HILOGE("curAction_ resume return %{public}u, error!", ret);
    return RUNNING_STATE_ERROR;
}

RunningState Task::OnTask(std::shared_ptr<Task> task)
{
    if (task == nullptr) {
        IMSA_HILOGE("task is NULL, error!");
        return state_;
    }
    auto src = task->GetSourceType();
    if (src == SOURCE_TYPE_INNER) {
        return Resume(task->GetSeqId());
    }
    if (src == SOURCE_TYPE_IMA) {
        Pend(task);
        return state_;
    }
    return state_;
}

TaskType Task::GetType() const
{
    return type_;
}

SourceType Task::GetSourceType() const
{
    if (type_ >= TASK_TYPE_AMS_BEGIN && type_ <= TASK_TYPE_AMS_END) {
        return SOURCE_TYPE_AMS;
    }
    if (type_ >= TASK_TYPE_IMA_BEGIN && type_ <= TASK_TYPE_IMA_END) {
        return SOURCE_TYPE_IMA;
    }
    if (type_ >= TASK_TYPE_IMSA_BEGIN && type_ <= TASK_TYPE_IMSA_END) {
        return SOURCE_TYPE_IMSA;
    }
    return SOURCE_TYPE_INNER;
}

uint64_t Task::GetSeqId() const
{
    return seqId_;
}

RunningState Task::GetState() const
{
    return state_;
}

bool Task::IsRunning() const
{
    return state_ == RUNNING_STATE_RUNNING;
}

const std::list<std::unique_ptr<Action>> &Task::GetActions() const
{
    return actions_;
}

uint64_t Task::GetNextSeqId()
{
    static std::atomic<uint64_t> maxSeqId(1);
    return maxSeqId.fetch_add(1);
}

RunningState Task::ExecuteInner()
{
    state_ = RUNNING_STATE_RUNNING;
    if (!pendingActions_.empty()) {
        pendingActions_.splice(pendingActions_.end(), actions_);
        actions_.swap(pendingActions_);
    }

    while (!actions_.empty()) {
        curAction_ = std::move(actions_.front());
        actions_.pop_front();

        auto ret = curAction_->Execute();

        // check pending tasks
        if (!pendingActions_.empty()) {
            pendingActions_.splice(pendingActions_.end(), actions_);
            actions_.swap(pendingActions_);
        }

        if (ret == RUNNING_STATE_COMPLETED) {
            curAction_.reset();
            continue;
        }

        state_ = RUNNING_STATE_PAUSED;
        return state_;
    }
    state_ = RUNNING_STATE_COMPLETED;
    return state_;
}

int32_t Task::Pend(std::shared_ptr<Task> task)
{
    pendingActions_.splice(pendingActions_.end(), task->actions_);
    actions_.swap(pendingActions_);
    return ErrorCode::NO_ERROR;
}

int32_t Task::Pend(std::unique_ptr<Action> action)
{
    pendingActions_.push_back(std::move(action));
    return ErrorCode::NO_ERROR;
}

} // namespace MiscServices
} // namespace OHOS