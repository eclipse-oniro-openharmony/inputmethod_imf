/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef ENABLE_UPGRADE_MANAGER_H
#define ENABLE_UPGRADE_MANAGER_H
#include <set>

#include "event_handler.h"
#include "ime_enabled_info_manager.h"
#include "input_method_property.h"
#include "input_method_status.h"
#include "serializable.h"
#include "settings_data_utils.h"

namespace OHOS {
namespace MiscServices {
struct UserImeConfig : public Serializable {
    std::string userId;
    std::vector<std::string> identities;
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, userId, identities);
    }
    bool Marshal(cJSON *node) const override
    {
        return SetValue(node, userId, identities);
    }
};
struct EnabledImeCfg : public Serializable {
    UserImeConfig userImeCfg;
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, GET_NAME(enableImeList), userImeCfg);
    }
    bool Marshal(cJSON *node) const override
    {
        return SetValue(node, GET_NAME(enableImeList), userImeCfg);
    }
};
struct SecurityModeCfg : public Serializable {
    UserImeConfig userImeCfg;
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, GET_NAME(fullExperienceList), userImeCfg);
    }
};
class EnableUpgradeManager {
public:
    static EnableUpgradeManager &GetInstance();
    int32_t Upgrade(int32_t userId);
    int32_t GetFullExperienceTable(int32_t userId, std::set<std::string> &bundleNames);
    /* add for compatibility that sys ime listen global table change for smart menu in tablet */
    void UpdateGlobalEnabledTable(int32_t userId, const std::vector<std::string> &bundleNames);

private:
    EnableUpgradeManager() = default;
    ~EnableUpgradeManager() = default;
    std::pair<int32_t, bool> GetUpgradeFlag(int32_t userId);
    int32_t GetEnabledTable(int32_t userId, std::set<std::string> &bundleNames);
    int32_t GetUserEnabledTable(int32_t userId, std::string &content);
    int32_t GetUserEnabledTable(int32_t userId, std::set<std::string> &bundleNames);
    int32_t GetGlobalEnabledTable(int32_t userId, std::set<std::string> &bundleNames);
    int32_t GetGlobalEnabledTable(int32_t userId, std::string &content);
    int32_t GetEnabledTable(int32_t userId, const std::string &uriProxy, std::set<std::string> &bundleNames);
    int32_t GetEnabledTable(int32_t userId, const std::string &uriProxy, std::string &content);
    int32_t ParseEnabledTable(int32_t userId, std::string &content, std::set<std::string> &bundleNames);
    void MergeTwoTable(const std::set<std::string> &enabledBundleNames,
        const std::set<std::string> &fullModeBundleNames, std::string &newContent);
    int32_t GetGlobalTableUserId(const std::string &valueStr);
    std::string GenerateGlobalContent(int32_t userId, const std::vector<std::string> &bundleNames);
    bool SetGlobalEnabledTable(const std::string &content);
    bool SetUserEnabledTable(int32_t userId, const std::string &content);
    bool SetEnabledTable(const std::string &uriProxy, const std::string &content);
    std::mutex upgradedLock_;
    std::set<int32_t> upgradedUserId_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // ENABLE_UPGRADE_MANAGER_H