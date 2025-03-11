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

#ifndef IME_ENABLED_INFO_MANAGER_H
#define IME_ENABLED_INFO_MANAGER_H

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "event_handler.h"
#include "input_method_property.h"
#include "input_method_status.h"
#include "serializable.h"
#include "settings_data_utils.h"

namespace OHOS {
namespace MiscServices {
// old ime enabled cfg
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
// old ime full experience cfg
struct SecurityModeCfg : public Serializable {
    UserImeConfig userImeCfg;
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, GET_NAME(fullExperienceList), userImeCfg);
    }
};

struct ImeEnabledInfo : public Serializable {
    std::string bundleName;
    std::string extensionName;
    EnabledStatus enabledStatus{ EnabledStatus::DISABLED };
    std::string stateUpdateTime; // user trigger
    std::string installTime;
    bool Unmarshal(cJSON *node) override
    {
        auto ret = GetValue(node, GET_NAME(bundleName), bundleName);
        ret = GetValue(node, GET_NAME(extensionName), extensionName) && ret;
        int32_t enabledStatusTmp = 0;
        ret = GetValue(node, GET_NAME(enabledStatus), enabledStatusTmp) && ret;
        enabledStatus = static_cast<EnabledStatus>(enabledStatusTmp);
        ret = GetValue(node, GET_NAME(stateUpdateTime), stateUpdateTime) && ret;
        return GetValue(node, GET_NAME(installTime), installTime) && ret;
    }
    bool Marshal(cJSON *node) const override
    {
        auto ret = SetValue(node, GET_NAME(bundleName), bundleName);
        ret = SetValue(node, GET_NAME(extensionName), extensionName) && ret;
        auto enabledStatusTmp = static_cast<int32_t>(enabledStatus);
        ret = SetValue(node, GET_NAME(enabledStatus), enabledStatusTmp) && ret;
        ret = SetValue(node, GET_NAME(stateUpdateTime), stateUpdateTime) && ret;
        return SetValue(node, GET_NAME(installTime), installTime) && ret;
    }
    bool operator==(const ImeEnabledInfo &enabledInfo) const
    {
        return bundleName == enabledInfo.bundleName && extensionName == enabledInfo.extensionName
               && enabledStatus == enabledInfo.enabledStatus && installTime == enabledInfo.installTime;
    }
};
struct ImeEnabledCfg : public Serializable {
    std::string version;
    std::vector<ImeEnabledInfo> enabledInfos;
    bool Unmarshal(cJSON *node) override
    {
        auto ret = GetValue(node, GET_NAME(version), version);
        return GetValue(node, GET_NAME(inputmethods), enabledInfos) && ret;
    }
    bool Marshal(cJSON *node) const override
    {
        auto ret = SetValue(node, GET_NAME(version), version);
        return SetValue(node, GET_NAME(inputmethods), enabledInfos) && ret;
    }
    bool operator==(const ImeEnabledCfg &enabledCfg) const
    {
        return version == enabledCfg.version && enabledInfos == enabledCfg.enabledInfos;
    }
};
using EnabledStatusChangedHandler =
    std::function<void(int32_t userId, const std::string &bundleName, EnabledStatus oldStatus)>;
class ImeEnabledInfoManager {
public:
    static constexpr const char *ENABLE_IME = "settings.inputmethod.enable_ime";
    static constexpr const char *SECURITY_MODE = "settings.inputmethod.full_experience";
    static ImeEnabledInfoManager &GetInstance();
    void SetEnabledStatusChangedHandler(EnabledStatusChangedHandler handler);
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler);
    int32_t Init(const std::map<int32_t, std::vector<FullImeInfo>> &fullImeInfos);
    int32_t Add(int32_t userId, const std::vector<FullImeInfo> &imeInfos);
    int32_t Delete(int32_t userId);
    int32_t Add(int32_t userId, const FullImeInfo &imeInfo);
    int32_t Delete(int32_t userId, const std::string &bundleName);
    int32_t Update(
        int32_t userId, const std::string &bundleName, const std::string &extensionName, EnabledStatus status);
    int32_t GetEnabledState(int32_t userId, const std::string &bundleName, EnabledStatus &status);
    int32_t GetEnabledStates(int32_t userId, std::vector<Property> &props); // props not has sysSpecialIme
    bool IsDefaultFullMode(int32_t userId, const std::string &bundleName);
    void OnFullExperienceTableChanged(int32_t userId); // add for compatibility

private:
    ImeEnabledInfoManager() = default;
    ~ImeEnabledInfoManager();
    int32_t AddUser(int32_t userId, const std::vector<FullImeInfo> &imeInfos = {});
    int32_t CorrectUserAdd(int32_t userId, const std::vector<FullImeInfo> &imeInfos = {});
    int32_t GetEnabledStateInner(int32_t userId, const std::string &bundleName, EnabledStatus &status);
    int32_t GetEnabledStateInner(int32_t userId, std::vector<Property> &props);
    int32_t GetEnabledCfg(
        int32_t userId, ImeEnabledCfg &cfg, bool isCheckByBmg, const std::vector<FullImeInfo> &imeInfos = {});
    int32_t GetEnabledTableCfg(int32_t userId, ImeEnabledCfg &cfg);
    int32_t GetOldEnabledTableCfg(int32_t userId, const std::string &content, ImeEnabledCfg &cfg);
    int32_t GetNewEnabledTableCfg(int32_t userId, const std::string &content, ImeEnabledCfg &cfg);
    int32_t MergeFullExperienceTableCfg(int32_t userId, ImeEnabledCfg &cfg);
    int32_t ParseFullExperienceTableCfg(int32_t userId, const std::string &content, std::set<std::string> &bundleNames);
    int32_t CheckByBundleMgr(
        int32_t userId, std::vector<ImeEnabledInfo> &enabledInfos, const std::vector<FullImeInfo> &imeInfos);
    void CheckBySysEnabledSwitch(ImeEnabledInfo &info);
    void CheckBySysEnabledSwitch(std::vector<ImeEnabledInfo> &infos);
    void CheckBySysIme(ImeEnabledInfo &info);
    void CheckBySysIme(std::vector<ImeEnabledInfo> &infos);
    int32_t SetEnabledCfg(int32_t userId, const ImeEnabledCfg &cfg);
    int32_t GetEnabledCfgFromCache(int32_t userId, ImeEnabledCfg &enabledCfg);
    int32_t GetEnabledCfgFromCacheWithCorrect(int32_t userId, ImeEnabledCfg &enabledCfg);
    void PostCorrectAddTask(int32_t userId);
    void NotifyEnableChange(int32_t userId, const std::string &bundleName, EnabledStatus oldStatus);
    int32_t NeedUpdate(
        int32_t userId, const std::string &bundleName, const std::string &extensionName, EnabledStatus status);
    bool HasEnabledSwitch();
    bool IsExpired(const std::string &expirationTime);
    void ModGlobalEnabledTable(int32_t userId, const ImeEnabledCfg &newEnabledCfg); // add for compatibility
    std::string GetGlobalTableUserId(const std::string &valueStr);                  // add for compatibility
    std::mutex imeEnabledCfgLock_;
    std::map<int32_t, ImeEnabledCfg> imeEnabledCfg_;
    EnabledStatusChangedHandler handler_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_{ nullptr };
    std::mutex settingOperateLock_;
    int32_t currentUserId_{ -1 };
};
} // namespace MiscServices
} // namespace OHOS

#endif // IME_ENABLED_INFO_MANAGER_H