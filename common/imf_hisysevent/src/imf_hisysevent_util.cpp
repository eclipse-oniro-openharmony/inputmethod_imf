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

#include "imf_hisysevent_util.h"

#include <algorithm>

namespace OHOS {
namespace MiscServices {

void ImfHiSysEventUtil::ReportClientAttachFault(
    const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info)
{
    HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "BASE_TEXT_OPERATOR_FAILED",
        HiSysEventNameSpace::EventType::FAULT, "SELF_NAME", selfName, "PEER_NAME", info.peerName, "PEER_PID",
        info.peerPid, "PEER_USERID", info.peerUserId, "CLIENT_TYPE", info.clientType, "INPUT_PATTERN",
        info.inputPattern, "ISSHOWKEYBOARD", info.isShowKeyboard, "IME_NAME", info.imeName, "ERR_CODE", info.eventCode,
        "FAULT_COUNT", faultNum);
}

void ImfHiSysEventUtil::ReportClientShowFault(
    const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info)
{
    HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "BASE_TEXT_OPERATOR_FAILED",
        HiSysEventNameSpace::EventType::FAULT, "SELF_NAME", selfName, "PEER_NAME", info.peerName, "PEER_PID",
        info.peerPid, "PEER_USERID", info.peerUserId, "CLIENT_TYPE", info.clientType, "INPUT_PATTERN",
        info.inputPattern, "IME_NAME", info.imeName, "EVENT_CODE", info.eventCode, "ERR_CODE", info.eventCode,
        "FAULT_COUNT", faultNum);
}

void ImfHiSysEventUtil::ReportImeStartInputFault(
    const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info)
{
    HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "BASE_TEXT_OPERATOR_FAILED",
        HiSysEventNameSpace::EventType::FAULT, "SELF_NAME", selfName, "PEER_NAME", info.peerName, "PEER_PID",
        info.peerPid, "ISSHOWKEYBOARD", info.isShowKeyboard, "EVENT_CODE", info.eventCode, "ERR_CODE", info.eventCode,
        "FAULT_COUNT", faultNum);
}

void ImfHiSysEventUtil::ReportBaseTextOperationFault(
    const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info)
{
    HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, "BASE_TEXT_OPERATOR_FAILED",
        HiSysEventNameSpace::EventType::FAULT, "SELF_NAME", selfName, "PEER_NAME", info.peerName, "PEER_PID",
        info.peerPid, "CLIENT_TYPE", info.clientType, "EVENT_CODE", info.eventCode, "ERR_CODE", info.eventCode,
        "FAULT_COUNT", faultNum);
}

void ImfHiSysEventUtil::ReportStatisticsEvent(const std::string &eventName,
    const std::unordered_set<std::string> &imeNames, const std::unordered_set<std::string> &appNames,
    const std::vector<std::string> &statistics)
{
    HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, eventName, HiSysEventNameSpace::EventType::STATISTIC,
        "IME_NAME", imeNames, "APP_NAME", appNames, "INFOS", statistics);
}

void ImfHiSysEventUtil::ReportStatisticsEvent(const std::string &eventName, const std::string &imeName,
    const std::unordered_set<std::string> &appNames, const std::vector<std::string> &statistics)
{
    HiSysEventWrite(HiSysEventNameSpace::Domain::INPUTMETHOD, eventName, HiSysEventNameSpace::EventType::STATISTIC,
        "SELF_NAME", imeName, "APP_NAME", appNames, "INFOS", statistics);
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

std::string ImfHiSysEventUtil::GetIndexInSet(
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
} // namespace MiscServices
} // namespace OHOS