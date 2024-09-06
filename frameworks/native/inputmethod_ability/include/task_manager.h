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
#ifndef FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_TASK_MANAGER_H
#define FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_TASK_MANAGER_H

#include <list>
#include <memory>

#include "actions/action.h"
#include "event_handler.h"
#include "tasks/task.h"

namespace OHOS {
namespace MiscServices {

using task_ptr_t = std::shared_ptr<Task>;
using action_ptr_t = std::unique_ptr<Action>;

class TaskManager final {
private:
    TaskManager();

public:
    ~TaskManager() = default;

    TaskManager(const TaskManager &) = delete;
    TaskManager(TaskManager &&) = delete;
    TaskManager &operator=(const TaskManager &) = delete;
    TaskManager &operator=(TaskManager &&) = delete;

    static TaskManager &GetInstance();

    // Post a task to work thread
    uint64_t PostTask(task_ptr_t task, uint32_t delayMs = 0);

    // Trigger task process async
    void ProcessAsync();

    // Resume paused task with seqId
    void Complete(uint64_t seqId);

    // Pend an action to current task during executing
    int32_t Pend(action_ptr_t action);
    int32_t Pend(std::function<void()>);

    // Wait for task and execute
    int32_t WaitExec(uint64_t seqId, uint32_t timeoutMs, std::function<void()>);

private:
    friend class InputMethodAbility;
    friend class TaskAmsInit;
    void SetInited(bool flag);

private:
    void OnNewTask(task_ptr_t task); // Accept a new task
    void Process();                  // Process next task
    void ProcessNextInnerTask();     // Process next inner task
    void ProcessNextAmsTask();       // Process next AMS task
    void ProcessNextImaTask();       // process next IMA task
    void ProcessNextImsaTask();      // process next IMSA task
    void ExecuteCurrentTask();       // Execute current task

    void Reset();

private:
    bool inited_ { false };
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ { nullptr };

    task_ptr_t curTask_ = { nullptr };
    std::list<task_ptr_t> amsTasks_;
    std::list<task_ptr_t> imaTasks_;
    std::list<task_ptr_t> imsaTasks_;
    std::list<task_ptr_t> innerTasks_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_TASK_MANAGER_H