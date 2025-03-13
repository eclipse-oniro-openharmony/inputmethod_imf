/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ime_info_inquirer.h"

#include "global.h"
namespace OHOS {
namespace MiscServices {
std::shared_ptr<ImeInfo> ImeInfoInquirer::defaultIme_ = nullptr;
std::shared_ptr<Property> ImeInfoInquirer::defaultImeProperty_ = nullptr;
std::shared_ptr<Property> ImeInfoInquirer::currentIme_ = nullptr;
constexpr const char *MOCK_APP_ID = "MockAppId";
ImeInfoInquirer &ImeInfoInquirer::GetInstance()
{
    static ImeInfoInquirer instance;
    return instance;
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetDefaultImeInfo(int32_t userId)
{
    return defaultIme_;
}

std::shared_ptr<Property> ImeInfoInquirer::GetCurrentInputMethod(int32_t userId)
{
    return currentIme_;
}

std::shared_ptr<Property> ImeInfoInquirer::GetDefaultImeCfgProp()
{
    return defaultImeProperty_;
}

bool ImeInfoInquirer::GetImeAppId(int32_t userId, const std::string &bundleName, std::string &appId)
{
    appId = MOCK_APP_ID;
    return true;
}

bool ImeInfoInquirer::GetImeVersionCode(int32_t userId, const std::string &bundleName, uint32_t &versionCode)
{
    versionCode = 0;
    return true;
}

int32_t ImeInfoInquirer::QueryFullImeInfo(std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> &fullImeInfos) const
{
    if (!isQueryAllFullImeInfosOk_) {
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    fullImeInfos = allFullImeInfos_;
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::QueryFullImeInfo(int32_t userId, std::vector<FullImeInfo> &imeInfos) const
{
    if (!isQueryFullImeInfosOk_) {
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    imeInfos = fullImeInfos_;
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::GetFullImeInfo(int32_t userId, const std::string &bundleName, FullImeInfo &imeInfo) const
{
    if (!isGetFullImeInfoOk_) {
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    imeInfo = fullImeInfo_;
    return ErrorCode::NO_ERROR;
}

void ImeInfoInquirer::SetFullImeInfo(bool isReturnOk, const FullImeInfo &imeInfo)
{
    isGetFullImeInfoOk_ = isReturnOk;
    fullImeInfo_ = imeInfo;
}

void ImeInfoInquirer::SetFullImeInfo(bool isReturnOk, const std::vector<FullImeInfo> &imeInfos)
{
    isQueryFullImeInfosOk_ = isReturnOk;
    fullImeInfos_ = imeInfos;
}

void ImeInfoInquirer::SetFullImeInfo(
    bool isReturnOk, const std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> &fullImeInfos)
{
    isQueryAllFullImeInfosOk_ = isReturnOk;
    allFullImeInfos_ = fullImeInfos;
}

ImeNativeCfg ImeInfoInquirer::GetDefaultIme()
{
    ImeNativeCfg imeCfg;
    return imeCfg;
}

EnabledStatus ImeInfoInquirer::GetSystemInitEnabledState()
{
    return EnabledStatus::DISABLED;
}
} // namespace MiscServices
} // namespace OHOS