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
    timerId_ = timer_.Register(callback(), HISYSEVENT_TIMER_TASK_INTERNAL, false);
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
    std::string key(std::to_string(event));
    key.append("/")
        .append(std::to_string(info.peerUserId))
        .append("/")
        .append(info.peerName)
        .append("/")
        .append(std::to_string(info.eventCode))
        .append("/")
        .append(std::to_string(info.errCode));
    return key;
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
} // namespace MiscServices
} // namespace OHOS