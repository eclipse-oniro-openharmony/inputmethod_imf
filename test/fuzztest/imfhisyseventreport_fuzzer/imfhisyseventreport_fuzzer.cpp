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
#undef private

#include <sys/time.h>
#include <unistd.h>

#include <condition_variable>
#include <cstdint>
#include <string>

#include "imfhisyseventreport_fuzzer.h"
#include "global.h"
#include "hisysevent_base_manager.h"
#include "hisysevent_listener.h"
#include "hisysevent_manager.h"
#include "hisysevent_query_callback.h"
#include "hisysevent_record.h"

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
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto paramPeerName = fuzzedString;
    auto paramPeerUserId = static_cast<int32_t>(size);
    auto paramPeerPid = static_cast<int64_t>(size);
    auto errCode = static_cast<int32_t>(size);
    auto paramIsShowkeyboard = static_cast<bool>(data[0] % 2);
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
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto paramPeerName = fuzzedString;
    auto paramPeerUserId = static_cast<int32_t>(size);
    auto paramPeerPid = static_cast<int64_t>(size);
    auto errCode = static_cast<int32_t>(size);
    auto paramIEventCode = static_cast<int32_t>(size);
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
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto paramPeerName = fuzzedString;
    auto paramPeerPid = static_cast<int64_t>(size);
    auto errCode = static_cast<int32_t>(size);
    auto paramIsShowkeyboard = static_cast<bool>(data[0] % 2);
    auto paramIEventCode = static_cast<int32_t>(size);
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
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto paramPeerName = fuzzedString;
    auto paramPeerPid = static_cast<int64_t>(size);
    auto errCode = static_cast<int32_t>(size);
    auto paramIEventCode = static_cast<int32_t>(size);
    auto info = HiSysOriginalInfo::Builder()
                    .SetPeerName(paramPeerName)
                    .SetPeerPid(paramPeerPid)
                    .SetClientType(PARAM_CLIENT_TYPE)
                    .SetEventCode(paramIEventCode)
                    .SetErrCode(errCode)
                    .Build();
    ImaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::BASE_TEXT_OPERATOR, *info);
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
    return 0;
}