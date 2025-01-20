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
std::mutex ImsaHiSysEventReporter::instanceLock_;
sptr<ImsaHiSysEventReporter> ImsaHiSysEventReporter::instance_;
sptr<ImsaHiSysEventReporter> ImsaHiSysEventReporter::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(instanceLock_);
        if (instance_ == nullptr) {
            instance_ = new (std::nothrow) ImsaHiSysEventReporter();
        }
    }
    return instance_;
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
    std::string attachStatistics;
    clientAttachInfo_.succeedRateInfo.Marshall(attachStatistics);
    ImfHiSysEventUtil::ReportStatisticsEvent(GET_NAME(CLIENT_ATTACH_STATISTICS), clientAttachInfo_.imeNames,
        clientAttachInfo_.appNames, { attachStatistics });
    clientAttachInfo_ = ClientAttachAllInfo(COUNT_STATISTICS_INTERVAL_NUM, COUNT_STATISTICS_INTERVAL_NUM);
    std::string showStatistics;
    clientShowInfo_.succeedRateInfo.Marshall(showStatistics);
    ImfHiSysEventUtil::ReportStatisticsEvent(
        GET_NAME(CLIENT_SHOW_STATISTICS), clientShowInfo_.imeNames, clientShowInfo_.appNames, { showStatistics });
    clientShowInfo_ = ClientShowAllInfo(COUNT_STATISTICS_INTERVAL_NUM, COUNT_STATISTICS_INTERVAL_NUM);
}

void ImsaHiSysEventReporter::RecordClientAttachStatistics(const HiSysOriginalInfo &info)
{
    std::lock_guard<std::mutex> lock(clientAttachInfoLock_);
    clientAttachInfo_.appNames.insert(info.peerName);
    clientAttachInfo_.imeNames.insert(info.imeName);
    auto intervalIndex = GetStatisticalIntervalIndex();
    auto appIndex = ImfHiSysEventUtil::GetIndexInSet(info.peerName, clientAttachInfo_.appNames);
    std::string key(appIndex);
    if (info.errCode == ErrorCode::NO_ERROR) {
        clientAttachInfo_.succeedRateInfo.succeedInfo.Mod(intervalIndex, key);
        return;
    }
    auto imeIndex = ImfHiSysEventUtil::GetIndexInSet(info.imeName, clientAttachInfo_.imeNames);
    key.append("/")
        .append(imeIndex)
        .append("/")
        .append(std::to_string(info.clientType))
        .append("/")
        .append(std::to_string(info.errCode));
    clientAttachInfo_.succeedRateInfo.failedInfo.Mod(intervalIndex, key);
}

void ImsaHiSysEventReporter::RecordClientShowStatistics(const HiSysOriginalInfo &info)
{
    std::lock_guard<std::mutex> lock(clientShowInfoLock_);
    clientShowInfo_.appNames.insert(info.peerName);
    clientShowInfo_.imeNames.insert(info.imeName);
    auto intervalIndex = GetStatisticalIntervalIndex();
    auto appIndex = ImfHiSysEventUtil::GetIndexInSet(info.peerName, clientShowInfo_.appNames);
    std::string key(appIndex);
    if (info.errCode == ErrorCode::NO_ERROR) {
        key.append("/").append(std::to_string(info.errCode));
        clientAttachInfo_.succeedRateInfo.succeedInfo.Mod(intervalIndex, key);
        return;
    }
    auto imeIndex = ImfHiSysEventUtil::GetIndexInSet(info.imeName, clientShowInfo_.imeNames);
    key.append("/")
        .append(imeIndex)
        .append("/")
        .append(std::to_string(info.clientType))
        .append("/")
        .append(std::to_string(info.eventCode))
        .append("/")
        .append(std::to_string(info.errCode));
    clientAttachInfo_.succeedRateInfo.failedInfo.Mod(intervalIndex, key);
}
} // namespace MiscServices
} // namespace OHOS