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

#include <algorithm>
#include <string>

#include "application_info.h"
#include "bundle_mgr_client_impl.h"
#include "file_operator.h"
#include "global.h"
#include "if_system_ability_manager.h"
#include "ime_cfg_manager.h"
#include "input_method_info.h"
#include "input_type_manager.h"
#include "iservice_registry.h"
#include "parameter.h"
#include "string_ex.h"
#include "system_ability.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
namespace {
using namespace OHOS::AppExecFwk;
constexpr const char *SUBTYPE_PROFILE_METADATA_NAME = "ohos.extension.input_method";
constexpr uint32_t SUBTYPE_PROFILE_NUM = 1;
constexpr const char *DEFAULT_IME_KEY = "persist.sys.default_ime";
constexpr int32_t CONFIG_LEN = 128;
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
        IMSA_HILOGE("Parse systemConfig failed");
        return;
    }
}

bool ImeInfoInquirer::IsEnableInputMethod()
{
    return systemConfig_.enableInputMethodFeature;
}

bool ImeInfoInquirer::IsEnableSecurityMode()
{
    return systemConfig_.enableFullExperienceFeature;
}

bool ImeInfoInquirer::QueryImeExtInfos(const int32_t userId, std::vector<ExtensionAbilityInfo> &infos)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    auto bundleMgr = GetBundleMgr();
    if (bundleMgr == nullptr) {
        IMSA_HILOGE("GetBundleMgr failed");
        return false;
    }
    if (!bundleMgr->QueryExtensionAbilityInfos(ExtensionAbilityType::INPUTMETHOD, userId, infos)) {
        IMSA_HILOGF("Query extension infos failed from bundleMgr!");
        return false;
    }
    return true;
}

