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

#include <application_info.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "bundle_mgr_proxy.h"
#include "element_name.h"
#include "enable_ime_data_parser.h"
#include "ime_cfg_manager.h"
#include "input_method_info.h"
#include "input_method_property.h"
#include "input_method_status.h"
#include "refbase.h"
#include "sys_cfg_parser.h"
namespace OHOS {
namespace MiscServices {

struct ImeInfo {
    std::string moduleName;
    Property prop;
    SubProperty subProp;
    std::vector<SubProperty> subProps;
    bool isNewIme{ false };
};

enum class Condition {
    UPPER = 0,
    LOWER,
    ENGLISH,
    CHINESE,
};

struct Subtype : public Serializable {
    std::string label;
    std::string id;
    std::string icon;
    std::string mode;
    std::string locale;
    bool Unmarshal(cJSON *node) override
    {
        GetValue(node, GET_NAME(label), label);
        auto ret = GetValue(node, GET_NAME(id), id);
        GetValue(node, GET_NAME(icon), icon);
        GetValue(node, GET_NAME(mode), mode);
        GetValue(node, GET_NAME(locale), locale);
        return ret;
    }
};
struct SubtypeCfg : public Serializable {
    static constexpr uint32_t MAX_SUBTYPE_NUM = 256;
    std::vector<Subtype> subtypes;
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, GET_NAME(subtypes), subtypes, MAX_SUBTYPE_NUM);
    }
};

class ImeInfoInquirer {
public:
    using CompareHandler = std::function<bool(const SubProperty &)>;
    static ImeInfoInquirer &GetInstance();
    std::string GetDumpInfo(int32_t userId);
    std::shared_ptr<ImeNativeCfg> GetImeToStart(int32_t userId);
    std::shared_ptr<Property> GetImeByBundleName(int32_t userId, const std::string &bundleName);
    std::shared_ptr<Property> GetCurrentInputMethod(int32_t userId);
    std::shared_ptr<SubProperty> GetCurrentSubtype(int32_t userId);
    std::shared_ptr<ImeInfo> GetImeInfo(int32_t userId, const std::string &bundleName, const std::string &subName);
    std::shared_ptr<ImeInfo> GetDefaultImeInfo(int32_t userId);
    std::shared_ptr<ImeInfo> GetCurrentImeInfo();
    std::shared_ptr<Property> GetDefaultImeCfgProp();
    void SetCurrentImeInfo(std::shared_ptr<ImeInfo> info);
    void RefreshCurrentImeInfo(int32_t userId);
    std::shared_ptr<SubProperty> FindTargetSubtypeByCondition(const std::vector<SubProperty> &subProps,
        const Condition &condition);
    int32_t GetDefaultInputMethod(const int32_t userId, std::shared_ptr<Property> &prop, bool isBrief = false);
    int32_t GetInputMethodConfig(const int32_t userId, AppExecFwk::ElementName &inputMethodConfig);
    int32_t ListInputMethod(int32_t userId, InputMethodStatus status, std::vector<Property> &props, bool enableOn);
    int32_t ListInputMethodSubtype(int32_t userId, const std::string &bundleName, std::vector<SubProperty> &subProps);
    int32_t ListCurrentInputMethodSubtype(int32_t userId, std::vector<SubProperty> &subProps);
    int32_t GetSwitchInfoBySwitchCount(SwitchInfo &switchInfo, int32_t userId, bool enableOn, uint32_t cacheCount);
    bool IsEnableInputMethod();
    bool IsEnableSecurityMode();
    void InitSystemConfig();

private:
    ImeInfoInquirer() = default;
    ~ImeInfoInquirer() = default;
    OHOS::sptr<OHOS::AppExecFwk::IBundleMgr> GetBundleMgr();
    void InitCache(int32_t userId);
    SubProperty GetExtends(const std::vector<OHOS::AppExecFwk::Metadata> &metaData);
    ImeNativeCfg GetDefaultIme();
    std::string GetStringById(const std::string &bundleName, const std::string &moduleName, const int32_t labelId,
        const int32_t userId);
    std::shared_ptr<ImeInfo> GetImeInfoFromCache(const int32_t userId, const std::string &bundleName,
        const std::string &subName);
    std::shared_ptr<ImeInfo> GetImeInfoFromBundleMgr(const int32_t userId, const std::string &bundleName,
        const std::string &subName);
    int32_t GetExtInfosByBundleName(const int32_t userId, const std::string &bundleName,
        std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> &extInfos);
    bool IsNewExtInfos(const std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> &extInfos);
    bool IsImeInstalled(const int32_t userId, const std::string &bundleName, const std::string &extName);
    std::vector<InputMethodInfo> ListInputMethodInfo(const int32_t userId);
    int32_t ListInputMethod(const int32_t userId, std::vector<Property> &props);
    int32_t ListEnabledInputMethod(const int32_t userId, std::vector<Property> &props, bool enableOn);
    int32_t ListDisabledInputMethod(const int32_t userId, std::vector<Property> &props, bool enableOn);
    int32_t ListInputMethodSubtype(const int32_t userId,
        const std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> &extInfos, std::vector<SubProperty> &subProps);
    int32_t ListInputMethodSubtype(const int32_t userId, const OHOS::AppExecFwk::ExtensionAbilityInfo &extInfo,
        std::vector<SubProperty> &subProps);
    int32_t GetSubProperty(int32_t userId, const std::string &subName,
        const std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> &extInfos, SubProperty &subProp);
    int32_t GetSubProperty(int32_t userId, const std::string &subName,
        const OHOS::AppExecFwk::ExtensionAbilityInfo &extInfo, SubProperty &subProp);
    int32_t ParseSubtype(const OHOS::AppExecFwk::ExtensionAbilityInfo &extInfo, std::vector<Subtype> &subtypes);
    bool ParseSubtypeProfile(const std::vector<std::string> &profiles, SubtypeCfg &subtypeCfg);
    void CovertToLanguage(const std::string &locale, std::string &language);
    bool QueryImeExtInfos(const int32_t userId, std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> &infos);

    SystemConfig systemConfig_;
    std::mutex currentImeInfoLock_;
    std::shared_ptr<ImeInfo> currentImeInfo_{ nullptr }; // current imeInfo of current user
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_IME_INFO_ENQUIRER_H
