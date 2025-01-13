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

#ifndef IMSA_HISYSEVENT_H
#define IMSA_HISYSEVENT_H

#include <cstdint>
#include <map>
#include <unordered_set>
#include <vector>

#include "global.h"
#include "imf_hisysevent_reporter.h"
namespace OHOS {
namespace MiscServices {
struct ClientAttachAllInfo {
    std::unordered_set<std::string> appNames;
    std::unordered_set<std::string> imeNames;
    SuccessRateStatistics statistics;
};

struct ClientShowAllInfo {
    std::unordered_set<std::string> appNames;
    std::unordered_set<std::string> imeNames;
    SuccessRateStatistics statistics;
};

class ImsaHiSysEventReporter : public ImfHiSysEventReporter {
public:
    ImsaHiSysEventReporter();
    ~ImsaHiSysEventReporter();
    bool IsValidErrCode(int32_t errCode) override;
    bool IsFault(int32_t errCode) override;
    void RecordStatisticsEvent(ImfStatisticsEvent event, const HiSysOriginalInfo &info) override;
    void ReportStatisticsEvent() override;
    std::pair<int64_t, std::string> GetCurrentImeInfo(int32_t userId);

private:
    void RecordClientAttachEvent(const HiSysOriginalInfo &info);
    void RecordClientShowEvent(const HiSysOriginalInfo &info);

    std::mutex clientAttachInfoLock_;
    ClientAttachAllInfo clientAttachInfo_;

    std::mutex clientShowInfoLock_;
    ClientShowAllInfo clientShowInfo_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMSA_HISYSEVENT_H