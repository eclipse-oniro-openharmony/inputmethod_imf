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

#include "task_manager.h"

#include <string>

#include "actions/action.h"
#include "actions/action_wait.h"
#include "global.h"
#include "tasks/task.h"
#include "tasks/task_inner.h"

namespace OHOS {
namespace MiscServices {

static const std::string THREAD_NAME = "imf_task_manager";

TaskManager::TaskManager()
{
    auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
}

TaskManager &TaskManager::GetInstance()
{
    static TaskManager instance;
    return instance;
}

uint64_t TaskManager::PostTask(task_ptr_t task, uint32_t delayMs)
{
    if (!task) {
        IMSA_HILOGE("task is NULL");
        return 0;
    }

    auto func = std::bind(&TaskManager::OnNewTask, this, task);
    eventHandler_->PostTask(func, __FUNCTION__, delayMs);
    return task->GetSeqId();
}

void TaskManager::ProcessAsync()
{
    auto func = [=] {
        Process();
    };
    eventHandler_->PostTask(func, __FUNCTION__);
}

void TaskManager::Complete(uint64_t seqId)
{
    PostTask(std::make_shared<TaskResume>(seqId));
}

int32_t TaskManager::Pend(action_ptr_t action)
{
    if (action == nullptr) {
        IMSA_HILOGE("curTask_ is NULL or not runing, pend failed");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (curTask_ == nullptr || !curTask_->IsRunning()) {
        IMSA_HILOGE("curTask_ is NULL or not runing, pend failed");
        return ErrorCode::ERROR_TASK_MANAGER_PEND_FAILED;
    }
    return curTask_->Pend(std::move(action));
}

int32_t TaskManager::Pend(std::function<void()> func)
{
    return Pend(std::make_unique<Action>(func));
}

int32_t TaskManager::WaitExec(uint64_t seqId, uint32_t timeoutMs, std::function<void()> func)
{
    auto wait = std::make_unique<ActionWait>(seqId, timeoutMs);
    int32_t ret = Pend(std::move(wait));
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Pend ActionWait failed, ret=%{public}d", ret);
        return ret;
    }

    auto exec = std::make_unique<Action>(func);
    ret = Pend(std::move(exec));
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Pend Action failed, ret=%{public}d", ret);
        return ret;
    }
    return ErrorCode::NO_ERROR;
}

void TaskManager::SetInited(bool flag)
{
    inited_ = flag;
}

void TaskManager::OnNewTask(task_ptr_t task)
{
    if (task == nullptr) {
        IMSA_HILOGE("task is NULL");
        return;
    }
    auto srcType = task->GetSourceType();
    switch (srcType) {
        case SOURCE_TYPE_AMS:
            amsTasks_.push_back(task);
            break;
        case SOURCE_TYPE_IMA:
            imaTasks_.push_back(task);
            break;
        case SOURCE_TYPE_IMSA:
            imsaTasks_.push_back(task);
            break;
        case SOURCE_TYPE_INNER:
            innerTasks_.push_back(task);
            break;
        default:
            IMSA_HILOGE("task type %{public}d unknown", srcType);
            return;
    }
    Process();
}

void TaskManager::Process()
{
    ProcessNextInnerTask();
    ProcessNextAmsTask();
    ProcessNextImaTask();
    ProcessNextImsaTask();
}

void TaskManager::ProcessNextInnerTask()
{
    while (curTask_) {
        // curTask_ is not NULL, it must be paused
        // Loop through innerTasks_, try resume
        if (innerTasks_.empty()) {
            IMSA_HILOGI("InnerTasks_ empty, return");
            return;
        }

        auto task = innerTasks_.front();
        innerTasks_.pop_front();
        auto state = curTask_->OnTask(task);
        if (state == RUNNING_STATE_COMPLETED) {
            // current task completed
            curTask_.reset();
            innerTasks_.clear();
            return;
        }
        if (state == RUNNING_STATE_PAUSED) {
            // current task still paused, try next inner task
            continue;
        }

        // unreachable
        IMSA_HILOGE("Unexpected OnTask result %{public}d", state);
        curTask_.reset();
        innerTasks_.clear();
    }
}

void TaskManager::ProcessNextAmsTask()
{
    if (amsTasks_.empty()) {
        return;
    }

    // AMS task has higher priority. If curTask_ is valid and not from AMS, drop it
    if (curTask_) {
        if (curTask_->GetSourceType() == SOURCE_TYPE_AMS) {
            return;
        }
        curTask_.reset();
    }

    while (!curTask_) {
        if (amsTasks_.empty()) {
            return;
        }
        curTask_ = amsTasks_.front();
        amsTasks_.pop_front();
        ExecuteCurrentTask();
    }
}

void TaskManager::ProcessNextImaTask()
{
    if (imaTasks_.empty()) {
        return;
    }

    if (curTask_) {
        // curTask_ must be paused here
        while (!imaTasks_.empty()) {
            auto task = imaTasks_.front();
            imaTasks_.pop_front();
            auto state = curTask_->OnTask(task);
            if (state == RUNNING_STATE_COMPLETED) {
                curTask_.reset();
                break;
            }
        }
    }

    while (!curTask_) {
        if (imaTasks_.empty()) {
            return;
        }
        curTask_ = imaTasks_.front();
        imaTasks_.pop_front();
        ExecuteCurrentTask();
    }
}

void TaskManager::ProcessNextImsaTask()
{
    if (!inited_) {
        return;
    }

    if (curTask_ || imsaTasks_.empty()) {
        return;
    }

    while (!curTask_) {
        if (imsaTasks_.empty()) {
            return;
        }
        curTask_ = imsaTasks_.front();
        imsaTasks_.pop_front();
        ExecuteCurrentTask();
    }
}

void TaskManager::ExecuteCurrentTask()
{
    if (curTask_ == nullptr) {
        return;
    }
    auto state = curTask_->Execute();
    if (state == RUNNING_STATE_COMPLETED) {
        IMSA_HILOGI("curTask_ completed");
        curTask_.reset();
        ProcessAsync();
        return;
    }
    if (state == RUNNING_STATE_PAUSED) {
        IMSA_HILOGI("curTask_ paused");
        return;
    }
    IMSA_HILOGE("Unexpected Execute result %{public}u", state);
    curTask_.reset();
}

void TaskManager::Reset()
{
    inited_ = false;
    curTask_ = nullptr;
    innerTasks_.clear();
    imaTasks_.clear();
    imsaTasks_.clear();
    amsTasks_.clear();
}

} // namespace MiscServices
} // namespace OHOS
