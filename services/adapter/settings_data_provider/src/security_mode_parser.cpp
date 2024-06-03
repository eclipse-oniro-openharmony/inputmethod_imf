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

#include "ime_info_inquirer.h"
#include "input_method_utils.h"
#include "iservice_registry.h"
#include "serializable.h"
#include "settings_data_utils.h"
#include "system_ability_definition.h"
namespace OHOS {
namespace MiscServices {
std::mutex SecurityModeParser::instanceMutex_;
sptr<SecurityModeParser> SecurityModeParser::instance_ = nullptr;

SecurityModeParser::~SecurityModeParser()
{
}

sptr<SecurityModeParser> SecurityModeParser::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceMutex_);
        if (instance_ == nullptr) {
            IMSA_HILOGI("GetInstance need new SecurityModeParser");
            instance_ = new (std::nothrow) SecurityModeParser();
            if (instance_ == nullptr) {
                IMSA_HILOGI("instance is nullptr.");
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
        IMSA_HILOGW("Get value failed, or valueStr is empty");
        return ErrorCode::ERROR_ENABLE_SECURITY_MODE;
    }

    if (!ParseSecurityMode(valueStr, userId)) {
        IMSA_HILOGE("Parse %{public}s failed by %{public}d", valueStr.c_str(), userId);
        return ErrorCode::ERROR_ENABLE_SECURITY_MODE;
    }
    return ErrorCode::NO_ERROR;
}

bool SecurityModeParser::IsSecurityChange(const std::string bundleName, const int32_t userId)
{
    bool oldExit = IsFullMode(bundleName);
    UpdateFullModeList(userId);
    bool onewExit = IsFullMode(bundleName);
    return oldExit!= onewExit;
}

bool SecurityModeParser::ParseSecurityMode(const std::string &valueStr, const int32_t userId)
{
    SecModeCfg secModeCfg;
    secModeCfg.userImeCfg.userId = std::to_string(userId);
    auto ret = secModeCfg.Unmarshall(valueStr);
    if (!ret) {
        return ret;
    }
    std::lock_guard<std::mutex> autoLock(listMutex_);
    fullModeList_ = secModeCfg.userImeCfg.identities;
    return true;
}

int32_t SecurityModeParser::GetSecurityMode(const std::string &bundleName, SecurityMode &security, int32_t userId)
{
    if (IsFullMode(bundleName)) {
        security = SecurityMode::FULL;
    } else {
        security = SecurityMode::BASIC;
    }
    return ErrorCode::NO_ERROR;
}

bool SecurityModeParser::IsFullMode(std::string bundleName)
{
    std::lock_guard<std::mutex> autoLock(listMutex_);
    auto it = std::find_if(fullModeList_.begin(), fullModeList_.end(), [&bundleName](const std::string& bundle) {
        return bundle == bundleName;
    });
    return it != fullModeList_.end();
}
} // namespace MiscServices
} // namespace OHOS