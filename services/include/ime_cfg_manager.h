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

#include "nlohmann/json.hpp"
namespace OHOS {
namespace MiscServices {
struct ImeCfg {
    int32_t userId;
    std::string currentIme;
};
struct FileInfo {
    std::string path;
    std::string fileName;
    mode_t pathMode;
    int32_t fileMode;
};
class ImeCfgManager {
public:
    static ImeCfgManager &GetInstance();
    void Init();
    void AddImeCfg(const ImeCfg &cfg);
    void ModifyImeCfg(const ImeCfg &cfg);
    void DeleteImeCfg(int32_t userId);
    ImeCfg GetImeCfg(int32_t userId);
    static std::string GetDefaultIme();

private:
    void ReadImeCfgFile();
    void WriteImeCfgFile();
    static int32_t CreateCacheFile(FileInfo &info);
    static bool IsCachePathExit(std::string &path);
    static bool ReadCacheFile(const std::string &path, nlohmann::json &jsonCfg);
    static bool WriteCacheFile(const std::string &path, const nlohmann::json &jsonCfg);
    inline static void FromJson(const nlohmann::json &jsonCfg, ImeCfg &cfg)
    {
        jsonCfg.at("userId").get_to(cfg.userId);
        jsonCfg.at("currentIme").get_to(cfg.currentIme);
    }
    inline static void ToJson(nlohmann::json &jsonCfg, const ImeCfg &cfg)
    {
        jsonCfg = nlohmann::json{ { "userId", cfg.userId }, { "currentIme", cfg.currentIme } };
    }
    static void FromJson(const nlohmann::json &jsonConfigs, std::vector<ImeCfg> &configs);
    static void ToJson(nlohmann::json &jsonConfigs, const std::vector<ImeCfg> &configs);
    std::recursive_mutex imeCfgLock_;
    std::vector<ImeCfg> imeConfigs_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_IME_CFG_MANAGER_H