int32_t ImeInfoInquirer::GetExtInfosByBundleName(
    const int32_t userId, const std::string &bundleName, std::vector<AppExecFwk::ExtensionAbilityInfo> &extInfos)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s", userId, bundleName.c_str());
    std::vector<AppExecFwk::ExtensionAbilityInfo> tempExtInfos;
    if (!QueryImeExtInfos(userId, tempExtInfos)) {
        IMSA_HILOGE("QueryImeExtInfos failed!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    for (const auto &extInfo : tempExtInfos) {
        if (extInfo.bundleName == bundleName) {
            extInfos.emplace_back(extInfo);
        }
    }
    if (extInfos.empty()) {
        IMSA_HILOGE("bundleName: %{public}s extInfos is empty", bundleName.c_str());
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetImeInfo(
    int32_t userId, const std::string &bundleName, const std::string &subName)
{
    auto info = GetImeInfoFromCache(userId, bundleName, subName);
    return info == nullptr ? GetImeInfoFromBundleMgr(userId, bundleName, subName) : info;
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetImeInfoFromCache(
    const int32_t userId, const std::string &bundleName, const std::string &subName)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s, subName: %{public}s", userId, bundleName.c_str(),
        subName.c_str());
    auto info = GetCurrentImeInfo();
    if (info == nullptr || bundleName != info->prop.name) {
        return nullptr;
    }
    if (subName == info->subProp.id) {
        return std::make_shared<ImeInfo>(*info);
    }

    auto newInfo = std::make_shared<ImeInfo>(*info);
    auto it = std::find_if(newInfo->subProps.begin(), newInfo->subProps.end(),
        [&subName](const SubProperty &subProp) { return subProp.id == subName; });
    if (it == newInfo->subProps.end()) {
        IMSA_HILOGE("Find subName: %{public}s failed", subName.c_str());
        return nullptr;
    }
    newInfo->subProp = *it;
    // old ime, make the id of prop same with the id of subProp.
    if (!newInfo->isNewIme) {
        newInfo->prop.id = newInfo->subProp.id;
    }
    return newInfo;
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetImeInfoFromBundleMgr(
    const int32_t userId, const std::string &bundleName, const std::string &subName)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s, subName: %{public}s", userId, bundleName.c_str(),
        subName.c_str());
    std::vector<AppExecFwk::ExtensionAbilityInfo> extInfos;
    auto ret = ImeInfoInquirer::GetInstance().GetExtInfosByBundleName(userId, bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR || extInfos.empty()) {
        IMSA_HILOGE("userId: %{public}d getExtInfosByBundleName %{public}s failed", userId, bundleName.c_str());
        return nullptr;
    }
    auto info = std::make_shared<ImeInfo>();
    info->moduleName = extInfos[0].moduleName;
    info->prop.name = extInfos[0].bundleName;
    info->prop.id = extInfos[0].name;
    info->prop.label =
        GetStringById(extInfos[0].bundleName, extInfos[0].moduleName, extInfos[0].applicationInfo.labelId, userId);
    info->prop.labelId = extInfos[0].applicationInfo.labelId;
    info->prop.iconId = extInfos[0].applicationInfo.iconId;

    std::vector<SubProperty> subProps;
    ExtensionAbilityInfo extInfo;
    info->isNewIme = GetExtInfoContainSubtypeCfg(extInfos, extInfo);
    ret = info->isNewIme ? ListInputMethodSubtype(userId, extInfo, subProps)
                         : ListInputMethodSubtype(userId, extInfos, subProps);
    if (ret != ErrorCode::NO_ERROR || subProps.empty()) {
        IMSA_HILOGE("userId: %{public}d listInputMethodSubtype failed", userId);
        return nullptr;
    }
    info->subProps = subProps;
    if (subName.empty()) {
        info->subProp = subProps[0];
    } else {
        auto it = std::find_if(subProps.begin(), subProps.end(),
            [&subName](const SubProperty &subProp) { return subProp.id == subName; });
        if (it == subProps.end()) {
            IMSA_HILOGE("Find subName: %{public}s failed", subName.c_str());
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

void ImeInfoInquirer::SetCurrentImeInfo(std::shared_ptr<ImeInfo> info)
{
    std::lock_guard<std::mutex> lock(currentImeInfoLock_);
    currentImeInfo_ = std::move(info);
}

void ImeInfoInquirer::InitCache(int32_t userId)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    auto info = GetImeInfoFromBundleMgr(userId, currentImeCfg->bundleName, currentImeCfg->subName);
    if (info == nullptr) {
        IMSA_HILOGE("userId: %{public}d, bundleName: %{public}s, subName: %{public}s getImeInfoFromBundleMgr failed",
            userId, currentImeCfg->bundleName.c_str(), currentImeCfg->subName.c_str());
        return;
    }
    SetCurrentImeInfo(info);
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetCurrentImeInfo()
{
    std::lock_guard<std::mutex> lock(currentImeInfoLock_);
    return currentImeInfo_;
}

void ImeInfoInquirer::RefreshCurrentImeInfo(int32_t userId)
{
    IMSA_HILOGD("run in");
    std::lock_guard<std::mutex> lock(currentImeInfoLock_);
    if (currentImeInfo_ == nullptr) {
        IMSA_HILOGD("currentImeInfo is nullptr");
        return;
    }
    for (auto &subProp : currentImeInfo_->subProps) {
        subProp.label = GetStringById(subProp.name, currentImeInfo_->moduleName, subProp.labelId, userId);
        if (currentImeInfo_->subProp.id == subProp.id) {
            currentImeInfo_->subProp.label = subProp.label;
        }
    }
    currentImeInfo_->prop.label =
        GetStringById(currentImeInfo_->prop.name, currentImeInfo_->moduleName, currentImeInfo_->prop.labelId, userId);
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
    IMSA_HILOGD("userId: %{public}d", userId);
    std::vector<ExtensionAbilityInfo> extensionInfos;
    if (!QueryImeExtInfos(userId, extensionInfos)) {
        IMSA_HILOGE("userId: %{public}d queryImeExtInfos failed", userId);
        return {};
    }
    std::vector<InputMethodInfo> properties;
    for (const auto &extension : extensionInfos) {
        auto applicationInfo = extension.applicationInfo;
        auto label = GetStringById(extension.bundleName, extension.moduleName, applicationInfo.labelId, userId);
        auto description =
            GetStringById(extension.bundleName, extension.moduleName, applicationInfo.descriptionId, userId);
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

int32_t ImeInfoInquirer::ListInputMethod(
    int32_t userId, InputMethodStatus status, std::vector<Property> &props, bool enableOn)
{
    IMSA_HILOGD("userId: %{public}d, status: %{public}d", userId, status);
    if (status == InputMethodStatus::ALL) {
        return ListInputMethod(userId, props);
    }
    if (status == InputMethodStatus::ENABLE) {
        return ListEnabledInputMethod(userId, props, enableOn);
    }
    if (status == InputMethodStatus::DISABLE) {
        return ListDisabledInputMethod(userId, props, enableOn);
    }
    return ErrorCode::ERROR_BAD_PARAMETERS;
}

int32_t ImeInfoInquirer::ListInputMethod(const int32_t userId, std::vector<Property> &props)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    std::vector<ExtensionAbilityInfo> extensionInfos;
    if (!QueryImeExtInfos(userId, extensionInfos)) {
        IMSA_HILOGE("QueryImeExtInfos failed!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    for (const auto &extension : extensionInfos) {
        auto it = std::find_if(props.begin(), props.end(),
            [&extension](const Property &prop) { return prop.name == extension.bundleName; });
        if (it != props.end()) {
            continue;
        }
        props.push_back({ .name = extension.bundleName,
            .id = extension.name,
            .label =
                GetStringById(extension.bundleName, extension.moduleName, extension.applicationInfo.labelId, userId),
            .labelId = extension.applicationInfo.labelId,
            .iconId = extension.applicationInfo.iconId });
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListEnabledInputMethod(const int32_t userId, std::vector<Property> &props, bool enableOn)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    int32_t ret = ListInputMethod(userId, props);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d listInputMethod failed", userId);
        return ret;
    }
    if (enableOn) {
        IMSA_HILOGD("enable on");
        std::vector<std::string> enableVec;
        ret = EnableImeDataParser::GetInstance()->GetEnableData(EnableImeDataParser::ENABLE_IME, enableVec, userId);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("Get enable data failed;");
            return ret;
        }
        auto info = GetDefaultImeInfo(userId);
        if (info != nullptr) {
            enableVec.insert(enableVec.begin(), info->prop.name);
        }

        auto newEnd = std::remove_if(props.begin(), props.end(), [&enableVec](const auto &prop) {
            return std::find(enableVec.begin(), enableVec.end(), prop.name) == enableVec.end();
        });
        props.erase(newEnd, props.end());
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListDisabledInputMethod(const int32_t userId, std::vector<Property> &props, bool enableOn)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    if (!enableOn) {
        IMSA_HILOGD("Enable mode off, get disabled ime.");
        return ErrorCode::NO_ERROR;
    }

    auto ret = ListInputMethod(userId, props);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d listInputMethod failed", userId);
        return ret;
    }

    std::vector<std::string> enableVec;
    ret = EnableImeDataParser::GetInstance()->GetEnableData(EnableImeDataParser::ENABLE_IME, enableVec, userId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Get enable data failed;");
        return ret;
    }
    auto info = GetDefaultImeInfo(userId);
    if (info != nullptr) {
        enableVec.insert(enableVec.begin(), info->prop.name);
    }

    auto newEnd = std::remove_if(props.begin(), props.end(), [&enableVec](const auto &prop) {
        return std::find(enableVec.begin(), enableVec.end(), prop.name) != enableVec.end();
    });
    props.erase(newEnd, props.end());
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::GetNextSwitchInfo(SwitchInfo &switchInfo, int32_t userId, bool enableOn)
{
    std::vector<Property> props;
    auto ret = ListEnabledInputMethod(userId, props, enableOn);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d ListEnabledInputMethod failed", userId);
        return ret;
    }
    auto currentImeBundle = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId)->bundleName;
    auto iter = std::find_if(props.begin(), props.end(),
        [&currentImeBundle](const Property &property) { return property.name == currentImeBundle; });
    if (iter == props.end()) {
        IMSA_HILOGE("Can not found current ime in enable list");
        auto info = GetDefaultImeInfo(userId);
        if (info != nullptr) {
            switchInfo.bundleName = info->prop.name;
            return ErrorCode::NO_ERROR;
        }
        IMSA_HILOGE("bundle manager error");
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    auto nextIter = std::next(iter);
    switchInfo.bundleName = nextIter == props.end() ? props[0].name : nextIter->name;
    IMSA_HILOGD("Next ime: %{public}s", switchInfo.bundleName.c_str());
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListInputMethodSubtype(
    int32_t userId, const std::string &bundleName, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s", userId, bundleName.c_str());
    std::vector<ExtensionAbilityInfo> extInfos;
    auto ret = GetExtInfosByBundleName(userId, bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d getExtInfosByBundleName %{public}s failed", userId, bundleName.c_str());
        return ret;
    }
    ExtensionAbilityInfo extInfo;
    return GetExtInfoContainSubtypeCfg(extInfos, extInfo) ? ListInputMethodSubtype(userId, extInfo, subProps)
                                                          : ListInputMethodSubtype(userId, extInfos, subProps);
}

int32_t ImeInfoInquirer::ListCurrentInputMethodSubtype(int32_t userId, std::vector<SubProperty> &subProps)
{
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    IMSA_HILOGD("currentIme: %{public}s", currentImeCfg->imeId.c_str());
    return ListInputMethodSubtype(userId, currentImeCfg->bundleName, subProps);
}

bool ImeInfoInquirer::GetExtInfoContainSubtypeCfg(
    const std::vector<ExtensionAbilityInfo> &extInfos, ExtensionAbilityInfo &extInfo)
{
    if (extInfos.empty()) {
        IMSA_HILOGE("extInfos is empty");
        return false;
    }
    for (const auto &info : extInfos) {
        auto iter = std::find_if(info.metadata.begin(), info.metadata.end(),
            [](const Metadata &metadata) { return metadata.name == SUBTYPE_PROFILE_METADATA_NAME; });
        if (iter != extInfo.metadata.end()) {
            extInfo = info;
            return true;
        }
    }
    return false;
}

int32_t ImeInfoInquirer::ListInputMethodSubtype(
    const int32_t userId, const std::vector<ExtensionAbilityInfo> &extInfos, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("oldIme, userId: %{public}d", userId);
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

int32_t ImeInfoInquirer::ListInputMethodSubtype(
    const int32_t userId, const ExtensionAbilityInfo &extInfo, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("newIme, userId: %{public}d", userId);
    OHOS::AppExecFwk::BundleMgrClientImpl clientImpl;
    std::vector<std::string> profiles;
    if (!clientImpl.GetResConfigFile(extInfo, SUBTYPE_PROFILE_METADATA_NAME, profiles)) {
        IMSA_HILOGE("GetProfileFromExtension failed");
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    SubtypeCfg subtypeCfg;
    if (!ParseSubType(profiles, subtypeCfg)) {
        IMSA_HILOGE("ParseSubTypeCfg failed");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto subtypes = subtypeCfg.subtypes;
    IMSA_HILOGD("subtypes size: %{public}zu", subtypes.size());
    for (const auto &subtype : subtypes) {
        // subtype which provides a particular input type should not appear in the subtype list
        if (InputTypeManager::GetInstance().IsInputType({ extInfo.bundleName, subtype.id })) {
            continue;
        }
        SubProperty subProp{ .name = extInfo.bundleName,
            .label = subtype.label,
            .id = subtype.id,
            .icon = subtype.icon,
            .mode = subtype.mode,
            .locale = subtype.locale };
        auto pos = subProp.label.find(':');
        if (pos != std::string::npos && pos + 1 < subProp.label.size()) {
            subProp.labelId = atoi(subProp.label.substr(pos + 1).c_str());
            subProp.label = GetStringById(extInfo.bundleName, extInfo.moduleName, subProp.labelId, userId);
        }
        pos = subProp.icon.find(':');
        if (pos != std::string::npos && pos + 1 < subProp.icon.size()) {
            subProp.iconId = atoi(subProp.icon.substr(pos + 1).c_str());
        }
        CovertToLanguage(subProp.locale, subProp.language);
        subProps.emplace_back(subProp);
    }
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

std::string ImeInfoInquirer::GetStringById(
    const std::string &bundleName, const std::string &moduleName, int32_t labelId, int32_t userId)
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

std::shared_ptr<Property> ImeInfoInquirer::GetImeByBundleName(int32_t userId, const std::string &bundleName)
{
    IMSA_HILOGD("run in, bundleName: %{public}s", bundleName.c_str());
    std::vector<AppExecFwk::ExtensionAbilityInfo> extInfos;
    auto ret = ImeInfoInquirer::GetInstance().GetExtInfosByBundleName(userId, bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR || extInfos.empty()) {
        IMSA_HILOGE("userId: %{public}d getExtInfosByBundleName %{public}s failed", userId, bundleName.c_str());
        return nullptr;
    }
    Property prop = { .name = extInfos[0].bundleName,
        .id = extInfos[0].name,
        .label = extInfos[0].applicationInfo.label,
        .labelId = extInfos[0].applicationInfo.labelId,
        .iconId = extInfos[0].applicationInfo.iconId };
    return std::make_shared<Property>(prop);
}

std::shared_ptr<Property> ImeInfoInquirer::GetCurrentInputMethod(int32_t userId)
{
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    IMSA_HILOGD("currentIme: %{public}s", currentImeCfg->imeId.c_str());
    std::vector<AppExecFwk::ExtensionAbilityInfo> extInfos;
    auto ret = ImeInfoInquirer::GetInstance().GetExtInfosByBundleName(userId, currentImeCfg->bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR || extInfos.empty()) {
        IMSA_HILOGE(
            "userId: %{public}d getExtInfosByBundleName %{public}s failed", userId, currentImeCfg->bundleName.c_str());
        return nullptr;
    }
    Property prop = { .name = extInfos[0].bundleName,
        .id = currentImeCfg->extName, // if old ime, the extInfos[0].name maybe not same with currentImeExtName
        .label =
            GetStringById(extInfos[0].bundleName, extInfos[0].moduleName, extInfos[0].applicationInfo.labelId, userId),
        .labelId = extInfos[0].applicationInfo.labelId,
        .iconId = extInfos[0].applicationInfo.iconId };
    return std::make_shared<Property>(prop);
}

std::shared_ptr<SubProperty> ImeInfoInquirer::GetCurrentSubtype(int32_t userId)
{
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    IMSA_HILOGD("currentIme: %{public}s", currentImeCfg->imeId.c_str());
    std::vector<SubProperty> subProps = {};
    auto ret = ListInputMethodSubtype(userId, currentImeCfg->bundleName, subProps);
    if (ret != ErrorCode::NO_ERROR || subProps.empty()) {
        IMSA_HILOGE("userId: %{public}d listInputMethodSubtype by bundleName: %{public}s failed", userId,
            currentImeCfg->bundleName.c_str());
        return nullptr;
    }
    auto it = std::find_if(subProps.begin(), subProps.end(),
        [&currentImeCfg](const SubProperty &subProp) { return subProp.id == currentImeCfg->subName; });
    if (it != subProps.end()) {
        return std::make_shared<SubProperty>(*it);
    }
    IMSA_HILOGE("Find subName: %{public}s failed", currentImeCfg->subName.c_str());
    return std::make_shared<SubProperty>(subProps[0]);
}

bool ImeInfoInquirer::IsImeInstalled(const int32_t userId, const std::string &bundleName, const std::string &extName)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s, extName: %{public}s", userId, bundleName.c_str(),
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
    IMSA_HILOGD("userId: %{public}d, currentIme: %{public}s", userId, currentImeCfg->imeId.c_str());
    if (currentImeCfg->imeId.empty() || !IsImeInstalled(userId, currentImeCfg->bundleName, currentImeCfg->extName)) {
        auto newIme = GetDefaultIme();
        auto info = GetDefaultImeInfo(userId);
        if (info == nullptr) {
            IMSA_HILOGE("GetDefaultImeInfo failed");
            newIme.subName = "";
        } else {
            newIme.subName = info->subProp.id;
            SetCurrentImeInfo(info);
        }
        currentImeCfg->imeId.empty()
            ? ImeCfgManager::GetInstance().AddImeCfg({ userId, newIme.imeId, newIme.subName })
            : ImeCfgManager::GetInstance().ModifyImeCfg({ userId, newIme.imeId, newIme.subName });
        return std::make_shared<ImeNativeCfg>(newIme);
    }
    // service start, user switch, set the currentImeInfo.
    InitCache(userId);
    return currentImeCfg;
}

int32_t ImeInfoInquirer::GetInputMethodConfig(const int32_t userId, AppExecFwk::ElementName &inputMethodConfig)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    if (systemConfig_.systemInputMethodConfigAbility.empty()) {
        IMSA_HILOGW("inputMethodConfig systemInputMethodConfigAbility is null");
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

int32_t ImeInfoInquirer::GetDefaultInputMethod(const int32_t userId, std::shared_ptr<Property> &prop)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    auto imeInfo = GetDefaultImeInfo(userId);
    if (imeInfo == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    IMSA_HILOGD("getDefaultInputMethod name: %{public}s", imeInfo->prop.name.c_str());
    prop->name = imeInfo->prop.name;
    prop->id = imeInfo->prop.id;
    prop->label = imeInfo->prop.label;
    prop->labelId = imeInfo->prop.labelId;
    prop->iconId = imeInfo->prop.iconId;
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetDefaultImeInfo(int32_t userId)
{
    auto defaultIme = GetDefaultImeCfgProp();
    if (defaultIme == nullptr) {
        IMSA_HILOGE("defaultIme is nullptr.");
        return nullptr;
    }
    auto info = GetImeInfoFromBundleMgr(userId, defaultIme->name, "");
    if (info == nullptr) {
        IMSA_HILOGE("userId: %{public}d, bundleName: %{public}s getImeInfoFromBundleMgr failed", userId,
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
        IMSA_HILOGI("defaultInputMethod: %{public}s", systemConfig_.defaultInputMethod.c_str());
        imeCfg.imeId = systemConfig_.defaultInputMethod;
    } else {
        char value[CONFIG_LEN] = { 0 };
        auto code = GetParameter(DEFAULT_IME_KEY, "", value, CONFIG_LEN);
        imeCfg.imeId = code > 0 ? value : "";
    }
    auto pos = imeCfg.imeId.find('/');
    if (pos == std::string::npos || pos + 1 >= imeCfg.imeId.size()) {
        IMSA_HILOGE("defaultIme: %{public}s is abnormal", imeCfg.imeId.c_str());
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
        IMSA_HILOGE("systemAbilityManager is nullptr");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        IMSA_HILOGE("remoteObject is nullptr");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
}

std::shared_ptr<SubProperty> ImeInfoInquirer::FindTargetSubtypeByCondition(
    const std::vector<SubProperty> &subProps, const Condition &condition)
{
    auto it = subProps.end();
    switch (condition) {
        case Condition::UPPER: {
            it = std::find_if(
                subProps.begin(), subProps.end(), [](const SubProperty &subProp) { return subProp.mode == "upper"; });
            break;
        }
        case Condition::LOWER: {
            it = std::find_if(
                subProps.begin(), subProps.end(), [](const SubProperty &subProp) { return subProp.mode == "lower"; });
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

bool ImeInfoInquirer::ParseSubType(const std::vector<std::string> &profiles, SubtypeCfg &subtypeCfg)
{
    if (profiles.empty() || profiles.size() != SUBTYPE_PROFILE_NUM) {
        IMSA_HILOGE("profiles size: %{public}zu", profiles.size());
        return false;
    }
    return subtypeCfg.Unmarshall(profiles[0]);
}

std::shared_ptr<Property> ImeInfoInquirer::GetDefaultImeCfgProp()
{
    auto ime = GetDefaultIme();
    if (ime.bundleName.empty() || ime.extName.empty()) {
        IMSA_HILOGE("defaultIme is abnormal");
        return nullptr;
    }
    auto defaultIme = std::make_shared<Property>();
    defaultIme->name = ime.bundleName;
    defaultIme->id = ime.extName;
    return defaultIme;
}
} // namespace MiscServices
} // namespace OHOS