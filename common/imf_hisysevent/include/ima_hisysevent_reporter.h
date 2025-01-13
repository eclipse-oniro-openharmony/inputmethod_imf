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

#ifndef IMA_HISYSEVENT_H
#define IMA_HISYSEVENT_H

#include <cstdint>
#include <map>
#include <vector>

#include "global.h"
#include "imf_hisysevent_reporter.h"

namespace OHOS {
namespace MiscServices {
struct ImeCbTimeConsumeStatistics : public Serializable {
    int32_t count;
    std::vector<int32_t> countDistributions;
    bool Marshal(cJSON *node) const override
    {
        auto ret = SetValue(node, GET_NAME(COUNT), count);
        std::string countInfo;
        for (const auto &num : countDistributions) {
            countInfo += std::to_string(num);
        }
        return SetValue(node, GET_NAME(COUNT_DISTRIBUTION), countInfo) && ret;
    }
};

struct ImeStartInputAllInfo : public Serializable {
    std::unordered_set<std::string> appNames;
    SuccessRateStatistics statistics;
    ImeCbTimeConsumeStatistics timeConsumeStatistics;
    bool Marshal(cJSON *node) const override
    {
        auto ret = statistics.Marshal(node);
        return SetValue(node, GET_NAME(CB_TIME_CONSUME), timeConsumeStatistics) && ret;
    }
};

struct BaseTextOperationAllInfo : public Serializable {
    std::unordered_set<std::string> appNames;
    SuccessRateStatistics statistics;
};

class ImaHiSysEventReporter : public ImfHiSysEventReporter {
public:
    ImaHiSysEventReporter();
    ~ImaHiSysEventReporter();
    bool IsValidErrCode(int32_t errCode) override;
    bool IsFault(int32_t errCode) override;
    void RecordStatisticsEvent(ImfStatisticsEvent event, const HiSysOriginalInfo &info) override;
    void ReportStatisticsEvent() override;

private:
    const std::vector<std::pair<uint8_t, uint8_t>> BASE_TEXT_OPERATOR_TIME_INTERVAL = { { 0, 4 }, { 4, 8 }, { 8, 16 },
        { 16, 24 }, { 24, 500 } }; // 0-4ms 4-8ms 8-16  16-24  24+
    const std::vector<std::pair<uint8_t, uint8_t>> IME_CB_TIME_INTERVAL = { { 0, 10 }, { 10, 50 }, { 50, 100 },
        { 100, 300 }, { 300, 500 }, { 500, 1000 } }; // 0-10ms 10-50ms 50-100  100-300 300-500 500+
    void RecordImeStartInputEvent(const HiSysOriginalInfo &info);
    void ModCbTimeConsumeStatistics(int32_t imeCbTime);
    void RecordBaseTextOperationEvent(const HiSysOriginalInfo &info);
    uint8_t GetBaseTextOperationSucceedIntervalIndex(int32_t baseTextOperationTime);
    std::mutex imeStartInputInfoLock_;
    ImeStartInputAllInfo imeStartInputAllInfo_;
    std::mutex baseTextOperationLock_;
    BaseTextOperationAllInfo baseTextOperationAllInfo_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMA_HISYSEVENT_H