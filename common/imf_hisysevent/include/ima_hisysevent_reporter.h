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

#ifndef IMA_HISYSEVENT_REPORTER_H
#define IMA_HISYSEVENT_REPORTER_H

#include <cstdint>
#include <map>
#include <vector>

#include "global.h"
#include "imf_hisysevent_reporter.h"

namespace OHOS {
namespace MiscServices {
struct ImeCbTimeConsumeStatistics : public Serializable {
    explicit ImeCbTimeConsumeStatistics(uint32_t num)
    {
        countDistributions.resize(num);
    }
    bool Marshal(cJSON *node) const override
    {
        auto ret = SetValue(node, GET_NAME(COUNT), count);
        std::string countInfo;
        for (auto i = 0; i < countDistributions.size(); i++) {
            countInfo.append(std::to_string(countDistributions[i]));
            if (i != countDistributions.size() - 1) {
                countInfo.append("/");
            }
        }
        return SetValue(node, GET_NAME(COUNT_DISTRIBUTION), countInfo) && ret;
    }
    void Mod(uint32_t intervalIndex)
    {
        count++;
        if (intervalIndex >= countDistributions.size()) {
            return;
        }
        countDistributions[intervalIndex]++;
    }
    int32_t count{ 0 };
    std::vector<uint32_t> countDistributions;
};

struct ImeStartInputAllInfo : public Serializable {
    ImeStartInputAllInfo(uint32_t succeedIntervalNum, uint32_t failedIntervalNum, uint32_t timeConsumeIntervalNum)
        : succeedRateInfo(SuccessRateStatistics(succeedIntervalNum, failedIntervalNum)),
          imeCbTimeConsumeInfo(ImeCbTimeConsumeStatistics(timeConsumeIntervalNum))
    {
    }
    bool Marshal(cJSON *node) const override
    {
        auto ret = succeedRateInfo.Marshal(node);
        return SetValue(node, GET_NAME(CB_TIME_CONSUME), imeCbTimeConsumeInfo) && ret;
    }
    std::vector<std::string> appNames;
    SuccessRateStatistics succeedRateInfo;
    ImeCbTimeConsumeStatistics imeCbTimeConsumeInfo;
};

struct BaseTextOperationAllInfo : public Serializable {
    BaseTextOperationAllInfo(uint32_t succeedIntervalNum, uint32_t failedIntervalNum)
        : succeedRateInfo(SuccessRateStatistics(succeedIntervalNum, failedIntervalNum))
    {
    }
    std::vector<std::string> appNames;
    SuccessRateStatistics succeedRateInfo;
};

class ImaHiSysEventReporter : public ImfHiSysEventReporter {
public:
    ~ImaHiSysEventReporter() override;
    static ImaHiSysEventReporter &GetInstance();

private:
    ImaHiSysEventReporter();
    static const std::vector<std::pair<uint32_t, uint32_t>> BASE_TEXT_OPERATION_TIME_INTERVAL;
    static const std::vector<std::pair<uint32_t, uint32_t>> IME_CB_TIME_INTERVAL;
    bool IsValidErrCode(int32_t errCode) override;
    bool IsFault(int32_t errCode) override;
    void RecordStatisticsEvent(ImfStatisticsEvent event, const HiSysOriginalInfo &info) override;
    void ReportStatisticsEvent() override;
    void RecordImeStartInputStatistics(const HiSysOriginalInfo &info);
    void ModImeCbTimeConsumeInfo(int32_t imeCbTime);
    void RecordBaseTextOperationStatistics(const HiSysOriginalInfo &info);
    uint32_t GetBaseTextOperationSucceedIntervalIndex(int32_t baseTextOperationTime);
    std::mutex statisticsEventLock_;
    ImeStartInputAllInfo imeStartInputAllInfo_;
    BaseTextOperationAllInfo baseTextOperationAllInfo_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMA_HISYSEVENT_REPORTER_H