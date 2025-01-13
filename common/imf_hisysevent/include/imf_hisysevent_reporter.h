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

#ifndef IMF_HISYSEVENT_H
#define IMF_HISYSEVENT_H

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "global.h"
#include "input_client_info.h"
#include "serializable.h"
#include "timer.h"
namespace OHOS {
namespace MiscServices {

enum ImfEventType : uint8_t {
    CLIENT_ATTACH,
    CLIENT_SHOW,
    IME_START_INPUT,
    BASE_TEXT_OPERATOR,
};

enum ImfFaultEvent : uint8_t {
    HI_SYS_FAULT_EVENT_BEGIN,
    CLIENT_ATTACH_FAILED = HI_SYS_FAULT_EVENT_BEGIN,
    CLIENT_SHOW_FAILED,
    IME_START_INPUT_FAILED,
    BASE_TEXT_OPERATOR_FAILED,
    HI_SYS_FAULT_EVENT_END,
};

enum ImfStatisticsEvent : uint8_t {
    HI_SYS_STATISTICS_EVENT_BEGIN,
    CLIENT_ATTACH_STATISTICS = HI_SYS_STATISTICS_EVENT_BEGIN,
    CLIENT_SHOW_STATISTICS,
    IME_START_INPUT_STATISTICS,
    BASE_TEXT_OPERATOR_STATISTICS,
    HI_SYS_STATISTICS_EVENT_END,
};

struct HiSysOriginalInfo {
    uint8_t eventCode;
    int32_t errCode;
    std::string peerName;
    int64_t peerPid;
    int32_t peerUserId;
    ClientType clientType;
    int32_t inputPattern;
    bool isShowKeyboard;
    std::string imeName;
    int32_t imeCbTime;             // ms
    int32_t baseTextOperationTime; // ms
    class Builder {
    public:
        Builder();
        Builder &SetEventCode(uint8_t eventCode);
        Builder &SetErrCode(int32_t errCode);
        Builder &SetPeerName(const std::string peerName);
        Builder &SetPeerUserId(int32_t peerUserId);
        Builder &SetPeerPid(int64_t peerPid);
        Builder &SetClientType(ClientType clientType);
        Builder &SetInputPattern(int32_t inputPattern);
        Builder &SetIsShowKeyboard(bool isShowKeyboard);
        Builder &SetImeName(const std::string &imeName);
        Builder &SetImeCbTime(int32_t imeCbTime);
        Builder &SetBaseTextOperatorTime(int32_t baseTextOperatorTime);
        std::shared_ptr<HiSysOriginalInfo> Build();

    private:
        std::shared_ptr<HiSysOriginalInfo> info_ = nullptr;
    };
};

struct CountDistributionInfo : public Serializable {
    int32_t count;
    std::vector<std::vector<std::pair<std::string, int32_t>>> countDistributions;
    bool Marshal(cJSON *node) const override
    {
        auto ret = SetValue(node, GET_NAME(COUNT), count);
        std::vector<std::vector<std::string>> distributions;
        for (const auto &distribution : countDistributions) {
            std::vector<std::string> infos;
            for (const auto &info : distribution) {
                auto str = info.first + "/" + std::to_string(info.second);
                infos.push_back(str);
            }
            distributions.push_back(infos);
        }
        return SetValue(node, GET_NAME(COUNT_DISTRIBUTION), distributions) && ret;
    }
};

struct SuccessRateStatistics : public Serializable {
    CountDistributionInfo succeedInfo;
    CountDistributionInfo failedInfo;
    bool Marshal(cJSON *node) const override
    {
        auto ret = SetValue(node, GET_NAME(SUCCEED), succeedInfo);
        return SetValue(node, GET_NAME(FAILED), failedInfo) && ret;
    }
};

class ImfHiSysEventUtil {
public:
    static std::string GetAppName(uint32_t tokenId);
    static void ReportClientAttachFault(const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info);
    static void ReportClientShowFault(const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info);
    static void ReportImeStartInputFault(const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info);
    static void ReportBaseTextOperationFault(
        const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info);
    static void ReportStatisticsEvent(const std::string &eventName, const std::unordered_set<std::string> &imeNames,
        const std::unordered_set<std::string> &appNames, const std::string &statistics);
};

class ImfHiSysEventReporter {
public:
    ImfHiSysEventReporter() = default;
    virtual ~ImfHiSysEventReporter();
    static constexpr uint32_t HISYSEVENT_TIMER_TASK_INTERNAL = 10800000; // updated three hourly
    static constexpr uint32_t HISYSEVENT_STATISTICS_INTERNAL = 1800000;
    static constexpr uint8_t COUNT_STATISTICS_INTERVAL_NUM =
        HISYSEVENT_TIMER_TASK_INTERNAL / HISYSEVENT_STATISTICS_INTERNAL; // 6
    void ReportEvent(ImfEventType eventType, const HiSysOriginalInfo &info);
    std::string GetIndexInSet(const std::string &bundleName, const std::unordered_set<std::string> &bundleNames);
    void ModCountDistributionInfo(uint8_t intervalIndex, const std::string &key, CountDistributionInfo &info);
    std::string GetSelfName();

protected:
    virtual bool IsValidErrCode(int32_t errCode){};
    virtual bool IsFault(int32_t errCode){};
    virtual void RecordStatisticsEvent(ImfStatisticsEvent event, const HiSysOriginalInfo &info){};
    virtual void ReportStatisticsEvent(){};

