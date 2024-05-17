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

#ifndef SECURITY_MODE_PARSER_H
#define SECURITY_MODE_PARSER_H

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "datashare_helper.h"
#include "global.h"
#include "serializable.h"
#include "settings_data_observer.h"
#include "settings_data_utils.h"

namespace OHOS {
namespace MiscServices {
struct SecModeCfg : public Serializable {
    UserImeConfig userImeCfg;
    bool Unmarshal(cJSON *node) override
    {
        GetValue(node, GET_NAME(fullExperienceList), userImeCfg);
        return true;
    }
};
class SecurityModeParser : public RefBase {
public:
    static sptr<SecurityModeParser> GetInstance();
    int32_t Initialize(const int32_t userId);
    int32_t GetSecurityMode(const std::string bundleName, int32_t &security, const int32_t userId);
    bool IsSecurityChange(const std::string bundleName, const int32_t userId);
    int32_t GetFullModeList(const int32_t userId);
    static constexpr const char *SECURITY_MODE = "settings.inputmethod.full_experience";

private:
    SecurityModeParser() = default;
    ~SecurityModeParser();

    bool ParseSecurityMode(const std::string &valueStr, const int32_t userId);
    bool IsFullMode(const std::string bundleName);
    static std::mutex instanceMutex_;
    static sptr<SecurityModeParser> instance_;
    std::mutex listMutex_;
    std::vector<std::string> fullModeList_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // SECURITY_MODE_PARSER_H