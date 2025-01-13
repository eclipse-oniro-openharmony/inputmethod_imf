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
#include "imf_hisysevent_reporter.h"

#include <algorithm>
#include <chrono>
namespace OHOS {
namespace MiscServices {
using namespace std::chrono;
ImfHiSysEventReporter::~ImfHiSysEventReporter()
{
    StopTimer();
}

void ImfHiSysEventReporter::ReportEvent(ImfEventType eventType, const HiSysOriginalInfo &info)
{
    auto it = EVENT_MAPPING.find(eventType);
    if (it == EVENT_MAPPING.end()) {
        return;
    }
    StartTimer();
    ReportFaultEvent(it->second.first, info);
    {
        std::lock_guard<std::mutex> lock(statisticsEventLock_);
        RecordStatisticsEvent(it->second.second, info);
    }
}

std::string ImfHiSysEventReporter::GetIndexInSet(
    const std::string &bundleName, const std::unordered_set<std::string> &bundleNames)
{
    int32_t index = 0;
    for (const auto &name : bundleNames) {
        if (name == bundleName) {
            break;
        }
        index++;
    }
    return std::to_string(index);
}

void ImfHiSysEventReporter::ModCountDistributionInfo(
    uint8_t intervalIndex, const std::string &key, CountDistributionInfo &info)
{
    info.count++;
    auto &intervalInfos = info.countDistributions[intervalIndex];
    auto it = std::find_if(intervalInfos.begin(), intervalInfos.end(),
        [key](const std::pair<std::string, uint64_t> &infoTmp) { return infoTmp.first == key; });
    if (it == intervalInfos.end()) {
        intervalInfos.emplace_back(key, 1);
        return;
    }
    it->second++;
}

void ImfHiSysEventReporter::ReportFaultEvent(ImfFaultEvent event, const HiSysOriginalInfo &info)
{
    if (event < ImfFaultEvent::HI_SYS_FAULT_EVENT_BEGIN || event >= ImfFaultEvent::HI_SYS_FAULT_EVENT_END) {
        return;
    }
    auto faultReportInfo = GenerateFaultReportInfo(event, info);
    if (!faultReportInfo.first) {
        return;
    }
    FAULT_EVENT_HANDLERS[event](GetSelfName(), faultReportInfo.second, info);
}

void ImfHiSysEventReporter::StartTimer()
{
    std::lock_guard<std::mutex> lock(timerLock_);
    if (timerId_ != 0) {
        return;
    }
    uint32_t ret = timer_.Setup();
    if (ret != Utils::TIMER_ERR_OK) {
        IMSA_HILOGE("failed to create timer!");
        return;
    }
    auto callback = [this]() { TimerCallback(); };
    timerId_ = timer_.Register(callback(), CountDistributionInfo::HISYSEVENT_TIMER_TASK_INTERNAL, false);
    timerStartTime_ = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void ImfHiSysEventReporter::TimerCallback()
{
    auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    ClearFaultEventInfo();
    {
        std::lock_guard<std::mutex> lock(statisticsEventLock_);
        ReportStatisticsEvent();
        timerStartTime_ = time;
    }
}

void ImfHiSysEventReporter::StopTimer()
{
    timer_.Unregister(timerId_);
    timer_.Shutdown();
}

uint8_t ImfHiSysEventReporter::GetStatisticalIntervalIndex()
{
    auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    auto index = (time - timerStartTime_) / HISYSEVENT_STATISTICS_INTERNAL;
    if (index >= COUNT_STATISTICS_INTERVAL_NUM) {
        return COUNT_STATISTICS_INTERVAL_NUM - 1;
    }
    if (index < 0) {
        return 0;
    }
    return index;
}

std::pair<bool, int64_t> ImfHiSysEventReporter::GenerateFaultReportInfo(
    ImfFaultEvent event, const HiSysOriginalInfo &info)
{
    std::pair<bool, int64_t> faultReportInfo{ false, 0 };
    if (info.errCode == ErrorCode::NO_ERROR || !IsValidErrCode(info.errCode) || !IsFault(info.errCode)) {
        return faultReportInfo;
    }
    auto key = GenerateFaultEventKey(event, info);
    auto curTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    std::lock_guard<std::mutex> lock(faultEventRecordsLock_);
    auto it = faultEventRecords_.find(key);
    if (it != faultEventRecords_.end()) {
        if (curTime - it->second.first < FAULT_REPORT_INTERVAL) {
            it->second.second++;
            return faultReportInfo;
        }
    }
    faultReportInfo.second = it != faultEventRecords_.end() ? it->second.second : 1;
    faultReportInfo.first = true;
    faultEventRecords_.insert_or_assign(key, { curTime, 1 });
    return faultReportInfo;
}

std::string ImfHiSysEventReporter::GenerateFaultEventKey(ImfFaultEvent event, const HiSysOriginalInfo &info)
{
    std::string key = std::to_string(event) + "/" + std::to_string(info.eventCode) + "/" + std::to_string(info.errCode)
                      + "/" + info.peerName + "/" + std::to_string(info.peerUserId);
}

void ImfHiSysEventReporter::ClearFaultEventInfo()
{
    auto curTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    std::lock_guard<std::mutex> lock(faultEventRecordsLock_);
    for (const auto &record : faultEventRecords_) {
        if (curTime - record.second.first > FAULT_REPORT_INTERVAL) {
            faultEventRecords_.erase(record.first);
        }
    }
}

std::string ImfHiSysEventReporter::GetSelfName()
{
    std::lock_guard<std::mutex> lock(selfNameLock_);
    if (!selfName_.empty()) {
        return selfName_;
    }
    auto selfToken = GetSelfTokenID();
    selfName_ = ImfHiSysEventUtil::GetAppName(selfToken);
    return selfName_;
}

HiSysOriginalInfo::Builder::Builder()
{
    info_ = std::make_shared<HiSysOriginalInfo>();
    info_->eventCode = 0;
    info_->errCode = 0;
    info_->peerPid = 0;
    info_->peerUserId = 0;
    info_->clientType = ClientType::CLIENT_TYPE_END;
    info_->inputPattern = 0;
    info_->isShowKeyboard = true;
    info_->imeCbTime = -1;
    info_->baseTextOperatorTime = -1;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetEventCode(uint8_t eventCode)
{
    info_->eventCode = eventCode;
    return *this;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetErrCode(int32_t errCode)
{
    info_->errCode = errCode;
    return *this;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetPeerName(std::string peerName)
{
    info_->peerName = peerName;
    return *this;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetPeerPid(int64_t peerPid)
{
    info_->peerPid = peerPid;
    return *this;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetPeerUserId(int32_t peerUserId)
{
    info_->peerUserId = peerUserId;
    return *this;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetClientType(ClientType clientType)
{
    info_->clientType = clientType;
    return *this;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetInputPattern(int32_t inputPattern)
{
    info_->inputPattern = inputPattern;
    return *this;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetIsShowKeyboard(bool isShowKeyboard)
{
    info_->isShowKeyboard = isShowKeyboard;
    return *this;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetImeName(const std::string &imeName)
{
    info_->imeName = imeName;
    return *this;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetImeCbTime(int32_t imeCbTime)
{
    info_->imeCbTime = imeCbTime;
    return *this;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetBaseTextOperatorTime(int32_t baseTextOperatorTime)
{
    info_->baseTextOperatorTime = baseTextOperatorTime;
    return *this;
}
std::shared_ptr<HiSysOriginalInfo> HiSysOriginalInfo::Builder::Build()
{
    return info_;
}

void ImfHiSysEventUtil::ReportClientAttachFault(
    const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info)
{
    HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "BASE_TEXT_OPERATOR_FAILED",
        HiSysEventNameSpace::EventType::FAULT, "SELF_NAME", selfName, "PEER_NAME", info.peerName, "PEER_PID",
        info.peerPid, "PEER_USERID", info.peerUserId, "CLIENT_TYPE", info.clientType, "INPUT_PATTERN",
        info.inputPattern, "ISSHOWKEYBOARD", info.isShowKeyboard, "IME_NAME", info.imeName, "ERR_CODE", info.eventCode,
        "FAULT_NUM", faultNum);
}

void ImfHiSysEventUtil::ReportClientShowFault(
    const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info)
{
    HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "BASE_TEXT_OPERATOR_FAILED",
        HiSysEventNameSpace::EventType::FAULT, "SELF_NAME", selfName, "PEER_NAME", info.peerName, "PEER_PID",
        info.peerPid, "PEER_USERID", info.peerUserId, "CLIENT_TYPE", info.clientType, "INPUT_PATTERN",
        info.inputPattern, "IME_NAME", info.imeName, "EVENT_CODE", info.eventCode, "ERR_CODE", info.eventCode,
        "FAULT_NUM", faultNum);
}

void ImfHiSysEventUtil::ReportImeStartInputFault(
    const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info)
{
    HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "BASE_TEXT_OPERATOR_FAILED",
        HiSysEventNameSpace::EventType::FAULT, "SELF_NAME", selfName, "PEER_NAME", info.peerName, "PEER_PID",
        info.peerPid, "ISSHOWKEYBOARD", info.isShowKeyboard, "EVENT_CODE", info.eventCode, "ERR_CODE", info.eventCode,
        "FAULT_NUM", faultNum);
}

void ImfHiSysEventUtil::ReportBaseTextOperationFault(
    const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info)
{
    HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "BASE_TEXT_OPERATOR_FAILED",
        HiSysEventNameSpace::EventType::FAULT, "SELF_NAME", selfName, "PEER_NAME", info.peerName, "PEER_PID",
        info.peerPid, "CLIENT_TYPE", info.clientType, "EVENT_CODE", info.eventCode, "ERR_CODE", info.eventCode,
        "FAULT_NUM", faultNum);
}

void ImfHiSysEventUtil::ReportStatisticsEvent(const std::string &eventName,
    const std::unordered_set<std::string> &imeNames, const std::unordered_set<std::string> &appNames,
    const std::string &statistics)
{
}

std::string ImfHiSysEventUtil::GetAppName(uint32_t tokenId)
{
    std::string name;
    auto tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    switch (tokenType) {
        case ATokenTypeEnum::TOKEN_HAP: {
            HapTokenInfo hapInfo;
            if (AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo) != 0) {
                return name;
            }
            name = hapInfo.bundleName;
            break;
        }
        case ATokenTypeEnum::TOKEN_NATIVE:
        case ATokenTypeEnum::TOKEN_SHELL: {
            NativeTokenInfo tokenInfo;
            if (AccessTokenKit::GetNativeTokenInfo(tokenId, tokenInfo) != 0) {
                return name;
            }
            name = tokenInfo.processName;
            break;
        }
        default: {
            break;
        }
    }
    return name;
}
} // namespace MiscServices
} // namespace OHOS