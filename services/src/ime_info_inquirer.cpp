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

#include "ime_info_inquirer.h"
#include "app_mgr_client.h"
#include "bundle_mgr_client_impl.h"
#include "full_ime_info_manager.h"
#include "ime_enabled_info_manager.h"
#include "input_type_manager.h"
#include "iservice_registry.h"
#include "locale_config.h"
#include "locale_info.h"
#include "os_account_adapter.h"
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
namespace {
using namespace OHOS::AppExecFwk;
using namespace Global::Resource;
using namespace OHOS::AAFwk;
constexpr const char *SUBTYPE_PROFILE_METADATA_NAME = "ohos.extension.input_method";
constexpr const char *TEMPORARY_INPUT_METHOD_METADATA_NAME = "ohos.extension.temporary_input_method";
constexpr uint32_t SUBTYPE_PROFILE_NUM = 1;
constexpr const char *DEFAULT_IME_KEY = "persist.sys.default_ime";
constexpr int32_t CONFIG_LEN = 128;
constexpr uint32_t DEFAULT_BMS_VALUE = 0;
} // namespace
ImeInfoInquirer &ImeInfoInquirer::GetInstance()
{
    static ImeInfoInquirer instance;
    return instance;
}

void ImeInfoInquirer::InitSystemConfig()
{
    auto ret = SysCfgParser::ParseSystemConfig(systemConfig_);
    if (!ret) {
        IMSA_HILOGE("parse systemConfig failed!");
        return;
    }
}

bool ImeInfoInquirer::IsEnableAppAgent()
{
    return systemConfig_.enableAppAgentFeature;
}

bool ImeInfoInquirer::IsVirtualProxyIme(int32_t callingUid)
{
    return systemConfig_.proxyImeUidList.find(callingUid) != systemConfig_.proxyImeUidList.end();
}

bool ImeInfoInquirer::IsSpecialSaUid(int32_t callingUid)
{
    return systemConfig_.specialSaUidList.find(callingUid) != systemConfig_.specialSaUidList.end();
}

SystemConfig ImeInfoInquirer::GetSystemConfig()
{
    return systemConfig_;
}

bool ImeInfoInquirer::QueryImeExtInfos(const int32_t userId, std::vector<ExtensionAbilityInfo> &infos)
{
    IMSA_HILOGD("userId: %{public}d.", userId);
    auto bundleMgr = GetBundleMgr();
    if (bundleMgr == nullptr) {
        IMSA_HILOGE("failed to GetBundleMgr!");
        return false;
    }
    if (!bundleMgr->QueryExtensionAbilityInfos(ExtensionAbilityType::INPUTMETHOD, userId, infos)) {
        IMSA_HILOGF("query extension infos failed from bundleMgr!");
        return false;
    }
    return true;
}

int32_t ImeInfoInquirer::GetExtInfosByBundleName(const int32_t userId, const std::string &bundleName,
    std::vector<AppExecFwk::ExtensionAbilityInfo> &extInfos)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s.", userId, bundleName.c_str());
    std::vector<AppExecFwk::ExtensionAbilityInfo> tempExtInfos;
    if (!QueryImeExtInfos(userId, tempExtInfos)) {
        IMSA_HILOGE("failed to QueryImeExtInfos!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    for (const auto &extInfo : tempExtInfos) {
        if (extInfo.bundleName == bundleName) {
            extInfos.emplace_back(extInfo);
        }
    }
    if (extInfos.empty()) {
        IMSA_HILOGE("bundleName: %{public}s extInfos is empty!", bundleName.c_str());
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetImeInfo(int32_t userId, const std::string &bundleName,
    const std::string &subName)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s, subName: %{public}s.", userId, bundleName.c_str(),
        subName.c_str());
    auto info = GetImeInfoFromCache(userId, bundleName, subName);
    return info == nullptr ? GetImeInfoFromBundleMgr(userId, bundleName, subName) : info;
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetImeInfoFromCache(const int32_t userId, const std::string &bundleName,
    const std::string &subName)
{
    FullImeInfo imeInfo;
    if (!FullImeInfoManager::GetInstance().Get(userId, bundleName, imeInfo)) {
        return nullptr;
    }
    auto info = std::make_shared<ImeInfo>();
    auto subProps = imeInfo.subProps;
    info->isSpecificSubName = !subName.empty();
    if (subName.empty() && !subProps.empty()) {
        info->subProp = subProps[0];
    } else {
        auto iter = std::find_if(subProps.begin(), subProps.end(),
            [&subName](const SubProperty &subProp) { return subProp.id == subName; });
        if (iter == subProps.end()) {
            IMSA_HILOGE("find subName: %{public}s failed!", subName.c_str());
            return nullptr;
        }
        info->subProp = *iter;
    }
    info->isNewIme = imeInfo.isNewIme;
    info->subProps = imeInfo.subProps;
    info->prop = imeInfo.prop;
    if (!info->isNewIme) {
        // old ime, make the id of prop same with the id of subProp.
        info->prop.id = info->subProp.id;
    }
    return info;
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetImeInfoFromBundleMgr(
    const int32_t userId, const std::string &bundleName, const std::string &subName)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s, subName: %{public}s.", userId, bundleName.c_str(),
        subName.c_str());
    std::vector<AppExecFwk::ExtensionAbilityInfo> extInfos;
    auto ret = ImeInfoInquirer::GetInstance().GetExtInfosByBundleName(userId, bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR || extInfos.empty()) {
        IMSA_HILOGE("userId: %{public}d getExtInfosByBundleName %{public}s failed!", userId, bundleName.c_str());
        return nullptr;
    }
    auto info = std::make_shared<ImeInfo>();
    info->prop.name = extInfos[0].bundleName;
    info->prop.id = extInfos[0].name;
    info->prop.label = GetTargetString(extInfos[0], ImeTargetString::LABEL, userId);
    info->prop.labelId = extInfos[0].applicationInfo.labelId;
    info->prop.iconId = extInfos[0].applicationInfo.iconId;

    std::vector<SubProperty> subProps;
    info->isNewIme = IsNewExtInfos(extInfos);
    ret = info->isNewIme ? ListInputMethodSubtype(userId, extInfos[0], subProps)
                         : ListInputMethodSubtype(userId, extInfos, subProps);
    if (ret != ErrorCode::NO_ERROR || subProps.empty()) {
        IMSA_HILOGE("userId: %{public}d listInputMethodSubtype failed!", userId);
        return nullptr;
    }
    info->subProps = subProps;
    if (subName.empty()) {
        info->isSpecificSubName = false;
        info->subProp = subProps[0];
    } else {
        auto it = std::find_if(subProps.begin(), subProps.end(),
            [&subName](const SubProperty &subProp) { return subProp.id == subName; });
        if (it == subProps.end()) {
            IMSA_HILOGE("find subName: %{public}s failed!", subName.c_str());
            return nullptr;
        }
        info->subProp = *it;
    }
    // old ime, make the id of prop same with the id of subProp.
    if (!info->isNewIme) {
        info->prop.id = info->subProp.id;
    }
    return info;
}

