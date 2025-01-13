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

#include "user_session_manager.h"
namespace OHOS {
namespace MiscServices {
ImsaHiSysEventReporter::ImsaHiSysEventReporter()
{
    clientAttachInfo_.statistics.succeedInfo.countDistributions.resize(COUNT_STATISTICS_INTERVAL_NUM);
    clientAttachInfo_.statistics.succeedInfo.countDistributions.resize(COUNT_STATISTICS_INTERVAL_NUM);
    clientShowInfo_.statistics.succeedInfo.countDistributions.resize(COUNT_STATISTICS_INTERVAL_NUM);
    clientShowInfo_.statistics.succeedInfo.countDistributions.resize(COUNT_STATISTICS_INTERVAL_NUM);
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
            RecordClientAttachEvent(info);
            break;
        }
        case ImfStatisticsEvent::CLIENT_SHOW_STATISTICS: {
            RecordClientShowEvent(info);
            break;
        }
        default:
            break;
    }
}

void ImsaHiSysEventReporter::ReportStatisticsEvent()
{
    std::string attachStatistics;
    clientAttachInfo_.statistics.Marshal(attachStatistics);
    ImfHiSysEventUtil::ReportStatisticsEvent(
        GET_NAME(CLIENT_ATTACH_STATISTICS), clientAttachInfo_.imeNames, clientAttachInfo_.appNames, attachStatistics);
    // todo clientAttachInfo_清空
    std::string showStatistics;
    clientShowInfo_.statistics.Marshal(showStatistics);
    ImfHiSysEventUtil::ReportStatisticsEvent(
        GET_NAME(CLIENT_SHOW_STATISTICS), clientShowInfo_.imeNames, clientShowInfo_.appNames, showStatistics);
    // todo clientShowInfo_
}

void ImsaHiSysEventReporter::RecordClientAttachEvent(const HiSysOriginalInfo &info)
{
    std::lock_guard<std::mutex> lock(clientAttachInfoLock_);
    clientAttachInfo_.appNames.insert(info.peerName);
    clientAttachInfo_.imeNames.insert(info.imeName);
    auto intervalIndex = GetStatisticalIntervalIndex();
    auto appIndex = GetIndexInSet(info.peerName, clientAttachInfo_.appNames);
    if (info.errCode == ErrorCode::NO_ERROR) {
        auto key = appIndex;
        ModCountDistributionInfo(intervalIndex, key, clientAttachInfo_.statistics.succeedInfo);
        return;
    }

    auto imeIndex = GetIndexInSet(info.imeName, clientAttachInfo_.imeNames);
    auto key = appIndex + "/" + imeIndex + "/" + std::to_string(info.clientType) + "/" + std::to_string(info.errCode);
    ModCountDistributionInfo(intervalIndex, key, clientAttachInfo_.statistics.failedInfo);
}

void ImsaHiSysEventReporter::RecordClientShowEvent(const HiSysOriginalInfo &info)
{
    std::lock_guard<std::mutex> lock(clientShowInfoLock_);
    clientShowInfo_.appNames.insert(info.peerName);
    clientShowInfo_.imeNames.insert(info.imeName);
    auto intervalIndex = GetStatisticalIntervalIndex();
    auto appIndex = GetIndexInSet(info.peerName, clientShowInfo_.appNames);
    if (info.errCode == ErrorCode::NO_ERROR) {
        auto key = appIndex + "/" + std::to_string(info.eventCode);
        ModCountDistributionInfo(intervalIndex, key, clientAttachInfo_.statistics.succeedInfo);
        return;
    }
    auto imeIndex = GetIndexInSet(info.imeName, clientShowInfo_.imeNames);
    auto key = appIndex + "/" + imeIndex + "/" + std::to_string(info.clientType) + "/" + std::to_string(info.eventCode)
               + "/" + std::to_string(info.errCode);
    ModCountDistributionInfo(intervalIndex, key, clientAttachInfo_.statistics.failedInfo);
}

std::pair<int64_t, std::string> ImsaHiSysEventReporter::GetCurrentImeInfo(int32_t userId)
{
    std::pair<int64_t, std::string> imeInfo{ 0, "" };
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
        imeInfo.second = currentImeCfg != nullptr ? currentImeCfg->bundleName : "";
        return imeInfo;
    }
    auto imeType = session->IsProxyImeEnable() ? ImeType::PROXY_IME : ImeType::IME;
    auto imeData = session->GetImeData(imeType);
    if (imeData != nullptr) {
        imeInfo.first = imeData->pid;
        imeInfo.second = imeData->ime.first;
    }
    return imeInfo;
}
} // namespace MiscServices
} // namespace OHOS