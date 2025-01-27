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

#include "imf_hisysevent_info.h"

#include <algorithm>

namespace OHOS {
namespace MiscServices {
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
    info_->baseTextOperationTime = -1;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetEventCode(int32_t eventCode)
{
    info_->eventCode = eventCode;
    return *this;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetErrCode(int32_t errCode)
{
    info_->errCode = errCode;
    return *this;
}
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetPeerName(const std::string &peerName)
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
HiSysOriginalInfo::Builder &HiSysOriginalInfo::Builder::SetBaseTextOperatorTime(int32_t baseTextOperationTime)
{
    info_->baseTextOperationTime = baseTextOperationTime;
    return *this;
}
std::shared_ptr<HiSysOriginalInfo> HiSysOriginalInfo::Builder::Build()
{
    return info_;
}

void CountDistributionInfo::ModCountDistributions(uint32_t intervalIndex, const std::string &key)
{
    count++;
    if (intervalIndex >= countDistributions.size()) {
        return;
    }
    auto &intervalInfos = countDistributions[intervalIndex];
    auto it = std::find_if(intervalInfos.begin(), intervalInfos.end(),
        [key](const std::pair<std::string, uint64_t> &infoTmp) { return infoTmp.first == key; });
    if (it == intervalInfos.end()) {
        intervalInfos.emplace_back(key, 1);
        return;
    }
    it->second++;
}

bool CountDistributionInfo::Marshal(cJSON *node) const
{
    auto ret = SetValue(node, GET_NAME(COUNT), count);
    std::vector<std::vector<std::string>> distributions;
    for (const auto &distribution : countDistributions) {
        std::vector<std::string> infos;
        for (const auto &info : distribution) {
            std::string str(info.first);
            str.append("/").append(std::to_string(info.second));
            infos.push_back(str);
        }
        distributions.push_back(infos);
    }
    return SetValue(node, GET_NAME(COUNT_DISTRIBUTION), distributions) && ret;
}
} // namespace MiscServices
} // namespace OHOS