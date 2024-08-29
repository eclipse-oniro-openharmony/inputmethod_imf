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

#ifndef FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_TASKS_TASK_H
#define FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_TASKS_TASK_H

#include "actions/action.h"

#include <list>
#include <memory>

namespace OHOS {
namespace MiscServices {

enum SourceType : uint32_t {
    SOURCE_TYPE_AMS = 0,
    SOURCE_TYPE_IMA,
    SOURCE_TYPE_IMSA,
    SOURCE_TYPE_INNER,
};

#define TASK_TYPE_OFFSET(src) ((src) * 10000)

enum TaskType : uint32_t {
    // Task from AMS
    TASK_TYPE_AMS_BEGIN = TASK_TYPE_OFFSET(SOURCE_TYPE_AMS),
    TASK_TYPE_AMS_INIT = TASK_TYPE_AMS_BEGIN,
    TASK_TYPE_AMS_END = TASK_TYPE_AMS_INIT,

    // Task from IMA
    TASK_TYPE_IMA_BEGIN = TASK_TYPE_OFFSET(SOURCE_TYPE_IMA),
    TASK_TYPE_IMA_SHOW_PANEL = TASK_TYPE_IMA_BEGIN,
    TASK_TYPE_IMA_HIDE_PANEL,
    TASK_TYPE_IMA_END = TASK_TYPE_IMA_HIDE_PANEL,

    // Task from IMSA
    TASK_TYPE_IMSA_BEGIN = TASK_TYPE_OFFSET(SOURCE_TYPE_IMSA),
    TASK_TYPE_IMSA_START_INPUT = TASK_TYPE_IMSA_BEGIN,
    TASK_TYPE_IMSA_STOP_INPUT,
    TASK_TYPE_IMSA_SHOW_KEYBOARD,
    TASK_TYPE_IMSA_HIDE_KEYBOARD,
    TASK_TYPE_IMSA_CLIENT_INACTIVE,
    TASK_TYPE_IMSA_INIT_INPUT_CTRL_CHANNEL,
    TASK_TYPE_IMSA_CURSOR_UPDATE,
    TASK_TYPE_IMSA_SELECTION_CHANGE,
    TASK_TYPE_IMSA_ATTRIBUTE_CHANGE,
    TASK_TYPE_IMSA_STOP_INPUT_SERVICE,
    TASK_TYPE_IMSA_SET_SUBPROPERTY,
    TASK_TYPE_IMSA_SET_CORE_AND_AGENT,
    TASK_TYPE_IMSA_END = TASK_TYPE_IMSA_SET_CORE_AND_AGENT,

    // Task from inner
    TASK_TYPE_RESUME,
};

class Task {
public:
    explicit Task(TaskType t);
    Task(TaskType t, uint64_t seqId);
    virtual ~Task() = default;

    RunningState Execute();
    RunningState Resume(uint64_t resumeId);

    virtual RunningState OnTask(std::shared_ptr<Task> task);

    int32_t Pend(std::shared_ptr<Task> task);
    int32_t Pend(std::unique_ptr<Action> action);

    TaskType GetType() const;
    SourceType GetSourceType() const;
    uint64_t GetSeqId() const;
    RunningState GetState() const;
    bool IsRunning() const;
    const std::list<std::unique_ptr<Action>> &GetActions() const;

    static uint64_t GetNextSeqId();

private:
    RunningState ExecuteInner();

protected:
    const TaskType type_;
    RunningState state_ { RUNNING_STATE_IDLE };
    const uint64_t seqId_;
    std::unique_ptr<Action> curAction_ { nullptr };
    std::list<std::unique_ptr<Action>> actions_;
    std::list<std::unique_ptr<Action>> pendingActions_;
};

} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_TASKS_TASK_H