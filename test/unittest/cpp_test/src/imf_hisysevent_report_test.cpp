/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#define private public
#define protected public
#include "ima_hisysevent_reporter.h"
#include "imc_hisysevent_reporter.h"
#include "imf_hisysevent_info.h"
#include "imf_hisysevent_reporter.h"
#include "imf_hisysevent_util.h"
#include "imsa_hisysevent_reporter.h"
#undef private

#include <gtest/gtest.h>
#include <gtest/hwext/gtest-multithread.h>
#include <sys/time.h>
#include <unistd.h>

#include <condition_variable>
#include <cstdint>
#include <sstream>
#include <string>

#include "global.h"
#include "hisysevent_base_manager.h"
#include "hisysevent_listener.h"
#include "hisysevent_manager.h"
#include "hisysevent_query_callback.h"
#include "hisysevent_record.h"

namespace OHOS {
namespace MiscServices {
using namespace testing::ext;
using namespace testing::mt;
using namespace OHOS::HiviewDFX;
constexpr const char *EVENT_NAME_CLIENT_ATTACH_FAILED = GET_NAME(CLIENT_ATTACH_FAILED);
constexpr const char *EVENT_NAME_CLIENT_SHOW_FAILED = GET_NAME(CLIENT_SHOW_FAILED);
constexpr const char *EVENT_NAME_IME_START_INPUT_FAILED = GET_NAME(IME_START_INPUT_FAILED);
constexpr const char *EVENT_NAME_BASIC_TEXT_OPERATION_FAILED = GET_NAME(BASE_TEXT_OPERATION_FAILED);
constexpr const char *EVENT_NAME_CLIENT_ATTACH_STATISTICS = GET_NAME(CLIENT_ATTACH_STATISTICS);
constexpr const char *EVENT_NAME_CLIENT_SHOW_STATISTICS = GET_NAME(CLIENT_SHOW_STATISTICS);
constexpr const char *EVENT_NAME_IME_START_INPUT_STATISTICS = GET_NAME(IME_START_INPUT_STATISTICS);
constexpr const char *EVENT_NAME_BASIC_TEXT_OPERATION_STATISTICS = GET_NAME(BASE_TEXT_OPERATION_STATISTICS);
constexpr const char *DOMAIN = "INPUTMETHOD";
constexpr const char *PARAM_PEER_NAME = "app";
constexpr const char *PARAM_PEER_NAME1 = "app1";
constexpr int32_t PARAM_PEER_USERID = 100;
constexpr int64_t PARAM_PEER_PID = 4545;
constexpr ClientType PARAM_CLIENT_TYPE = ClientType::INNER_KIT;
constexpr int32_t PARAM_INPUT_PATTERN = 6;
constexpr bool PARAM_ISSHOWKEYBOARD = true;
constexpr const char *PARAM_IME_NAME = "ime";
constexpr const char *PARAM_IME_NAME1 = "ime1";
constexpr int32_t PARAM_EVENT_CODE = 4;
constexpr uint32_t WATI_HISYSEVENT_TIMEOUT = 300; //ms
struct BaseStatistics : public Serializable {
    int32_t count{ 0 };
    std::vector<std::vector<std::string>> info;
    bool operator==(const BaseStatistics &baseInfo) const
    {
        return count == baseInfo.count && info == baseInfo.info;
    }
    bool Unmarshal(cJSON *node) override
    {
        auto ret = GetValue(node, GET_NAME(COUNT), count);
        return GetValue(node, GET_NAME(COUNT_DISTRIBUTION), info) && ret;
    }
};

struct ClientAttachStatistics : public BaseStatistics {
    std::vector<std::string> appNames;
    std::vector<std::string> imeNames;
    BaseStatistics succeedInfo;
    BaseStatistics failedInfo;
    bool operator==(const ClientAttachStatistics &info) const
    {
        return (imeNames == info.imeNames && appNames == info.appNames && succeedInfo == info.succeedInfo
                && failedInfo == info.failedInfo);
    }
    bool Unmarshal(cJSON *node) override
    {
        auto ret = GetValue(node, GET_NAME(SUCCEED), succeedInfo);
        return GetValue(node, GET_NAME(FAILED), failedInfo) && ret;
    }
};

struct ClientShowStatistics : public BaseStatistics {
    std::vector<std::string> appNames;
    std::vector<std::string> imeNames;
    BaseStatistics succeedInfo;
    BaseStatistics failedInfo;
    bool operator==(const ClientShowStatistics &info) const
    {
        return (imeNames == info.imeNames && appNames == info.appNames && succeedInfo == info.succeedInfo
                && failedInfo == info.failedInfo);
    }
    bool Unmarshal(cJSON *node) override
    {
        auto ret = GetValue(node, GET_NAME(SUCCEED), succeedInfo);
        return GetValue(node, GET_NAME(FAILED), failedInfo) && ret;
    }
};

struct CbTimeConsume : public Serializable {
    int32_t count{ 0 };
    std::string timeConsumeStatistics;
    bool operator==(const CbTimeConsume &info) const
    {
        return (count == info.count && timeConsumeStatistics == info.timeConsumeStatistics);
    }
    bool Unmarshal(cJSON *node) override
    {
        auto ret = GetValue(node, GET_NAME(COUNT), count);
        return GetValue(node, GET_NAME(COUNT_DISTRIBUTION), timeConsumeStatistics) && ret;
    }
};

struct ImeStartInputStatistics : public BaseStatistics {
    std::vector<std::string> appNames;
    BaseStatistics succeedInfo;
    BaseStatistics failedInfo;
    CbTimeConsume cbTimeConsume;
    bool operator==(const ImeStartInputStatistics &info) const
    {
        return (appNames == info.appNames && succeedInfo == info.succeedInfo && failedInfo == info.failedInfo
                && cbTimeConsume == info.cbTimeConsume);
    }
    bool Unmarshal(cJSON *node) override
    {
        auto ret = GetValue(node, GET_NAME(SUCCEED), succeedInfo);
        ret = GetValue(node, GET_NAME(FAILED), failedInfo) && ret;
        return GetValue(node, GET_NAME(CB_TIME_CONSUME), cbTimeConsume) && ret;
    }
};

struct BaseTextOperationStatistics : public BaseStatistics {
    std::vector<std::string> appNames;
    BaseStatistics succeedInfo;
    BaseStatistics failedInfo;
    bool operator==(const BaseTextOperationStatistics &info) const
    {
        return (appNames == info.appNames && succeedInfo == info.succeedInfo && failedInfo == info.failedInfo);
    }
    bool Unmarshal(cJSON *node) override
    {
        auto ret = GetValue(node, GET_NAME(SUCCEED), succeedInfo);
        return GetValue(node, GET_NAME(FAILED), failedInfo) && ret;
    }
};

class ImfHiSysEventReporterTest : public testing::Test {
public:
    static constexpr int32_t THREAD_NUM = 5;
    static constexpr int32_t EACH_THREAD_CIRCULATION_TIME = 100;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static bool WaitFaultEvent(const std::string &eventName, const HiSysOriginalInfo &info);
    static bool WaitClientAttachStatistic(const ClientAttachStatistics &statistic);
    static bool WaitClientShowStatistic(const ClientShowStatistics &statistic);
    static bool WaitImeStartInputStatistic(const ImeStartInputStatistics &statistic);
    static bool WaitBaseTextOperatorStatistic(const BaseTextOperationStatistics &statistic);
    static void TestStatisticsEventConcurrent();
    static void TestClientAttach(const uint8_t *data, size_t size);
    static void TestClientShow(const uint8_t *data, size_t size);
    static void TestStartInput(const uint8_t *data, size_t size);
    static void TestBaseTextOperation(const uint8_t *data, size_t size);
    static std::mutex cvMutex_;
    static std::condition_variable watcherCv_;
    static std::string eventName_;
    static std::shared_ptr<HiSysOriginalInfo> faultInfo_;
    static ClientAttachStatistics clientAttachStatistics_;
    static ClientShowStatistics clientShowStatistics_;
    static ImeStartInputStatistics imeStartInputStatistics_;
    static BaseTextOperationStatistics baseTextOperationStatistics_;
    static int32_t multiThreadExecTotalNum_;

private:
    static void WatchHiSysEvent();
    static void AddListenerRules(const std::string &eventName, std::vector<ListenerRule> &sysRules);
};

class ImfHiSysEventWatcher : public HiSysEventListener {
public:
    virtual ~ImfHiSysEventWatcher()
    {
    }
    virtual void OnEvent(std::shared_ptr<HiSysEventRecord> sysEvent) final
    {
        std::unique_lock<std::mutex> lock(ImfHiSysEventReporterTest::cvMutex_);
        if (sysEvent == nullptr) {
            IMSA_HILOGE("sysEvent is nullptr.");
            return;
        }
        ImfHiSysEventReporterTest::eventName_ = sysEvent->GetEventName();
        IMSA_HILOGI("HiSysEventWatcher::OnEvent:%{public}s", ImfHiSysEventReporterTest::eventName_.c_str());
        auto it = TEST_EVENT_MAPPING.find(ImfHiSysEventReporterTest::eventName_);
        if (it == TEST_EVENT_MAPPING.end()) {
            return;
        }
        if (it->second != nullptr) {
            it->second(sysEvent);
        }
        ImfHiSysEventReporterTest::watcherCv_.notify_one();
    }
    virtual void OnServiceDied() final
    {
        IMSA_HILOGI("HiSysEventWatcher::OnServiceDied");
    }
    static void NotifyFaultEvent(std::shared_ptr<HiSysEventRecord> sysEvent);
    static void NotifyClientAttachStatistic(std::shared_ptr<HiSysEventRecord> sysEvent);
    static void NotifyClientShowStatistic(std::shared_ptr<HiSysEventRecord> sysEvent);
    static void NotifyImeStartInputStatistic(std::shared_ptr<HiSysEventRecord> sysEvent);
    static void NotifyBaseTextOperatorStatistic(std::shared_ptr<HiSysEventRecord> sysEvent);

private:
    using EventHandler = void (*)(std::shared_ptr<HiSysEventRecord> sysEvent);
    const std::unordered_map<std::string, EventHandler> TEST_EVENT_MAPPING = {
        { EVENT_NAME_CLIENT_ATTACH_FAILED, NotifyFaultEvent },
        { EVENT_NAME_CLIENT_SHOW_FAILED, NotifyFaultEvent },
        { EVENT_NAME_IME_START_INPUT_FAILED, NotifyFaultEvent },
        { EVENT_NAME_BASIC_TEXT_OPERATION_FAILED, NotifyFaultEvent },
        { EVENT_NAME_CLIENT_ATTACH_STATISTICS, NotifyClientAttachStatistic },
        { EVENT_NAME_CLIENT_SHOW_STATISTICS, NotifyClientShowStatistic },
        { EVENT_NAME_IME_START_INPUT_STATISTICS, NotifyImeStartInputStatistic },
        { EVENT_NAME_BASIC_TEXT_OPERATION_STATISTICS, NotifyBaseTextOperatorStatistic },
    };
};

void ImfHiSysEventReporterTest::SetUpTestCase(void)
{
    IMSA_HILOGI("ImfHiSysEventReporterTest::SetUpTestCase");
    WatchHiSysEvent();
}

void ImfHiSysEventReporterTest::TearDownTestCase(void)
{
    IMSA_HILOGI("ImfHiSysEventReporterTest::TearDownTestCase");
}

void ImfHiSysEventReporterTest::SetUp(void)
{
    IMSA_HILOGI("ImfHiSysEventReporterTest::SetUp");
    ImfHiSysEventReporterTest::faultInfo_ = HiSysOriginalInfo::Builder().Build();
    eventName_.clear();
    clientAttachStatistics_ = ClientAttachStatistics();
    clientShowStatistics_ = ClientShowStatistics();
    imeStartInputStatistics_ = ImeStartInputStatistics();
    baseTextOperationStatistics_ = BaseTextOperationStatistics();
    ImsaHiSysEventReporter::GetInstance().clientAttachInfo_ = ClientAttachAllInfo(
        ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM, ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
    ImsaHiSysEventReporter::GetInstance().clientShowInfo_ = ClientShowAllInfo(
        ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM, ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
    ImaHiSysEventReporter::GetInstance().imeStartInputAllInfo_ =
        ImeStartInputAllInfo(ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM,
            ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM, ImaHiSysEventReporter::IME_CB_TIME_INTERVAL.size());
    ImaHiSysEventReporter::GetInstance().baseTextOperationAllInfo_ =
        BaseTextOperationAllInfo(ImaHiSysEventReporter::BASE_TEXT_OPERATION_TIME_INTERVAL.size(),
            ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
}

void ImfHiSysEventReporterTest::TearDown(void)
{
    IMSA_HILOGI("ImfHiSysEventReporterTest::TearDown");
}
void ImfHiSysEventReporterTest::WatchHiSysEvent()
{
    std::vector<ListenerRule> sysRules;
    AddListenerRules(EVENT_NAME_CLIENT_ATTACH_FAILED, sysRules);
    AddListenerRules(EVENT_NAME_CLIENT_SHOW_FAILED, sysRules);
    AddListenerRules(EVENT_NAME_IME_START_INPUT_FAILED, sysRules);
    AddListenerRules(EVENT_NAME_BASIC_TEXT_OPERATION_FAILED, sysRules);
    AddListenerRules(EVENT_NAME_CLIENT_ATTACH_STATISTICS, sysRules);
    AddListenerRules(EVENT_NAME_CLIENT_SHOW_STATISTICS, sysRules);
    AddListenerRules(EVENT_NAME_IME_START_INPUT_STATISTICS, sysRules);
    AddListenerRules(EVENT_NAME_BASIC_TEXT_OPERATION_STATISTICS, sysRules);
    AddListenerRules(DOMAIN, sysRules);
    std::shared_ptr<HiSysEventListener> watcher = std::make_shared<ImfHiSysEventWatcher>();
    auto ret = HiSysEventManager::AddListener(watcher, sysRules);
    if (ret != SUCCESS) {
        IMSA_HILOGE("AddListener failed! ret = %{public}d", ret);
    }
}

void ImfHiSysEventReporterTest::AddListenerRules(const std::string &eventName, std::vector<ListenerRule> &sysRules)
{
    ListenerRule listenerRule(DOMAIN, eventName, "", OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    sysRules.emplace_back(listenerRule);
}

void ImfHiSysEventWatcher::NotifyFaultEvent(std::shared_ptr<HiSysEventRecord> sysEvent)
{
    if (sysEvent == nullptr) {
        IMSA_HILOGE("sysEvent is nullptr.");
        return;
    }
    sysEvent->GetParamValue(GET_NAME(PEER_NAME), ImfHiSysEventReporterTest::faultInfo_->peerName);
    int64_t param = 0;
    sysEvent->GetParamValue(GET_NAME(PEER_USERID), param);
    ImfHiSysEventReporterTest::faultInfo_->peerUserId = param;
    sysEvent->GetParamValue(GET_NAME(PEER_PID), ImfHiSysEventReporterTest::faultInfo_->peerPid);
    param = CLIENT_TYPE_END;
    sysEvent->GetParamValue(GET_NAME(CLIENT_TYPE), param);
    ImfHiSysEventReporterTest::faultInfo_->clientType = static_cast<ClientType>(param);
    param = 0;
    sysEvent->GetParamValue(GET_NAME(INPUT_PATTERN), param);
    ImfHiSysEventReporterTest::faultInfo_->inputPattern = param;
    param = 1;
    sysEvent->GetParamValue(GET_NAME(ISSHOWKEYBOARD), param);
    ImfHiSysEventReporterTest::faultInfo_->isShowKeyboard = param;
    sysEvent->GetParamValue(GET_NAME(IME_NAME), ImfHiSysEventReporterTest::faultInfo_->imeName);
    param = 0;
    sysEvent->GetParamValue(GET_NAME(ERR_CODE), param);
    ImfHiSysEventReporterTest::faultInfo_->errCode = param;
    param = 0;
    sysEvent->GetParamValue(GET_NAME(EVENT_CODE), param);
    ImfHiSysEventReporterTest::faultInfo_->eventCode = param;
}
void ImfHiSysEventWatcher::NotifyClientAttachStatistic(std::shared_ptr<HiSysEventRecord> sysEvent)
{
    if (sysEvent == nullptr) {
        IMSA_HILOGE("sysEvent is nullptr.");
        return;
    }
    sysEvent->GetParamValue(GET_NAME(IME_NAME), ImfHiSysEventReporterTest::clientAttachStatistics_.imeNames);
    sysEvent->GetParamValue(GET_NAME(APP_NAME), ImfHiSysEventReporterTest::clientAttachStatistics_.appNames);
    std::vector<std::string> infos;
    sysEvent->GetParamValue(GET_NAME(INFOS), infos);
    if (infos.size() != 1) {
        IMSA_HILOGE("infos size is %{public}zu.", infos.size());
        return;
    }
    ImfHiSysEventReporterTest::clientAttachStatistics_.Unmarshall(infos[0]);
}
void ImfHiSysEventWatcher::NotifyClientShowStatistic(std::shared_ptr<HiSysEventRecord> sysEvent)
{
    if (sysEvent == nullptr) {
        IMSA_HILOGE("sysEvent is nullptr.");
        return;
    }
    sysEvent->GetParamValue(GET_NAME(IME_NAME), ImfHiSysEventReporterTest::clientShowStatistics_.imeNames);
    sysEvent->GetParamValue(GET_NAME(APP_NAME), ImfHiSysEventReporterTest::clientShowStatistics_.appNames);
    std::vector<std::string> infos;
    sysEvent->GetParamValue(GET_NAME(INFOS), infos);
    if (infos.size() != 1) {
        IMSA_HILOGE("infos size is %{public}zu.", infos.size());
        return;
    }
    ImfHiSysEventReporterTest::clientShowStatistics_.Unmarshall(infos[0]);
}

void ImfHiSysEventWatcher::NotifyImeStartInputStatistic(std::shared_ptr<HiSysEventRecord> sysEvent)
{
    if (sysEvent == nullptr) {
        IMSA_HILOGE("sysEvent is nullptr.");
        return;
    }
    sysEvent->GetParamValue(GET_NAME(APP_NAME), ImfHiSysEventReporterTest::imeStartInputStatistics_.appNames);
    std::vector<std::string> infos;
    sysEvent->GetParamValue(GET_NAME(INFOS), infos);
    if (infos.size() != 1) {
        IMSA_HILOGE("infos size is %{public}zu.", infos.size());
        return;
    }
    ImfHiSysEventReporterTest::imeStartInputStatistics_.Unmarshall(infos[0]);
}
void ImfHiSysEventWatcher::NotifyBaseTextOperatorStatistic(std::shared_ptr<HiSysEventRecord> sysEvent)
{
    if (sysEvent == nullptr) {
        IMSA_HILOGE("sysEvent is nullptr.");
        return;
    }
    sysEvent->GetParamValue(GET_NAME(APP_NAME), ImfHiSysEventReporterTest::baseTextOperationStatistics_.appNames);
    std::vector<std::string> infos;
    sysEvent->GetParamValue(GET_NAME(INFOS), infos);
    if (infos.size() != 1) {
        IMSA_HILOGE("infos size is %{public}zu.", infos.size());
        return;
    }
    ImfHiSysEventReporterTest::baseTextOperationStatistics_.Unmarshall(infos[0]);
}

bool ImfHiSysEventReporterTest::WaitFaultEvent(const std::string &eventName, const HiSysOriginalInfo &info)
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    watcherCv_.wait_for(lock, std::chrono::milliseconds(WATI_HISYSEVENT_TIMEOUT),
        [&eventName, &info]() { return eventName_ == eventName && info == *faultInfo_; });
    IMSA_HILOGI("notify param:[%{public}d, %{public}d, %{public}s, %{public}d, %{public}d, "
                "%{public}d, "
                "%{public}d, %{public}s, %{public}d, %{public}d]",
        faultInfo_->eventCode, faultInfo_->errCode, faultInfo_->peerName.c_str(), faultInfo_->peerUserId,
        faultInfo_->clientType, faultInfo_->inputPattern, faultInfo_->isShowKeyboard, faultInfo_->imeName.c_str(),
        faultInfo_->imeCbTime, faultInfo_->baseTextOperationTime);
    IMSA_HILOGI("expect param:[%{public}d, %{public}d, %{public}s, %{public}d, %{public}d, "
                "%{public}d, "
                "%{public}d, %{public}s, %{public}d, %{public}d]",
        info.eventCode, info.errCode, info.peerName.c_str(), info.peerUserId, info.clientType, info.inputPattern,
        info.isShowKeyboard, info.imeName.c_str(), info.imeCbTime, info.baseTextOperationTime);
    return eventName_ == eventName && info == *faultInfo_;
}

bool ImfHiSysEventReporterTest::WaitClientAttachStatistic(const ClientAttachStatistics &statistic)
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    watcherCv_.wait_for(lock, std::chrono::milliseconds(WATI_HISYSEVENT_TIMEOUT),
        [&statistic]() { return clientAttachStatistics_ == statistic; });
    return clientAttachStatistics_ == statistic;
}
bool ImfHiSysEventReporterTest::WaitClientShowStatistic(const ClientShowStatistics &statistic)
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    watcherCv_.wait_for(lock, std::chrono::milliseconds(WATI_HISYSEVENT_TIMEOUT),
        [&statistic]() { return clientShowStatistics_ == statistic; });
    return clientShowStatistics_ == statistic;
}
bool ImfHiSysEventReporterTest::WaitImeStartInputStatistic(const ImeStartInputStatistics &statistic)
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    watcherCv_.wait_for(lock, std::chrono::milliseconds(WATI_HISYSEVENT_TIMEOUT),
        [&statistic]() { return imeStartInputStatistics_ == statistic; });
    return imeStartInputStatistics_ == statistic;
}
bool ImfHiSysEventReporterTest::WaitBaseTextOperatorStatistic(const BaseTextOperationStatistics &statistic)
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    watcherCv_.wait_for(lock, std::chrono::milliseconds(WATI_HISYSEVENT_TIMEOUT),
        [&statistic]() { return baseTextOperationStatistics_ == statistic; });
    return baseTextOperationStatistics_ == statistic;
}

void ImfHiSysEventReporterTest::TestClientAttach(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    std::string testString(reinterpret_cast<const char *>(data), size);
    auto paramPeerName = testString;
    auto paramPeerUserId = static_cast<int32_t>(size);
    auto paramPeerPid = static_cast<int64_t>(size);
    auto errCode = static_cast<int32_t>(size);
    auto paramIsShowkeyboard = static_cast<bool>(data[0] % 2); // remainder 2 can generate a random number of 0 or 1
    auto paramImeName = testString;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(paramPeerName)
                    .SetPeerPid(paramPeerPid)
                    .SetPeerUserId(paramPeerUserId)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetInputPattern(PARAM_INPUT_PATTERN)
                    .SetIsShowKeyboard(paramIsShowkeyboard)
                    .SetImeName(paramImeName)
                    .SetErrCode(errCode)
                    .Build();
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_ATTACH, *info);
}

