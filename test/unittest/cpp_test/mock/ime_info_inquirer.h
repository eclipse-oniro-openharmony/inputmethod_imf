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

#ifndef SERVICES_INCLUDE_IME_INFO_ENQUIRER_H
#define SERVICES_INCLUDE_IME_INFO_ENQUIRER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "input_method_property.h"
#include "input_method_status.h"

namespace OHOS {
namespace MiscServices {
struct ImeNativeCfg {
    std::string imeId;
    std::string bundleName;
    std::string subName;
    std::string extName;
};

struct SystemConfig {
    std::string systemInputMethodConfigAbility;
    std::string defaultInputMethod;
    std::string systemSpecialInputMethod;
    bool enableInputMethodFeature = false;
    bool enableFullExperienceFeature = false;
    EnabledStatus initEnabledState{ EnabledStatus::DISABLED };
    bool enableAppAgentFeature = false;
    bool enableNumKeyFeature = false;
    std::unordered_set<std::string> disableNumKeyAppDeviceTypes;
    std::unordered_set<int32_t> proxyImeUidList;
    std::unordered_set<int32_t> specialSaUidList;
    std::unordered_set<std::string> defaultImeScreenList;
    std::unordered_set<std::string> supportedCapacityList;
    std::string dynamicStartImeSysParam;
    std::string dynamicStartImeValue;
};

enum class Condition {
    UPPER = 0,
    LOWER,
    ENGLISH,
    CHINESE,
};
class ImeInfoInquirer {
public:
    static ImeInfoInquirer &GetInstance();
    std::shared_ptr<ImeInfo> GetDefaultImeInfo(int32_t userId);
    std::shared_ptr<Property> GetDefaultImeCfgProp();
    ImeNativeCfg GetDefaultIme();
    std::shared_ptr<ImeNativeCfg> GetDefaultImeCfg();
    std::shared_ptr<Property> GetCurrentInputMethod(int32_t userId);
    void SetFullImeInfo(bool isReturnOk, const FullImeInfo &imeInfo);
    void SetFullImeInfo(bool isReturnOk, const std::vector<FullImeInfo> &imeInfos);
    void SetFullImeInfo(bool isReturnOk, const std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> &fullImeInfos);
    int32_t QueryFullImeInfo(std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> &fullImeInfos) const;
    int32_t QueryFullImeInfo(int32_t userId, std::vector<FullImeInfo> &imeInfos, bool needBrief = false) const;
    int32_t GetFullImeInfo(int32_t userId, const std::string &bundleName, FullImeInfo &imeInfo) const;
    static bool GetImeAppId(int32_t userId, const std::string &bundleName, std::string &appId);
    static bool GetImeVersionCode(int32_t userId, const std::string &bundleName, uint32_t &versionCode);
    std::string GetDumpInfo(int32_t userId);
    std::shared_ptr<ImeNativeCfg> GetImeToStart(int32_t userId);
    std::shared_ptr<Property> GetImeProperty(
        int32_t userId, const std::string &bundleName, const std::string &extName = "");
    std::shared_ptr<SubProperty> GetCurrentSubtype(int32_t userId);
    std::shared_ptr<ImeInfo> GetImeInfo(int32_t userId, const std::string &bundleName, const std::string &subName);
    std::shared_ptr<SubProperty> FindTargetSubtypeByCondition(
        const std::vector<SubProperty> &subProps, const Condition &condition);
    int32_t GetDefaultInputMethod(const int32_t userId, std::shared_ptr<Property> &prop, bool isBrief = false);
    int32_t ListInputMethod(int32_t userId, InputMethodStatus status, std::vector<Property> &props);
    int32_t ListInputMethodSubtype(int32_t userId, const std::string &bundleName, std::vector<SubProperty> &subProps);
    int32_t ListCurrentInputMethodSubtype(int32_t userId, std::vector<SubProperty> &subProps);
    int32_t GetSwitchInfoBySwitchCount(SwitchInfo &switchInfo, int32_t userId, uint32_t cacheCount);
    bool IsEnableInputMethod();
    bool IsEnableSecurityMode();
    bool IsEnableAppAgent();
    bool IsEnableNumKey();
    bool IsProxyIme(int32_t callingUid);
    bool IsSpecialSaUid(int32_t callingUid);
    void InitSystemConfig();
    SystemConfig GetSystemConfig();
    std::string GetSystemSpecialIme();
    bool IsInputMethod(int32_t userId, const std::string &bundleName);
    bool IsRunningIme(int32_t userId, const std::string &bundleName);
    std::vector<std::string> GetRunningIme(int32_t userId);
    bool IsDefaultImeSet(int32_t userId);
    bool IsImeInstalled(const int32_t userId, const std::string &bundleName, const std::string &extName);
    bool IsInputMethodExtension(pid_t pid);
    bool IsRestrictedDefaultImeByDisplay(uint64_t displayId);
    bool IsDynamicStartIme();
    std::unordered_set<std::string> GetDisableNumKeyAppDeviceTypes();
    bool IsCapacitySupport(const std::string &capacityName);
    bool GetCompatibleDeviceType(const std::string &bundleName, std::string &compatibleDeviceType);
    void SetDumpInfo(int32_t userId, const std::string &dumpInfo);
    void SetImeToStart(int32_t userId, const std::shared_ptr<ImeNativeCfg> &imeToStart);
    void SetImeProperty(int32_t userId, const std::shared_ptr<Property> &prop);
    void SetCompatibleDeviceType(const std::string &bundleName, const std::string &compatibleDeviceType);
    void SetCapacitySupportFlag(const std::string &capacityName, bool isSupport);
    void SetDynamicStartImeFlag(bool isDynamicStart);
    void SetDisableNumKeyAppDeviceTypes(const std::unordered_set<std::string> &types);
    void SetRunningIme(int32_t userId, const std::vector<std::string> &ime);
    void SetRestrictedDefaultImeDisplay(uint64_t displayId);
    void SetInputMethodExtension(pid_t pid);
    void SetSystemConfig(const SystemConfig &sysCfg);
    void SetDefaultInputMethod(const std::shared_ptr<ImeInfo> &imeInfo);
    void SetCurrentSubtype(int32_t userId, const std::shared_ptr<SubProperty> &subProp);
    void SetImeInfo(int32_t userId, const std::vector<std::shared_ptr<ImeInfo>> &imeInfos);

private:
    bool isQueryAllFullImeInfosOk_{ false };
    std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> allFullImeInfos_;
    bool isQueryFullImeInfosOk_{ false };
    std::vector<FullImeInfo> fullImeInfos_;
    bool isGetFullImeInfoOk_{ false };
    FullImeInfo fullImeInfo_;
    std::mutex dumpInfosLock_;
    std::map<int32_t, std::string> dumpInfos_;
    std::mutex imeToStartLock_;
    std::map<int32_t, std::shared_ptr<ImeNativeCfg>> imeToStart_;
    std::mutex imePropsLock_;
    std::map<int32_t, std::shared_ptr<Property>> imeProps_;
    std::mutex imeSubPropsLock_;
    std::map<int32_t, std::shared_ptr<SubProperty>> imeSubProps_;
    std::mutex compatibleDeviceTypeLock_;
    std::map<std::string, std::string> compatibleDeviceType_;
    std::mutex capacitySupportInfosLock_;
    std::map<std::string, bool> capacitySupportInfos_;
    std::atomic<bool> isDynamicStartIme_{ false };
    std::mutex disableNumKeyAppDeviceTypesLock_;
    std::unordered_set<std::string> disableNumKeyAppDeviceTypes_;
    std::mutex runningImeLock_;
    std::map<int32_t, std::vector<std::string>> runningIme_;
    std::atomic<uint64_t> restrictedDefaultImeDisplayId_{ 0 };
    pid_t inputMethodExt_{ 0 };
    std::mutex sysCfgLock_;
    SystemConfig sysCfg_;
    std::mutex defaultImeInfoLock_;
    std::shared_ptr<ImeInfo> defaultImeInfo_;
    std::mutex imeInfosLock_;
    std::map<int32_t, std::vector<std::shared_ptr<ImeInfo>>> imeInfos_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_IME_INFO_ENQUIRER_H