std::string ImeInfoInquirer::GetDumpInfo(int32_t userId)
{
    auto properties = ListInputMethodInfo(userId);
    if (properties.empty()) {
        return "";
    }
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    bool isBegin = true;
    std::string params = "{\"imeList\":[";
    for (const auto &property : properties) {
        params += isBegin ? "" : "},";
        isBegin = false;

        std::string imeId = property.mPackageName + "/" + property.mAbilityName;
        params += "{\"ime\": \"" + imeId + "\",";
        params += "\"labelId\": \"" + std::to_string(property.labelId) + "\",";
        params += "\"descriptionId\": \"" + std::to_string(property.descriptionId) + "\",";
        std::string isCurrentIme = currentImeCfg->imeId == imeId ? "true" : "false";
        params += "\"isCurrentIme\": \"" + isCurrentIme + "\",";
        params += "\"label\": \"" + property.label + "\",";
        params += "\"description\": \"" + property.description + "\"";
    }
    params += "}]}";
    return params;
}

std::vector<InputMethodInfo> ImeInfoInquirer::ListInputMethodInfo(const int32_t userId)
{
    IMSA_HILOGD("userId: %{public}d.", userId);
    std::vector<ExtensionAbilityInfo> extensionInfos;
    if (!QueryImeExtInfos(userId, extensionInfos)) {
        IMSA_HILOGE("userId: %{public}d queryImeExtInfos failed!", userId);
        return {};
    }
    std::vector<InputMethodInfo> properties;
    for (const auto &extension : extensionInfos) {
        auto applicationInfo = extension.applicationInfo;
        auto label = GetTargetString(extension, ImeTargetString::LABEL, userId);
        auto description = GetTargetString(extension, ImeTargetString::DESCRIPTION, userId);
        InputMethodInfo property;
        property.mPackageName = extension.bundleName;
        property.mAbilityName = extension.name;
        property.labelId = applicationInfo.labelId;
        property.descriptionId = applicationInfo.descriptionId;
        property.label = label;
        property.description = description;
        properties.emplace_back(property);
    }
    return properties;
}

int32_t ImeInfoInquirer::ListInputMethod(int32_t userId, InputMethodStatus status, std::vector<Property> &props)
{
    IMSA_HILOGD("userId: %{public}d, status: %{public}d.", userId, status);
    if (status == InputMethodStatus::ALL) {
        return ListAllInputMethod(userId, props);
    }
    if (status == InputMethodStatus::ENABLE) {
        return ListEnabledInputMethod(userId, props);
    }
    if (status == InputMethodStatus::DISABLE) {
        return ListDisabledInputMethod(userId, props);
    }
    return ErrorCode::ERROR_BAD_PARAMETERS;
}

int32_t ImeInfoInquirer::ListAllInputMethod(const int32_t userId, std::vector<Property> &props)
{
    auto ret = ListInputMethod(userId, props);
    if (ret == ErrorCode::ERROR_ENABLE_IME) {
        return ErrorCode::NO_ERROR;
    }
    return ret;
}