void ImfHiSysEventReporterTest::TestClientShow(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    std::string testString(reinterpret_cast<const char *>(data), size);
    auto paramPeerName = testString;
    auto paramPeerUserId = static_cast<int32_t>(size);
    auto paramPeerPid = static_cast<int64_t>(size);
    auto errCode = static_cast<int32_t>(size);
    auto paramIEventCode = static_cast<int32_t>(size);
    auto paramImeName = testString;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(paramPeerName)
                    .SetPeerPid(paramPeerPid)
                    .SetPeerUserId(paramPeerUserId)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetImeName(paramImeName)
                    .SetEventCode(paramIEventCode)
                    .SetErrCode(errCode)
                    .Build();
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_SHOW, *info);
}

void ImfHiSysEventReporterTest::TestStartInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    std::string testString(reinterpret_cast<const char *>(data), size);
    auto paramPeerName = testString;
    auto paramPeerPid = static_cast<int64_t>(size);
    auto errCode = static_cast<int32_t>(size);
    auto paramIsShowkeyboard = static_cast<bool>(data[0] % 2); // remainder 2 can generate a random number of 0 or 1
    auto paramIEventCode = static_cast<int32_t>(size);
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(paramPeerName)
                    .SetPeerPid(paramPeerPid)
                    .SetIsShowKeyboard(paramIsShowkeyboard)
                    .SetEventCode(paramIEventCode)
                    .SetErrCode(errCode)
                    .SetImeCbTime(rand())
                    .Build();
    ImaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::IME_START_INPUT, *info);
}

