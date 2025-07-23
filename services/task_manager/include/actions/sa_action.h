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

#ifndef IMF_SERVICES_SA_ACTION_NEW_H
#define IMF_SERVICES_SA_ACTION_NEW_H

#include <list>
#include <utility>

#include "action.h"
#include "input_client_info.h"
#include "service_response_data.h"

namespace OHOS {
namespace MiscServices {
static constexpr int32_t INVALID_RET_CODE = -1;
enum class PauseType : int32_t {
    PAUSED_TYPE_INVALID = -1,
    PAUSE_TYPE_START_IME = 0,
    PAUSE_TYPE_STOP_IME = 1,
};

struct PauseInfo {
    PauseType type{ PauseType::PAUSED_TYPE_INVALID }; // pause type
    std::string target;                               // bundleName of waiting target
    uint64_t resumeId{ 0 };
    inline std::string ToString() const
    {
        std::stringstream ss;
        ss << "PausedInfo:["
           << "type: " << static_cast<int32_t>(type) << " target: " << target << " resumeId: " << resumeId
           << "]";
        return ss.str();
    }
};

using ActionInnerData = std::variant<std::monostate, ImeLaunchType, InputClientInfo>;
using ResultHandler = std::function<void(int32_t)>;
using SaActionFunc = std::function<int32_t(ServiceResponseData &data, ActionInnerData &innerData)>;
class SaAction {
public:
    SaAction() = default;
    explicit SaAction(SaActionFunc func) : func_(std::move(func)), isResultAffectParent_(true)
    {
    }
    SaAction(SaActionFunc func, ResultHandler onSuccess, ResultHandler onFailure)
        : func_(std::move(func)), onSuccess_(std::move(onSuccess)), onFailure_(std::move(onFailure)),
          isResultAffectParent_(true), failureCode_(INVALID_RET_CODE)
    {
    }
    SaAction(SaActionFunc func, ResultHandler onSuccess, ResultHandler onFailure, bool isAffect,
        int32_t failureCode = INVALID_RET_CODE)
        : func_(std::move(func)), onSuccess_(std::move(onSuccess)), onFailure_(std::move(onFailure)),
          isResultAffectParent_(isAffect), failureCode_(failureCode)
    {
    }
    virtual ~SaAction() = default;

    virtual RunningState Execute(int32_t &ret, ServiceResponseData &responseData);
    virtual RunningState Resume(uint64_t resumedId, int32_t &ret, ServiceResponseData &data);
    virtual int32_t Pend(std::unique_ptr<SaAction> action);
    virtual PauseInfo GetPausedInfo();

    virtual RunningState Execute(int32_t &ret, ServiceResponseData &responseData, ActionInnerData &innerData);
    virtual RunningState Resume(
        uint64_t resumedId, int32_t &ret, ServiceResponseData &data, ActionInnerData &innerData);

    virtual RunningState GetState();

    virtual bool IsWaitAction();

protected:
    RunningState state_{ RUNNING_STATE_IDLE };

private:
    RunningState ExecuteInner(int32_t &ret, ServiceResponseData &responseData);

    RunningState ExecuteImpl(ServiceResponseData &responseData);

    bool hasFuncExecuted_{ false };
    SaActionFunc func_;
    ResultHandler onSuccess_{ nullptr };
    ResultHandler onFailure_{ nullptr };

    bool isResultAffectParent_{ true };

    std::unique_ptr<SaAction> curSubAction_{ nullptr };
    std::list<std::unique_ptr<SaAction>> subActions_;

    int32_t retCode_{ 0 };
    int32_t failureCode_{ INVALID_RET_CODE };
    ActionInnerData innerData_{ std::monostate{} };
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_SERVICES_SA_ACTION_NEW_H
