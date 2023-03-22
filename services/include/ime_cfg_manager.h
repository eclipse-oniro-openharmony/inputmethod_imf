/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef SERVICES_INCLUDE_IME_CFG_MANAGER_H
#define SERVICES_INCLUDE_IME_CFG_MANAGER_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <sys/types.h>

#include "nlohmann/json.hpp"
namespace OHOS {
namespace MiscServices {
struct ImeCfg {
    int32_t userId;
    std::string currentIme;
    std::string currentSubName;
};
class ImeCfgManager {
public:
    static ImeCfgManager &GetInstance();
    void Init();
    void AddImeCfg(const ImeCfg &cfg);
    void ModifyImeCfg(const ImeCfg &cfg);
    void DeleteImeCfg(int32_t userId);
    std::string GetCurrentIme(int32_t userId);
    std::string GetCurrentImeBundleName(int32_t userId);
    std::string GetCurrentImeExtName(int32_t userId);
    std::string GetCurrentImeSubName(int32_t userId);

private:
    ImeCfgManager()= default;
    ~ImeCfgManager() = default;
    void ReadImeCfgFile();
    void WriteImeCfgFile();
    ImeCfg GetImeCfg(int32_t userId);
    static int32_t CreateCachePath(std::string &path, mode_t pathMode);
    static bool IsCachePathExit(std::string &path);
    static bool ReadCacheFile(const std::string &path, nlohmann::json &jsonCfg);
    static bool WriteCacheFile(const std::string &path, const nlohmann::json &jsonCfg);
    inline static void FromJson(const nlohmann::json &jsonCfg, ImeCfg &cfg)
    {
        if (jsonCfg.find("userId") != jsonCfg.end() && jsonCfg["userId"].is_number()) {
            jsonCfg.at("userId").get_to(cfg.userId);
        }
        if (jsonCfg.find("currentIme") != jsonCfg.end() && jsonCfg["currentIme"].is_string()) {
            jsonCfg.at("currentIme").get_to(cfg.currentIme);
        }
        if (jsonCfg.find("currentSubName") != jsonCfg.end() && jsonCfg["currentSubName"].is_string()) {
            jsonCfg.at("currentSubName").get_to(cfg.currentSubName);
        }
    }
    inline static void ToJson(nlohmann::json &jsonCfg, const ImeCfg &cfg)
    {
        jsonCfg = nlohmann::json{ { "userId", cfg.userId }, { "currentIme", cfg.currentIme },
            { "currentSubName", cfg.currentSubName } };
    }
    static void FromJson(const nlohmann::json &jsonConfigs, std::vector<ImeCfg> &configs);
    static void ToJson(nlohmann::json &jsonConfigs, const std::vector<ImeCfg> &configs);
    std::recursive_mutex imeCfgLock_;
    std::vector<ImeCfg> imeConfigs_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_IME_CFG_MANAGER_H
