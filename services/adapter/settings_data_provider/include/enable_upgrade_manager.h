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
struct ImePersistInfo : public Serializable {
    ImePersistInfo() = default;
    ImePersistInfo(int32_t userId, std::string currentIme, std::string currentSubName, bool isDefaultImeSet)
        : userId(userId), currentIme(std::move(currentIme)), currentSubName(std::move(currentSubName)),
          isDefaultImeSet(isDefaultImeSet){};
    static constexpr int32_t INVALID_USERID = -1;
    int32_t userId{ INVALID_USERID };
    std::string currentIme;
    std::string currentSubName;
    std::string tempScreenLockIme;
    bool isDefaultImeSet{ false };

    bool Marshal(cJSON *node) const override
    {
        auto ret = SetValue(node, GET_NAME(userId), userId);
        ret = SetValue(node, GET_NAME(currentIme), currentIme) && ret;
        ret = SetValue(node, GET_NAME(currentSubName), currentSubName) && ret;
        SetValue(node, GET_NAME(tempScreenLockIme), tempScreenLockIme);
        ret = SetValue(node, GET_NAME(isDefaultImeSet), isDefaultImeSet) && ret;
        return ret;
    }
    bool Unmarshal(cJSON *node) override
    {
        auto ret = GetValue(node, GET_NAME(userId), userId);
        ret = GetValue(node, GET_NAME(currentIme), currentIme) && ret;
        ret = GetValue(node, GET_NAME(currentSubName), currentSubName) && ret;
        GetValue(node, GET_NAME(tempScreenLockIme), tempScreenLockIme);
        ret = GetValue(node, GET_NAME(isDefaultImeSet), isDefaultImeSet) && ret;
        return ret;
    }
};

struct ImePersistCfg : public Serializable {
    std::vector<ImePersistInfo> imePersistInfo;
    bool Marshal(cJSON *node) const override
    {
        return SetValue(node, GET_NAME(imeCfgList), imePersistInfo);
    }
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, GET_NAME(imeCfgList), imePersistInfo);
    }
};

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
    int32_t Upgrade(int32_t userId, const std::vector<FullImeInfo> &imeInfos);
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
    std::pair<int32_t, std::string> GenerateNewUserEnabledTable(int32_t userId,
        const std::set<std::string> &enabledBundleNames, const std::set<std::string> &fullModeBundleNames,
        const ImePersistInfo &persistInfo, const std::vector<FullImeInfo> &imeInfos);
    int32_t GetGlobalTableUserId(const std::string &valueStr);
    std::string GenerateGlobalContent(int32_t userId, const std::vector<std::string> &bundleNames);
    bool SetGlobalEnabledTable(const std::string &content);
    bool SetUserEnabledTable(int32_t userId, const std::string &content);
    bool SetEnabledTable(const std::string &uriProxy, const std::string &content);
    void PaddedByBundleMgr(
        int32_t userId, const std::vector<FullImeInfo> &imeInfos, std::vector<ImeEnabledInfo> &enabledInfos);
    void PaddedByImePersistCfg(const ImePersistInfo &persistInfo, std::vector<ImeEnabledInfo> &enabledInfos);
    int32_t GetImePersistCfg(int32_t userId, ImePersistInfo &persisInfo);
    std::mutex upgradedLock_;
    std::set<int32_t> upgradedUserId_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // ENABLE_UPGRADE_MANAGER_H