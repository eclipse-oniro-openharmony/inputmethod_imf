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
#include "enable_upgrade_manager.h"

#include "settings_data_utils.h"
namespace OHOS {
namespace MiscServices {
EnableUpgradeManager &EnableUpgradeManager::GetInstance()
{
    static EnableUpgradeManager instance;
    return instance;
}

int32_t EnableUpgradeManager::Upgrade(int32_t userId)
{
    std::lock_guard<std::mutex> lock(upgradedLock_);
    if (upgradedUserId_.count(userId)) {
        return ErrorCode::NO_ERROR;
    }
    auto [ret, needUpgrade] = GetUpgradeFlag(userId);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    if (!needUpgrade) {
        upgradedUserId_.insert(userId);
        return ErrorCode::NO_ERROR;
    }
    std::set<std::string> enabledBundleNames;
    ret = GetEnabledTable(userId, enabledBundleNames);
    if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND
        && ret != ErrorCode::ERROR_EX_PARCELABLE) {
        return ret;
    }
    std::set<std::string> fullModeBundleNames;
    ret = GetFullExperienceTable(userId, fullModeBundleNames);
    if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND
        && ret != ErrorCode::ERROR_EX_PARCELABLE) {
        return ret;
    }
    std::string newContent;
    MergeTwoTable(enabledBundleNames, fullModeBundleNames, newContent);
    if (!SetUserEnabledTable(userId, newContent)) {
        IMSA_HILOGE("%{public}d set enabled table failed.", userId);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    upgradedUserId_.insert(userId);
    return ErrorCode::NO_ERROR;
}

std::pair<int32_t, bool> EnableUpgradeManager::GetUpgradeFlag(int32_t userId)
{
    std::string userContent;
    int32_t ret = GetUserEnabledTable(userId, userContent);
    if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        IMSA_HILOGW("%{public}d get user enabled table failed:%{public}d.", userId, ret);
        return std::make_pair(ret, false);
    }
    return std::make_pair(ErrorCode::NO_ERROR, userContent.find("version") == std::string::npos);
}

int32_t EnableUpgradeManager::GetEnabledTable(int32_t userId, std::set<std::string> &bundleNames)
{
    auto ret = GetGlobalEnabledTable(userId, bundleNames);
    if (ret == ErrorCode::ERROR_EX_PARCELABLE) {
        return GetUserEnabledTable(userId, bundleNames);
    }
    return ret;
}

int32_t EnableUpgradeManager::GetGlobalEnabledTable(int32_t userId, std::string &content)
{
    return GetEnabledTable(userId, SETTING_URI_PROXY, content);
}

int32_t EnableUpgradeManager::GetUserEnabledTable(int32_t userId, std::string &content)
{
    std::string uriProxy = SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true";
    return GetEnabledTable(userId, uriProxy, content);
}

int32_t EnableUpgradeManager::GetGlobalEnabledTable(int32_t userId, std::set<std::string> &bundleNames)
{
    return GetEnabledTable(userId, SETTING_URI_PROXY, bundleNames);
}

int32_t EnableUpgradeManager::GetUserEnabledTable(int32_t userId, std::set<std::string> &bundleNames)
{
    std::string uriProxy = SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true";
    return GetEnabledTable(userId, uriProxy, bundleNames);
}

int32_t EnableUpgradeManager::GetEnabledTable(
    int32_t userId, const std::string &uriProxy, std::set<std::string> &bundleNames)
{
    std::string content;
    auto ret = GetEnabledTable(userId, uriProxy, content);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    return ParseEnabledTable(userId, content, bundleNames);
}

int32_t EnableUpgradeManager::GetEnabledTable(int32_t userId, const std::string &uriProxy, std::string &content)
{
    return SettingsDataUtils::GetInstance()->GetStringValue(uriProxy, SettingsDataUtils::ENABLE_IME, content);
}

