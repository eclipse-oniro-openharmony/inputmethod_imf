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

#ifndef IMF_SERVICES_SA_ACTION_REPORT_H
#define IMF_SERVICES_SA_ACTION_REPORT_H

#include "sa_action.h"

namespace OHOS {
namespace MiscServices {
using ReportFunc = std::function<void(int32_t, const ServiceResponseData &)>;
class SaActionReport : public SaAction {
public:
    SaActionReport() = default;
    explicit SaActionReport(ReportFunc func) : func_(std::move(func))
    {
    }
    ~SaActionReport() = default;

    RunningState Execute(int32_t &ret, ServiceResponseData &responseData) override
    {
        if (state_ != RUNNING_STATE_IDLE) {
            return RUNNING_STATE_ERROR;
        }

        state_ = RUNNING_STATE_RUNNING;
        if (func_ != nullptr) {
            func_(ret, responseData);
        }
        state_ = RUNNING_STATE_COMPLETED;
        return state_;
    }

private:
    ReportFunc func_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_SERVICES_SA_ACTION_REPORT_H
