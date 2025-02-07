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

#ifndef IMF_HISYSEVENT_INFO_H
#define IMF_HISYSEVENT_INFO_H
#include <algorithm>
#include <utility>

#include "input_client_info.h"
#include "serializable.h"
namespace OHOS {
namespace MiscServices {

enum ImfEventType : uint32_t {
    CLIENT_ATTACH,
    CLIENT_SHOW,
    IME_START_INPUT,
    BASE_TEXT_OPERATOR,
};

enum ImfFaultEvent : uint32_t {
    HI_SYS_FAULT_EVENT_BEGIN,
    CLIENT_ATTACH_FAILED = HI_SYS_FAULT_EVENT_BEGIN,
    CLIENT_SHOW_FAILED,
    IME_START_INPUT_FAILED,
    BASE_TEXT_OPERATION_FAILED,
    HI_SYS_FAULT_EVENT_END,
};

enum ImfStatisticsEvent : uint32_t {
    HI_SYS_STATISTICS_EVENT_BEGIN,
    CLIENT_ATTACH_STATISTICS = HI_SYS_STATISTICS_EVENT_BEGIN,
    CLIENT_SHOW_STATISTICS,
    IME_START_INPUT_STATISTICS,
    BASE_TEXT_OPERATION_STATISTICS,
    HI_SYS_STATISTICS_EVENT_END,
};

struct HiSysOriginalInfo {
    int32_t eventCode;
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
    bool operator==(const HiSysOriginalInfo &info) const
    {
        return (eventCode == info.eventCode && errCode == info.errCode && peerName == info.peerName
                && peerUserId == info.peerUserId && clientType == info.clientType && inputPattern == info.inputPattern
                && isShowKeyboard == info.isShowKeyboard && imeName == info.imeName && imeCbTime == info.imeCbTime
                && baseTextOperationTime == info.baseTextOperationTime);
    }
    class Builder {
    public:
        Builder();
        Builder &SetEventCode(int32_t eventCode);
        Builder &SetErrCode(int32_t errCode);
        Builder &SetPeerName(const std::string &peerName);
        Builder &SetPeerUserId(int32_t peerUserId);
        Builder &SetPeerPid(int64_t peerPid);
        Builder &SetClientType(ClientType clientType);
        Builder &SetInputPattern(int32_t inputPattern);
        Builder &SetIsShowKeyboard(bool isShowKeyboard);
        Builder &SetImeName(const std::string &imeName);
        Builder &SetImeCbTime(int32_t imeCbTime);
        Builder &SetBaseTextOperatorTime(int32_t baseTextOperationTime);
        std::shared_ptr<HiSysOriginalInfo> Build();

    private:
        std::shared_ptr<HiSysOriginalInfo> info_ = nullptr;
    };
};

struct CountDistributionInfo : public Serializable {
    static constexpr uint32_t MAX_COUNT_DISTRIBUTIONS_INTERVAL = 10;
    explicit CountDistributionInfo(uint32_t intervalNum)
    {
        if (intervalNum > MAX_COUNT_DISTRIBUTIONS_INTERVAL) {
            intervalNum = MAX_COUNT_DISTRIBUTIONS_INTERVAL;
        }
        countDistributions.resize(intervalNum);
    };
    void ModCountDistributions(uint32_t intervalIndex, const std::string &key);
    bool Marshal(cJSON *node) const override;
    int32_t count{ 0 };
    std::vector<std::vector<std::pair<std::string, uint32_t>>> countDistributions;
};

struct SuccessRateStatistics : public Serializable {
    SuccessRateStatistics(uint32_t succeedIntervalNum, uint32_t failedIntervalNum)
        : succeedInfo(CountDistributionInfo(succeedIntervalNum)), failedInfo(CountDistributionInfo(failedIntervalNum))
    {
    }
    bool Marshal(cJSON *node) const override
    {
        auto ret = SetValue(node, GET_NAME(SUCCEED), succeedInfo);
        return SetValue(node, GET_NAME(FAILED), failedInfo) && ret;
    }
    CountDistributionInfo succeedInfo;
    CountDistributionInfo failedInfo;
};

} // namespace MiscServices
} // namespace OHOS

#endif // IMF_HISYSEVENT_INFO_H