int32_t EnableUpgradeManager::ParseEnabledTable(
    int32_t userId, std::string &content, std::set<std::string> &bundleNames)
{
    EnabledImeCfg oldCfg;
    oldCfg.userImeCfg.userId = std::to_string(userId);
    if (!oldCfg.Unmarshall(content)) {
        IMSA_HILOGE("%{public}d Unmarshall failed:%{public}s.", userId, content.c_str());
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    bundleNames = std::set<std::string>(oldCfg.userImeCfg.identities.begin(), oldCfg.userImeCfg.identities.end());
    return ErrorCode::NO_ERROR;
}

int32_t EnableUpgradeManager::GetFullExperienceTable(int32_t userId, std::set<std::string> &bundleNames)
{
    std::string content;
    int32_t ret =
        SettingsDataUtils::GetInstance()->GetStringValue(SETTING_URI_PROXY, SettingsDataUtils::SECURITY_MODE, content);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGW("%{public}d get full experience table failed:%{public}d.", userId, ret);
        return ret;
    }
    SecurityModeCfg cfg;
    cfg.userImeCfg.userId = std::to_string(userId);
    if (!cfg.Unmarshall(content)) {
        IMSA_HILOGE("%{public}d Unmarshall failed:%{public}s.", userId, content.c_str());
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    bundleNames = std::set<std::string>(cfg.userImeCfg.identities.begin(), cfg.userImeCfg.identities.end());
    return ErrorCode::NO_ERROR;
}

void EnableUpgradeManager::MergeTwoTable(const std::set<std::string> &enabledBundleNames,
    const std::set<std::string> &fullModeBundleNames, std::string &newContent)
{
    ImeEnabledCfg cfg;
    cfg.version = GetDisplayVersion();
    for (const auto &bundleName : enabledBundleNames) {
        ImeEnabledInfo tmpInfo;
        tmpInfo.bundleName = bundleName;
        tmpInfo.enabledStatus = EnabledStatus::BASIC_MODE;
        cfg.enabledInfos.push_back(tmpInfo);
    }
    for (const auto &bundleName : fullModeBundleNames) {
        auto iter = std::find_if(cfg.enabledInfos.begin(), cfg.enabledInfos.end(),
            [&bundleName](const ImeEnabledInfo &info) { return info.bundleName == bundleName; });
        if (iter != cfg.enabledInfos.end()) {
            iter->enabledStatus = EnabledStatus::FULL_EXPERIENCE_MODE;
            continue;
        }
        ImeEnabledInfo tmpInfo;
        tmpInfo.bundleName = bundleName;
        tmpInfo.enabledStatus = EnabledStatus::FULL_EXPERIENCE_MODE;
        cfg.enabledInfos.push_back(tmpInfo);
    }
    cfg.Marshall(newContent);
}

void EnableUpgradeManager::UpdateGlobalEnabledTable(int32_t userId, const std::vector<std::string> &bundleNames)
{
    std::string oldGlobalContent;
    auto ret = GetGlobalEnabledTable(userId, oldGlobalContent);
    if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        IMSA_HILOGW("%{public}d get global enabled table failed:%{public}d.", userId, ret);
        return;
    }
    IMSA_HILOGD("old global content:%{public}s.", oldGlobalContent.c_str());
    auto newGlobalContent = GenerateGlobalContent(userId, bundleNames);
    if (newGlobalContent == oldGlobalContent) {
        IMSA_HILOGD("content same, not deal.");
        return;
    }
    IMSA_HILOGI("global content, old:%{public}s, new:%{public}s.", oldGlobalContent.c_str(), newGlobalContent.c_str());
    auto globalTableUserId = GetGlobalTableUserId(oldGlobalContent);
    if (globalTableUserId == -1 || userId == globalTableUserId) {
        SetGlobalEnabledTable(newGlobalContent);
        return;
    }
    ret = Upgrade(globalTableUserId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGW("%{public}d Upgrade failed:%{public}d.", userId, ret);
        return;
    }
    SetGlobalEnabledTable(newGlobalContent);
}

int32_t EnableUpgradeManager::GetGlobalTableUserId(const std::string &valueStr)
{
    auto root = cJSON_Parse(valueStr.c_str());
    if (root == nullptr) {
        IMSA_HILOGE("valueStr content parse failed!");
        return -1;
    }
    auto subNode = Serializable::GetSubNode(root, "enableImeList");
    if (subNode == nullptr || !cJSON_IsObject(subNode)) {
        IMSA_HILOGW("subNode is null or not object");
        cJSON_Delete(root);
        return -1;
    }
    if (subNode->child == nullptr) {
        IMSA_HILOGW("subNode has not child");
        cJSON_Delete(root);
        return -1;
    }
    std::string userId = subNode->child->string;
    cJSON_Delete(root);
    return atoi(userId.c_str());
}

std::string EnableUpgradeManager::GenerateGlobalContent(int32_t userId, const std::vector<std::string> &bundleNames)
{
    EnabledImeCfg cfg;
    cfg.userImeCfg.identities = bundleNames;
    std::string newGlobalContent;
    cfg.userImeCfg.userId = std::to_string(userId);
    if (!cfg.Marshall(newGlobalContent)) {
        IMSA_HILOGE("%{public}d Marshall failed:%{public}s.", userId);
    }
    return newGlobalContent;
}

bool EnableUpgradeManager::SetGlobalEnabledTable(const std::string &content)
{
    return SetEnabledTable(SETTING_URI_PROXY, content);
}

bool EnableUpgradeManager::SetUserEnabledTable(int32_t userId, const std::string &content)
{
    std::string uriProxy = SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true";
    return SetEnabledTable(uriProxy, content);
}

bool EnableUpgradeManager::SetEnabledTable(const std::string &uriProxy, const std::string &content)
{
    return SettingsDataUtils::GetInstance()->SetStringValue(uriProxy, SettingsDataUtils::ENABLE_IME, content)
}
} // namespace MiscServices
} // namespace OHOS