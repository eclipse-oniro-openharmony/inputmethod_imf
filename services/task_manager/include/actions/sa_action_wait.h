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

#ifndef IMF_SERVICES_SA_ACTION_WAIT_H
#define IMF_SERVICES_SA_ACTION_WAIT_H

#include <utility>

#include "sa_action.h"
#include "sa_task.h"

namespace OHOS {
namespace MiscServices {
class SaActionWait : public SaAction {
public:
    SaActionWait(uint32_t timeoutMs, PauseInfo info)
        : completeId_(SaTask::GetNextSeqId()), timeoutId_(SaTask::GetNextSeqId()), timeoutMs_(timeoutMs),
          pauseInfo_(std::move(info)), onComplete_(nullptr), onTimeout_(nullptr)
    {
        pauseInfo_.resumeId = completeId_;
    }
    SaActionWait(uint32_t timeoutMs, PauseInfo info, SaActionFunc onComplete, SaActionFunc onTimeout)
        : completeId_(SaTask::GetNextSeqId()), timeoutId_(SaTask::GetNextSeqId()), timeoutMs_(timeoutMs),
          pauseInfo_(std::move(info)), onComplete_(std::move(onComplete)), onTimeout_(std::move(onTimeout))
    {
        pauseInfo_.resumeId = completeId_;
    }

    ~SaActionWait() = default;

    RunningState Execute(int32_t &ret, ServiceResponseData &responseData) override;
    RunningState Execute(int32_t &ret, ServiceResponseData &responseData, ActionInnerData &innerData) override;

    RunningState Resume(uint64_t resumedId, int32_t &ret, ServiceResponseData &data) override;
    RunningState Resume(
        uint64_t resumedId, int32_t &ret, ServiceResponseData &data, ActionInnerData &innerData) override;

    PauseInfo GetPausedInfo() override;

    RunningState GetState() override;

    bool IsWaitAction() override;

private:
    const uint64_t completeId_;
    const uint64_t timeoutId_;
    const uint32_t timeoutMs_;
    PauseInfo pauseInfo_{};
    SaActionFunc onComplete_;
    SaActionFunc onTimeout_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // IMF_SERVICES_SA_ACTION_WAIT_H