    int64_t timerStartTime_ = 0;

private:
    void StartTimer();
    void StopTimer();
    void TimerCallback();
    uint8_t GetStatisticalIntervalIndex();
    void ReportFaultEvent(ImfFaultEvent event, const HiSysOriginalInfo &info);
    std::string GenerateFaultEventKey(ImfFaultEvent event, const HiSysOriginalInfo &info);
    std::pair<bool, int64_t> GenerateFaultReportInfo(ImfFaultEvent event, const HiSysOriginalInfo &info);
    void ClearFaultEventInfo();
    static constexpr uint32_t FAULT_REPORT_INTERVAL = 300000;
    const std::unordered_map<ImfEventType, std::pair<ImfFaultEvent, ImfStatisticsEvent>> EVENT_MAPPING = {
        { CLIENT_ATTACH, { CLIENT_ATTACH_FAILED, CLIENT_ATTACH_STATISTICS } },
        { CLIENT_SHOW, { CLIENT_SHOW_FAILED, CLIENT_SHOW_STATISTICS } },
        { IME_START_INPUT, { IME_START_INPUT_FAILED, IME_START_INPUT_STATISTICS } },
        { BASE_TEXT_OPERATOR, { BASE_TEXT_OPERATOR_FAILED, BASE_TEXT_OPERATOR_STATISTICS } },
    };
    using FaultEventHandler = void (*)(const std::string selfName, int64_t faultNum, const HiSysOriginalInfo &info);
    static constexpr FaultEventHandler FAULT_EVENT_HANDLERS[HI_SYS_FAULT_EVENT_END] = {
        [CLIENT_ATTACH_FAILED] = ImfHiSysEventUtil::ReportClientAttachFault,
        [CLIENT_SHOW_FAILED] = ImfHiSysEventUtil::ReportClientShowFault,
        [IME_START_INPUT_FAILED] = ImfHiSysEventUtil::ReportImeStartInputFault,
        [BASE_TEXT_OPERATOR_FAILED] = ImfHiSysEventUtil::ReportBaseTextOperationFault,
    };
    std::mutex selfNameLock_;
    std::string selfName_;
    std::mutex faultEventRecordsLock_;
    // first:event+eventCode+errCode+peerName+peerUserId  second: first:last report time  second:report num
    std::unordered_map<std::string, std::pair<int64_t, int64_t>> faultEventRecords_;
    std::mutex timerLock_;
    Utils::Timer timer_{ "imfHiSysEventTimer" };
    uint32_t timerId_{ 0 };
    std::mutex statisticsEventLock_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_HISYSEVENT_H