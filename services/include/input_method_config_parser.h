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

#ifndef SERVICES_INCLUDE_IME_CONFIG_PARSE_H
#define SERVICES_INCLUDE_IME_CONFIG_PARSE_H

#include <sys/types.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "global.h"
#include "config_policy_utils.h"

#include "nlohmann/json.hpp"

namespace OHOS {
namespace MiscServices {
class ImeConfigParse {
public:
    static std::string ReadFile(const std::string &path);
    static CfgFiles* ParseFromCustom();
    static bool ParseJson(const std::string &cfgPath, const std::string &parseKey, nlohmann::json &jsonCfg);
    template<typename T>
    static bool ParseFromCustomSystem(const std::string &parseKey, T &data);
    template<typename T>
    static bool ParseFromCustomSystem(const std::string &parseKey, std::vector<T> &data);
    template<typename T>
    static bool GetCfgsFromFile(const std::string &cfgPath, const std::string &parseKey, T &data);
    template<typename T>
    static bool GetCfgsFromFile(const std::string &cfgPath, const std::string &parseKey, std::vector<T> &data);
};

template<typename T>
bool ImeConfigParse::ParseFromCustomSystem(const std::string &parseKey, T &data)
{
    CfgFiles* cfgFiles = ParseFromCustom();
    if (cfgFiles == nullptr) {
        IMSA_HILOGE("cfgFiles is nullptr");
        return false;
    }
    bool isSuccess = true;
    // parse config files, ordered by priority from high to low
    for (int32_t i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        auto path = cfgFiles->paths[i];
        if (path == nullptr || *path == '\0') {
            continue;
        }
        isSuccess = false;
        char realPath[PATH_MAX + 1] = { 0x00 };
        if (strlen(path) == 0 || strlen(path) > PATH_MAX || realpath(path, realPath) == nullptr) {
            IMSA_HILOGE("failed to get realpath");
            break;
        }
        std::string cfgPath(realPath);
        if (GetCfgsFromFile(cfgPath, parseKey, data)) {
            isSuccess = true;
            break;
        }
    }
    FreeCfgFiles(cfgFiles);
    IMSA_HILOGI("parse result: %{public}d", isSuccess);
    return isSuccess;
}

template<typename T>
bool ImeConfigParse::ParseFromCustomSystem(const std::string &parseKey, std::vector<T> &data)
{
    CfgFiles* cfgFiles = ParseFromCustom();
    if (cfgFiles == nullptr) {
        IMSA_HILOGE("cfgFiles is nullptr");
        return false;
    }
    bool isSuccess = true;
    // parse config files, ordered by priority from high to low
    for (int32_t i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        auto path = cfgFiles->paths[i];
        if (path == nullptr || *path == '\0') {
            continue;
        }
        isSuccess = false;
        char realPath[PATH_MAX + 1] = { 0x00 };
        if (strlen(path) == 0 || strlen(path) > PATH_MAX || realpath(path, realPath) == nullptr) {
            IMSA_HILOGE("failed to get realpath");
            break;
        }
        std::string cfgPath(realPath);
        if (!GetCfgsFromFile(cfgPath, parseKey, data)) {
            break;
        }
        isSuccess = true;
    }
    FreeCfgFiles(cfgFiles);
    IMSA_HILOGI("parse result: %{public}d", isSuccess);
    return isSuccess;
}

template<typename T>
bool ImeConfigParse::GetCfgsFromFile(const std::string &cfgPath, const std::string &parseKey, T &data)
{
    nlohmann::json jsonCfg;
    if (ParseJson(cfgPath, parseKey, jsonCfg)) {
        data = jsonCfg.at(parseKey).get<T>();
        return true;
    }
    return false;
}

template<typename T>
bool ImeConfigParse::GetCfgsFromFile(const std::string &cfgPath, const std::string &parseKey, std::vector<T> &data)
{
    nlohmann::json jsonCfg;
    if (ParseJson(cfgPath, parseKey, jsonCfg)) {
        if (!jsonCfg.at(parseKey).is_array()) {
            IMSA_HILOGE("%{public}s is not array", parseKey.c_str());
            return false;
        }
        data = jsonCfg.at(parseKey).get<std::vector<T>>();
        return true;
    }
    return false;
}
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_IME_CONFIG_PARSE_H
