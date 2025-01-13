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

#include "ima_hisysevent_reporter.h"

#include <chrono>

namespace OHOS {
namespace MiscServices {
using namespace std::chrono;
ImaHiSysEventReporter::ImaHiSysEventReporter()
{
    imeStartInputAllInfo_.successRateStatistics.succeedInfo.countDistributions.resize(COUNT_STATISTICS_INTERVAL_NUM);
    imeStartInputAllInfo_.successRateStatistics.succeedInfo.countDistributions.resize(COUNT_STATISTICS_INTERVAL_NUM);
    imeStartInputAllInfo_.timeConsumeStatistics.countDistributions.resize(IME_CB_TIME_INTERVAL.size());
    baseTextOperationAllInfo_.statistics.succeedInfo.countDistributions.resize(BASE_TEXT_OPERATOR_TIME_INTERVAL.size());
    baseTextOperationAllInfo_.statistics.failedInfo.countDistributions.resize(COUNT_STATISTICS_INTERVAL_NUM);
}

ImaHiSysEventReporter::~ImaHiSysEventReporter()
{
}

bool MiscServices::ImaHiSysEventReporter::IsValidErrCode(int32_t errCode)
{
    return !((ErrorCode::ERROR_IMC_BEGIN < errCode && errCode < ErrorCode::ERROR_IMC_END)
             || (ErrorCode::ERROR_IMSA_BEGIN < errCode && errCode < ErrorCode::ERROR_IMSA_END));
}

bool ImaHiSysEventReporter::IsFault(int32_t errCode)
{
    return IsValidErrCode(errCode);
}

void ImaHiSysEventReporter::RecordStatisticsEvent(ImfStatisticsEvent event, const HiSysOriginalInfo &info)
{
    switch (event) {
        case ImfStatisticsEvent::IME_START_INPUT_STATISTICS: {
            RecordImeStartInputEvent(info);
            break;
        }
        case ImfStatisticsEvent::BASE_TEXT_OPERATOR_STATISTICS: {
            RecordBaseTextOperationEvent(info);
            break;
        }
        default:
            break;
    }
}

void ImaHiSysEventReporter::RecordImeStartInputEvent(const HiSysOriginalInfo &info)
{
    std::lock_guard<std::mutex> lock(imeStartInputInfoLock_);
    imeStartInputAllInfo_.appNames.insert(info.peerName);
    ModCbTimeConsumeStatistics(info.imeCbTime);
    auto intervalIndex = GetStatisticalIntervalIndex();
    auto appIndex = GetIndexInSet(info.peerName, imeStartInputAllInfo_.appNames);
    if (info.errCode == ErrorCode::NO_ERROR) {
        auto key = appIndex + "/" + std::to_string(info.isShowKeyboard);
        ModCountDistributionInfo(intervalIndex, key, imeStartInputAllInfo_.statistics.succeedInfo);
        return;
    }
    auto key = appIndex + "/" + std::to_string(info.eventCode) + "/" + std::to_string(info.errCode);
    ModCountDistributionInfo(intervalIndex, key, imeStartInputAllInfo_.statistics.failedInfo);
}

void ImaHiSysEventReporter::ModCbTimeConsumeStatistics(int32_t imeCbTime)
{
    if (imeCbTime < 0) {
        return;
    }
    imeStartInputAllInfo_.timeConsumeStatistics.count++;
    int8_t index = -1;
    for (auto i = 0; i < IME_CB_TIME_INTERVAL.size() - 1; i++) {
        if (IME_CB_TIME_INTERVAL[i].first <= imeCbTime && imeCbTime <= IME_CB_TIME_INTERVAL[i].second) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        index = IME_CB_TIME_INTERVAL.size() - 1;
    }
    imeStartInputAllInfo_.timeConsumeStatistics.countDistributions[index]++;
}

void ImaHiSysEventReporter::RecordBaseTextOperationEvent(const HiSysOriginalInfo &info)
{
    std::lock_guard<std::mutex> lock(baseTextOperationLock_);
    baseTextOperationAllInfo_.appNames.insert(info.peerName);
    auto appIndex = GetIndexInSet(info.peerName, baseTextOperationAllInfo_.appNames);
    if (info.errCode == ErrorCode::NO_ERROR) {
        auto key = appIndex + "/" + std::to_string(info.eventCode);
        ModCountDistributionInfo(GetStatisticalIntervalIndex(), key, baseTextOperationAllInfo_.statistics.succeedInfo);
        return;
    }
    auto key = appIndex + "/" + std::to_string(info.clientType) + "/" + std::to_string(info.eventCode) + "/"
               + std::to_string(info.errCode);
    ModCountDistributionInfo(GetBaseTextOperationSucceedIntervalIndex(info.baseTextOperationTime), key,
        baseTextOperationAllInfo_.statistics.failedInfo);
}

uint8_t ImaHiSysEventReporter::GetBaseTextOperationSucceedIntervalIndex(int32_t baseTextOperationTime)
{
    if (baseTextOperationTime < 0) {
        return 0;
    }
    int8_t index = -1;
    for (auto i = 0; i < BASE_TEXT_OPERATOR_TIME_INTERVAL.size() - 1; i++) {
        if (BASE_TEXT_OPERATOR_TIME_INTERVAL[i].first <= baseTextOperationTime
            && baseTextOperationTime <= BASE_TEXT_OPERATOR_TIME_INTERVAL[i].second) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        index = BASE_TEXT_OPERATOR_TIME_INTERVAL.size() - 1;
    }
    return index;
}

void ImaHiSysEventReporter::ReportStatisticsEvent()
{
    std::string imeStartInputStatistics;
    imeStartInputAllInfo_.Marshal(imeStartInputStatistics);
    ImfHiSysEventUtil::ReportStatisticsEvent(GET_NAME(IME_START_INPUT_STATISTICS), { GetSelfName() },
        imeStartInputAllInfo_.appNames, imeStartInputStatistics);
    // todo imeStartInputAllInfo_清空
    std::string baseTextOperationStatistics;
    baseTextOperationAllInfo_.statistics.Marshal(baseTextOperationStatistics);
    ImfHiSysEventUtil::ReportStatisticsEvent(GET_NAME(BASE_TEXT_OPERATOR_STATISTICS), { GetSelfName() },
        baseTextOperationAllInfo_.appNames, baseTextOperationStatistics);
    // todo baseTextOperationAllInfo_
}
} // namespace MiscServices
} // namespace OHOS