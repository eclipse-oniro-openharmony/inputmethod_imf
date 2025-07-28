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

#define private public
#define protected public
#include "ima_hisysevent_reporter.h"
#include "imc_hisysevent_reporter.h"
#include "imf_hisysevent_info.h"
#include "imf_hisysevent_reporter.h"
#include "imf_hisysevent_util.h"
#include "imsa_hisysevent_reporter.h"
#include "inputmethod_sysevent.h"
#undef private

#include <sys/time.h>
#include <unistd.h>

#include <condition_variable>
#include <cstdint>
#include <string>

#include "fuzzer/FuzzedDataProvider.h"
#include "global.h"
#include "hisysevent_base_manager.h"
#include "hisysevent_listener.h"
#include "hisysevent_manager.h"
#include "hisysevent_query_callback.h"
#include "hisysevent_record.h"
#include "imfhisyseventreport_fuzzer.h"

using namespace OHOS::MiscServices;
namespace OHOS {
using namespace OHOS::HiviewDFX;
constexpr ClientType PARAM_CLIENT_TYPE = ClientType::INNER_KIT;
constexpr int32_t PARAM_INPUT_PATTERN = 6;
void TestClientAttach01(const uint8_t *data, size_t size)
{
    auto errCode = static_cast<int32_t>(size);
    auto paramIsShowkeyboard = static_cast<bool>(data[0] % 2);
    auto info = HiSysOriginalInfo::Builder()
                .SetErrCode(errCode)
                .SetInputPattern(PARAM_INPUT_PATTERN)
                .SetIsShowKeyboard(paramIsShowkeyboard)
                .SetClientType(PARAM_CLIENT_TYPE)
                .Build();
    ImcHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_ATTACH, *info);
}

void TestClientAttach02(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto paramPeerName = fuzzedString;
    auto paramPeerUserId = provider.ConsumeIntegral<int32_t>();
    auto paramPeerPid = provider.ConsumeIntegral<int64_t>();
    auto errCode = provider.ConsumeIntegral<int32_t>();
    auto paramIsShowkeyboard = provider.ConsumeBool();
    auto paramImeName = fuzzedString;
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

void TestClientShow(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto paramPeerName = fuzzedString;
    auto paramPeerUserId = provider.ConsumeIntegral<int32_t>();
    auto paramPeerPid = provider.ConsumeIntegral<int64_t>();
    auto errCode = provider.ConsumeIntegral<int32_t>();
    auto paramIEventCode = provider.ConsumeIntegral<int32_t>();
    auto paramImeName = fuzzedString;
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(paramPeerName)
                    .SetPeerPid(paramPeerPid)
                    .SetPeerUserId(paramPeerUserId)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetImeName(paramImeName)
                    .SetEventCode(paramIEventCode)
                    .SetErrCode(errCode)
                    .Build();
    ImcHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_SHOW, *info);
}

void TestStartInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto paramPeerName = fuzzedString;
    auto paramPeerPid = provider.ConsumeIntegral<int64_t>();
    auto errCode = provider.ConsumeIntegral<int32_t>();
    auto paramIsShowkeyboard = provider.ConsumeBool();
    auto paramIEventCode = provider.ConsumeIntegral<int32_t>();
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(paramPeerName)
                    .SetPeerPid(paramPeerPid)
                    .SetIsShowKeyboard(paramIsShowkeyboard)
                    .SetEventCode(paramIEventCode)
                    .SetErrCode(errCode)
                    .Build();
    ImaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::IME_START_INPUT, *info);
}

void TestBaseTextOperation(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto paramPeerName = fuzzedString;
    auto paramPeerPid = provider.ConsumeIntegral<int64_t>();
    auto errCode = provider.ConsumeIntegral<int32_t>();
    auto paramIEventCode = provider.ConsumeIntegral<int32_t>();
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(paramPeerName)
                    .SetPeerPid(paramPeerPid)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetEventCode(paramIEventCode)
                    .SetErrCode(errCode)
                    .Build();
    ImaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::BASE_TEXT_OPERATOR, *info);
}

void TestRecordBaseTextOperationStatistics(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    auto code = provider.ConsumeIntegral<int32_t>();
    auto info = HiSysOriginalInfo::Builder()
                    .SetErrCode(code)
                    .Build();
    ImaHiSysEventReporter::GetInstance().RecordBaseTextOperationStatistics(*info);
    ImaHiSysEventReporter::GetInstance().RecordImeStartInputStatistics(*info);
}

void TestIntervalIndex(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    auto fuzzInt32 = provider.ConsumeIntegral<int32_t>();
    ImaHiSysEventReporter::GetInstance().GetBaseTextOperationSucceedIntervalIndex(fuzzInt32);
    ImaHiSysEventReporter::GetInstance().ReportStatisticsEvent();
    ImaHiSysEventReporter::GetInstance().ModImeCbTimeConsumeInfo(fuzzInt32);
}

void TestInputMethodSysEvent(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    auto fuzzInt32 = provider.ConsumeIntegral<int32_t>();
    auto fuzzUint32 = provider.ConsumeIntegral<uint32_t>();
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    using TimerCallback = std::function<void()>;
    TimerCallback tc;
    InputMethodSysEvent::GetInstance().ServiceFaultReporter(fuzzedString, fuzzInt32);
    InputMethodSysEvent::GetInstance().ImeUsageBehaviourReporter();
    InputMethodSysEvent::GetInstance().GetOperateInfo(fuzzInt32);
    InputMethodSysEvent::GetInstance().StartTimer(tc, fuzzUint32);
    InputMethodSysEvent::GetInstance().StartTimerForReport();
    InputMethodSysEvent::GetInstance().ReportSystemShortCut(fuzzedString);
}

void TestOnDemandStartStopSa(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    auto fuzzInt32 = provider.ConsumeIntegral<int32_t>();
    auto fuzzUint32 = provider.ConsumeIntegral<uint32_t>();
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    using TimerCallback = std::function<void()>;
    TimerCallback tc;
    InputMethodSysEvent::GetInstance().ServiceFaultReporter(fuzzedString, fuzzInt32);
    InputMethodSysEvent::GetInstance().ImeUsageBehaviourReporter();
    InputMethodSysEvent::GetInstance().GetOperateInfo(fuzzInt32);
    InputMethodSysEvent::GetInstance().StartTimer(tc, fuzzUint32);
    InputMethodSysEvent::GetInstance().StartTimerForReport();
    InputMethodSysEvent::GetInstance().ReportSystemShortCut(fuzzedString);
}

void TestReportStatisticsEvent(const uint8_t *data, size_t size)
{
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    static std::vector<std::string> appNames;
    static std::vector<std::string> statistics;
    appNames.push_back(fuzzedString);
    statistics.push_back(fuzzedString);
    ImsaHiSysEventReporter::GetInstance().ReportStatisticsEvent();
    ImfHiSysEventUtil::ReportStatisticsEvent(fuzzedString, fuzzedString, appNames, statistics);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    /* Run your code on data */
    OHOS::TestClientAttach01(data, size);
    OHOS::TestClientAttach02(data, size);
    OHOS::TestClientShow(data, size);
    OHOS::TestStartInput(data, size);
    OHOS::TestBaseTextOperation(data, size);
    OHOS::TestRecordBaseTextOperationStatistics(data, size);
    OHOS::TestIntervalIndex(data, size);
    OHOS::TestInputMethodSysEvent(data, size);
    OHOS::TestReportStatisticsEvent(data, size);
    return 0;
}