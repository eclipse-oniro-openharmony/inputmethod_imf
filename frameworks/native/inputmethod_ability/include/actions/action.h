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

#ifndef FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_ACTIONS_ACTION_H
#define FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_ACTIONS_ACTION_H

#include <cstdint>
#include <functional>

namespace OHOS {
namespace MiscServices {

enum RunningState : uint32_t {
    RUNNING_STATE_IDLE = 0,
    RUNNING_STATE_RUNNING,
    RUNNING_STATE_PAUSED,
    RUNNING_STATE_COMPLETED,
    RUNNING_STATE_ERROR,
};

class Action {
public:
    Action() = default;
    Action(std::function<void()> func) : func_(func) { }
    virtual ~Action() = default;

    virtual RunningState Execute()
    {
        if (state_ != RUNNING_STATE_IDLE) {
            return RUNNING_STATE_ERROR;
        }

        state_ = RUNNING_STATE_RUNNING;
        if (func_) {
            func_();
        }
        state_ = RUNNING_STATE_COMPLETED;
        return state_;
    }

    virtual RunningState Resume(uint64_t resumeId)
    {
        return state_;
    }

    RunningState GetState() const
    {
        return state_;
    }

protected:
    RunningState state_ { RUNNING_STATE_IDLE };
    std::function<void()> func_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_ACTIONS_ACTION_H