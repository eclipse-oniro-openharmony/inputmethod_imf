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

#include "security_mode_parser.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "full_ime_info_manager.h"
#include "ime_info_inquirer.h"
#include "input_method_utils.h"
#include "iservice_registry.h"
#include "serializable.h"
#include "settings_data_utils.h"
#include "sys_cfg_parser.h"
#include "system_ability_definition.h"
namespace OHOS {
namespace MiscServices {
std::mutex SecurityModeParser::instanceMutex_;
sptr<SecurityModeParser> SecurityModeParser::instance_ = nullptr;
constexpr const char *SYSTEM_SPECIAL_IME = "";
SecurityModeParser::~SecurityModeParser()
{
}

sptr<SecurityModeParser> SecurityModeParser::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceMutex_);
        if (instance_ == nullptr) {
            IMSA_HILOGI("need to create SecurityModeParser.");
            instance_ = new (std::nothrow) SecurityModeParser();
            if (instance_ == nullptr) {
                IMSA_HILOGE("instance is nullptr.");
                return instance_;
            }
        }
    }
    return instance_;
}

int32_t SecurityModeParser::Initialize(const int32_t userId)
{
    return UpdateFullModeList(userId);
}

int32_t SecurityModeParser::UpdateFullModeList(int32_t userId)
{
    IMSA_HILOGD("key: %{public}s.", SECURITY_MODE);
    std::string valueStr;
    int32_t ret = SettingsDataUtils::GetInstance()->GetStringValue(SECURITY_MODE, valueStr);
    if (ret != ErrorCode::NO_ERROR || valueStr.empty()) {
        IMSA_HILOGW("get value failed, or valueStr is empty");
        return ErrorCode::ERROR_ENABLE_SECURITY_MODE;
    }

    if (!ParseSecurityMode(valueStr, userId)) {
        IMSA_HILOGE("parse %{public}s failed by %{public}d", valueStr.c_str(), userId);
        return ErrorCode::ERROR_ENABLE_SECURITY_MODE;
    }
    return ErrorCode::NO_ERROR;
}

bool SecurityModeParser::ParseSecurityMode(const std::string &valueStr, const int32_t userId)
{
    SecModeCfg secModeCfg;
    secModeCfg.userImeCfg.userId = std::to_string(userId);
    auto ret = secModeCfg.Unmarshall(valueStr);
    if (!ret) {
        IMSA_HILOGE("unmarshall failed!");
        return ret;
    }
    std::lock_guard<std::mutex> autoLock(listMutex_);
    fullModeList_ = secModeCfg.userImeCfg.identities;
    return true;
}

SecurityMode SecurityModeParser::GetSecurityMode(const std::string &bundleName, int32_t userId)
{
    if (bundleName == SYSTEM_SPECIAL_IME) {
        return SecurityMode::FULL;
    }
    if (!initialized_) {
        std::lock_guard<std::mutex> lock(initLock_);
        if (!initialized_) {
            UpdateFullModeList(userId);
            initialized_ = true;
        }
    }
    if (IsFullMode(bundleName)) {
        return SecurityMode::FULL;
    } else {
        return SecurityMode::BASIC;
    }
}

bool SecurityModeParser::IsFullMode(std::string bundleName)
{
    std::lock_guard<std::mutex> autoLock(listMutex_);
    auto it = std::find_if(fullModeList_.begin(), fullModeList_.end(), [&bundleName](const std::string& bundle) {
        return bundle == bundleName;
    });
    return it != fullModeList_.end();
}

bool SecurityModeParser::IsDefaultFullMode(const std::string &bundleName, int32_t userId)
{
    auto defaultIme = ImeInfoInquirer::GetInstance().GetDefaultImeCfgProp();
    if (defaultIme != nullptr && bundleName == defaultIme->name) {
        return true;
    }
    std::string appId;
    uint32_t versionCode;
    if (!ImeInfoInquirer::GetInstance().GetImeAppId(userId, bundleName, appId)
        || !ImeInfoInquirer::GetInstance().GetImeVersionCode(userId, bundleName, versionCode)) {
        IMSA_HILOGE("%{public}s failed to get appId and versionCode", bundleName.c_str());
        return false;
    }
    std::vector<DefaultFullImeInfo> defaultFullImeList;
    if (!SysCfgParser::ParseDefaultFullIme(defaultFullImeList)) {
        IMSA_HILOGE("failed to parse config");
        return false;
    }
    auto ime = std::find_if(defaultFullImeList.begin(), defaultFullImeList.end(),
        [&appId](const auto &ime) { return ime.appId == appId; });
    if (ime == defaultFullImeList.end()) {
        IMSA_HILOGD("not default FULL");
        return false;
    }
    bool isDefaultFull = false;
    if (ime->expirationVersionCode > 0) {
        isDefaultFull = !IsExpired(ime->expirationTime) || versionCode < ime->expirationVersionCode;
    } else {
        isDefaultFull = !IsExpired(ime->expirationTime);
    }
    IMSA_HILOGI("ime: %{public}s, isDefaultFull: %{public}d", bundleName.c_str(), isDefaultFull);
    return isDefaultFull;
}

bool SecurityModeParser::IsExpired(const std::string &expirationTime)
{
    std::istringstream expirationTimeStr(expirationTime);
    std::tm expTime = {};
    expirationTimeStr >> std::get_time(&expTime, "%Y-%m-%d %H:%M:%S");
    if (expirationTimeStr.fail()) {
        IMSA_HILOGE("get time error, expirationTime: %{public}s", expirationTime.c_str());
        return false;
    }
    auto expTimePoint = std::chrono::system_clock::from_time_t(std::mktime(&expTime));
    auto now = std::chrono::system_clock::now();
    return expTimePoint < now;
}
} // namespace MiscServices
} // namespace OHOS