void ImfHiSysEventReporterTest::TestBaseTextOperation(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    std::string testString(reinterpret_cast<const char *>(data), size);
    auto paramPeerName = testString;
    auto paramPeerPid = static_cast<int64_t>(size);
    auto errCode = static_cast<int32_t>(size);
    auto paramIEventCode = static_cast<int32_t>(size);
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(paramPeerName)
                    .SetPeerPid(paramPeerPid)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetEventCode(paramIEventCode)
                    .SetErrCode(errCode)
                    .SetBaseTextOperatorTime(rand())
                    .Build();
    ImaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::BASE_TEXT_OPERATOR, *info);
}

void ImfHiSysEventReporterTest::TestStatisticsEventConcurrent()
{
    for (int32_t i = 0; i < EACH_THREAD_CIRCULATION_TIME; i++) {
        size_t size = static_cast<size_t>(rand() % 100); // A remainder of 100 can generate a random number from 0-99
        uint8_t *data = static_cast<uint8_t *>(malloc(size * sizeof(uint8_t)));
        if (data == nullptr) {
            return;
        }
        for (auto j = 0; j < size; j++) {
            data[j] = static_cast<uint8_t>(rand() % 100); // A remainder of 100 can generate a random number from 0-99
        }

        TestClientAttach(data, size);
        TestClientShow(data, size);
        TestStartInput(data, size);
        TestBaseTextOperation(data, size);
        ImaHiSysEventReporter::GetInstance().TimerCallback();
        ImsaHiSysEventReporter::GetInstance().TimerCallback();
        free(data);
        multiThreadExecTotalNum_++;
    }
}

