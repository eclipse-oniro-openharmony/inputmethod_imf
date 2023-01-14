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

#ifndef SERVICES_INCLUDE_USERIMECFG_MANAGER_H
#define SERVICES_INCLUDE_USERIMECFG_MANAGER_H

#include <mutex>
#include <string>
#include <vector>
#include <memory>
#include "third_party/json/include/nlohmann/json.hpp"
struct ImeCfg {
    int32_t userId;
    std::string currentIme;
};

namespace OHOS {
namespace MiscServices {
class ImeCfgManager {
public:
    static ImeCfgManager &GetInstance();
    void Init();
    void AddImeCfg(const ImeCfg &cfg);
    void ModifyImeCfg(const ImeCfg &cfg);
    void DeleteImeCfg(int32_t userId);
    ImeCfg GetImeCfg(int32_t userId);

private:
    bool CreateCfgFile() ;
    void ReadCfgFile();
    void WriteCfgFile();
    inline void from_json(const nlohmann::json &jsonCfg, ImeCfg &cfg)
    {
        jsonCfg.at("userId").get_to(cfg.userId);
        jsonCfg.at("currentIme").get_to(cfg.currentIme);
    }
    inline void to_json(nlohmann::json &jsonCfg, const ImeCfg &cfg)
    {
        jsonCfg = nlohmann::json{ { "userId", cfg.userId }, { "currentIme", cfg.currentIme } };
    }
    void from_json(const nlohmann::json &jsonConfigs, std::vector<ImeCfg> &configs);
    void to_json(nlohmann::json &jsonConfigs, const std::vector<ImeCfg> &configs);
    std::recursive_mutex imeCfgLock_;
    std::vector<ImeCfg> imeConfigs_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_USERIMECFG_MANAGER_H