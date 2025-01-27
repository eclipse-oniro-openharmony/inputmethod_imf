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
const std::vector<std::pair<uint32_t, uint32_t>> ImaHiSysEventReporter::BASE_TEXT_OPERATION_TIME_INTERVAL = { { 0, 4 },
    { 4, 8 }, { 8, 16 }, { 16, 24 }, { 24, 500 } }; // 0-4ms 4-8ms 8-16ms 16-24ms  24ms+
const std::vector<std::pair<uint32_t, uint32_t>> ImaHiSysEventReporter::IME_CB_TIME_INTERVAL = { { 0, 10 }, { 10, 50 },
    { 50, 100 }, { 100, 500 }, { 500, 1000 }, { 1000, 2000 } }; // 0-10ms 10-50ms 50-100  100-500 500-500 1000+
ImaHiSysEventReporter &ImaHiSysEventReporter::GetInstance()
{
    static ImaHiSysEventReporter instance;
    return instance;
}

ImaHiSysEventReporter::ImaHiSysEventReporter()
    : imeStartInputAllInfo_(ImeStartInputAllInfo(
        COUNT_STATISTICS_INTERVAL_NUM, COUNT_STATISTICS_INTERVAL_NUM, IME_CB_TIME_INTERVAL.size())),
      baseTextOperationAllInfo_(
          BaseTextOperationAllInfo(BASE_TEXT_OPERATION_TIME_INTERVAL.size(), COUNT_STATISTICS_INTERVAL_NUM))
{
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
    std::lock_guard<std::mutex> lock(statisticsEventLock_);
    switch (event) {
        case ImfStatisticsEvent::IME_START_INPUT_STATISTICS: {
            RecordImeStartInputStatistics(info);
            break;
        }
        case ImfStatisticsEvent::BASE_TEXT_OPERATION_STATISTICS: {
            RecordBaseTextOperationStatistics(info);
            break;
        }
        default:
            break;
    }
}

void ImaHiSysEventReporter::ReportStatisticsEvent()
{
    ImeStartInputAllInfo imeStartInputInfo(
        COUNT_STATISTICS_INTERVAL_NUM, COUNT_STATISTICS_INTERVAL_NUM, IME_CB_TIME_INTERVAL.size());
    BaseTextOperationAllInfo baseTextOperationInfo(
        BASE_TEXT_OPERATION_TIME_INTERVAL.size(), COUNT_STATISTICS_INTERVAL_NUM);
    {
        std::lock_guard<std::mutex> lock(statisticsEventLock_);
        auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        ResetTimerStartTime(time);
        imeStartInputInfo = imeStartInputAllInfo_;
        baseTextOperationInfo = baseTextOperationAllInfo_;
        imeStartInputAllInfo_ = ImeStartInputAllInfo(
            COUNT_STATISTICS_INTERVAL_NUM, COUNT_STATISTICS_INTERVAL_NUM, IME_CB_TIME_INTERVAL.size());
        baseTextOperationAllInfo_ =
            BaseTextOperationAllInfo(BASE_TEXT_OPERATION_TIME_INTERVAL.size(), COUNT_STATISTICS_INTERVAL_NUM);
    }
    if (!imeStartInputInfo.appNames.empty()) {
        std::string imeStartInputStatistics;
        imeStartInputInfo.Marshall(imeStartInputStatistics);
        ImfHiSysEventUtil::ReportStatisticsEvent(GET_NAME(IME_START_INPUT_STATISTICS), GetSelfName(),
            imeStartInputInfo.appNames, { imeStartInputStatistics });
    }
    if (!baseTextOperationInfo.appNames.empty()) {
        std::string baseTextOperationStatistics;
        baseTextOperationInfo.succeedRateInfo.Marshall(baseTextOperationStatistics);
        ImfHiSysEventUtil::ReportStatisticsEvent(GET_NAME(BASE_TEXT_OPERATION_STATISTICS), GetSelfName(),
            baseTextOperationInfo.appNames, { baseTextOperationStatistics });
    }
}

void ImaHiSysEventReporter::RecordImeStartInputStatistics(const HiSysOriginalInfo &info)
{
    std::string appName = "*";  // for security reasons, the package name is not printed at this time
    auto appIndex = ImfHiSysEventUtil::AddIfAbsent(appName, imeStartInputAllInfo_.appNames);
    ModImeCbTimeConsumeInfo(info.imeCbTime);
    auto intervalIndex = GetStatisticalIntervalIndex();
    std::string key(appIndex);
    if (info.errCode == ErrorCode::NO_ERROR) {
        key.append("/").append(std::to_string(info.isShowKeyboard));
        imeStartInputAllInfo_.succeedRateInfo.succeedInfo.ModCountDistributions(intervalIndex, key);
        return;
    }
    key.append("/").append(std::to_string(info.eventCode)).append("/").append(std::to_string(info.errCode));
    imeStartInputAllInfo_.succeedRateInfo.failedInfo.ModCountDistributions(intervalIndex, key);
}

void ImaHiSysEventReporter::ModImeCbTimeConsumeInfo(int32_t imeCbTime)
{
    if (imeCbTime < 0) {
        return;
    }
    int32_t index = -1;
    for (auto i = 0; i < IME_CB_TIME_INTERVAL.size() - 1; i++) {
        if (IME_CB_TIME_INTERVAL[i].first <= imeCbTime && imeCbTime <= IME_CB_TIME_INTERVAL[i].second) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        index = IME_CB_TIME_INTERVAL.size() - 1;
    }

    imeStartInputAllInfo_.imeCbTimeConsumeInfo.ModCountDistributions(index);
}

void ImaHiSysEventReporter::RecordBaseTextOperationStatistics(const HiSysOriginalInfo &info)
{
    std::string appName = "*";
    auto appIndex = ImfHiSysEventUtil::AddIfAbsent(appName, baseTextOperationAllInfo_.appNames);
    std::string key(appIndex);
    if (info.errCode == ErrorCode::NO_ERROR) {
        key.append("/").append(std::to_string(info.eventCode));
        baseTextOperationAllInfo_.succeedRateInfo.succeedInfo.ModCountDistributions(
            GetBaseTextOperationSucceedIntervalIndex(info.baseTextOperationTime), key);
        return;
    }
    key.append("/")
        .append(std::to_string(info.clientType))
        .append("/")
        .append(std::to_string(info.eventCode))
        .append("/")
        .append(std::to_string(info.errCode));
    baseTextOperationAllInfo_.succeedRateInfo.failedInfo.ModCountDistributions(GetStatisticalIntervalIndex(), key);
}

uint32_t ImaHiSysEventReporter::GetBaseTextOperationSucceedIntervalIndex(int32_t baseTextOperationTime)
{
    if (baseTextOperationTime < 0) {
        return 0;
    }
    int32_t index = -1;
    for (auto i = 0; i < BASE_TEXT_OPERATION_TIME_INTERVAL.size() - 1; i++) {
        if (BASE_TEXT_OPERATION_TIME_INTERVAL[i].first <= baseTextOperationTime
            && baseTextOperationTime <= BASE_TEXT_OPERATION_TIME_INTERVAL[i].second) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        index = BASE_TEXT_OPERATION_TIME_INTERVAL.size() - 1;
    }
    return index;
}
} // namespace MiscServices
} // namespace OHOS