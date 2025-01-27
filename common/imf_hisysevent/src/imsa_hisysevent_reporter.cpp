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

#include "imsa_hisysevent_reporter.h"

namespace OHOS {
namespace MiscServices {
using namespace std::chrono;
ImsaHiSysEventReporter &ImsaHiSysEventReporter::GetInstance()
{
    static ImsaHiSysEventReporter instance;
    return instance;
}

ImsaHiSysEventReporter::ImsaHiSysEventReporter()
    : clientAttachInfo_(ClientAttachAllInfo(COUNT_STATISTICS_INTERVAL_NUM, COUNT_STATISTICS_INTERVAL_NUM)),
      clientShowInfo_(ClientShowAllInfo(COUNT_STATISTICS_INTERVAL_NUM, COUNT_STATISTICS_INTERVAL_NUM))
{
}

ImsaHiSysEventReporter::~ImsaHiSysEventReporter()
{
}

bool ImsaHiSysEventReporter::IsValidErrCode(int32_t errCode)
{
    return ErrorCode::ERROR_IMSA_BEGIN < errCode && errCode < ErrorCode::ERROR_IMSA_END;
}

bool ImsaHiSysEventReporter::IsFault(int32_t errCode)
{
    return errCode != ErrorCode::ERROR_STATUS_PERMISSION_DENIED && errCode != ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
}

void ImsaHiSysEventReporter::RecordStatisticsEvent(ImfStatisticsEvent event, const HiSysOriginalInfo &info)
{
    std::lock_guard<std::mutex> lock(statisticsEventLock_);
    switch (event) {
        case ImfStatisticsEvent::CLIENT_ATTACH_STATISTICS: {
            RecordClientAttachStatistics(info);
            break;
        }
        case ImfStatisticsEvent::CLIENT_SHOW_STATISTICS: {
            RecordClientShowStatistics(info);
            break;
        }
        default:
            break;
    }
}

void ImsaHiSysEventReporter::ReportStatisticsEvent()
{
    ClientAttachAllInfo clientAttachInfo(COUNT_STATISTICS_INTERVAL_NUM, COUNT_STATISTICS_INTERVAL_NUM);
    ClientShowAllInfo clientShowInfo(COUNT_STATISTICS_INTERVAL_NUM, COUNT_STATISTICS_INTERVAL_NUM);
    {
        std::lock_guard<std::mutex> lock(statisticsEventLock_);
        auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        ResetTimerStartTime(time);
        clientAttachInfo = clientAttachInfo_;
        clientShowInfo = clientShowInfo_;
        clientAttachInfo_ = ClientAttachAllInfo(COUNT_STATISTICS_INTERVAL_NUM, COUNT_STATISTICS_INTERVAL_NUM);
        clientShowInfo_ = ClientShowAllInfo(COUNT_STATISTICS_INTERVAL_NUM, COUNT_STATISTICS_INTERVAL_NUM);
    }
    std::string attachStatistics;
    if (!clientAttachInfo.appNames.empty()) {
        clientAttachInfo.succeedRateInfo.Marshall(attachStatistics);
        ImfHiSysEventUtil::ReportStatisticsEvent(GET_NAME(CLIENT_ATTACH_STATISTICS), clientAttachInfo.imeNames,
            clientAttachInfo.appNames, { attachStatistics });
    }
    if (!clientShowInfo.appNames.empty()) {
        std::string showStatistics;
        clientShowInfo.succeedRateInfo.Marshall(showStatistics);
        ImfHiSysEventUtil::ReportStatisticsEvent(
            GET_NAME(CLIENT_SHOW_STATISTICS), clientShowInfo.imeNames, clientShowInfo.appNames, { showStatistics });
    }
}

void ImsaHiSysEventReporter::RecordClientAttachStatistics(const HiSysOriginalInfo &info)
{
    std::string appName = "*";
    auto appIndex = ImfHiSysEventUtil::AddIfAbsent(appName, clientAttachInfo_.appNames);
    auto imeIndex = ImfHiSysEventUtil::AddIfAbsent(info.imeName, clientAttachInfo_.imeNames);
    auto intervalIndex = GetStatisticalIntervalIndex();
    std::string key(appIndex);
    if (info.errCode == ErrorCode::NO_ERROR) {
        clientAttachInfo_.succeedRateInfo.succeedInfo.ModCountDistributions(intervalIndex, key);
        return;
    }
    key.append("/")
        .append(imeIndex)
        .append("/")
        .append(std::to_string(info.clientType))
        .append("/")
        .append(std::to_string(info.errCode));
    clientAttachInfo_.succeedRateInfo.failedInfo.ModCountDistributions(intervalIndex, key);
}

void ImsaHiSysEventReporter::RecordClientShowStatistics(const HiSysOriginalInfo &info)
{
    std::string appName = "*";
    auto appIndex = ImfHiSysEventUtil::AddIfAbsent(appName, clientShowInfo_.appNames);
    auto imeIndex = ImfHiSysEventUtil::AddIfAbsent(info.imeName, clientShowInfo_.imeNames);
    auto intervalIndex = GetStatisticalIntervalIndex();
    std::string key(appIndex);
    if (info.errCode == ErrorCode::NO_ERROR) {
        key.append("/").append(std::to_string(info.errCode));
        clientShowInfo_.succeedRateInfo.succeedInfo.ModCountDistributions(intervalIndex, key);
        return;
    }
    key.append("/")
        .append(imeIndex)
        .append("/")
        .append(std::to_string(info.clientType))
        .append("/")
        .append(std::to_string(info.eventCode))
        .append("/")
        .append(std::to_string(info.errCode));
    clientShowInfo_.succeedRateInfo.failedInfo.ModCountDistributions(intervalIndex, key);
}
} // namespace MiscServices
} // namespace OHOS