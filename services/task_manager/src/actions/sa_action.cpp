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

#include "sa_action.h"

#include "variant_util.h"

namespace OHOS {
namespace MiscServices {
RunningState SaAction::Execute(int32_t &ret, ServiceResponseData &responseData)
{
    if (state_ != RUNNING_STATE_IDLE) {
        return RUNNING_STATE_ERROR;
    }
    return ExecuteInner(ret, responseData);
}

RunningState SaAction::Execute(int32_t &ret, ServiceResponseData &responseData, ActionInnerData &innerData)
{
    if (state_ != RUNNING_STATE_IDLE) {
        return RUNNING_STATE_ERROR;
    }
    innerData_ = innerData;
    auto state = ExecuteInner(ret, responseData);
    innerData = innerData_;
    return state;
}

RunningState SaAction::Resume(uint64_t resumedId, int32_t &ret, ServiceResponseData &data)
{
    if (curSubAction_ == nullptr) {
        IMSA_HILOGE("curSubAction_ is nullptr");
        return RUNNING_STATE_ERROR;
    }

    if (state_ != RUNNING_STATE_PAUSED) {
        IMSA_HILOGE("state_ is %{public}u, do not need to resume", state_);
        return RUNNING_STATE_ERROR;
    }

    auto state = curSubAction_->Resume(resumedId, ret, data);
    if (state == RUNNING_STATE_PAUSED) {
        return state_;
    }
    if (state == RUNNING_STATE_COMPLETED) {
        curSubAction_.reset();
        return ExecuteInner(ret, data);
    }
    IMSA_HILOGE("curAction_ resume return %{public}u, error!", state);
    return RUNNING_STATE_ERROR;
}

RunningState SaAction::Resume(uint64_t resumedId, int32_t &ret, ServiceResponseData &data, ActionInnerData &innerData)
{
    if (curSubAction_ == nullptr) {
        IMSA_HILOGE("curSubAction_ is nullptr");
        return RUNNING_STATE_ERROR;
    }

    if (state_ != RUNNING_STATE_PAUSED) {
        IMSA_HILOGE("state_ is %{public}u, do not need to resume", state_);
        return RUNNING_STATE_ERROR;
    }

    auto state = curSubAction_->Resume(resumedId, ret, data, innerData);
    if (state == RUNNING_STATE_PAUSED) {
        return state_;
    }
    if (state == RUNNING_STATE_COMPLETED) {
        curSubAction_.reset();
        return ExecuteInner(ret, data);
    }
    IMSA_HILOGE("curAction_ resume return %{public}u, error!", state);
    return RUNNING_STATE_ERROR;
}

int32_t SaAction::Pend(std::unique_ptr<SaAction> action)
{
    if (action == nullptr) {
        IMSA_HILOGE("action is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }

    if (state_ != RUNNING_STATE_RUNNING && state_ != RUNNING_STATE_PAUSED) {
        IMSA_HILOGE("curTask_ is not running or paused, pend failed!");
        return ErrorCode::ERROR_SA_TASK_MANAGER_PEND_ACTION_FAILED;
    }

    if (curSubAction_ == nullptr) {
        subActions_.push_back(std::move(action));
        return ErrorCode::NO_ERROR;
    }

    if (curSubAction_->IsWaitAction()) {
        subActions_.push_front(std::move(action));
        return ErrorCode::NO_ERROR;
    }

    return curSubAction_->Pend(std::move(action));
}

PauseInfo SaAction::GetPausedInfo()
{
    if (state_ != RUNNING_STATE_PAUSED) {
        return {};
    }
    if (curSubAction_ == nullptr) {
        return {};
    }
    return curSubAction_->GetPausedInfo();
}

RunningState SaAction::GetState()
{
    if (curSubAction_ == nullptr) {
        return state_;
    }
    return curSubAction_->GetState();
}

RunningState SaAction::ExecuteInner(int32_t &ret, ServiceResponseData &responseData)
{
    state_ = RUNNING_STATE_RUNNING;

    state_ = ExecuteImpl(responseData);

    // state is paused, return.
    if (state_ == RUNNING_STATE_PAUSED) {
        return state_;
    }
    // state error
    if (state_ == RUNNING_STATE_ERROR) {
        return state_;
    }
    // state is RUNNING_STATE_COMPLETED
    ResultHandler handler = nullptr;
    if (retCode_ != ErrorCode::NO_ERROR) {
        // return failureCode_ if valid
        retCode_ = failureCode_ != INVALID_RET_CODE ? failureCode_ : retCode_;
        handler = onFailure_;
    } else {
        handler = onSuccess_;
    }
    if (handler != nullptr) {
        handler(retCode_);
    }
    ret = retCode_;
    return state_;
}

RunningState SaAction::ExecuteImpl(ServiceResponseData &responseData)
{
    // execute func_ first
    if (func_ != nullptr && !hasFuncExecuted_) {
        retCode_ = func_(responseData, innerData_);
        hasFuncExecuted_ = true;
        if (retCode_ != ERR_OK) {
            return RUNNING_STATE_COMPLETED;
        }
    }

    // check sub actions
    while (!subActions_.empty()) {
        curSubAction_ = std::move(subActions_.front());
        subActions_.pop_front();

        int32_t ret = 0;
        auto state = curSubAction_->Execute(ret, responseData, innerData_);

        // current sub action is paused, return.
        if (state == RUNNING_STATE_PAUSED) {
            return RUNNING_STATE_PAUSED;
        }
        // no need to handle current sub action's result.
        if (!curSubAction_->isResultAffectParent_) {
            curSubAction_.reset();
            continue;
        }
        // handle current sub action's result.
        retCode_ = ret;
        curSubAction_.reset();
        if (retCode_ != ErrorCode::NO_ERROR) {
            IMSA_HILOGD("current sub action failed, drop the actions left unexecuted");
            subActions_.clear();
        }
    }

    return RUNNING_STATE_COMPLETED;
}

bool SaAction::IsWaitAction()
{
    return false;
}
} // namespace MiscServices
} // namespace OHOS