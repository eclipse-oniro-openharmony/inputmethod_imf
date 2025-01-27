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

#ifndef IMC_HISYSEVENT_REPORTER_H
#define IMC_HISYSEVENT_REPORTER_H

#include <cstdint>

#include "imf_hisysevent_reporter.h"

namespace OHOS {
namespace MiscServices {
class ImcHiSysEventReporter : public ImfHiSysEventReporter {
public:
    ~ImcHiSysEventReporter() override;
    static ImcHiSysEventReporter &GetInstance();

private:
    ImcHiSysEventReporter();
    bool IsValidErrCode(int32_t errCode) override;
    bool IsFault(int32_t errCode) override;
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMC_HISYSEVENT_REPORTER_H