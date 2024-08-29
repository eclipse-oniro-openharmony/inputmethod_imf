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

#ifndef FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_ACTIONS_ACTION_WAIT_H
#define FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_ACTIONS_ACTION_WAIT_H

#include <cstdint>
#include <functional>

#include "action.h"
#include "task_manager.h"
#include "tasks/task_inner.h"

namespace OHOS {
namespace MiscServices {

class ActionWait : public Action {
public:
    using callback_t = std::function<void()>;

    ActionWait(uint64_t completeId, uint32_t timeoutMs)
        : timeoutMs_(timeoutMs),
          completeId_(completeId),
          timeoutId_(Task::GetNextSeqId())
    {
    }

    ActionWait(uint64_t completeId, uint32_t timeoutMs, callback_t onComplete, callback_t onTimeout)
        : timeoutMs_(timeoutMs),
          completeId_(completeId),
          timeoutId_(Task::GetNextSeqId()),
          onComplete_(onComplete),
          onTimeout_(onTimeout)
    {
    }

    ~ActionWait() = default;

    RunningState Execute() override
    {
        state_ = RUNNING_STATE_PAUSED;
        // trigger timeout with delay
        auto task = std::make_shared<TaskResume>(timeoutId_);
        TaskManager::GetInstance().PostTask(task, timeoutMs_);
        return state_;
    }

    RunningState Resume(uint64_t seqId) override
    {
        if (state_ != RUNNING_STATE_PAUSED) {
            return RUNNING_STATE_ERROR;
        }

        if (seqId == completeId_) {
            if (onComplete_) {
                onComplete_();
            }
            state_ = RUNNING_STATE_COMPLETED;
            return state_;
        }
        if (seqId == timeoutId_) {
            if (onTimeout_) {
                onTimeout_();
            }
            state_ = RUNNING_STATE_COMPLETED;
            return state_;
        }

        return state_;
    }

private:
    const uint32_t timeoutMs_;
    const uint64_t completeId_;
    const uint64_t timeoutId_;
    std::function<void()> onComplete_;
    std::function<void()> onTimeout_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_ACTIONS_ACTION_WAIT_H