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
    ClientAttachAllInfo(uint8_t succeedIntervalNum, uint8_t failedIntervalNum)
    {
        succeedRateInfo = SuccessRateStatistics(succeedIntervalNum, failedIntervalNum);
    }
    std::unordered_set<std::string> appNames;
    std::unordered_set<std::string> imeNames;
    SuccessRateStatistics succeedRateInfo;
};

struct ClientShowAllInfo {
    ClientShowAllInfo(uint8_t succeedIntervalNum, uint8_t failedIntervalNum)
    {
        succeedRateInfo = SuccessRateStatistics(succeedIntervalNum, failedIntervalNum);
    }
    std::unordered_set<std::string> appNames;
    std::unordered_set<std::string> imeNames;
    SuccessRateStatistics succeedRateInfo;
};

class ImsaHiSysEventReporter : public ImfHiSysEventReporter {
public:
    ~ImsaHiSysEventReporter() override;
    static std::shared_ptr<ImsaHiSysEventReporter> GetInstance();

private:
    ImsaHiSysEventReporter();
    bool IsValidErrCode(int32_t errCode) override;
    bool IsFault(int32_t errCode) override;
    void RecordStatisticsEvent(ImfStatisticsEvent event, const HiSysOriginalInfo &info) override;
    void ReportStatisticsEvent() override;
    void RecordClientAttachStatistics(const HiSysOriginalInfo &info);
    void RecordClientShowStatistics(const HiSysOriginalInfo &info);
    static std::mutex instanceLock_;
    static std::shared_ptr<ImsaHiSysEventReporter> instance_;
    std::mutex clientAttachInfoLock_;
    ClientAttachAllInfo clientAttachInfo_;

    std::mutex clientShowInfoLock_;
    ClientShowAllInfo clientShowInfo_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMSA_HISYSEVENT_H