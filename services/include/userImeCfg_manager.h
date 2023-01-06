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

#include <string>
#include <unordered_set>
#include <mutex>
#include "nlohmann/json.hpp"
#include <map>
namespace OHOS {
namespace MiscServices {
class UserImeCfgManager {
public:
    static sptr<UserImeCfgManager> GetInstance();
    void Init();
    void AddCurrentIme(int32_t userId, const std::string &currentImeCfg);
    void ModifyCurrentIme(int32_t userId, const std::string &currentImeCfg);
    void DeleteCurrentIme(int32_t userId);
    std::string GetCurrentIme(int32_t userId);

private:
    bool LoadUserImeCfg(const std::string &filePath, nlohmann::json &userImeCfgJson);
    static std::mutex instanceLock_;
    static sptr<UserImeCfgManager> instance_;
    std::map<int32_t, std::string> userCurrentIme_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_USERIMECFG_MANAGER_H