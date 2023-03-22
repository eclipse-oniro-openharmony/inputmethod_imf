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
#include "global.h"
#include "if_system_ability_manager.h"
#include "ime_cfg_manager.h"
#include "input_method_info.h"
#include "iservice_registry.h"
#include "parameter.h"
#include "string_ex.h"
#include "system_ability.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
namespace {
using json = nlohmann::json;
using namespace OHOS::AppExecFwk;
constexpr const char *SUBTYPE_PROFILE_METADATA_NAME = "ohos.extension.input_method";
constexpr uint32_t SUBTYPE_PROFILE_NUM = 1;
constexpr uint32_t MAX_SUBTYPE_NUM = 10;
constexpr const char *DEFAULT_IME_KEY = "persist.sys.default_ime";
constexpr int32_t CONFIG_LEN = 128;
} // namespace
ImeInfoInquirer &ImeInfoInquirer::GetInstance()
{
    static ImeInfoInquirer instance;
    return instance;
}

int32_t ImeInfoInquirer::QueryImeExtInfos(const int32_t userId, std::vector<ExtensionAbilityInfo> &infos)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    auto bundleMgr = GetBundleMgr();
    if (bundleMgr == nullptr) {
        IMSA_HILOGE("GetBundleMgr failed");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (!bundleMgr->QueryExtensionAbilityInfos(ExtensionAbilityType::INPUTMETHOD, userId, infos)) {
        IMSA_HILOGE("userId: %{public}d queryExtensionAbilityInfos failed", userId);
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::GetExtInfosByBundleName(
    const int32_t userId, const std::string &bundleName, std::vector<AppExecFwk::ExtensionAbilityInfo> &extInfos)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s", userId, bundleName.c_str());
    std::vector<AppExecFwk::ExtensionAbilityInfo> tempExtInfos;
    auto ret = QueryImeExtInfos(userId, tempExtInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d queryImeExtInfos failed", userId);
        return ret;
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

int32_t ImeInfoInquirer::GetSwitchedImeInfo(
    const int32_t userId, const std::string &bundleName, const std::string &subName, ImeInfo &info)
{
    // Switch subType, get prop and subProp from currentImeInfos_
    if (bundleName == ImeCfgManager::GetInstance().GetCurrentImeBundleName(userId)) {
        return GetCurrentImeInfoFromNative(userId, subName, info);
    }
    return GetImeInfoFromBundleMgr(userId, bundleName, subName, info);
}

int32_t ImeInfoInquirer::GetCurrentImeInfoFromNative(const int32_t userId, const std::string &subName, ImeInfo &info)
{
    auto bundleName = ImeCfgManager::GetInstance().GetCurrentImeBundleName(userId);
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s, subName: %{public}s", userId, bundleName.c_str(),
        subName.c_str());
    std::lock_guard<std::mutex> lock(currentImeInfoLock_);
    // Exception protection
    if (currentImeInfo_ == nullptr || currentImeInfo_->prop.name != bundleName
        || currentImeInfo_->prop.id != ImeCfgManager::GetInstance().GetCurrentImeExtName(userId)) {
        IMSA_HILOGE("userId: %{public}d currentImeInfo_ is abnormal", userId);
        return GetImeInfoFromBundleMgr(userId, bundleName, subName, info);
    }
    info.subProps = currentImeInfo_->subProps;
    info.prop = currentImeInfo_->prop;
    info.isNewIme = currentImeInfo_->isNewIme;
    if (subName == ImeCfgManager::GetInstance().GetCurrentImeSubName(userId)
        && subName == currentImeInfo_->subProp.id) {
        info.subProp = currentImeInfo_->subProp;
        return ErrorCode::NO_ERROR;
    }

    auto it = std::find_if(currentImeInfo_->subProps.begin(), currentImeInfo_->subProps.end(),
        [subName](const SubProperty &subProp) { return subProp.id == subName; });
    if (it == currentImeInfo_->subProps.end()) {
        IMSA_HILOGE("Find subName: %{public}s failed", subName.c_str());
        return GetImeInfoFromBundleMgr(userId, bundleName, subName, info);
    }
    info.subProp = *it;
    // old ime, make the id of prop same with the id of subProp.
    if (!info.isNewIme) {
        info.prop.id = info.subProp.id;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::GetImeInfoFromBundleMgr(
    const int32_t userId, const std::string &bundleName, const std::string &subName, ImeInfo &info)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s, subName: %{public}s", userId, bundleName.c_str(),
        subName.c_str());
    std::vector<AppExecFwk::ExtensionAbilityInfo> extInfos;
    auto ret = ImeInfoInquirer::GetInstance().GetExtInfosByBundleName(userId, bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR || extInfos.empty()) {
        IMSA_HILOGE("userId: %{public}d getExtInfosByBundleName %{public}s failed", userId, bundleName.c_str());
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    info.prop = { .name = extInfos[0].bundleName,
        .id = extInfos[0].name,
        .label =
            GetStringById(extInfos[0].bundleName, extInfos[0].moduleName, extInfos[0].applicationInfo.labelId, userId),
        .labelId = extInfos[0].applicationInfo.labelId,
        .iconId = extInfos[0].applicationInfo.iconId };
    std::vector<SubProperty> subProps;
    info.isNewIme = IsNewExtInfos(extInfos);
    ret = info.isNewIme ? ListInputMethodSubtype(userId, extInfos[0], subProps)
                        : ListInputMethodSubtype(userId, extInfos, subProps);
    if (ret != ErrorCode::NO_ERROR || subProps.empty()) {
        IMSA_HILOGE("userId: %{public}d listInputMethodSubtype failed", userId);
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    info.subProps = subProps;
    if (subName.empty()) {
        info.subProp = subProps[0];
    } else {
        auto it = std::find_if(
            subProps.begin(), subProps.end(), [subName](const SubProperty &subProp) { return subProp.id == subName; });
        if (it == subProps.end()) {
            IMSA_HILOGE("Find subName: %{public}s failed", subName.c_str());
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        info.subProp = *it;
    }
    // old ime, make the id of prop same with the id of subProp.
    if (!info.isNewIme) {
        info.prop.id = info.subProp.id;
    }
    return ErrorCode::NO_ERROR;
}

void ImeInfoInquirer::SetCurrentImeInfo(const ImeInfo &info)
{
    std::lock_guard<std::mutex> lock(currentImeInfoLock_);
    currentImeInfo_ = std::make_shared<ImeInfo>(info);
}

void ImeInfoInquirer::SetCurrentImeInfo(const int32_t userId)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    auto bundleName = ImeCfgManager::GetInstance().GetCurrentImeBundleName(userId);
    auto subName = ImeCfgManager::GetInstance().GetCurrentImeSubName(userId);
    ImeInfo info;
    auto ret = GetImeInfoFromBundleMgr(userId, bundleName, subName, info);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d, bundleName: %{public}s, subName: %{public}s getImeInfoFromBundleMgr failed",
            userId, bundleName.c_str(), subName.c_str());
        return;
    }
    std::lock_guard<std::mutex> lock(currentImeInfoLock_);
    currentImeInfo_ = std::make_shared<ImeInfo>(info);
}

std::string ImeInfoInquirer::GetInputMethodParam(const int32_t userId)
{
    auto properties = ListInputMethodInfo(userId);
    if (properties.empty()) {
        return "";
    }
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentIme(userId);
    bool isBegin = true;
    std::string params = "{\"imeList\":[";
    for (const auto &property : properties) {
        params += isBegin ? "" : "},";
        isBegin = false;

        std::string imeId = Str16ToStr8(property.mPackageName) + "/" + Str16ToStr8(property.mAbilityName);
        params += "{\"ime\": \"" + imeId + "\",";
        params += "\"labelId\": \"" + std::to_string(property.labelId) + "\",";
        params += "\"descriptionId\": \"" + std::to_string(property.descriptionId) + "\",";
        std::string isCurrentIme = currentIme == imeId ? "true" : "false";
        params += "\"isCurrentIme\": \"" + isCurrentIme + "\",";
        params += "\"label\": \"" + Str16ToStr8(property.label) + "\",";
        params += "\"description\": \"" + Str16ToStr8(property.description) + "\"";
    }
    params += "}]}";
    return params;
}

std::vector<InputMethodInfo> ImeInfoInquirer::ListInputMethodInfo(const int32_t userId)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    std::vector<ExtensionAbilityInfo> extensionInfos;
    if (QueryImeExtInfos(userId, extensionInfos) != ErrorCode::NO_ERROR) {
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
        property.mPackageName = Str8ToStr16(extension.bundleName);
        property.mAbilityName = Str8ToStr16(extension.name);
        property.labelId = applicationInfo.labelId;
        property.descriptionId = applicationInfo.descriptionId;
        property.label = Str8ToStr16(label);
        property.description = Str8ToStr16(description);
        properties.emplace_back(property);
    }
    return properties;
}

int32_t ImeInfoInquirer::ListInputMethod(
    const int32_t userId, const InputMethodStatus status, std::vector<Property> &props)
{
    IMSA_HILOGD("userId: %{public}d, status: %{public}d", userId, status);
    if (status == InputMethodStatus::ALL) {
        return ListInputMethod(userId, props);
    }
    if (status == InputMethodStatus::ENABLE) {
        return ListEnabledInputMethod(userId, props);
    }
    if (status == InputMethodStatus::DISABLE) {
        return ListDisabledInputMethod(userId, props);
    }
    return ErrorCode::ERROR_BAD_PARAMETERS;
}

int32_t ImeInfoInquirer::ListInputMethod(const int32_t userId, std::vector<Property> &props)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    std::vector<ExtensionAbilityInfo> extensionInfos;
    int32_t ret = QueryImeExtInfos(userId, extensionInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d queryImeExtInfos failed", userId);
        return ret;
    }
    for (const auto &extension : extensionInfos) {
        auto it = std::find_if(props.begin(), props.end(),
            [extension](const Property &prop) { return prop.name == extension.bundleName; });
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

int32_t ImeInfoInquirer::ListEnabledInputMethod(const int32_t userId, std::vector<Property> &props)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    auto prop = GetCurrentInputMethod(userId);
    if (prop == nullptr) {
        IMSA_HILOGI("userId: %{public}d getCurrentInputMethod failed", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    props = { *prop };
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListDisabledInputMethod(const int32_t userId, std::vector<Property> &props)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    auto ret = ListInputMethod(userId, props);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d listInputMethod failed", userId);
        return ret;
    }
    auto bundleName = ImeCfgManager::GetInstance().GetCurrentImeBundleName(userId);
    for (auto iter = props.begin(); iter != props.end();) {
        if (iter->name == bundleName) {
            iter = props.erase(iter);
            continue;
        }
        ++iter;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListInputMethodSubtype(
    const int32_t userId, const std::string &bundleName, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s", userId, bundleName.c_str());
    std::lock_guard<std::mutex> lock(currentImeInfoLock_);
    if (bundleName == ImeCfgManager::GetInstance().GetCurrentImeBundleName(userId)
        && (currentImeInfo_ != nullptr && currentImeInfo_->prop.name == bundleName)) {
        subProps = currentImeInfo_->subProps;
        return ErrorCode::NO_ERROR;
    }
    std::vector<ExtensionAbilityInfo> extInfos;
    auto ret = GetExtInfosByBundleName(userId, bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d getExtInfosByBundleName %{public}s failed", userId, bundleName.c_str());
        return ret;
    }
    return IsNewExtInfos(extInfos) ? ListInputMethodSubtype(userId, extInfos[0], subProps)
                                   : ListInputMethodSubtype(userId, extInfos, subProps);
}

int32_t ImeInfoInquirer::ListCurrentInputMethodSubtype(const int32_t userId, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("currentIme: %{public}s", ImeCfgManager::GetInstance().GetCurrentIme(userId).c_str());
    auto bundleName = ImeCfgManager::GetInstance().GetCurrentImeBundleName(userId);
    auto extName = ImeCfgManager::GetInstance().GetCurrentImeExtName(userId);
    if (bundleName.empty() || extName.empty()) {
        IMSA_HILOGE("currentIme: %{public}s or extName: %{public}s is empty",
            ImeCfgManager::GetInstance().GetCurrentIme(userId).c_str(), extName.c_str());
        return ErrorCode::ERROR_PERSIST_CONFIG;
    }
    // If the local currentImeInfo_ exists, the value is taken directly form currentImeInfo_
    std::lock_guard<std::mutex> lock(currentImeInfoLock_);
    if (currentImeInfo_ != nullptr && currentImeInfo_->prop.name == bundleName && currentImeInfo_->prop.id == extName) {
        subProps = currentImeInfo_->subProps;
        return ErrorCode::NO_ERROR;
    }
    return ListInputMethodSubtype(userId, bundleName, subProps);
}

bool ImeInfoInquirer::IsNewExtInfos(const std::vector<ExtensionAbilityInfo> &extInfos)
{
    if (extInfos.empty()) {
        IMSA_HILOGE("extInfos is empty");
        return false;
    }
    auto iter = std::find_if(extInfos[0].metadata.begin(), extInfos[0].metadata.end(),
        [](const Metadata &metadata) { return metadata.name == SUBTYPE_PROFILE_METADATA_NAME; });
    return iter != extInfos[0].metadata.end();
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
    bool ret = false;
    std::vector<std::string> profiles;
    for (const auto &metadata : extInfo.metadata) {
        if (metadata.name == SUBTYPE_PROFILE_METADATA_NAME) {
            OHOS::AppExecFwk::BundleMgrClientImpl clientImpl;
            ret = clientImpl.GetResConfigFile(extInfo, metadata.name, profiles);
            break;
        }
    }
    if (!ret) {
        IMSA_HILOGE("GetProfileFromExtension failed");
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    if (!ParseSubProp(profiles, subProps)) {
        IMSA_HILOGE("ParseSubProp failed");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    IMSA_HILOGD("subProps size: %{public}zu", subProps.size());
    for (auto &subProp : subProps) {
        subProp.name = extInfo.bundleName;
        auto pos = subProp.label.find(':');
        if (pos != std::string::npos && pos + 1 < subProp.label.size()) {
            subProp.labelId = atoi(subProp.label.substr(pos + 1).c_str());
            subProp.label = GetStringById(extInfo.bundleName, extInfo.moduleName, subProp.labelId, userId);
        }
        pos = subProp.icon.find(':');
        if (pos != std::string::npos && pos + 1 < subProp.icon.size()) {
            subProp.iconId = atoi(subProp.icon.substr(pos + 1).c_str());
        }
    }
    return ErrorCode::NO_ERROR;
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

std::shared_ptr<Property> ImeInfoInquirer::GetCurrentInputMethod(const int32_t userId)
{
    IMSA_HILOGI("currentIme: %{public}s", ImeCfgManager::GetInstance().GetCurrentIme(userId).c_str());
    auto bundleName = ImeCfgManager::GetInstance().GetCurrentImeBundleName(userId);
    auto extName = ImeCfgManager::GetInstance().GetCurrentImeExtName(userId);
    if (bundleName.empty() || extName.empty()) {
        IMSA_HILOGE("currentIme: %{public}s is abnormal", ImeCfgManager::GetInstance().GetCurrentIme(userId).c_str());
        return nullptr;
    }

    // If the local currentImeInfo_ exists, the value is taken directly form currentImeInfo_
    std::lock_guard<std::mutex> lock(currentImeInfoLock_);
    if (currentImeInfo_ != nullptr && currentImeInfo_->prop.name == bundleName && currentImeInfo_->prop.id == extName) {
        return std::make_shared<Property>(currentImeInfo_->prop);
    }

    std::vector<AppExecFwk::ExtensionAbilityInfo> extInfos;
    auto ret = ImeInfoInquirer::GetInstance().GetExtInfosByBundleName(userId, bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR || extInfos.empty()) {
        IMSA_HILOGE("userId: %{public}d getExtInfosByBundleName %{public}s failed", userId, bundleName.c_str());
        return nullptr;
    }
    Property prop = { .name = extInfos[0].bundleName,
        .id = extName, // if old ime, the extInfos[0].name maybe not same with currentImeExtName
        .label =
            GetStringById(extInfos[0].bundleName, extInfos[0].moduleName, extInfos[0].applicationInfo.labelId, userId),
        .labelId = extInfos[0].applicationInfo.labelId,
        .iconId = extInfos[0].applicationInfo.iconId };
    return std::make_shared<Property>(prop);
}

std::shared_ptr<SubProperty> ImeInfoInquirer::GetCurrentInputMethodSubtype(const int32_t userId)
{
    auto bundleName = ImeCfgManager::GetInstance().GetCurrentImeBundleName(userId);
    auto subName = ImeCfgManager::GetInstance().GetCurrentImeSubName(userId);
    auto extName = ImeCfgManager::GetInstance().GetCurrentImeExtName(userId);
    IMSA_HILOGD("currentIme: %{public}s, subName: %{public}s",
        ImeCfgManager::GetInstance().GetCurrentIme(userId).c_str(), subName.c_str());
    if (bundleName.empty() || subName.empty() || subName.empty()) {
        IMSA_HILOGE("currentIme: %{public}s or subName: %{public}s is empty",
            ImeCfgManager::GetInstance().GetCurrentIme(userId).c_str(), subName.c_str());
        return nullptr;
    }

    // If the local currentImeInfo_ exists, the value is taken directly form currentImeInfo_
    std::lock_guard<std::mutex> lock(currentImeInfoLock_);
    if (currentImeInfo_ != nullptr && currentImeInfo_->prop.name == bundleName && currentImeInfo_->prop.id == extName
        && currentImeInfo_->subProp.id == subName) {
        return std::make_shared<SubProperty>(currentImeInfo_->subProp);
    }

    std::vector<SubProperty> subProps = {};
    auto ret = ListInputMethodSubtype(userId, bundleName, subProps);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE(
            "userId: %{public}d listInputMethodSubtype by bundleName: %{public}s failed", userId, bundleName.c_str());
        return nullptr;
    }
    auto it = std::find_if(
        subProps.begin(), subProps.end(), [subName](const SubProperty &subProp) { return subProp.id == subName; });
    if (it != subProps.end()) {
        return std::make_shared<SubProperty>(*it);
    }
    IMSA_HILOGE("Find subName: %{public}s failed", subName.c_str());
    return nullptr;
}

bool ImeInfoInquirer::IsImeInstalled(const int32_t userId, const std::string &bundleName, const std::string &extName)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s, extName: %{public}s", userId, bundleName.c_str(),
        extName.c_str());
    std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> extInfos;
    GetExtInfosByBundleName(userId, bundleName, extInfos);
    for (auto const &extInfo : extInfos) {
        if (bundleName == extInfo.bundleName && extName == extInfo.name) {
            IMSA_HILOGI("true");
            return true;
        }
    }
    IMSA_HILOGI("false");
    return false;
}

std::string ImeInfoInquirer::GetStartedIme(const int32_t userId)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    auto bundleName = ImeCfgManager::GetInstance().GetCurrentImeBundleName(userId);
    auto extname = ImeCfgManager::GetInstance().GetCurrentImeExtName(userId);
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentIme(userId);
    if (currentIme.empty() || !IsImeInstalled(userId, bundleName, extname)) {
        auto info = GetDefaultImeInfo(userId);
        if (info == nullptr) {
            IMSA_HILOGI("GetDefaultImeInfo failed");
            return "";
        }
        std::string newUserIme = info->prop.name + "/" + info->prop.id;
        currentIme.empty() ? ImeCfgManager::GetInstance().AddImeCfg({ userId, newUserIme, info->subProp.id })
                           : ImeCfgManager::GetInstance().ModifyImeCfg({ userId, newUserIme, info->subProp.id });
        SetCurrentImeInfo(*info);
        return newUserIme;
    }
    // service start, user switch, set the currentImeInfo_.
    SetCurrentImeInfo(userId);
    return currentIme;
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetDefaultImeInfo(const int32_t userId)
{
    std::lock_guard<std::mutex> lock(defaultImeInfoLock_);
    auto bundleName = GetDefaultImeBundleName();
    auto extName = GetDefaultImeExtName();
    if (defaultImeInfo_ != nullptr && defaultImeInfo_->prop.name == bundleName && defaultImeInfo_->prop.id == extName) {
        return defaultImeInfo_;
    }
    ImeInfo info;
    auto ret = GetImeInfoFromBundleMgr(userId, bundleName, "", info);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE(
            "userId: %{public}d, bundleName: %{public}s getImeInfoFromBundleMgr failed", userId, bundleName.c_str());
        return nullptr;
    }
    if (!info.isNewIme) {
        info.prop.id = extName;
        auto it = std::find_if(info.subProps.begin(), info.subProps.end(),
            [extName](const SubProperty &subProp) { return subProp.id == extName; });
        if (it != info.subProps.end()) {
            info.subProp = *it;
        }
    }
    defaultImeInfo_ = std::make_shared<ImeInfo>(info);
    return defaultImeInfo_;
}

std::string ImeInfoInquirer::GetDefaultImeExtName()
{
    auto defaultIme = GetDefaultIme();
    auto pos = defaultIme.find('/');
    return pos != std::string::npos && pos + 1 < defaultIme.size() ? defaultIme.substr(pos + 1) : "";
}

std::string ImeInfoInquirer::GetDefaultIme()
{
    char value[CONFIG_LEN] = { 0 };
    auto code = GetParameter(DEFAULT_IME_KEY, "", value, CONFIG_LEN);
    return code > 0 ? value : "";
}

std::string ImeInfoInquirer::GetDefaultImeBundleName()
{
    auto defaultIme = GetDefaultIme();
    auto pos = defaultIme.find('/');
    if (pos == std::string::npos) {
        IMSA_HILOGE("defaultIme: %{public}s is abnormal", defaultIme.c_str());
        return "";
    }
    return defaultIme.substr(0, pos);
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

SubProperty ImeInfoInquirer::FindSubPropertyByCompare(
    const std::vector<SubProperty> &subProps, const CompareHandler compare)
{
    for (const auto &subProp : subProps) {
        if (compare(subProp)) {
            return subProp;
        }
    }
    IMSA_HILOGE("failed");
    return {};
}

bool ImeInfoInquirer::ParseSubProp(const std::vector<std::string> &profiles, std::vector<SubProperty> &subProps)
{
    if (profiles.empty() || profiles.size() != SUBTYPE_PROFILE_NUM) {
        IMSA_HILOGE("profiles size: %{public}zu", profiles.size());
        return false;
    }
    json jsonSubProps;
    SubProperty subProp;
    IMSA_HILOGD("profiles[0]: %{public}s", profiles[0].c_str());
    jsonSubProps = json::parse(profiles[0], nullptr, false);
    if (jsonSubProps.is_null() || jsonSubProps.is_discarded() || !jsonSubProps.contains("subtypes")
        || !jsonSubProps["subtypes"].is_array() || jsonSubProps["subtypes"].empty()) {
        IMSA_HILOGE("json parse failed");
        return false;
    }
    ParseSubProp(jsonSubProps, subProps);
    return true;
}

void ImeInfoInquirer::ParseSubProp(const json &jsonSubProps, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("subType num: %{public}zu", jsonSubProps["subtypes"].size());
    for (auto &jsonCfg : jsonSubProps["subtypes"]) {
        if (subProps.size() >= MAX_SUBTYPE_NUM) {
            break;
        }
        SubProperty subProp;
        ParseSubProp(jsonCfg, subProp);
        subProps.push_back(subProp);
    }
}

void ImeInfoInquirer::ParseSubProp(const json &jsonSubProp, SubProperty &subProp)
{
    // label: 子类型对外显示名称
    if (jsonSubProp.find("label") != jsonSubProp.end() && jsonSubProp["label"].is_string()) {
        jsonSubProp.at("label").get_to(subProp.label);
    }
    if (jsonSubProp.find("labelId") != jsonSubProp.end() && jsonSubProp["labelId"].is_number()) {
        jsonSubProp.at("labelId").get_to(subProp.labelId);
    }
    // id: 子类型名
    if (jsonSubProp.find("id") != jsonSubProp.end() && jsonSubProp["id"].is_string()) {
        jsonSubProp.at("id").get_to(subProp.id);
    }

    if (jsonSubProp.find("icon") != jsonSubProp.end() && jsonSubProp["icon"].is_string()) {
        jsonSubProp.at("icon").get_to(subProp.icon);
    }
    if (jsonSubProp.find("iconId") != jsonSubProp.end() && jsonSubProp["iconId"].is_number()) {
        jsonSubProp.at("iconId").get_to(subProp.iconId);
    }
    if (jsonSubProp.find("mode") != jsonSubProp.end() && jsonSubProp["mode"].is_string()) {
        jsonSubProp.at("mode").get_to(subProp.mode);
    }
    if (jsonSubProp.find("locale") != jsonSubProp.end() && jsonSubProp["locale"].is_string()) {
        jsonSubProp.at("locale").get_to(subProp.locale);
    }
}
} // namespace MiscServices
} // namespace OHOS