int32_t ImeInfoInquirer::ListInputMethod(const int32_t userId, std::vector<Property> &props)
{
    IMSA_HILOGD("userId: %{public}d.", userId);
    auto ret = FullImeInfoManager::GetInstance().Get(userId, props);
    if (!props.empty()) {
        return ret;
    }
    IMSA_HILOGI("%{public}d get all prop form bms.", userId);
    std::vector<ExtensionAbilityInfo> extensionInfos;
    if (!QueryImeExtInfos(userId, extensionInfos)) {
        IMSA_HILOGE("failed to QueryImeExtInfos!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    for (const auto &extension : extensionInfos) {
        auto it = std::find_if(props.begin(), props.end(),
            [&extension](const Property &prop) { return prop.name == extension.bundleName; });
        if (it != props.end()) {
            continue;
        }
        if (IsTempInputMethod(extension)) {
            continue;
        }
        Property prop;
        prop.name = extension.bundleName;
        prop.id = extension.name;
        prop.label = GetTargetString(extension, ImeTargetString::LABEL, userId);
        prop.labelId = extension.applicationInfo.labelId;
        prop.iconId = extension.applicationInfo.iconId;
        props.push_back(prop);
    }
    ret = ImeEnabledInfoManager::GetInstance().GetEnabledStates(userId, props);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("get enabled status failed:%{public}d!", ret);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListEnabledInputMethod(const int32_t userId, std::vector<Property> &props)
{
    IMSA_HILOGD("userId: %{public}d.", userId);
    int32_t ret = ListInputMethod(userId, props);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d listInputMethod failed!", userId);
        return ret;
    }
    auto start = std::remove_if(
        props.begin(), props.end(), [](const auto &prop) { return prop.status == EnabledStatus::DISABLED; });
    props.erase(start, props.end());
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListDisabledInputMethod(const int32_t userId, std::vector<Property> &props)
{
    IMSA_HILOGD("userId: %{public}d.", userId);
    auto ret = ListInputMethod(userId, props);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d listInputMethod failed!", userId);
        return ret;
    }
    auto start = std::remove_if(
        props.begin(), props.end(), [](const auto &prop) { return prop.status != EnabledStatus::DISABLED; });
    props.erase(start, props.end());
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::GetSwitchInfoBySwitchCount(SwitchInfo &switchInfo, int32_t userId, uint32_t cacheCount)
{
    std::vector<Property> props;
    auto ret = ListEnabledInputMethod(userId, props);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d ListEnabledInputMethod failed!", userId);
        return ret;
    }
    auto currentImeBundle = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId)->bundleName;
    auto iter = std::find_if(props.begin(), props.end(),
        [&currentImeBundle](const Property &property) { return property.name == currentImeBundle; });
    if (iter == props.end()) {
        IMSA_HILOGE("can not found current ime in enable list!");
        auto info = GetDefaultImeInfo(userId);
        if (info != nullptr) {
            switchInfo.bundleName = info->prop.name;
            return ErrorCode::NO_ERROR;
        }
        IMSA_HILOGE("bundle manager error!");
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    uint32_t nextIndex = (cacheCount + static_cast<uint32_t>(std::distance(props.begin(), iter))) % props.size();
    switchInfo.bundleName = props[nextIndex].name;
    IMSA_HILOGD("next ime: %{public}s", switchInfo.bundleName.c_str());
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListInputMethodSubtype(int32_t userId, const std::string &bundleName,
    std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s.", userId, bundleName.c_str());
    FullImeInfo imeInfo;
    if (FullImeInfoManager::GetInstance().Get(userId, bundleName, imeInfo)) {
        subProps = imeInfo.subProps;
        return ErrorCode::NO_ERROR;
    }

    IMSA_HILOGD("%{public}d get %{public}s all subProp form bms.", userId, bundleName.c_str());
    std::vector<ExtensionAbilityInfo> extInfos;
    auto ret = GetExtInfosByBundleName(userId, bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d getExtInfosByBundleName %{public}s failed!", userId, bundleName.c_str());
        return ret;
    }
    return IsNewExtInfos(extInfos) ? ListInputMethodSubtype(userId, extInfos[0], subProps)
                                   : ListInputMethodSubtype(userId, extInfos, subProps);
}

int32_t ImeInfoInquirer::ListCurrentInputMethodSubtype(int32_t userId, std::vector<SubProperty> &subProps)
{
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    IMSA_HILOGD("currentIme: %{public}s.", currentImeCfg->imeId.c_str());
    return ListInputMethodSubtype(userId, currentImeCfg->bundleName, subProps);
}

bool ImeInfoInquirer::IsNewExtInfos(const std::vector<ExtensionAbilityInfo> &extInfos)
{
    if (extInfos.empty()) {
        IMSA_HILOGE("extInfos is empty!");
        return false;
    }
    auto iter = std::find_if(extInfos[0].metadata.begin(), extInfos[0].metadata.end(),
        [](const Metadata &metadata) { return metadata.name == SUBTYPE_PROFILE_METADATA_NAME; });
    return iter != extInfos[0].metadata.end();
}

int32_t ImeInfoInquirer::GetSubProperty(int32_t userId, const std::string &subName,
    const std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> &extInfos, SubProperty &subProp)
{
    IMSA_HILOGD("oldIme, userId: %{public}d.", userId);
    if (extInfos.empty()) {
        IMSA_HILOGE("extInfos is empty!");
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    auto extInfo = std::find_if(extInfos.begin(), extInfos.end(),
        [&subName](const ExtensionAbilityInfo &info) { return info.name == subName; });
    if (extInfo == extInfos.end()) {
        IMSA_HILOGE("subtype %{public}s not found!", subName.c_str());
        extInfo = extInfos.begin();
    }
    subProp.labelId = extInfo->labelId;
    subProp.label = GetStringById(extInfo->bundleName, extInfo->moduleName, extInfo->labelId, userId);
    subProp.id = extInfo->name;
    subProp.name = extInfo->bundleName;
    subProp.iconId = extInfo->iconId;
    std::vector<Metadata> extends = extInfo->metadata;
    auto property = GetExtends(extends);
    subProp.language = property.language;
    subProp.mode = property.mode;
    subProp.locale = property.locale;
    subProp.icon = property.icon;
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListInputMethodSubtype(const int32_t userId,
    const std::vector<ExtensionAbilityInfo> &extInfos, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("oldIme, userId: %{public}d.", userId);
    for (const auto &extInfo : extInfos) {
        SubProperty subProperty;
        subProperty.labelId = extInfo.labelId;
        subProperty.label = GetStringById(extInfo.bundleName, extInfo.moduleName, extInfo.labelId, userId);
        subProperty.id = extInfo.name;
        subProperty.name = extInfo.bundleName;
        subProperty.iconId = extInfo.iconId;
        std::vector<Metadata> extends = extInfo.metadata;
        auto property = GetExtends(extends);
        subProperty.language = property.language;
        subProperty.mode = property.mode;
        subProperty.locale = property.locale;
        subProperty.icon = property.icon;
        subProps.emplace_back(subProperty);
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::GetSubProperty(int32_t userId, const std::string &subName,
    const OHOS::AppExecFwk::ExtensionAbilityInfo &extInfo, SubProperty &subProp)
{
    IMSA_HILOGD("newIme, userId: %{public}d.", userId);
    std::vector<Subtype> subtypes;
    auto ret = ParseSubtype(extInfo, subtypes);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to parse subtype!");
        return ret;
    }
    if (subtypes.empty()) {
        IMSA_HILOGE("subtypes is empty!");
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    auto subtype = std::find_if(
        subtypes.begin(), subtypes.end(), [&subName](const Subtype &subtype) { return subtype.id == subName; });
    if (subtype == subtypes.end()) {
        IMSA_HILOGE("subtype %{public}s not found!", subName.c_str());
        subtype = subtypes.begin();
    }
    subProp.label = subtype->label;
    subProp.name = extInfo.bundleName;
    subProp.id = subtype->id;
    subProp.mode = subtype->mode;
    subProp.locale = subtype->locale;
    subProp.icon = subtype->icon;
    auto pos = subProp.label.find(':');
    if (pos != std::string::npos && pos + 1 < subProp.label.size()) {
        int32_t labelId = atoi(subProp.label.substr(pos + 1).c_str());
        if (labelId > 0) {
            subProp.labelId = static_cast<uint32_t>(labelId);
            subProp.label = GetStringById(extInfo.bundleName, extInfo.moduleName, subProp.labelId, userId);
        }
    }
    pos = subProp.icon.find(':');
    if (pos != std::string::npos && pos + 1 < subProp.icon.size()) {
        int32_t iconId = atoi(subProp.icon.substr(pos + 1).c_str());
        if (iconId > 0) {
            subProp.iconId = static_cast<uint32_t>(iconId);
        }
    }
    CovertToLanguage(subProp.locale, subProp.language);
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListInputMethodSubtype(const int32_t userId, const ExtensionAbilityInfo &extInfo,
    std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("newIme, userId: %{public}d.", userId);
    std::vector<Subtype> subtypes;
    auto ret = ParseSubtype(extInfo, subtypes);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to parse subtype!");
        return ret;
    }

    std::string resPath = extInfo.hapPath.empty() ? extInfo.resourcePath : extInfo.hapPath;
    auto resMgr = GetResMgr(resPath);
    IMSA_HILOGD("subtypes size: %{public}zu.", subtypes.size());
    for (const auto &subtype : subtypes) {
        // subtype which provides a particular input type should not appear in the subtype list
        if (InputTypeManager::GetInstance().IsInputType({ extInfo.bundleName, subtype.id })) {
            continue;
        }
        SubProperty subProp;
        subProp.label = subtype.label;
        subProp.name = extInfo.bundleName;
        subProp.id = subtype.id;
        subProp.mode = subtype.mode;
        subProp.locale = subtype.locale;
        subProp.icon = subtype.icon;
        auto pos = subProp.label.find(':');
        if (pos != std::string::npos && pos + 1 < subProp.label.size()) {
            int32_t labelId = atoi(subProp.label.substr(pos + 1).c_str());
            if (labelId > 0) {
                subProp.labelId = static_cast<uint32_t>(labelId);
            }
        }
        if (resMgr != nullptr) {
            auto errValue = resMgr->GetStringById(subProp.labelId, subProp.label);
            if (errValue != RState::SUCCESS) {
                IMSA_HILOGE("GetStringById failed, bundleName:%{public}s, id:%{public}d.", extInfo.bundleName.c_str(),
                    subProp.labelId);
            }
        }
        pos = subProp.icon.find(':');
        if (pos != std::string::npos && pos + 1 < subProp.icon.size()) {
            int32_t iconId = atoi(subProp.icon.substr(pos + 1).c_str());
            if (iconId > 0) {
                subProp.iconId = static_cast<uint32_t>(iconId);
            }
        }
        CovertToLanguage(subProp.locale, subProp.language);
        subProps.emplace_back(subProp);
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ParseSubtype(const OHOS::AppExecFwk::ExtensionAbilityInfo &extInfo,
    std::vector<Subtype> &subtypes)
{
    if (extInfo.metadata.empty()) {
        IMSA_HILOGE("metadata is empty!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto iter = std::find_if(extInfo.metadata.begin(), extInfo.metadata.end(),
        [](const Metadata &metadata) { return metadata.name == SUBTYPE_PROFILE_METADATA_NAME; });
    if (iter == extInfo.metadata.end()) {
        IMSA_HILOGE("find metadata name: SUBTYPE_PROFILE_METADATA_NAME failed!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    OHOS::AppExecFwk::BundleMgrClientImpl clientImpl;
    std::vector<std::string> profiles;
    if (!clientImpl.GetResConfigFile(extInfo, iter->name, profiles)) {
        IMSA_HILOGE("failed to GetProfileFromExtension!");
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    SubtypeCfg subtypeCfg;
    if (!ParseSubtypeProfile(profiles, subtypeCfg)) {
        IMSA_HILOGE("failed to ParseSubTypeCfg!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    subtypes = subtypeCfg.subtypes;
    IMSA_HILOGD("success.");
    return ErrorCode::NO_ERROR;
}

void ImeInfoInquirer::CovertToLanguage(const std::string &locale, std::string &language)
{
    language = locale;
    auto pos = locale.find('-');
    if (pos != std::string::npos) {
        language = locale.substr(0, pos);
    }
    // compatible with the locale configuration of original ime
    pos = locale.find('_');
    if (pos != std::string::npos) {
        language = locale.substr(0, pos);
    }
    if (language == "en") {
        language = "english";
    }
    if (language == "zh") {
        language = "chinese";
    }
}

std::string ImeInfoInquirer::GetStringById(const std::string &bundleName, const std::string &moduleName,
    uint32_t labelId, int32_t userId)
{
    auto bundleMgr = GetBundleMgr();
    return bundleMgr == nullptr ? "" : bundleMgr->GetStringById(bundleName, moduleName, labelId, userId);
}

SubProperty ImeInfoInquirer::GetExtends(const std::vector<Metadata> &metaData)
{
    SubProperty property;
    for (const auto &data : metaData) {
        if (data.name == "language") {
            property.language = data.value;
            continue;
        }
        if (data.name == "mode") {
            property.mode = data.value;
            continue;
        }
        if (data.name == "locale") {
            property.locale = data.value;
            continue;
        }
        if (data.name == "icon") {
            property.icon = data.value;
        }
    }
    return property;
}

std::shared_ptr<Property> ImeInfoInquirer::GetImeProperty(
    int32_t userId, const std::string &bundleName, const std::string &extName)
{
    IMSA_HILOGD("start, bundleName: %{public}s", bundleName.c_str());
    std::vector<AppExecFwk::ExtensionAbilityInfo> extInfos;
    auto ret = ImeInfoInquirer::GetInstance().GetExtInfosByBundleName(userId, bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR || extInfos.empty()) {
        IMSA_HILOGE("userId: %{public}d getExtInfosByBundleName %{public}s failed!", userId, bundleName.c_str());
        return nullptr;
    }
    Property prop;
    prop.name = extInfos[0].bundleName;
    prop.id = extName.empty() ? extInfos[0].name : extName;
    prop.label = GetTargetString(extInfos[0], ImeTargetString::LABEL, userId);
    prop.labelId = extInfos[0].applicationInfo.labelId;
    prop.iconId = extInfos[0].applicationInfo.iconId;
    ret = ImeEnabledInfoManager::GetInstance().GetEnabledState(userId, prop.name, prop.status);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("get enabled status failed:%{public}d!", ret);
    }
    return std::make_shared<Property>(prop);
}

std::shared_ptr<Property> ImeInfoInquirer::GetCurrentInputMethod(int32_t userId)
{
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    IMSA_HILOGD("currentIme: %{public}s.", currentImeCfg->imeId.c_str());
    FullImeInfo imeInfo;
    if (FullImeInfoManager::GetInstance().Get(userId, currentImeCfg->bundleName, imeInfo)) {
        auto prop = std::make_shared<Property>(imeInfo.prop);
        prop->id = currentImeCfg->extName;
        return prop;
    }

    IMSA_HILOGD("%{public}d get %{public}s prop form bms.", userId, currentImeCfg->bundleName.c_str());
    return GetImeProperty(userId, currentImeCfg->bundleName, currentImeCfg->extName);
}

std::shared_ptr<SubProperty> ImeInfoInquirer::GetCurrentSubtype(int32_t userId)
{
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    IMSA_HILOGD("currentIme: %{public}s.", currentIme->imeId.c_str());
    FullImeInfo imeInfo;
    if (FullImeInfoManager::GetInstance().Get(userId, currentIme->bundleName, imeInfo) && !imeInfo.subProps.empty()) {
        auto iter = std::find_if(imeInfo.subProps.begin(), imeInfo.subProps.end(),
            [&currentIme](const SubProperty &subProp) { return subProp.id == currentIme->subName; });
        if (iter != imeInfo.subProps.end()) {
            return std::make_shared<SubProperty>(*iter);
        }
        IMSA_HILOGW("subtype %{public}s not found.", currentIme->subName.c_str());
        return std::make_shared<SubProperty>(imeInfo.subProps[0]);
    }

    IMSA_HILOGD("%{public}d get [%{public}s, %{public}s] form bms.", userId, currentIme->bundleName.c_str(),
        currentIme->subName.c_str());
    std::vector<ExtensionAbilityInfo> extInfos;
    auto ret = GetExtInfosByBundleName(userId, currentIme->bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to GetExtInfosByBundleName: %{public}s, ret: %{public}d", currentIme->bundleName.c_str(),
            ret);
        return nullptr;
    }
    SubProperty subProp;
    ret = IsNewExtInfos(extInfos) ? GetSubProperty(userId, currentIme->subName, extInfos[0], subProp)
                                  : GetSubProperty(userId, currentIme->subName, extInfos, subProp);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("get %{public}s property failed, ret: %{public}d!", currentIme->subName.c_str(), ret);
        return nullptr;
    }
    return std::make_shared<SubProperty>(subProp);
}

bool ImeInfoInquirer::IsImeInstalled(const int32_t userId, const std::string &bundleName, const std::string &extName)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s, extName: %{public}s.", userId, bundleName.c_str(),
        extName.c_str());
    std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> extInfos;
    GetExtInfosByBundleName(userId, bundleName, extInfos);
    auto iter = std::find_if(extInfos.begin(), extInfos.end(),
        [&bundleName, &extName](const OHOS::AppExecFwk::ExtensionAbilityInfo &extInfo) {
            return extInfo.bundleName == bundleName && extName == extInfo.name;
        });
    if (iter == extInfos.end()) {
        IMSA_HILOGE("false");
        return false;
    }
    IMSA_HILOGI("true");
    return true;
}

std::shared_ptr<ImeNativeCfg> ImeInfoInquirer::GetImeToStart(int32_t userId)
{
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    IMSA_HILOGD("userId: %{public}d, currentIme: %{public}s.", userId, currentImeCfg->imeId.c_str());
    if (currentImeCfg->imeId.empty() || !IsImeInstalled(userId, currentImeCfg->bundleName, currentImeCfg->extName)) {
        auto newIme = GetDefaultIme();
        newIme.subName = "";
        currentImeCfg->imeId.empty()
            ? ImeCfgManager::GetInstance().AddImeCfg({ userId, newIme.imeId, "", false })
            : ImeCfgManager::GetInstance().ModifyImeCfg({ userId, newIme.imeId, "", false});
        return std::make_shared<ImeNativeCfg>(newIme);
    }
    return currentImeCfg;
}

int32_t ImeInfoInquirer::GetInputMethodConfig(const int32_t userId, AppExecFwk::ElementName &inputMethodConfig)
{
    IMSA_HILOGD("userId: %{public}d.", userId);
    if (systemConfig_.systemInputMethodConfigAbility.empty()) {
        IMSA_HILOGW("inputMethodConfig systemInputMethodConfigAbility is nullptr");
        return ErrorCode::NO_ERROR;
    }
    std::string bundleName = systemConfig_.systemInputMethodConfigAbility;
    std::string moduleName;
    std::string abilityName;
    auto pos = bundleName.find('/');
    if (pos != std::string::npos) {
        abilityName = (pos + 1 < bundleName.size()) ? bundleName.substr(pos + 1) : "";
        bundleName = bundleName.substr(0, pos);
    }
    pos = abilityName.find('/');
    if (pos != std::string::npos) {
        moduleName = abilityName.substr(0, pos);
        abilityName = (pos + 1 < abilityName.size()) ? abilityName.substr(pos + 1) : "";
    }
    inputMethodConfig.SetBundleName(std::move(bundleName));
    inputMethodConfig.SetModuleName(std::move(moduleName));
    inputMethodConfig.SetAbilityName(std::move(abilityName));
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::GetDefaultInputMethod(const int32_t userId, std::shared_ptr<Property> &prop, bool isBrief)
{
    IMSA_HILOGD("userId: %{public}d.", userId);
    auto defaultIme = GetDefaultImeCfgProp();
    if (defaultIme == nullptr) {
        IMSA_HILOGE("abnormal default ime cfg!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    FullImeInfo imeInfo;
    if (FullImeInfoManager::GetInstance().Get(userId, defaultIme->name, imeInfo)) {
        prop = std::make_shared<Property>(imeInfo.prop);
        prop->id = defaultIme->id;
        return ErrorCode::NO_ERROR;
    }

    IMSA_HILOGD("%{public}d get %{public}s form bms.", userId, defaultIme->name.c_str());
    if (isBrief) {
        IMSA_HILOGD("get brief info.");
        if (prop == nullptr) {
            prop = std::make_shared<Property>();
        }
        prop->name = defaultIme->name;
        prop->id = defaultIme->id;
        return ErrorCode::NO_ERROR;
    }
    prop = GetImeProperty(userId, defaultIme->name, defaultIme->id);
    if (prop == nullptr) {
        IMSA_HILOGE("prop is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetDefaultImeInfo(int32_t userId)
{
    auto defaultIme = GetDefaultImeCfgProp();
    if (defaultIme == nullptr) {
        IMSA_HILOGE("defaultIme is nullptr!");
        return nullptr;
    }
    auto info = GetImeInfo(userId, defaultIme->name, "");
    if (info == nullptr) {
        IMSA_HILOGE("userId: %{public}d, bundleName: %{public}s getImeInfoFromBundleMgr failed!", userId,
            defaultIme->name.c_str());
        return nullptr;
    }
    if (!info->isNewIme) {
        info->prop.id = defaultIme->id;
        auto it = std::find_if(info->subProps.begin(), info->subProps.end(),
            [defaultIme](const SubProperty &subProp) { return subProp.id == defaultIme->id; });
        if (it != info->subProps.end()) {
            info->subProp = *it;
        }
    }
    return info;
}

ImeNativeCfg ImeInfoInquirer::GetDefaultIme()
{
    ImeNativeCfg imeCfg;
    if (!systemConfig_.defaultInputMethod.empty()) {
        IMSA_HILOGI("defaultInputMethod: %{public}s.", systemConfig_.defaultInputMethod.c_str());
        imeCfg.imeId = systemConfig_.defaultInputMethod;
    } else {
        char value[CONFIG_LEN] = { 0 };
        auto code = GetParameter(DEFAULT_IME_KEY, "", value, CONFIG_LEN);
        imeCfg.imeId = code > 0 ? value : "";
    }
    auto pos = imeCfg.imeId.find('/');
    if (pos == std::string::npos || pos + 1 >= imeCfg.imeId.size()) {
        IMSA_HILOGE("defaultIme: %{public}s is abnormal!", imeCfg.imeId.c_str());
        return {};
    }
    imeCfg.bundleName = imeCfg.imeId.substr(0, pos);
    imeCfg.extName = imeCfg.imeId.substr(pos + 1);
    return imeCfg;
}

sptr<OHOS::AppExecFwk::IBundleMgr> ImeInfoInquirer::GetBundleMgr()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGE("systemAbilityManager is nullptr!");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        IMSA_HILOGE("remoteObject is nullptr!");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
}

std::shared_ptr<SubProperty> ImeInfoInquirer::FindTargetSubtypeByCondition(const std::vector<SubProperty> &subProps,
    const Condition &condition)
{
    auto it = subProps.end();
    switch (condition) {
        case Condition::UPPER: {
            it = std::find_if(subProps.begin(), subProps.end(),
                [](const SubProperty &subProp) { return subProp.mode == "upper"; });
            break;
        }
        case Condition::LOWER: {
            it = std::find_if(subProps.begin(), subProps.end(),
                [](const SubProperty &subProp) { return subProp.mode == "lower"; });
            break;
        }
        case Condition::ENGLISH: {
            it = std::find_if(subProps.begin(), subProps.end(),
                [](const SubProperty &subProp) { return subProp.language == "english" && subProp.mode == "lower"; });
            break;
        }
        case Condition::CHINESE: {
            it = std::find_if(subProps.begin(), subProps.end(),
                [](const SubProperty &subProp) { return subProp.language == "chinese"; });
            break;
        }
        default: {
            break;
        }
    }
    if (it == subProps.end()) {
        return nullptr;
    }
    return std::make_shared<SubProperty>(*it);
}

bool ImeInfoInquirer::ParseSubtypeProfile(const std::vector<std::string> &profiles, SubtypeCfg &subtypeCfg)
{
    if (profiles.empty() || profiles.size() != SUBTYPE_PROFILE_NUM) {
        IMSA_HILOGE("profiles size: %{public}zu!", profiles.size());
        return false;
    }
    return subtypeCfg.Unmarshall(profiles[0]);
}

std::shared_ptr<Property> ImeInfoInquirer::GetDefaultImeCfgProp()
{
    auto ime = GetDefaultIme();
    if (ime.bundleName.empty() || ime.extName.empty()) {
        IMSA_HILOGE("defaultIme is abnormal!");
        return nullptr;
    }
    auto defaultIme = std::make_shared<Property>();
    defaultIme->name = ime.bundleName;
    defaultIme->id = ime.extName;
    return defaultIme;
}

std::shared_ptr<ImeNativeCfg> ImeInfoInquirer::GetDefaultImeCfg()
{
    auto ime = GetDefaultIme();
    if (ime.bundleName.empty() || ime.extName.empty()) {
        IMSA_HILOGE("defaultIme is abnormal!");
        return nullptr;
    }
    return std::make_shared<ImeNativeCfg>(ime);
}

std::shared_ptr<ResourceManager> ImeInfoInquirer::GetResMgr(const std::string &resourcePath)
{
    if (resourcePath.empty()) {
        IMSA_HILOGE("resourcePath is empty!");
        return nullptr;
    }
    std::shared_ptr<ResourceManager> resMgr(CreateResourceManager());
    if (resMgr == nullptr) {
        IMSA_HILOGE("resMgr is nullptr!");
        return nullptr;
    }
    resMgr->AddResource(resourcePath.c_str());
    std::unique_ptr<ResConfig> resConfig(CreateResConfig());
    if (resConfig == nullptr) {
        IMSA_HILOGE("resConfig is nullptr!");
        return nullptr;
    }
    std::map<std::string, std::string> configs;
    OHOS::Global::I18n::LocaleInfo locale(Global::I18n::LocaleConfig::GetSystemLocale(), configs);
    resConfig->SetLocaleInfo(locale.GetLanguage().c_str(), locale.GetScript().c_str(), locale.GetRegion().c_str());
    resMgr->UpdateResConfig(*resConfig);
    return resMgr;
}

int32_t ImeInfoInquirer::QueryFullImeInfo(std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> &fullImeInfos)
{
    auto userIds = OsAccountAdapter::QueryActiveOsAccountIds();  // todo 全部用户
    if (userIds.empty()) {
        return ErrorCode::ERROR_OS_ACCOUNT;
    }
    for (auto &userId : userIds) {
        std::vector<FullImeInfo> infos;
        auto errNo = QueryFullImeInfo(userId, infos);
        if (errNo != ErrorCode::NO_ERROR) {
            continue;
        }
        fullImeInfos.emplace_back(userId, infos);
    }
    if (fullImeInfos.empty()) {
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::QueryFullImeInfo(int32_t userId, std::vector<FullImeInfo> &imeInfo, bool needSubProps)
{
    std::vector<ExtensionAbilityInfo> extInfos;
    auto ret = ImeInfoInquirer::GetInstance().QueryImeExtInfos(userId, extInfos);
    if (!ret || extInfos.empty()) {
        IMSA_HILOGE("%{public}d QueryImeExtInfos failed!", userId);
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    std::map<std::string, std::vector<ExtensionAbilityInfo>> tempExtInfos;
    for (const auto &extInfo : extInfos) {
        if (IsTempInputMethod(extInfo)) {
            continue;
        }
        auto it = tempExtInfos.find(extInfo.bundleName);
        if (it != tempExtInfos.end()) {
            it->second.push_back(extInfo);
            continue;
        }
        tempExtInfos.insert({ extInfo.bundleName, { extInfo } });
    }

    for (const auto &extInfo : tempExtInfos) {
        FullImeInfo info;
        auto errNo = GetFullImeInfo(userId, extInfo.second, info, needSubProps);
        if (errNo != ErrorCode::NO_ERROR) {
            return errNo;
        }
        imeInfo.push_back(info);
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::GetFullImeInfo(int32_t userId, const std::string &bundleName, FullImeInfo &imeInfo)
{
    std::vector<ExtensionAbilityInfo> extInfos;
    auto ret = ImeInfoInquirer::GetInstance().QueryImeExtInfos(userId, extInfos);
    if (!ret || extInfos.empty()) {
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    std::vector<ExtensionAbilityInfo> tempExtInfos;
    for (const auto &extInfo : extInfos) {
        if (IsTempInputMethod(extInfo)) {
            continue;
        }
        if (extInfo.bundleName == bundleName) {
            tempExtInfos.push_back(extInfo);
        }
    }
    return GetFullImeInfo(userId, tempExtInfos, imeInfo);
}

int32_t ImeInfoInquirer::GetFullImeInfo(
    int32_t userId, const std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> &extInfos, FullImeInfo &imeInfo, bool needSubProps)
{
    if (extInfos.empty()) {
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    if (needSubProps) {
        imeInfo.isNewIme = IsNewExtInfos(extInfos);
        auto ret = imeInfo.isNewIme ? ListInputMethodSubtype(userId, extInfos[0], imeInfo.subProps)
                                    : ListInputMethodSubtype(userId, extInfos, imeInfo.subProps);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("[%{public}d,%{public}s] list Subtype failed!", userId, extInfos[0].bundleName.c_str());
            return ret;
        }
    }
    imeInfo.tokenId = extInfos[0].applicationInfo.accessTokenId;
    imeInfo.prop.name = extInfos[0].bundleName;
    imeInfo.prop.id = extInfos[0].name;
    imeInfo.prop.label = GetTargetString(extInfos[0], ImeTargetString::LABEL, userId);
    imeInfo.prop.labelId = extInfos[0].applicationInfo.labelId;
    imeInfo.prop.iconId = extInfos[0].applicationInfo.iconId;
    BundleInfo bundleInfo;
    if (!GetBundleInfoByBundleName(userId, imeInfo.prop.name, bundleInfo)) {
        IMSA_HILOGE("[%{public}d,%{public}s] GetBundleInfoByBundleName failed!", userId, imeInfo.prop.name.c_str());
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    imeInfo.appId = bundleInfo.signatureInfo.appIdentifier;
    imeInfo.versionCode = bundleInfo.versionCode;
    imeInfo.installTime = std::to_string(bundleInfo.installTime);
    return ErrorCode::NO_ERROR;
}

bool ImeInfoInquirer::IsInputMethod(int32_t userId, const std::string &bundleName)
{
    auto bmg = GetBundleMgr();
    if (bmg == nullptr) {
        return false;
    }
    BundleInfo bundleInfo;
    auto ret = bmg->GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO, bundleInfo, userId);
    if (!ret) {
        return false;
    }
    for (const auto &extInfo : bundleInfo.extensionInfos) {
        if (extInfo.type == ExtensionAbilityType::INPUTMETHOD) {
            return true;
        }
    }
    return false;
}
 
bool ImeInfoInquirer::IsTempInputMethod(const ExtensionAbilityInfo &extInfo)
{
    auto iter = std::find_if(extInfo.metadata.begin(), extInfo.metadata.end(),
        [](const Metadata &metadata) {
            return metadata.name == TEMPORARY_INPUT_METHOD_METADATA_NAME;
        });
    return iter != extInfo.metadata.end();
}

std::vector<std::string> ImeInfoInquirer::GetRunningIme(int32_t userId)
{
    std::vector<std::string> bundleNames;
    std::vector<RunningProcessInfo> infos;
    AppMgrClient client;
    auto ret = client.GetProcessRunningInfosByUserId(infos, userId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("GetAllRunningProcesses failed, ret: %{public}d!", ret);
        return bundleNames;
    }
    for (const auto &info : infos) {
        if (info.extensionType_ == ExtensionAbilityType::INPUTMETHOD && !info.bundleNames.empty()) {
            bundleNames.push_back(info.bundleNames[0]);
        }
    }
    return bundleNames;
}

bool ImeInfoInquirer::IsDefaultImeSet(int32_t userId)
{
    return ImeCfgManager::GetInstance().IsDefaultImeSet(userId);
}

bool ImeInfoInquirer::IsRunningIme(int32_t userId, const std::string &bundleName)
{
    auto bundleNames = GetRunningIme(userId);
    auto it = std::find_if(bundleNames.begin(), bundleNames.end(),
        [&bundleName](const std::string &bundleNameTemp) { return bundleName == bundleNameTemp; });
    return it != bundleNames.end();
}

bool ImeInfoInquirer::GetImeAppId(int32_t userId, const std::string &bundleName, std::string &appId)
{
    FullImeInfo imeInfo;
    if (FullImeInfoManager::GetInstance().Get(userId, bundleName, imeInfo) && !imeInfo.appId.empty()) {
        appId = imeInfo.appId;
        return true;
    }
    BundleInfo bundleInfo;
    if (!GetBundleInfoByBundleName(userId, bundleName, bundleInfo)) {
        return false;
    }
    appId = bundleInfo.signatureInfo.appIdentifier;
    return !appId.empty();
}

bool ImeInfoInquirer::GetImeVersionCode(int32_t userId, const std::string &bundleName, uint32_t &versionCode)
{
    FullImeInfo imeInfo;
    if (FullImeInfoManager::GetInstance().Get(userId, bundleName, imeInfo)) {
        versionCode = imeInfo.versionCode;
        return true;
    }
    BundleInfo bundleInfo;
    if (!GetBundleInfoByBundleName(userId, bundleName, bundleInfo)) {
        return false;
    }
    versionCode = bundleInfo.versionCode;
    return true;
}

bool ImeInfoInquirer::GetBundleInfoByBundleName(
    int32_t userId, const std::string &bundleName, AppExecFwk::BundleInfo &bundleInfo)
{
    auto bundleMgr = GetBundleMgr();
    if (bundleMgr == nullptr) {
        IMSA_HILOGE("failed to get bundleMgr!");
        return false;
    }
    auto ret = bundleMgr->GetBundleInfo(
        bundleName, static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO), bundleInfo, userId);
    if (!ret) {
        IMSA_HILOGE("failed to get bundle info");
        return false;
    }
    return true;
}

std::string ImeInfoInquirer::GetTargetString(
    const AppExecFwk::ExtensionAbilityInfo &extension, ImeTargetString target, int32_t userId)
{
    if (target == ImeTargetString::LABEL) {
        if (extension.labelId != DEFAULT_BMS_VALUE) {
            return GetStringById(extension.bundleName, extension.moduleName, extension.labelId, userId);
        }
        IMSA_HILOGD("Extension label is empty, get application label");
        return GetStringById(extension.bundleName, extension.applicationInfo.labelResource.moduleName,
            extension.applicationInfo.labelResource.id, userId);
    }
    if (target == ImeTargetString::DESCRIPTION) {
        if (extension.descriptionId != DEFAULT_BMS_VALUE) {
            return GetStringById(extension.bundleName, extension.moduleName, extension.descriptionId, userId);
        }
        IMSA_HILOGD("extension description is empty, get application description");
        return GetStringById(extension.bundleName, extension.applicationInfo.descriptionResource.moduleName,
            extension.applicationInfo.descriptionResource.id, userId);
    }
    IMSA_HILOGD("No match target string");
    return "";
}

bool ImeInfoInquirer::IsInputMethodExtension(pid_t pid)
{
    RunningProcessInfo info;
    AppMgrClient client;
    client.GetRunningProcessInfoByPid(pid, info);
    return info.extensionType_ == ExtensionAbilityType::INPUTMETHOD;
}
} // namespace MiscServices
} // namespace OHOS