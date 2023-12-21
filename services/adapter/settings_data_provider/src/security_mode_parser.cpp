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
#include "nlohmann/json.hpp"
#include "settings_data_utils.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
using json = nlohmann::json;
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
    return GetFullModeList(userId);
}

int32_t SecurityModeParser::GetFullModeList(const int32_t userId)
{
    IMSA_HILOGD("key: %{public}s.", SECURITY_MODE);
    std::string valueStr;
    int32_t ret = SettingsDataUtils::GetInstance()->GetStringValue(SECURITY_MODE, valueStr);
    if (ret != ErrorCode::NO_ERROR || valueStr.empty()) {
        IMSA_HILOGW("Get value failed, or valueStr is empty");
        return ErrorCode::ERROR_ENABLE_SECURITY_MODE;
    }

    if (!ParseJsonData(valueStr, userId)) {
        IMSA_HILOGE("valueStr is empty");
        return ErrorCode::ERROR_ENABLE_SECURITY_MODE;
    }
    return ErrorCode::NO_ERROR;
}

bool SecurityModeParser::IsSecurityChange(const std::string bundleName, const int32_t userId)
{
    bool oldExit = IsFullMode(bundleName);
    GetFullModeList(userId);
    bool onewExit = IsFullMode(bundleName);
    return oldExit!= onewExit;
}

bool SecurityModeParser::ParseJsonData(const std::string& valueStr, const int32_t userId)
{
    IMSA_HILOGD("valueStr: %{public}s.", valueStr.c_str());
    json jsonData = json::parse(valueStr.c_str());
    if (jsonData.is_null() || jsonData.is_discarded()) {
        IMSA_HILOGE("json parse failed.");
        return false;
    }
    if (!jsonData.contains(SECURITY_KEY) || !jsonData[SECURITY_KEY].is_object()) {
        IMSA_HILOGE("listName not find or abnormal");
        return false;
    }
    std::string id = std::to_string(userId);
    if (!jsonData[SECURITY_KEY].contains(id) || !jsonData[SECURITY_KEY][id].is_array()) {
        IMSA_HILOGE("user id not find or abnormal");
        return false;
    }
    std::vector<std::string> vecTemp;
    for (const auto& bundleName : jsonData[SECURITY_KEY][id]) {
        IMSA_HILOGD(" full mode app is : %{public}s.", std::string(bundleName).c_str());
        vecTemp.push_back(bundleName);
    }
    std::lock_guard<std::mutex> autoLock(listMutex_);
    fullModeList_.assign(vecTemp.begin(), vecTemp.end());
    return true;
}

int32_t SecurityModeParser::GetSecurityMode(const std::string bundleName, int32_t &security, const int32_t userId)
{
    GetFullModeList(userId);
    if (IsFullMode(bundleName)) {
        security = static_cast<int32_t>(SecurityMode::FULL);
    } else {
        security = static_cast<int32_t>(SecurityMode::BASIC);
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