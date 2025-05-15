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
#include "input_method_utils.h"
#include "serializable.h"
#include "settings_data_utils.h"

namespace OHOS {
namespace MiscServices {
struct ImeExtendInfo {
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
};

struct ImeNativeCfg {
    std::string imeId;
    std::string bundleName;
    std::string subName;
    std::string extName;
    ImeExtendInfo imeExtendInfo;
};
struct ExtraInfo : public Serializable {
    bool isDefaultIme{ false };
    bool isDefaultImeSet{ false };
    bool isTmpIme{ false };
    std::string currentSubName;
    bool Unmarshal(cJSON *node) override
    {
        auto ret = GetValue(node, GET_NAME(isDefaultIme), isDefaultIme);
        ret = GetValue(node, GET_NAME(isDefaultImeSet), isDefaultImeSet) && ret;
        ret = GetValue(node, GET_NAME(isTmpIme), isTmpIme) && ret;
        return GetValue(node, GET_NAME(currentSubName), currentSubName) && ret;
    }
    bool Marshal(cJSON *node) const override
    {
        auto ret = SetValue(node, GET_NAME(isDefaultIme), isDefaultIme);
        ret = SetValue(node, GET_NAME(isDefaultImeSet), isDefaultImeSet) && ret;
        ret = SetValue(node, GET_NAME(isTmpIme), isTmpIme) && ret;
        return SetValue(node, GET_NAME(currentSubName), currentSubName) && ret;
    }

    bool operator==(const ExtraInfo &extraInfo) const // for tdd
    {
        return isDefaultIme == extraInfo.isDefaultIme && isDefaultImeSet == extraInfo.isDefaultImeSet &&
               isTmpIme == extraInfo.isTmpIme && currentSubName == extraInfo.currentSubName;
    }
};

struct ImeEnabledInfo : public Serializable {
    ImeEnabledInfo() = default;
    ImeEnabledInfo(const std::string &bundleName, const std::string &extensionName, EnabledStatus enabledStatus)
        : bundleName(bundleName), extensionName(extensionName), enabledStatus(enabledStatus){};
    std::string bundleName;
    std::string extensionName;
    EnabledStatus enabledStatus{ EnabledStatus::DISABLED };
    std::string stateUpdateTime; // user trigger
    ExtraInfo extraInfo;
    bool Unmarshal(cJSON *node) override
    {
        auto ret = GetValue(node, GET_NAME(bundleName), bundleName);
        ret = GetValue(node, GET_NAME(extensionName), extensionName) && ret;
        int32_t enabledStatusTmp = 0;
        ret = GetValue(node, GET_NAME(enabledStatus), enabledStatusTmp) && ret;
        enabledStatus = static_cast<EnabledStatus>(enabledStatusTmp);
        ret = GetValue(node, GET_NAME(stateUpdateTime), stateUpdateTime) && ret;
        return GetValue(node, GET_NAME(extraInfo), extraInfo) && ret;
    }
    bool Marshal(cJSON *node) const override
    {
        auto ret = SetValue(node, GET_NAME(bundleName), bundleName);
        ret = SetValue(node, GET_NAME(extensionName), extensionName) && ret;
        auto enabledStatusTmp = static_cast<int32_t>(enabledStatus);
        ret = SetValue(node, GET_NAME(enabledStatus), enabledStatusTmp) && ret;
        ret = SetValue(node, GET_NAME(stateUpdateTime), stateUpdateTime) && ret;
        return SetValue(node, GET_NAME(extraInfo), extraInfo) && ret;
    }
    bool operator==(const ImeEnabledInfo &enabledInfo) const // for tdd
    {
        return bundleName == enabledInfo.bundleName && extensionName == enabledInfo.extensionName &&
               enabledStatus == enabledInfo.enabledStatus && extraInfo == enabledInfo.extraInfo;
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
using CurrentImeStatusChangedHandler =
    std::function<void(int32_t userId, const std::string &bundleName, EnabledStatus oldStatus)>;
class ImeEnabledInfoManager {
public:
    static ImeEnabledInfoManager &GetInstance();
    void SetCurrentImeStatusChangedHandler(CurrentImeStatusChangedHandler handler);
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler);
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
    int32_t SetCurrentIme(int32_t userId, const std::string &imeId, const std::string &subName, bool isSetByUser);
    int32_t SetTmpIme(int32_t userId, const std::string &imeId);
    std::shared_ptr<ImeNativeCfg> GetCurrentImeCfg(int32_t userId);
    bool IsDefaultImeSet(int32_t userId);
    /* add for compatibility that sys ime mod full experience table in it's full experience switch changed */
    void OnFullExperienceTableChanged(int32_t userId);

private:
    ImeEnabledInfoManager() = default;
    ~ImeEnabledInfoManager();
    int32_t UpdateEnabledCfgCache(int32_t userId, const std::vector<FullImeInfo> &imeInfos = {});
    int32_t UpdateEnabledCfgCache(int32_t userId, const ImeEnabledCfg &cfg);
    int32_t GetEnabledCfg(int32_t userId, const std::vector<FullImeInfo> &imeInfos, ImeEnabledCfg &cfg);
    int32_t CorrectByBundleMgr(
        int32_t userId, const std::vector<FullImeInfo> &imeInfos, std::vector<ImeEnabledInfo> &enabledInfos);
    EnabledStatus ComputeEnabledStatus(const std::string &bundleName, EnabledStatus initStatus);
    int32_t GetEnabledStateInner(int32_t userId, const std::string &bundleName, EnabledStatus &status);
    int32_t GetEnabledStatesInner(int32_t userId, std::vector<Property> &props);
    void SetEnabledCache(int32_t userId, const ImeEnabledCfg &cfg);
    ImeEnabledCfg GetEnabledCache(int32_t userId);
    void ClearEnabledCache(int32_t userId);
    bool IsInEnabledCache(int32_t userId, const std::string &bundleName, const std::string &extensionName);
    int32_t GetEnabledCacheWithCorrect(int32_t userId, ImeEnabledCfg &enabledCfg);
    int32_t GetEnabledCacheWithCorrect(
        int32_t userId, const std::string &bundleName, const std::string &extensionName, ImeEnabledCfg &enabledCfg);
    int32_t CheckUpdate(
        int32_t userId, const std::string &bundleName, const std::string &extensionName, EnabledStatus status);
    void NotifyCurrentImeStatusChanged(int32_t userId, const std::string &bundleName, EnabledStatus newStatus);
    bool HasEnabledSwitch();
    bool IsExpired(const std::string &expirationTime);
    std::pair<std::string, std::string> SplitImeId(const std::string &imeId);
    void ModCurrentIme(std::vector<ImeEnabledInfo> &enabledInfos);
    bool IsCurrentIme(const std::string &bundleName, const std::vector<ImeEnabledInfo> &enabledInfos);
    /* add for compatibility that sys ime listen global table change for smart menu in tablet */
    void UpdateGlobalEnabledTable(int32_t userId, const ImeEnabledCfg &newEnabledCfg);
    std::mutex imeEnabledCfgLock_;
    std::map<int32_t, ImeEnabledCfg> imeEnabledCfg_;
    CurrentImeStatusChangedHandler currentImeStatusChangedHandler_;
    std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_{ nullptr };
    std::mutex operateLock_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // IME_ENABLED_INFO_MANAGER_H