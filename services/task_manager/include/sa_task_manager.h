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

#ifndef IMF_SERVICES_SA_TASK_MANAGER_H
#define IMF_SERVICES_SA_TASK_MANAGER_H

#include "event_handler.h"
#include "identity_checker_impl.h"
#include "input_method_utils.h"
#include "sa_action.h"
#include "sa_action_wait.h"
#include "sa_task.h"

namespace OHOS {
namespace MiscServices {
using CallBack = std::function<void()>;

using SaTaskPtr = std::shared_ptr<SaTask>;
using SaActionPtr = std::unique_ptr<SaAction>;

#define GET_SHARED_THIS(weakThis, sharedThis, retVal) \
    do {                                              \
        sharedThis = weakThis.lock();                 \
        if (sharedThis == nullptr) {                  \
            IMSA_HILOGE("sharedThis is nullptr");     \
            return retVal;                            \
        }                                             \
    } while (0)

#define GET_SHARED_THIS_RETURN_VOID(weakThis, sharedThis) \
    do {                                                  \
        sharedThis = weakThis.lock();                     \
        if (sharedThis == nullptr) {                      \
            IMSA_HILOGE("sharedThis is nullptr");         \
            return;                                       \
        }                                                 \
    } while (0)

class SaTaskManager final {
private:
    SaTaskManager();

public:
    ~SaTaskManager() = default;

    SaTaskManager(const SaTaskManager &) = delete;
    SaTaskManager(SaTaskManager &&) = delete;
    SaTaskManager &operator=(const SaTaskManager &) = delete;
    SaTaskManager &operator=(SaTaskManager &&) = delete;

    static SaTaskManager &GetInstance();

    void Init();

    // Post a task to work thread
    int32_t PostTask(SaTaskPtr task, uint32_t delayMs = 0);

    // Trigger task process async
    void ProcessAsync();

    // Resume paused task with seqId
    void Complete(uint64_t resumeId);

    // Wait for task and execute
    int32_t WaitExec(std::unique_ptr<SaActionWait> waitAction, const SaActionFunc &execFunc = nullptr);
    // Try to resume the current paused task
    void TryResume(const PauseType &pauseType, const CallerInfo &callerInfo);

    // Pend an action to current task during executing
    int32_t Pend(SaActionPtr action);
    int32_t Pend(const SaActionFunc &func);

private:
    friend class InputMethodSystemAbility;
    friend class SaActionWait;
    void SetInited(bool flag);
    void Reset();
    int32_t PendWaitResult(const SaActionFunc &func);

private:
    void OnNewTask(SaTaskPtr task); // Accept a new task
    void Process();                 // Process next task

    void ProcessNextCriticalTask();
    void ProcessNextSwitchImeTask();
    void ProcessNextHigherRequestTask();
    void ProcessNextNormalRequestTask();
    void ProcessNextQueryTask();
    void ProcessNextResumeTask();
    void ProcessNextInnerTask();

    void ExecuteCurrentTask(); // Execute current task

private:
    bool IsWhiteListRequest(SaTaskCode taskCode);
    bool inited_{ false };
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_{ nullptr };
    std::shared_ptr<IdentityChecker> identityChecker_{ nullptr };

    SaTaskPtr curTask_ = { nullptr };
    SaTaskPtr pausedTask_ = { nullptr };
    std::list<SaTaskPtr> criticalTasks_;
    std::list<SaTaskPtr> switchImeTasks_;
    std::list<SaTaskPtr> higherRequestTasks_;
    std::list<SaTaskPtr> normalRequestTasks_;
    std::list<SaTaskPtr> queryTasks_;
    std::list<SaTaskPtr> innerTasks_;
    std::list<SaTaskPtr> resumeTasks_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // IMF_SERVICES_SA_TASK_MANAGER_H