std::mutex ImfHiSysEventReporterTest::cvMutex_;
std::condition_variable ImfHiSysEventReporterTest::watcherCv_;
std::shared_ptr<HiSysOriginalInfo> ImfHiSysEventReporterTest::faultInfo_ = HiSysOriginalInfo::Builder().Build();
std::string ImfHiSysEventReporterTest::eventName_;
ClientAttachStatistics ImfHiSysEventReporterTest::clientAttachStatistics_;
ClientShowStatistics ImfHiSysEventReporterTest::clientShowStatistics_;
ImeStartInputStatistics ImfHiSysEventReporterTest::imeStartInputStatistics_;
BaseTextOperationStatistics ImfHiSysEventReporterTest::baseTextOperationStatistics_;
int32_t ImfHiSysEventReporterTest::multiThreadExecTotalNum_{ 0 };

/**
 * @tc.name: ImcClientAttachFailed_001
 * @tc.desc: imc client attach failed, report fault, report fault again within 5 miniutes
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfHiSysEventReporterTest, ImcClientAttachFailed_001, TestSize.Level1)
{
    IMSA_HILOGI("ImcClientAttachFailed_001");
    int32_t errCode = ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    auto info = HiSysOriginalInfo::Builder()
                    .SetErrCode(errCode)
                    .SetInputPattern(PARAM_INPUT_PATTERN)
                    .SetIsShowKeyboard(PARAM_ISSHOWKEYBOARD)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .Build();
    ImcHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_ATTACH, *info);
    EXPECT_TRUE(ImfHiSysEventReporterTest::WaitFaultEvent(EVENT_NAME_CLIENT_ATTACH_FAILED, *info));

    ImfHiSysEventReporterTest::faultInfo_ = HiSysOriginalInfo::Builder().Build();
    ImfHiSysEventReporterTest::eventName_.clear();
    // report again
    ImcHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_ATTACH, *info);
    EXPECT_FALSE(ImfHiSysEventReporterTest::WaitFaultEvent(EVENT_NAME_CLIENT_ATTACH_FAILED, *info));
    auto key = ImcHiSysEventReporter::GetInstance().GenerateFaultEventKey(ImfFaultEvent::CLIENT_ATTACH_FAILED, *info);
    auto it = ImcHiSysEventReporter::GetInstance().faultEventRecords_.find(key);
    ASSERT_NE(it, ImcHiSysEventReporter::GetInstance().faultEventRecords_.end());
    EXPECT_EQ(it->second.second, 2);
    ImcHiSysEventReporter::GetInstance().faultEventRecords_.clear();
}
/**
 * @tc.name: ImsaClientAttachFailed_002
 * @tc.desc: imsa client attach failed, report fault
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfHiSysEventReporterTest, ImsaClientAttachFailed_002, TestSize.Level1)
{
    IMSA_HILOGI("ImsaClientAttachFailed_002");
    int32_t errCode = ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(PARAM_PEER_NAME)
                    .SetPeerPid(PARAM_PEER_PID)
                    .SetPeerUserId(PARAM_PEER_USERID)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetInputPattern(PARAM_INPUT_PATTERN)
                    .SetIsShowKeyboard(PARAM_ISSHOWKEYBOARD)
                    .SetImeName(PARAM_IME_NAME)
                    .SetErrCode(errCode)
                    .Build();
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_ATTACH, *info);
    EXPECT_TRUE(ImfHiSysEventReporterTest::WaitFaultEvent(EVENT_NAME_CLIENT_ATTACH_FAILED, *info));
    auto key = ImsaHiSysEventReporter::GetInstance().GenerateFaultEventKey(ImfFaultEvent::CLIENT_ATTACH_FAILED, *info);
    auto it = ImsaHiSysEventReporter::GetInstance().faultEventRecords_.find(key);
    ASSERT_NE(it, ImsaHiSysEventReporter::GetInstance().faultEventRecords_.end());
    EXPECT_EQ(it->second.second, 1);
    ImsaHiSysEventReporter::GetInstance().faultEventRecords_.clear();
}
/**
 * @tc.name: ImsaClientAttachFailed_003
 * @tc.desc: imsa client attach failed, err but not fault, not report
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfHiSysEventReporterTest, ImsaClientAttachFailed_003, TestSize.Level1)
{
    IMSA_HILOGI("ImsaClientAttachFailed_003");
    int32_t errCode = ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(PARAM_PEER_NAME)
                    .SetPeerPid(PARAM_PEER_PID)
                    .SetPeerUserId(PARAM_PEER_USERID)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetInputPattern(PARAM_INPUT_PATTERN)
                    .SetIsShowKeyboard(PARAM_ISSHOWKEYBOARD)
                    .SetImeName(PARAM_IME_NAME)
                    .SetErrCode(errCode)
                    .Build();
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_ATTACH, *info);
    EXPECT_FALSE(ImfHiSysEventReporterTest::WaitFaultEvent(EVENT_NAME_CLIENT_ATTACH_FAILED, *info));
    auto key = ImsaHiSysEventReporter::GetInstance().GenerateFaultEventKey(ImfFaultEvent::CLIENT_ATTACH_FAILED, *info);
    auto it = ImsaHiSysEventReporter::GetInstance().faultEventRecords_.find(key);
    ASSERT_EQ(it, ImsaHiSysEventReporter::GetInstance().faultEventRecords_.end());
}
/**
 * @tc.name: ImcClientShowFailed_004
 * @tc.desc: imc client show failed, report fault
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfHiSysEventReporterTest, ImcClientShowFailed_004, TestSize.Level1)
{
    IMSA_HILOGI("ImcClientShowFailed_004");
    int32_t errCode = ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(PARAM_PEER_NAME)
                    .SetPeerPid(PARAM_PEER_PID)
                    .SetPeerUserId(PARAM_PEER_USERID)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetImeName(PARAM_IME_NAME)
                    .SetEventCode(PARAM_EVENT_CODE)
                    .SetErrCode(errCode)
                    .Build();
    ImcHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_SHOW, *info);
    EXPECT_TRUE(ImfHiSysEventReporterTest::WaitFaultEvent(EVENT_NAME_CLIENT_SHOW_FAILED, *info));
    auto key = ImcHiSysEventReporter::GetInstance().GenerateFaultEventKey(ImfFaultEvent::CLIENT_SHOW_FAILED, *info);
    auto it = ImcHiSysEventReporter::GetInstance().faultEventRecords_.find(key);
    ASSERT_NE(it, ImcHiSysEventReporter::GetInstance().faultEventRecords_.end());
    EXPECT_EQ(it->second.second, 1);
    ImcHiSysEventReporter::GetInstance().faultEventRecords_.clear();
}

/**
 * @tc.name: ImsaClientShowFailed_005
 * @tc.desc: imsa client show failed, report fault
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfHiSysEventReporterTest, ImsaClientShowFailed_005, TestSize.Level1)
{
    IMSA_HILOGI("ImsaClientShowFailed_005");
    int32_t errCode = ErrorCode::ERROR_CLIENT_NULL_POINTER;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(PARAM_PEER_NAME)
                    .SetPeerPid(PARAM_PEER_PID)
                    .SetPeerUserId(PARAM_PEER_USERID)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetImeName(PARAM_IME_NAME)
                    .SetEventCode(PARAM_EVENT_CODE)
                    .SetErrCode(errCode)
                    .Build();
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_SHOW, *info);
    EXPECT_TRUE(ImfHiSysEventReporterTest::WaitFaultEvent(EVENT_NAME_CLIENT_SHOW_FAILED, *info));
    auto key = ImsaHiSysEventReporter::GetInstance().GenerateFaultEventKey(ImfFaultEvent::CLIENT_SHOW_FAILED, *info);
    auto it = ImsaHiSysEventReporter::GetInstance().faultEventRecords_.find(key);
    ASSERT_NE(it, ImsaHiSysEventReporter::GetInstance().faultEventRecords_.end());
    EXPECT_EQ(it->second.second, 1);
    ImsaHiSysEventReporter::GetInstance().faultEventRecords_.clear();
}

/**
 * @tc.name: ImcClientShowFailed_006
 * @tc.desc: imc client show failed, report fault, but errCode not imc, not report
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfHiSysEventReporterTest, ImcClientShowFailed_006, TestSize.Level1)
{
    IMSA_HILOGI("ImcClientShowFailed_006");
    int32_t errCode = ErrorCode::ERROR_CLIENT_NULL_POINTER;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(PARAM_PEER_NAME)
                    .SetPeerPid(PARAM_PEER_PID)
                    .SetPeerUserId(PARAM_PEER_USERID)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetImeName(PARAM_IME_NAME)
                    .SetEventCode(PARAM_EVENT_CODE)
                    .SetErrCode(errCode)
                    .Build();
    ImcHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_SHOW, *info);
    EXPECT_FALSE(ImfHiSysEventReporterTest::WaitFaultEvent(EVENT_NAME_CLIENT_SHOW_FAILED, *info));
    auto key = ImcHiSysEventReporter::GetInstance().GenerateFaultEventKey(ImfFaultEvent::CLIENT_SHOW_FAILED, *info);
    auto it = ImcHiSysEventReporter::GetInstance().faultEventRecords_.find(key);
    ASSERT_EQ(it, ImcHiSysEventReporter::GetInstance().faultEventRecords_.end());
}

/**
 * @tc.name: ImeStartInputFailed_007
 * @tc.desc: imc start input failed, report fault
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfHiSysEventReporterTest, ImeStartInputFailed_007, TestSize.Level1)
{
    IMSA_HILOGI("ImeStartInputFailed_007");
    int32_t errCode = ErrorCode::ERROR_IME;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(PARAM_PEER_NAME)
                    .SetPeerPid(PARAM_PEER_PID)
                    .SetIsShowKeyboard(PARAM_ISSHOWKEYBOARD)
                    .SetEventCode(PARAM_EVENT_CODE)
                    .SetErrCode(errCode)
                    .Build();
    ImaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::IME_START_INPUT, *info);
    EXPECT_TRUE(ImfHiSysEventReporterTest::WaitFaultEvent(EVENT_NAME_IME_START_INPUT_FAILED, *info));
    auto key = ImaHiSysEventReporter::GetInstance().GenerateFaultEventKey(ImfFaultEvent::IME_START_INPUT_FAILED, *info);
    auto it = ImaHiSysEventReporter::GetInstance().faultEventRecords_.find(key);
    ASSERT_NE(it, ImaHiSysEventReporter::GetInstance().faultEventRecords_.end());
    EXPECT_EQ(it->second.second, 1);
    ImaHiSysEventReporter::GetInstance().faultEventRecords_.clear();
}

/**
 * @tc.name: BaseTextOperationFailed_008
 * @tc.desc: base text operation failed, report fault
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfHiSysEventReporterTest, BaseTextOperationFailed_008, TestSize.Level1)
{
    IMSA_HILOGI("BaseTextOperationFailed_008");
    int32_t errCode = ErrorCode::ERROR_IMA_CHANNEL_NULLPTR;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(PARAM_PEER_NAME)
                    .SetPeerPid(PARAM_PEER_PID)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetEventCode(PARAM_EVENT_CODE)
                    .SetErrCode(errCode)
                    .Build();
    ImaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::BASE_TEXT_OPERATOR, *info);
    EXPECT_TRUE(ImfHiSysEventReporterTest::WaitFaultEvent(EVENT_NAME_BASIC_TEXT_OPERATION_FAILED, *info));
    auto key =
        ImaHiSysEventReporter::GetInstance().GenerateFaultEventKey(ImfFaultEvent::BASE_TEXT_OPERATION_FAILED, *info);
    auto it = ImaHiSysEventReporter::GetInstance().faultEventRecords_.find(key);
    ASSERT_NE(it, ImaHiSysEventReporter::GetInstance().faultEventRecords_.end());
    EXPECT_EQ(it->second.second, 1);
    ImaHiSysEventReporter::GetInstance().faultEventRecords_.clear();
}

/**
 * @tc.name: ClientAttachStatistic_009
 * @tc.desc: imsa report statistic that contain two different succeed and two different failed event
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfHiSysEventReporterTest, ClientAttachStatistic_009, TestSize.Level1)
{
    IMSA_HILOGI("ClientAttachStatistic_009");
    int32_t errCode1 = ErrorCode::ERROR_CLIENT_NULL_POINTER;
    int32_t errCode2 = ErrorCode::ERROR_CLIENT_ADD_FAILED;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(PARAM_PEER_NAME)
                    .SetPeerPid(PARAM_PEER_PID)
                    .SetPeerUserId(PARAM_PEER_USERID)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetImeName(PARAM_IME_NAME)
                    .SetErrCode(errCode1)
                    .Build();
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_ATTACH, *info);
    info->errCode = errCode2;
    info->imeName = PARAM_IME_NAME1;
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_ATTACH, *info);
    info->errCode = ErrorCode::NO_ERROR;
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_ATTACH, *info);
    info->errCode = ErrorCode::NO_ERROR;
    info->peerName = PARAM_PEER_NAME1;
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_ATTACH, *info);
    auto index = ImsaHiSysEventReporter::GetInstance().GetStatisticalIntervalIndex();
    ASSERT_TRUE(index < ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
    ClientAttachStatistics statistics;
    statistics.imeNames.emplace_back(PARAM_IME_NAME);
    statistics.imeNames.emplace_back(PARAM_IME_NAME1);
    statistics.appNames.emplace_back("*");
    statistics.succeedInfo.count = 2;
    statistics.succeedInfo.info.resize(ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
    statistics.succeedInfo.info[index] = { "0/2" };
    statistics.failedInfo.count = 2;
    statistics.failedInfo.info.resize(ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
    std::string failedStr1("0");
    failedStr1.append("/").append("0").append("/").append(std::to_string(PARAM_CLIENT_TYPE)).append("/")
        .append(std::to_string(errCode1)).append("/").append("1");
    std::string failedStr2("0");
    failedStr2.append("/").append("1").append("/").append(std::to_string(PARAM_CLIENT_TYPE)).append("/")
        .append(std::to_string(errCode2)).append("/").append("1");
    statistics.failedInfo.info[index] = { failedStr1, failedStr2 };
    ImsaHiSysEventReporter::GetInstance().TimerCallback();
    EXPECT_TRUE(ImfHiSysEventReporterTest::WaitClientAttachStatistic(statistics));
    EXPECT_TRUE(ImsaHiSysEventReporter::GetInstance().clientAttachInfo_.appNames.empty());
}
/**
 * @tc.name: ClientShowStatistic_010
 * @tc.desc: imsa report statistic that contain two same failed event
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfHiSysEventReporterTest, ClientShowStatistic_010, TestSize.Level1)
{
    IMSA_HILOGI("ClientShowStatistic_010");
    int32_t errCode = ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(PARAM_PEER_NAME)
                    .SetPeerPid(PARAM_PEER_PID)
                    .SetPeerUserId(PARAM_PEER_USERID)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetImeName(PARAM_IME_NAME)
                    .SetEventCode(PARAM_EVENT_CODE)
                    .SetErrCode(errCode)
                    .Build();
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_SHOW, *info);
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_SHOW, *info);
    auto index = ImsaHiSysEventReporter::GetInstance().GetStatisticalIntervalIndex();
    ASSERT_TRUE(index < ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
    ClientShowStatistics statistics;
    statistics.imeNames.emplace_back(PARAM_IME_NAME);
    statistics.appNames.emplace_back("*");
    statistics.succeedInfo.count = 0;
    statistics.succeedInfo.info.resize(ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
    statistics.failedInfo.count = 2;
    statistics.failedInfo.info.resize(ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
    std::string failedStr("0");
    failedStr.append("/")
        .append("0")
        .append("/")
        .append(std::to_string(PARAM_CLIENT_TYPE))
        .append("/")
        .append(std::to_string(PARAM_EVENT_CODE))
        .append("/")
        .append(std::to_string(errCode))
        .append("/")
        .append("2");
    statistics.failedInfo.info[index] = { failedStr };
    ImsaHiSysEventReporter::GetInstance().TimerCallback();
    EXPECT_TRUE(ImfHiSysEventReporterTest::WaitClientShowStatistic(statistics));
    EXPECT_TRUE(ImsaHiSysEventReporter::GetInstance().clientShowInfo_.appNames.empty());
}
/**
 * @tc.name: ImeStartInputStatistic_011
 * @tc.desc: imsa report statistic that contain one succeed event and one failed event(not contain cbTime)
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfHiSysEventReporterTest, ImeStartInputStatistic_011, TestSize.Level1)
{
    IMSA_HILOGI("ImeStartInputStatistic_011");
    int32_t errCode = ErrorCode::ERROR_IME;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(PARAM_PEER_NAME)
                    .SetPeerPid(PARAM_PEER_PID)
                    .SetIsShowKeyboard(true)
                    .SetEventCode(PARAM_EVENT_CODE)
                    .SetErrCode(errCode)
                    .Build();
    ImaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::IME_START_INPUT, *info);
    info->errCode = ErrorCode::NO_ERROR;
    info->imeCbTime = 40;
    ImaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::IME_START_INPUT, *info);
    auto index = ImaHiSysEventReporter::GetInstance().GetStatisticalIntervalIndex();
    ASSERT_TRUE(index < ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
    ImeStartInputStatistics statistics;
    statistics.appNames.emplace_back("*");
    statistics.succeedInfo.count = 1;
    statistics.succeedInfo.info.resize(ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
    std::string succeedStr("0");
    succeedStr.append("/").append(std::to_string(true)).append("/").append("1");
    statistics.succeedInfo.info[index] = { succeedStr };
    statistics.failedInfo.count = 1;
    statistics.failedInfo.info.resize(ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
    std::string failedStr("0");
    failedStr.append("/")
        .append(std::to_string(PARAM_EVENT_CODE))
        .append("/")
        .append(std::to_string(errCode))
        .append("/")
        .append("1");
    statistics.failedInfo.info[index] = { failedStr };
    statistics.cbTimeConsume.count = 1;
    statistics.cbTimeConsume.timeConsumeStatistics = "0/1/0/0/0/0";
    ImaHiSysEventReporter::GetInstance().TimerCallback();
    EXPECT_TRUE(ImfHiSysEventReporterTest::WaitImeStartInputStatistic(statistics));
    EXPECT_TRUE(ImaHiSysEventReporter::GetInstance().imeStartInputAllInfo_.appNames.empty());
}

/**
 * @tc.name: BaseTextOperationStatistic_012
 * @tc.desc: ima report statistic that contain one succeed event and one failed event
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfHiSysEventReporterTest, BaseTextOperationStatistic_012, TestSize.Level1)
{
    IMSA_HILOGI("BaseTextOperationStatistic_012");
    int32_t failedConsumeTime = 60;
    int32_t suceedConsumeTime = 10;
    int32_t errCode = ErrorCode::ERROR_CLIENT_NULL_POINTER;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(PARAM_PEER_NAME)
                    .SetPeerPid(PARAM_PEER_PID)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetEventCode(PARAM_EVENT_CODE)
                    .SetErrCode(errCode)
                    .SetBaseTextOperatorTime(failedConsumeTime)
                    .Build();
    ImaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::BASE_TEXT_OPERATOR, *info);
    info->errCode = ErrorCode::NO_ERROR;
    info->baseTextOperationTime = suceedConsumeTime;
    ImaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::BASE_TEXT_OPERATOR, *info);

    BaseTextOperationStatistics statistics;
    statistics.appNames.emplace_back("*");
    statistics.succeedInfo.count = 1;
    statistics.succeedInfo.info.resize(ImaHiSysEventReporter::BASE_TEXT_OPERATION_TIME_INTERVAL.size());
    auto succeedIndex =
        ImaHiSysEventReporter::GetInstance().GetBaseTextOperationSucceedIntervalIndex(suceedConsumeTime);
    ASSERT_TRUE(succeedIndex < ImaHiSysEventReporter::BASE_TEXT_OPERATION_TIME_INTERVAL.size());
    std::string succeedStr("0");
    succeedStr.append("/").append(std::to_string(PARAM_EVENT_CODE)).append("/").append("1");
    statistics.succeedInfo.info[succeedIndex] = { succeedStr };
    auto failedIndex = ImaHiSysEventReporter::GetInstance().GetStatisticalIntervalIndex();
    ASSERT_TRUE(failedIndex < ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
    statistics.failedInfo.count = 1;
    statistics.failedInfo.info.resize(ImfHiSysEventReporter::COUNT_STATISTICS_INTERVAL_NUM);
    std::string failedStr("0");
    failedStr.append("/")
        .append(std::to_string(PARAM_CLIENT_TYPE))
        .append("/")
        .append(std::to_string(PARAM_EVENT_CODE))
        .append("/")
        .append(std::to_string(errCode))
        .append("/")
        .append("1");
    statistics.failedInfo.info[failedIndex] = { failedStr };
    ImaHiSysEventReporter::GetInstance().TimerCallback();
    EXPECT_TRUE(ImfHiSysEventReporterTest::WaitBaseTextOperatorStatistic(statistics));
    EXPECT_TRUE(ImaHiSysEventReporter::GetInstance().baseTextOperationAllInfo_.appNames.empty());
}

/**
 * @tc.name: StatisticsEventConcurrentTest_013
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfHiSysEventReporterTest, StatisticsEventConcurrentTest_013, TestSize.Level1)
{
    IMSA_HILOGI("StatisticsEventConcurrentTest_013");
    SET_THREAD_NUM(THREAD_NUM);
    GTEST_RUN_TASK(TestStatisticsEventConcurrent);
    EXPECT_EQ(multiThreadExecTotalNum_, THREAD_NUM * EACH_THREAD_CIRCULATION_TIME);
}
} // namespace MiscServices
} // namespace OHOS
