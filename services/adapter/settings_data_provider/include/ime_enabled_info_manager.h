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

#include "event_handler.h"
#include "input_method_property.h"
#include "input_method_status.h"
#include "serializable.h"
#include "settings_data_utils.h"

namespace OHOS {
namespace MiscServices {
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
    std::string version{ "empty" };
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
using EnableChangedHandler =
    std::function<void(int32_t userId, const std::string &bundleName, EnabledStatus oldStatus)>;
class ImeEnabledInfoManager {
public:
    static ImeEnabledInfoManager &GetInstance();
    void SetEnableChangedHandler(EnableChangedHandler handler);
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler);
    int32_t RegularInit(const std::map<int32_t, std::vector<FullImeInfo>> &fullImeInfos);
    int32_t Init(const std::map<int32_t, std::vector<FullImeInfo>> &fullImeInfos);
    int32_t Switch(int32_t userId, const std::vector<FullImeInfo> &imeInfos);
    int32_t Delete(int32_t userId);
    int32_t Add(int32_t userId, const FullImeInfo &imeInfo);
    int32_t Delete(int32_t userId, const std::string &bundleName);
    int32_t Update(
        int32_t userId, const std::string &bundleName, const std::string &extensionName, EnabledStatus status);
    int32_t GetEnabledState(int32_t userId, const std::string &bundleName, EnabledStatus &status);
    int32_t GetEnabledStates(int32_t userId, std::vector<Property> &props); // props not has sysSpecialIme
    bool IsDefaultFullMode(int32_t userId, const std::string &bundleName);
    /* add for compatibility that sys ime mod full experience table in it's full experience switch changed */
    void OnFullExperienceTableChanged(int32_t userId);

private:
    ImeEnabledInfoManager() = default;
    ~ImeEnabledInfoManager();
    int32_t UpdateEnabledCfgCacheIfNoCache(int32_t userId, const std::vector<FullImeInfo> &imeInfos = {});
    int32_t UpdateEnabledCfgCache(int32_t userId, const std::vector<FullImeInfo> &imeInfos);
    int32_t UpdateEnabledCfgCache(int32_t userId, const ImeEnabledCfg &cfg);
    int32_t GetEnabledCfg(int32_t userId, ImeEnabledCfg &cfg, bool needCorrectByBundleMgr = false, const std::vector<FullImeInfo> &imeInfos = {});
    int32_t GetEnabledTableCfg(int32_t userId, ImeEnabledCfg &cfg);
    int32_t CorrectByBundleMgr(
        int32_t userId, std::vector<ImeEnabledInfo> &enabledInfos, const std::vector<FullImeInfo> &imeInfos);
    void ComputeEnabledStatus(ImeEnabledInfo &info);
    void ComputeEnabledStatus(std::vector<ImeEnabledInfo> &infos);
    int32_t GetEnabledStateInner(int32_t userId, const std::string &bundleName, EnabledStatus &status);
    int32_t GetEnabledStatesInner(int32_t userId, std::vector<Property> &props);
    int32_t GetEnabledCache(int32_t userId, ImeEnabledCfg &enabledCfg);
    int32_t GetEnabledCacheWithCorrect(int32_t userId, ImeEnabledCfg &enabledCfg);
    void PostUpdateCfgCacheTask(int32_t userId);
    std::pair<int32_t, bool> CheckUpdate(
        int32_t userId, const std::string &bundleName, const std::string &extensionName, EnabledStatus status);
    void NotifyEnableChanged(int32_t userId, const std::string &bundleName, EnabledStatus oldStatus);
    bool HasEnabledSwitch();
    bool IsExpired(const std::string &expirationTime);
    /* add for compatibility that sys ime listen global table change for smart menu in tablet */
    void UpdateGlobalEnabledTable(int32_t userId, const ImeEnabledCfg &newEnabledCfg);
    std::mutex imeEnabledCfgLock_;
    std::map<int32_t, ImeEnabledCfg> imeEnabledCfg_;
    EnableChangedHandler enableChangedHandler_;
    std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_{ nullptr };
    int32_t currentUserId_{ -1 };
};
} // namespace MiscServices
} // namespace OHOS

#endif // IME_ENABLED_INFO_MANAGER_H