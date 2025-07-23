/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "sa_task_manager.h"

#include <string>
#include <utility>

#include "global.h"
#include "ime_info_inquirer.h"
#include "requester_manager.h"
#include "sa_action_wait.h"
#include "xcollie/watchdog.h"

namespace OHOS {
namespace MiscServices {
using namespace AppExecFwk;
constexpr const char *THREAD_NAME = "OS_ImsaTaskHandler";
constexpr const uint64_t WATCHDOG_TIMEOUT = 10000; // 10S
const std::unordered_set<SaTaskCode> WHITE_LIST_REQUESTS = { SaTaskCode::UPDATE_LISTEN_EVENT_FLAG };
SaTaskManager::SaTaskManager()
{
    // initialized the event handler with 10s timeout watchdog
    auto runner = EventRunner::Create(THREAD_NAME);
    eventHandler_ = std::make_shared<EventHandler>(runner);
    auto ret = HiviewDFX::Watchdog::GetInstance().AddThread(THREAD_NAME, eventHandler_, nullptr, WATCHDOG_TIMEOUT);
    if (ret != 0) {
        IMSA_HILOGW("failed to add watch dog ret: %{public}d", ret);
    }
}

void SaTaskManager::Init()
{
    identityChecker_ = std::make_shared<IdentityCheckerImpl>();
    inited_ = true;
}

SaTaskManager &SaTaskManager::GetInstance()
{
    static SaTaskManager instance;
    return instance;
}

int32_t SaTaskManager::PostTask(SaTaskPtr task, uint32_t delayMs)
{
    if (task == nullptr) {
        IMSA_HILOGE("task is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto func = [this, task]() { OnNewTask(task); };
    bool ret = eventHandler_->PostTask(func, __FUNCTION__, delayMs);
    if (!ret) {
        IMSA_HILOGE("failed to post task: %{public}u", static_cast<uint32_t>(task->GetCode()));
        return ErrorCode::ERROR_SA_POST_TASK_FAILED;
    }
    RequesterManager::GetInstance().TaskIn(task->GetCallerInfo().pid);
    return ErrorCode::NO_ERROR;
}

void SaTaskManager::ProcessAsync()
{
    auto func = [=] { Process(); };
    eventHandler_->PostTask(func, __FUNCTION__);
}

void SaTaskManager::Complete(uint64_t resumeId)
{
    PostTask(std::make_shared<SaTask>(SaTaskCode::RESUME_WAIT, resumeId));
}

int32_t SaTaskManager::Pend(SaActionPtr action)
{
    if (action == nullptr) {
        IMSA_HILOGE("action is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (curTask_ == nullptr || !curTask_->IsRunning()) {
        IMSA_HILOGE("curTask_ is NULL or not running, pend failed!");
        return ErrorCode::ERROR_SA_TASK_MANAGER_PEND_ACTION_FAILED;
    }
    return curTask_->Pend(std::move(action));
}

int32_t SaTaskManager::Pend(const SaActionFunc &func)
{
    return Pend(std::make_unique<SaAction>(func));
}

int32_t SaTaskManager::PendWaitResult(const SaActionFunc &func)
{
    if (curTask_ == nullptr || !curTask_->IsPaused()) {
        IMSA_HILOGE("curTask_ is NULL or not paused, pend failed!");
        return ErrorCode::ERROR_SA_TASK_MANAGER_PEND_ACTION_FAILED;
    }
    auto action = std::make_unique<SaAction>(func);
    return curTask_->PendWaitResult(std::move(action));
}

int32_t SaTaskManager::WaitExec(std::unique_ptr<SaActionWait> waitAction, const SaActionFunc &execFunc)
{
    if (waitAction == nullptr) {
        IMSA_HILOGE("wait action nullptr");
        return ErrorCode::ERROR_IMSA_NULLPTR;
    }
    int32_t ret = Pend(std::move(waitAction));
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Pend ActionWait failed, ret: %{public}d", ret);
        return ret;
    }

    auto exec = std::make_unique<SaAction>(execFunc);
    ret = Pend(execFunc);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Pend Action failed, ret: %{public}d", ret);
        return ret;
    }
    return ErrorCode::NO_ERROR;
}

void SaTaskManager::SetInited(bool flag)
{
    inited_ = flag;
}

void SaTaskManager::OnNewTask(SaTaskPtr task)
{
    if (task == nullptr) {
        IMSA_HILOGE("task is nullptr");
        return;
    }
    auto taskType = task->GetType();
    if (taskType <= SaTaskType::TYPE_INVALID || taskType >= SaTaskType::TYPE_TOTAL_COUNT) {
        IMSA_HILOGE("task type %{public}d unknown!", taskType);
        return;
    }
    switch (taskType) {
        case SaTaskType::TYPE_CRITICAL_CHANGE: {
            criticalTasks_.push_back(task);
            break;
        }
        case SaTaskType::TYPE_SWITCH_IME: {
            switchImeTasks_.push_back(task);
            break;
        }
        case SaTaskType::TYPE_HIGHER_REQUEST: {
            higherRequestTasks_.push_back(task);
            break;
        }
        case SaTaskType::TYPE_NORMAL_REQUEST: {
            normalRequestTasks_.push_back(task);
            break;
        }
        case SaTaskType::TYPE_QUERY: {
            queryTasks_.push_back(task);
            break;
        }
        case SaTaskType::TYPE_RESUME: {
            resumeTasks_.push_back(task);
            break;
        }
        case SaTaskType::TYPE_INNER: {
            innerTasks_.push_back(task);
            break;
        }
        default: {
            IMSA_HILOGE("task type %{public}d unknown!", taskType);
            return;
        }
    }

    Process();
}

void SaTaskManager::Process()
{
    // tasks creating resume inner tasks
    ProcessNextResumeTask();
    // tasks acting on the current paused task
    ProcessNextInnerTask();

    // tasks bringing critical changes or updates
    ProcessNextCriticalTask();
    // tasks causing ime switch
    ProcessNextSwitchImeTask();

    // tasks with a higher priority request
    ProcessNextHigherRequestTask();
    // tasks with a normal priority request
    ProcessNextNormalRequestTask();

    // tasks used for querying
    ProcessNextQueryTask();
}

void SaTaskManager::ProcessNextCriticalTask()
{
    if (criticalTasks_.empty()) {
        return;
    }
    // CRITICAL_CHANGE task has the highest priority. If curTask_ exists and is not CRITICAL_CHANGE task, drop it.
    if (curTask_ != nullptr) {
        if (curTask_->GetType() == SaTaskType::TYPE_CRITICAL_CHANGE && curTask_->IsPaused()) {
            return;
        }
        curTask_.reset();
    }

    while (curTask_ == nullptr) {
        if (criticalTasks_.empty()) {
            return;
        }
        curTask_ = criticalTasks_.front();
        criticalTasks_.pop_front();
        ExecuteCurrentTask();
    }
}

void SaTaskManager::ProcessNextSwitchImeTask()
{
    if (switchImeTasks_.empty()) {
        return;
    }
    // SWITCH_IME task has a priority second to type CRITICAL_CHANGE.
    if (curTask_ != nullptr) {
        if (curTask_->IsPaused()) {
            return;
        }
        IMSA_HILOGW("curTask_ state abnormal! Reset");
        curTask_.reset();
    }

    while (curTask_ == nullptr) {
        if (switchImeTasks_.empty()) {
            return;
        }
        curTask_ = switchImeTasks_.front();
        switchImeTasks_.pop_front();
        ExecuteCurrentTask();
    }
}

void SaTaskManager::ProcessNextHigherRequestTask()
{
    if (!inited_) {
        IMSA_HILOGW("not initialized yet");
        return;
    }
    if (higherRequestTasks_.empty()) {
        IMSA_HILOGD("immeRequestTasks_ empty");
        return;
    }

    // If curTask_ is NULL or state abnormal, execute higherRequestTasks_ directly.
    if (curTask_ == nullptr || !curTask_->IsPaused()) {
        curTask_.reset();
        while (!higherRequestTasks_.empty() && curTask_ == nullptr) {
            curTask_ = higherRequestTasks_.front();
            higherRequestTasks_.pop_front();
            ExecuteCurrentTask();
        }
        return;
    }

    // If curTask_ not NULL, task which comes from target app and is in white list can be executed.
    pausedTask_ = std::move(curTask_);
    std::list<SaTaskPtr> remainingTasks;
    while (!higherRequestTasks_.empty()) {
        auto task = higherRequestTasks_.front();
        higherRequestTasks_.pop_front();
        // Task not from target app, keep waiting.
        auto callerBundleName = identityChecker_->GetBundleNameByToken(task->GetCallerInfo().tokenId);
        if (pausedTask_->GetPauseInfo().target != callerBundleName) {
            remainingTasks.push_back(task);
            IMSA_HILOGW(
                "task %{public}u not from target app, push back to tasks", static_cast<uint32_t>(task->GetCode()));
            continue;
        }
        // Task from target app but not in whitelist, reject directly.
        if (!IsWhiteListRequest(task->GetCode())) {
            task->OnResponse(ErrorCode::ERROR_IMSA_TASK_TIMEOUT);
            IMSA_HILOGW("task %{public}u from target app is dropped", static_cast<uint32_t>(task->GetCode()));
            continue;
        }
        // Task from target app and in white list, execute it.
        curTask_ = task;
        ExecuteCurrentTask();
    }
    // Restore curTask_ with pausedTask, restore immeRequestTasks_ with tasks still waiting.
    curTask_ = std::move(pausedTask_);
    higherRequestTasks_ = std::move(remainingTasks);
}

void SaTaskManager::ProcessNextNormalRequestTask()
{
    if (!inited_) {
        IMSA_HILOGW("not initialized yet");
        return;
    }
    if (normalRequestTasks_.empty()) {
        IMSA_HILOGD("requestTasks_ empty");
        return;
    }

    // If curTask_ is NULL or state abnormal, execute normalRequestTasks_ directly.
    if (curTask_ == nullptr || !curTask_->IsPaused()) {
        while (!normalRequestTasks_.empty() && curTask_ == nullptr) {
            curTask_ = normalRequestTasks_.front();
            normalRequestTasks_.pop_front();
            ExecuteCurrentTask();
        }
        return;
    }

    // If curTask_ not NULL, task which is from target app and in white list can be executed.
    pausedTask_ = std::move(curTask_);
    std::list<SaTaskPtr> remainingTask;
    while (!normalRequestTasks_.empty()) {
        auto task = normalRequestTasks_.front();
        normalRequestTasks_.pop_front();
        // Task not from target app, keep waiting.
        auto callerBundleName = identityChecker_->GetBundleNameByToken(task->GetCallerInfo().tokenId);
        if (pausedTask_->GetPauseInfo().target != callerBundleName) {
            remainingTask.push_back(task);
            IMSA_HILOGW(
                "task %{public}u not from target app, push back to tasks", static_cast<uint32_t>(task->GetCode()));
            continue;
        }
        // Task from target app but not in whitelist, reject it.
        if (!IsWhiteListRequest(task->GetCode())) {
            task->OnResponse(ErrorCode::ERROR_IMSA_TASK_TIMEOUT);
            IMSA_HILOGW("task %{public}u from target app is dropped", static_cast<uint32_t>(task->GetCode()));
            continue;
        }
        // Task from target app and in white list, execute it.
        curTask_ = task;
        ExecuteCurrentTask();
    }
    // Restore curTask_ with pausedTask, restore normalRequestTasks_ with tasks still waiting.
    curTask_ = std::move(pausedTask_);
    normalRequestTasks_ = std::move(remainingTask);
}

void SaTaskManager::ProcessNextQueryTask()
{
    if (!inited_) {
        IMSA_HILOGW("not initialized yet");
        return;
    }
    if (queryTasks_.empty()) {
        IMSA_HILOGD("queryTasks_ empty");
        return;
    }
    // QUERY tasks can be executed when curTask_ exists.
    auto pausedTask = std::move(curTask_);
    while (curTask_ == nullptr && !queryTasks_.empty()) {
        curTask_ = queryTasks_.front();
        queryTasks_.pop_front();
        ExecuteCurrentTask();
    }
    curTask_ = std::move(pausedTask);
}

void SaTaskManager::ProcessNextResumeTask()
{
    if (resumeTasks_.empty()) {
        IMSA_HILOGD("resumeTasks_ empty, return");
        return;
    }
    // RESUME tasks can be executed when curTask_ exists.
    pausedTask_ = std::move(curTask_);
    while (curTask_ == nullptr && !resumeTasks_.empty()) {
        curTask_ = resumeTasks_.front();
        resumeTasks_.pop_front();
        ExecuteCurrentTask();
    }
    curTask_ = std::move(pausedTask_);
}

void SaTaskManager::ProcessNextInnerTask()
{
    while (curTask_ != nullptr) {
        // curTask_ is not NULL, it must be paused
        // Loop through innerTasks_, try resume
        if (innerTasks_.empty()) {
            IMSA_HILOGD("innerTasks_ empty, return");
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

void SaTaskManager::ExecuteCurrentTask()
{
    if (curTask_ == nullptr) {
        return;
    }
    auto state = curTask_->Execute();
    if (state == RUNNING_STATE_COMPLETED) {
        IMSA_HILOGI("curTask_ %{public}u completed", static_cast<uint32_t>(curTask_->GetCode()));
        curTask_.reset();
        ProcessAsync();
        return;
    }
    if (state == RUNNING_STATE_PAUSED) {
        IMSA_HILOGI("curTask_ %{public}u paused", static_cast<uint32_t>(curTask_->GetCode()));
        return;
    }
    IMSA_HILOGE("Unexpected execute result state: %{public}u", state);
    curTask_.reset();
}

void SaTaskManager::TryResume(const PauseType &pauseType, const CallerInfo &callerInfo)
{
    if (pausedTask_ == nullptr || !pausedTask_->IsPaused()) {
        IMSA_HILOGD("curTask_ nullptr or not paused state");
        return;
    }
    PauseInfo pausedInfo = pausedTask_->GetPauseInfo();
    if (pausedInfo.type == PauseType::PAUSED_TYPE_INVALID) {
        IMSA_HILOGE("invalid pause type");
        return;
    }
    if (pauseType != pausedInfo.type) {
        IMSA_HILOGD("type not match, type: %{public}d, target: %{public}d", static_cast<int32_t>(pauseType),
            static_cast<int32_t>(pausedInfo.type));
        return;
    }
    if (callerInfo.bundleName != pausedInfo.target) {
        IMSA_HILOGD("bundleName not match, caller: %{public}s, target: %{public}s", callerInfo.bundleName.c_str(),
            pausedInfo.target.c_str());
        return;
    }
    IMSA_HILOGI("start resume, %{public}s", pausedInfo.ToString().c_str());
    Complete(pausedInfo.resumeId);
}

void SaTaskManager::Reset()
{
    inited_ = false;
    curTask_ = nullptr;
    eventHandler_ = nullptr;
    innerTasks_.clear();
    criticalTasks_.clear();
    normalRequestTasks_.clear();
    queryTasks_.clear();
}

bool SaTaskManager::IsWhiteListRequest(SaTaskCode taskCode)
{
    return WHITE_LIST_REQUESTS.find(taskCode) != WHITE_LIST_REQUESTS.end();
}
} // namespace MiscServices
} // namespace OHOS