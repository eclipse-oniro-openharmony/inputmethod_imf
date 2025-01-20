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
std::mutex ImaHiSysEventReporter::instanceLock_;
sptr<ImaHiSysEventReporter> ImaHiSysEventReporter::instance_;
sptr<ImaHiSysEventReporter> ImaHiSysEventReporter::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(instanceLock_);
        if (instance_ == nullptr) {
            instance_ = new (std::nothrow) ImaHiSysEventReporter();
        }
    }
    return instance_;
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
    std::string imeStartInputStatistics;
    imeStartInputAllInfo_.Marshall(imeStartInputStatistics);
    ImfHiSysEventUtil::ReportStatisticsEvent(GET_NAME(IME_START_INPUT_STATISTICS), GetSelfName(),
        imeStartInputAllInfo_.appNames, { imeStartInputStatistics });
    imeStartInputAllInfo_ = ImeStartInputAllInfo(
        COUNT_STATISTICS_INTERVAL_NUM, COUNT_STATISTICS_INTERVAL_NUM, IME_CB_TIME_INTERVAL.size());
    std::string baseTextOperationStatistics;
    baseTextOperationAllInfo_.succeedRateInfo.Marshall(baseTextOperationStatistics);
    ImfHiSysEventUtil::ReportStatisticsEvent(GET_NAME(BASE_TEXT_OPERATION_STATISTICS), GetSelfName(),
        baseTextOperationAllInfo_.appNames, { baseTextOperationStatistics });
    baseTextOperationAllInfo_ =
        BaseTextOperationAllInfo(BASE_TEXT_OPERATION_TIME_INTERVAL.size(), COUNT_STATISTICS_INTERVAL_NUM);
}

void ImaHiSysEventReporter::RecordImeStartInputStatistics(const HiSysOriginalInfo &info)
{
    std::lock_guard<std::mutex> lock(imeStartInputInfoLock_);
    imeStartInputAllInfo_.appNames.insert(info.peerName);
    ModImeCbTimeConsumeInfo(info.imeCbTime);
    auto intervalIndex = GetStatisticalIntervalIndex();
    auto appIndex = ImfHiSysEventUtil::GetIndexInSet(info.peerName, imeStartInputAllInfo_.appNames);
    std::string key(appIndex);
    if (info.errCode == ErrorCode::NO_ERROR) {
        key.append("/").append(std::to_string(info.isShowKeyboard));
        imeStartInputAllInfo_.succeedRateInfo.succeedInfo.Mod(intervalIndex, key);
        return;
    }
    key.append("/").append(std::to_string(info.eventCode)).append("/").append(std::to_string(info.errCode));
    imeStartInputAllInfo_.succeedRateInfo.failedInfo.Mod(intervalIndex, key);
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

    imeStartInputAllInfo_.imeCbTimeConsumeInfo.Mod(index);
}

void ImaHiSysEventReporter::RecordBaseTextOperationStatistics(const HiSysOriginalInfo &info)
{
    std::lock_guard<std::mutex> lock(baseTextOperationLock_);
    baseTextOperationAllInfo_.appNames.insert(info.peerName);
    auto appIndex = ImfHiSysEventUtil::GetIndexInSet(info.peerName, baseTextOperationAllInfo_.appNames);
    std::string key(appIndex);
    if (info.errCode == ErrorCode::NO_ERROR) {
        key.append("/").append(std::to_string(info.eventCode));
        baseTextOperationAllInfo_.succeedRateInfo.succeedInfo.Mod(GetStatisticalIntervalIndex(), key);
        return;
    }
    key.append("/")
        .append(std::to_string(info.clientType))
        .append("/")
        .append(std::to_string(info.eventCode))
        .append("/")
        .append(std::to_string(info.errCode));
    baseTextOperationAllInfo_.succeedRateInfo.failedInfo.Mod(
        GetBaseTextOperationSucceedIntervalIndex(info.baseTextOperationTime), key);
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