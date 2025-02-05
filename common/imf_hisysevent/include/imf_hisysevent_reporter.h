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

#ifndef IMF_HISYSEVENT_REPORTER_H
#define IMF_HISYSEVENT_REPORTER_H

#include <cstdint>
#include <unordered_map>

#include "global.h"
#include "imf_hisysevent_info.h"
#include "imf_hisysevent_util.h"
#include "timer.h"
namespace OHOS {
namespace MiscServices {
class ImfHiSysEventReporter {
public:
    static constexpr uint32_t HISYSEVENT_TIMER_TASK_INTERNAL = 3 * 60 * 60 * 1000; // updated three hourly
    static constexpr uint32_t HISYSEVENT_STATISTICS_INTERNAL = 30 * 60 * 1000;
    static constexpr uint32_t COUNT_STATISTICS_INTERVAL_NUM =
        HISYSEVENT_TIMER_TASK_INTERNAL / HISYSEVENT_STATISTICS_INTERNAL; // 6
    void ReportEvent(ImfEventType eventType, const HiSysOriginalInfo &info);
    std::string GetSelfName();
    uint32_t GetStatisticalIntervalIndex();
    void ResetTimerStartTime(int64_t time);

protected:
    ImfHiSysEventReporter() = default;
    ~ImfHiSysEventReporter();

private:
    virtual bool IsValidErrCode(int32_t errCode)
    {
        return false;
    }
    virtual bool IsFault(int32_t errCode)
    {
        return false;
    }
    virtual void RecordStatisticsEvent(ImfStatisticsEvent event, const HiSysOriginalInfo &info){};
    virtual void ReportStatisticsEvent(){};
    void StartTimer();
    void StopTimer();
    void TimerCallback();
    void ReportFaultEvent(ImfFaultEvent event, const HiSysOriginalInfo &info);
    std::string GenerateFaultEventKey(ImfFaultEvent event, const HiSysOriginalInfo &info);
    std::pair<bool, int64_t> GenerateFaultReportInfo(ImfFaultEvent event, const HiSysOriginalInfo &info);
    void ClearFaultEventInfo();
    static constexpr uint32_t FAULT_REPORT_INTERVAL = 5 * 60 * 1000;
    static constexpr uint32_t FAULT_RETENTION_PERIOD = 10 * 60 * 1000;
    const std::unordered_map<ImfEventType, std::pair<ImfFaultEvent, ImfStatisticsEvent>> EVENT_MAPPING = {
        { CLIENT_ATTACH, { CLIENT_ATTACH_FAILED, CLIENT_ATTACH_STATISTICS } },
        { CLIENT_SHOW, { CLIENT_SHOW_FAILED, CLIENT_SHOW_STATISTICS } },
        { IME_START_INPUT, { IME_START_INPUT_FAILED, IME_START_INPUT_STATISTICS } },
        { BASE_TEXT_OPERATOR, { BASE_TEXT_OPERATION_FAILED, BASE_TEXT_OPERATION_STATISTICS } },
    };
    using FaultEventHandler = void (*)(const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info);
    static constexpr FaultEventHandler FAULT_EVENT_HANDLERS[HI_SYS_FAULT_EVENT_END] = {
        [CLIENT_ATTACH_FAILED] = ImfHiSysEventUtil::ReportClientAttachFault,
        [CLIENT_SHOW_FAILED] = ImfHiSysEventUtil::ReportClientShowFault,
        [IME_START_INPUT_FAILED] = ImfHiSysEventUtil::ReportImeStartInputFault,
        [BASE_TEXT_OPERATION_FAILED] = ImfHiSysEventUtil::ReportBaseTextOperationFault,
    };
    int64_t timerStartTime_ = 0;
    std::mutex selfNameLock_;
    std::string selfName_;
    std::mutex faultEventRecordsLock_;
    // first:event+eventCode+errCode+peerName+peerUserId  second: first:last report time  second:report num
    std::unordered_map<std::string, std::pair<int64_t, uint32_t>> faultEventRecords_;
    std::mutex timerLock_;
    Utils::Timer timer_{ "imfHiSysEventTimer" };
    uint32_t timerId_{ 0 };
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_HISYSEVENT_REPORTER_H