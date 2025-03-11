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

#include "ime_enabled_info_manager.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <set>
#include <sstream>

#include "ime_info_inquirer.h"
#include "parameter.h"
namespace OHOS {
namespace MiscServices {
using namespace std::chrono;
ImeEnabledInfoManager &ImeEnabledInfoManager::GetInstance()
{
    static ImeEnabledInfoManager instance;
    return instance;
}

ImeEnabledInfoManager::~ImeEnabledInfoManager()
{
}

void ImeEnabledInfoManager::SetEnabledStatusChangedHandler(EnabledStatusChangedHandler handler)
{
    if (handler_ != nullptr) {
        return;
    }
    handler_ = std::move(handler);
}

void ImeEnabledInfoManager::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler)
{
    eventHandler_ = eventHandler;
}

int32_t ImeEnabledInfoManager::Init(const std::map<int32_t, std::vector<FullImeInfo>> &fullImeInfos)
{
    if (!HasEnabledSwitch()) {
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("enable Init run in, user num:%{public}zu.", fullImeInfos.size());
    if (!SettingsDataUtils::GetInstance()->IsDataShareReady()) {
        IMSA_HILOGE("data share not ready.");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    for (const auto &fullImeInfo : fullImeInfos) {
        AddUser(fullImeInfo.first, fullImeInfo.second);
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::Add(int32_t userId, const std::vector<FullImeInfo> &imeInfos)
{
    IMSA_HILOGI("userId:%{public}d", userId);
    currentUserId_ = userId;
    ImeEnabledCfg cfg;
    auto ret = GetEnabledCfgFromCache(userId, cfg);
    if (ret == ErrorCode::NO_ERROR) {
        ModGlobalEnabledTable(userId, cfg);
        return ret;
    }
    return AddUser(userId, imeInfos);
}

int32_t ImeEnabledInfoManager::CorrectUserAdd(int32_t userId, const std::vector<FullImeInfo> &imeInfos)
{
    ImeEnabledCfg cfg;
    auto ret = GetEnabledCfgFromCache(userId, cfg);
    if (ret == ErrorCode::NO_ERROR) {
        return ret;
    }
    return AddUser(userId, imeInfos);
}

int32_t ImeEnabledInfoManager::AddUser(int32_t userId, const std::vector<FullImeInfo> &imeInfos)
{
    if (!HasEnabledSwitch()) {
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("run in, userId:%{public}d", userId);
    if (!SettingsDataUtils::GetInstance()->IsDataShareReady()) {
        IMSA_HILOGE("data share not ready.");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    ImeEnabledCfg cfg;
    auto ret = GetEnabledCfg(userId, cfg, true, imeInfos);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    return SetEnabledCfg(userId, cfg);
}

int32_t ImeEnabledInfoManager::Delete(int32_t userId)
{
    if (!HasEnabledSwitch()) {
        return ErrorCode::NO_ERROR;
    }
    std::lock_guard<std::mutex> lock(imeEnabledCfgLock_);
    imeEnabledCfg_.erase(userId);
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::Add(int32_t userId, const FullImeInfo &imeInfo)
{
    if (!HasEnabledSwitch()) {
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("userId:%{public}d, bundleName:%{public}s", userId, imeInfo.prop.name.c_str());
    if (!SettingsDataUtils::GetInstance()->IsDataShareReady()) {
        IMSA_HILOGE("data share not ready.");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    if (imeInfo.prop.name == ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName) {
        IMSA_HILOGI("[%{public}d,%{public}s] is sys ime, deal in init or user add.", userId, imeInfo.prop.name.c_str());
        return ErrorCode::NO_ERROR;
    }
    ImeEnabledCfg enabledCfg;
    auto ret = GetEnabledCfgFromCacheWithCorrect(userId, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("user:%{public}d get enabledCfg failed:%{public}d.", userId, ret);
        return ret;
    }
    auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
        [&imeInfo](const ImeEnabledInfo &info) { return imeInfo.prop.name == info.bundleName; });
    if (iter != enabledCfg.enabledInfos.end()) {
        enabledCfg.enabledInfos.erase(iter);
    }
    ImeEnabledInfo infoTmp;
    infoTmp.bundleName = imeInfo.prop.name;
    infoTmp.extensionName = imeInfo.prop.id;
    infoTmp.enabledStatus = ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState;
    infoTmp.installTime = imeInfo.installTime;
    CheckBySysEnabledSwitch(infoTmp);
    CheckBySysIme(infoTmp);
    enabledCfg.enabledInfos.push_back(infoTmp);
    return SetEnabledCfg(userId, enabledCfg);
}

int32_t ImeEnabledInfoManager::Delete(int32_t userId, const std::string &bundleName)
{
    if (!HasEnabledSwitch()) {
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("enable Delete run in, userId:%{public}d, bundleName:%{public}s", userId, bundleName.c_str());
    if (!SettingsDataUtils::GetInstance()->IsDataShareReady()) {
        IMSA_HILOGE("data share not ready.");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    ImeEnabledCfg enabledCfg;
    auto ret = GetEnabledCfgFromCacheWithCorrect(userId, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("user:%{public}d get enabledCfg failed:%{public}d.", userId, ret);
        return ret;
    }
    auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
        [&bundleName](const ImeEnabledInfo &info) { return info.bundleName == bundleName; });
    if (iter == enabledCfg.enabledInfos.end()) {
        return ErrorCode::NO_ERROR;
    }
    enabledCfg.enabledInfos.erase(iter);
    return SetEnabledCfg(userId, enabledCfg);
}

int32_t ImeEnabledInfoManager::NeedUpdate(
    int32_t userId, const std::string &bundleName, const std::string &extensionName, EnabledStatus status)
{
    IMSA_HILOGI("update info:[%{public}d, %{public}s, %{public}s, %{public}d].", userId, bundleName.c_str(),
        extensionName.c_str(), static_cast<int32_t>(status));
    if (bundleName.empty()) {
        IMSA_HILOGW("%{public}d bundleName is empty.", userId);
        return ErrorCode::ERROR_PARAMETER_CHECK_FAILED;
    }
    if (ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName == bundleName && status == EnabledStatus::DISABLED) {
        IMSA_HILOGW("[%{public}d,%{public}s] is sys ime, do not set DISABLED.", userId, bundleName.c_str());
        return ErrorCode::ERROR_DISABLE_SYS_IME;
    }
    if (!HasEnabledSwitch() || !SettingsDataUtils::GetInstance()->IsDataShareReady()) {
        IMSA_HILOGE("enabled feature not on or data share not ready, [%{public}d, %{public}s] operator not allow.",
            userId, bundleName.c_str());
        return ErrorCode::ERROR_ENABLE_IME;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::Update(
    int32_t userId, const std::string &bundleName, const std::string &extensionName, EnabledStatus status)
{
    auto ret = NeedUpdate(userId, bundleName, extensionName, status);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    ImeEnabledCfg enabledCfg;
    ret = GetEnabledCfgFromCacheWithCorrect(userId, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("user:%{public}d get enabledCfg failed:%{public}d.", userId, ret);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
        [&bundleName](const ImeEnabledInfo &info) { return bundleName == info.bundleName; });
    if (iter == enabledCfg.enabledInfos.end()) {
        if (!ImeInfoInquirer::GetInstance().IsInputMethod(userId, bundleName)) {
            IMSA_HILOGE("not find [%{public}d, %{public}s, %{public}s] in ime install list.", userId,
                bundleName.c_str(), extensionName.c_str());
            return ErrorCode::ERROR_IME_NOT_FOUND;
        }
        ret = AddUser(userId);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("user:%{public}d AddUser failed:%{public}d.", userId, ret);
            return ErrorCode::ERROR_ENABLE_IME;
        }
        GetEnabledCfgFromCache(userId, enabledCfg);
        iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
            [&bundleName](const ImeEnabledInfo &info) { return bundleName == info.bundleName; });
    }
    if (iter == enabledCfg.enabledInfos.end()) {
        IMSA_HILOGE("not find [%{public}d, %{public}s, %{public}s] in enabledInfos.", userId, bundleName.c_str(),
            extensionName.c_str());
        return ErrorCode::ERROR_IME_NOT_FOUND;
    }
    auto oldStatus = iter->enabledStatus;
    iter->enabledStatus = status;
    iter->stateUpdateTime =
        std::to_string(duration_cast<std::chrono::milliseconds>(system_clock::now().time_since_epoch()).count());
    CheckBySysEnabledSwitch(*iter);
    ret = SetEnabledCfg(userId, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d SetEnabledCfg failed.", userId);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    NotifyEnableChange(userId, bundleName, oldStatus);
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledState(int32_t userId, const std::string &bundleName, EnabledStatus &status)
{
    IMSA_HILOGD("[%{public}d, %{public}s] start.", userId, bundleName.c_str());
    if (bundleName.empty()) {
        IMSA_HILOGW("%{public}d bundleName is empty.", userId);
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    if (!HasEnabledSwitch()) {
        status = EnabledStatus::FULL_EXPERIENCE_MODE;
        return ErrorCode::NO_ERROR;
    }
    if (bundleName == ImeInfoInquirer::GetInstance().GetSystemConfig().sysSpecialIme) {
        status = EnabledStatus::FULL_EXPERIENCE_MODE;
        return ErrorCode::NO_ERROR;
    }
    auto ret = GetEnabledStateInner(userId, bundleName, status);
    if (bundleName == ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName
        && (ret != ErrorCode::NO_ERROR || status == EnabledStatus::DISABLED)) {
        IMSA_HILOGI("mod sys ime enabledStatus.");
        status = EnabledStatus::BASIC_MODE;
        return ErrorCode::NO_ERROR;
    }
    if (ret != ErrorCode::NO_ERROR) {
        return ErrorCode::ERROR_ENABLE_IME;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledStates(int32_t userId, std::vector<Property> &props)
{
    if (props.empty()) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    if (!HasEnabledSwitch()) {
        for (auto &prop : props) {
            prop.status = EnabledStatus::FULL_EXPERIENCE_MODE;
        }
        return ErrorCode::NO_ERROR;
    }
    auto ret = GetEnabledStateInner(userId, props);
    for (auto &prop : props) {
        if (prop.name == ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName
            && prop.status == EnabledStatus::DISABLED) {
            IMSA_HILOGI("mod sys ime enabledStatus.");
            prop.status = EnabledStatus::BASIC_MODE;
            break;
        }
    }
    if (ret != ErrorCode::NO_ERROR) {
        return ErrorCode::ERROR_ENABLE_IME;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledStateInner(int32_t userId, const std::string &bundleName, EnabledStatus &status)
{
    // get from cache
    ImeEnabledCfg cacheCfg;
    GetEnabledCfgFromCache(userId, cacheCfg);
    auto iter = std::find_if(cacheCfg.enabledInfos.begin(), cacheCfg.enabledInfos.end(),
        [&bundleName](const ImeEnabledInfo &info) { return bundleName == info.bundleName; });
    if (iter != cacheCfg.enabledInfos.end()) {
        status = iter->enabledStatus;
        IMSA_HILOGD("[%{public}d, %{public}s] enabledStatus by cache: %{public}d.", userId, bundleName.c_str(),
            static_cast<int32_t>(status));
        return ErrorCode::NO_ERROR;
    }
    // get directly
    ImeEnabledCfg cfg;
    auto ret = GetEnabledCfg(userId, cfg, false);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("[%{public}d, %{public}s] GetEnabledCfg failed.", userId, bundleName.c_str());
        return ret;
    }
    iter = std::find_if(cfg.enabledInfos.begin(), cfg.enabledInfos.end(),
        [&bundleName](const ImeEnabledInfo &info) { return info.bundleName == bundleName; });
    if (iter == cfg.enabledInfos.end()) {
        IMSA_HILOGE("not find [%{public}d, %{public}s] in enableCfg.", userId, bundleName.c_str());
        return ErrorCode::ERROR_ENABLE_IME;
    }
    status = iter->enabledStatus;
    IMSA_HILOGD("[%{public}d, %{public}s] enabledStatus directly: %{public}d.", userId, bundleName.c_str(),
        static_cast<int32_t>(status));
    // post correct task
    PostCorrectAddTask(userId);
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledStateInner(int32_t userId, std::vector<Property> &props)
{
    // get from cache
    ImeEnabledCfg cacheCfg;
    auto ret = GetEnabledCfgFromCache(userId, cacheCfg);
    if (ret == ErrorCode::NO_ERROR) {
        IMSA_HILOGI("%{public}d get from cache start.", userId);
        for (auto &prop : props) {
            auto iter = std::find_if(cacheCfg.enabledInfos.begin(), cacheCfg.enabledInfos.end(),
                [&prop](const ImeEnabledInfo &info) { return prop.name == info.bundleName; });
            if (iter != cacheCfg.enabledInfos.end()) {
                prop.status = iter->enabledStatus;
                IMSA_HILOGD("get [%{public}d, %{public}s,%{public}d] from cache.", userId, prop.name.c_str(),
                    static_cast<int32_t>(prop.status));
                continue;
            }
        }
        IMSA_HILOGI("%{public}d get from cache end.", userId);
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("%{public}d get directly start.", userId);
    // get directly
    ImeEnabledCfg cfg;
    ret = GetEnabledCfg(userId, cfg, false);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    for (auto &prop : props) {
        auto iter = std::find_if(cfg.enabledInfos.begin(), cfg.enabledInfos.end(),
            [&prop](const ImeEnabledInfo &info) { return prop.name == info.bundleName; });
        if (iter != cfg.enabledInfos.end()) {
            prop.status = iter->enabledStatus;
            IMSA_HILOGD("get [%{public}d, %{public}s,%{public}d] directly.", userId, prop.name.c_str(),
                static_cast<int32_t>(prop.status));
        }
    }
    IMSA_HILOGI("%{public}d get directly end.", userId);
    // post correct task
    PostCorrectAddTask(userId);
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledCfgFromCacheWithCorrect(int32_t userId, ImeEnabledCfg &enabledCfg)
{
    auto ret = GetEnabledCfgFromCache(userId, enabledCfg);
    if (ret == ErrorCode::NO_ERROR) {
        return ret;
    }
    ret = CorrectUserAdd(userId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("correct %{public}d failed.", userId);
        return ret;
    }
    return GetEnabledCfgFromCache(userId, enabledCfg);
}

int32_t ImeEnabledInfoManager::GetEnabledCfgFromCache(int32_t userId, ImeEnabledCfg &enabledCfg)
{
    std::lock_guard<std::mutex> lock(imeEnabledCfgLock_);
    auto it = imeEnabledCfg_.find(userId);
    if (it == imeEnabledCfg_.end()) {
        IMSA_HILOGE("not find %{public}d in cache.", userId);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    enabledCfg = it->second;
    IMSA_HILOGI("num %{public}zu in cache.", enabledCfg.enabledInfos.size());
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::MergeFullExperienceTableCfg(int32_t userId, ImeEnabledCfg &cfg)
{
    if (!ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature) {
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("run in: %{public}d.", userId);
    std::string content;
    int32_t ret = SettingsDataUtils::GetInstance()->GetStringValue(SETTING_URI_PROXY, SECURITY_MODE, content);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGW("%{public}d get global full experience table failed:%{public}d.", userId, ret);
        return ret;
    }
    std::set<std::string> bundleNames;
    ret = ParseFullExperienceTableCfg(userId, content, bundleNames);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    for (const auto &bundleName : bundleNames) {
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
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledCfg(
    int32_t userId, ImeEnabledCfg &cfg, bool isCheckByBmg, const std::vector<FullImeInfo> &imeInfos)
{
    if (!SettingsDataUtils::GetInstance()->IsDataShareReady()) {
        IMSA_HILOGE("data share not ready.");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    {
        std::lock_guard<std::mutex> lock(settingOperateLock_);
        auto ret = GetEnabledTableCfg(userId, cfg);
        /* ERROR_EX_PARCELABLE is added to prevent the new table can not be write
           due to the parsing error of original table */
        /* ERROR_KEYWORD_NOT_FOUND is added to prevent the new table can not be write
           due to no original table */
        if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND
            && ret != ErrorCode::ERROR_EX_PARCELABLE) {
            return ret;
        }
        if (cfg.version.empty()) {
            ret = MergeFullExperienceTableCfg(userId, cfg);
            if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND
                && ret != ErrorCode::ERROR_EX_PARCELABLE) {
                return ret;
            }
        }
    }
    cfg.version = GetDisplayVersion();
    if (isCheckByBmg) {
        auto ret = CheckByBundleMgr(userId, cfg.enabledInfos, imeInfos);
        if (ret != ErrorCode::NO_ERROR) {
            return ret;
        }
    }
    CheckBySysEnabledSwitch(cfg.enabledInfos);
    CheckBySysIme(cfg.enabledInfos);
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledTableCfg(int32_t userId, ImeEnabledCfg &cfg)
{
    std::string userContent;
    int32_t ret = SettingsDataUtils::GetInstance()->GetStringValue(
        SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true", ENABLE_IME, userContent);
    if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        IMSA_HILOGW("%{public}d get user enabled table failed:%{public}d.", userId, ret);
        return ret;
    }
    bool hasUserEnabledTable = (ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND);
    bool isNewUserEnabledTable = (userContent.find("version") != std::string::npos);
    if (hasUserEnabledTable && isNewUserEnabledTable) {
        return GetNewEnabledTableCfg(userId, userContent, cfg);
    }
    std::string globalContent;
    ret = SettingsDataUtils::GetInstance()->GetStringValue(SETTING_URI_PROXY, ENABLE_IME, globalContent);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGW("%{public}d get global enabled table failed:%{public}d.", userId, ret);
        return ret;
    }
    ret = ImeEnabledInfoManager::GetOldEnabledTableCfg(userId, globalContent, cfg);
    if (ret == ErrorCode::NO_ERROR || !hasUserEnabledTable) {
        return ret;
    }
    return GetOldEnabledTableCfg(userId, userContent, cfg);
}

int32_t ImeEnabledInfoManager::GetOldEnabledTableCfg(int32_t userId, const std::string &content, ImeEnabledCfg &cfg)
{
    IMSA_HILOGI("%{public}d run in:%{public}s.", userId, content.c_str());
    EnabledImeCfg oldCfg;
    oldCfg.userImeCfg.userId = std::to_string(userId);
    if (!oldCfg.Unmarshall(content)) {
        IMSA_HILOGE("%{public}d Unmarshall failed:%{public}s.", userId, content.c_str());
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto bundleNames = std::set<std::string>(oldCfg.userImeCfg.identities.begin(), oldCfg.userImeCfg.identities.end());
    for (const auto &bundleName : bundleNames) {
        ImeEnabledInfo tmpInfo;
        tmpInfo.bundleName = bundleName;
        tmpInfo.enabledStatus = EnabledStatus::BASIC_MODE;
        cfg.enabledInfos.push_back(tmpInfo);
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetNewEnabledTableCfg(int32_t userId, const std::string &content, ImeEnabledCfg &cfg)
{
    IMSA_HILOGI("%{public}d run in.", userId);
    if (!cfg.Unmarshall(content)) {
        IMSA_HILOGE("%{public}d Unmarshall failed:%{public}s.", userId, content.c_str());
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::CheckByBundleMgr(
    int32_t userId, std::vector<ImeEnabledInfo> &enabledInfos, const std::vector<FullImeInfo> &imeInfos)
{
    IMSA_HILOGI("ime size, enabled:%{public}zu, installed:%{public}zu.", enabledInfos.size(), imeInfos.size());
    auto imeTmpInfos = imeInfos;
    if (imeTmpInfos.empty()) {
        auto ret = ImeInfoInquirer::GetInstance().QueryFullImeInfo(userId, imeTmpInfos);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("%{public}d QueryFullImeInfo failed.", userId);
            return ret;
        }
    }
    auto start =
        std::remove_if(enabledInfos.begin(), enabledInfos.end(), [&imeTmpInfos](const ImeEnabledInfo &enabledInfoTmp) {
            auto iter =
                std::find_if(imeTmpInfos.begin(), imeTmpInfos.end(), [&enabledInfoTmp](const FullImeInfo &imeInfoTmp) {
                    return enabledInfoTmp.bundleName == imeInfoTmp.prop.name;
                });
            return iter == imeTmpInfos.end();
        });
    enabledInfos.erase(start, enabledInfos.end());
    for (const auto &info : imeTmpInfos) {
        auto iter = std::find_if(enabledInfos.begin(), enabledInfos.end(),
            [&info](const ImeEnabledInfo &enabledInfoTmp) { return enabledInfoTmp.bundleName == info.prop.name; });
        if (iter != enabledInfos.end()) {
            // maybe uninstall and install when imsa abnormal
            if (!iter->installTime.empty() && info.installTime != iter->installTime) {
                iter->enabledStatus = ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState;
            }
            iter->installTime = info.installTime;
            iter->extensionName = info.prop.id;
            continue;
        }
        IMSA_HILOGW("%{public}s add.", info.prop.name.c_str());
        // maybe install when imsa abnormal or upgrade(install but disabled) or table abnormal
        ImeEnabledInfo infoTmp;
        infoTmp.bundleName = info.prop.name;
        infoTmp.extensionName = info.prop.id;
        infoTmp.enabledStatus = ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState;
        infoTmp.installTime = info.installTime;
        enabledInfos.push_back(infoTmp);
    }
    return ErrorCode::NO_ERROR;
}

void ImeEnabledInfoManager::CheckBySysEnabledSwitch(std::vector<ImeEnabledInfo> &infos)
{
    for (auto &info : infos) {
        CheckBySysEnabledSwitch(info);
    }
}

void ImeEnabledInfoManager::CheckBySysEnabledSwitch(ImeEnabledInfo &info)
{
    auto hasEnableSwitch = ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature;
    auto hasFullExperienceSwitch = ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature;
    IMSA_HILOGI("enable cfg:[%{public}d, %{public}d].", hasEnableSwitch, hasFullExperienceSwitch);
    if (!hasEnableSwitch && !hasFullExperienceSwitch) {
        info.enabledStatus = EnabledStatus::FULL_EXPERIENCE_MODE;
    }
    if (hasEnableSwitch && !hasFullExperienceSwitch) {
        if (info.enabledStatus == EnabledStatus::BASIC_MODE) {
            info.enabledStatus = EnabledStatus::FULL_EXPERIENCE_MODE;
        }
    }
    if (!hasEnableSwitch && hasFullExperienceSwitch) {
        if (info.enabledStatus == EnabledStatus::DISABLED) {
            info.enabledStatus = EnabledStatus::BASIC_MODE;
        }
    }
}

void ImeEnabledInfoManager::CheckBySysIme(std::vector<ImeEnabledInfo> &infos)
{
    for (auto &info : infos) {
        CheckBySysIme(info);
    }
}

void ImeEnabledInfoManager::CheckBySysIme(ImeEnabledInfo &info)
{
    auto sysIme = ImeInfoInquirer::GetInstance().GetDefaultIme();
    if (info.bundleName != sysIme.bundleName || info.enabledStatus != EnabledStatus::DISABLED) {
        return;
    }
    info.enabledStatus = EnabledStatus::BASIC_MODE;
}

int32_t ImeEnabledInfoManager::ParseFullExperienceTableCfg(
    int32_t userId, const std::string &content, std::set<std::string> &bundleNames)
{
    SecurityModeCfg cfg;
    cfg.userImeCfg.userId = std::to_string(userId);
    if (!cfg.Unmarshall(content)) {
        IMSA_HILOGE("%{public}d Unmarshall failed:%{public}s.", userId, content.c_str());
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    bundleNames = std::set<std::string>(cfg.userImeCfg.identities.begin(), cfg.userImeCfg.identities.end());
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::SetEnabledCfg(int32_t userId, const ImeEnabledCfg &cfg)
{
    std::lock_guard<std::mutex> lock(settingOperateLock_);
    std::string content;
    if (!cfg.Marshall(content)) {
        IMSA_HILOGE("Marshall failed");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    IMSA_HILOGI("[%{public}d, %{public}s].", userId, content.c_str());
    if (!SettingsDataUtils::GetInstance()->SetStringValue(
            SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true", ENABLE_IME, content)) {
        return ErrorCode::ERROR_ENABLE_IME;
    }
    {
        std::lock_guard<std::mutex> cgfLock(imeEnabledCfgLock_);
        imeEnabledCfg_.insert_or_assign(userId, cfg);
    }
    ModGlobalEnabledTable(userId, cfg);
    return ErrorCode::NO_ERROR;
}

void ImeEnabledInfoManager::PostCorrectAddTask(int32_t userId)
{
    auto task = [this, userId]() { CorrectUserAdd(userId); };
    if (eventHandler_ == nullptr) {
        return;
    }
    // 60000: ms, the task is time-consuming, prevent it is performed during the device boot
    eventHandler_->PostTask(task, "correctAdd", 60000, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void ImeEnabledInfoManager::NotifyEnableChange(int32_t userId, const std::string &bundleName, EnabledStatus oldStatus)
{
    IMSA_HILOGI(
        "notify:[%{public}d,%{public}s,%{public}d].", userId, bundleName.c_str(), static_cast<int32_t>(oldStatus));
    auto notifyTask = [this, userId, bundleName, oldStatus]() {
        if (handler_ != nullptr) {
            handler_(userId, bundleName, oldStatus);
        }
    };
    if (eventHandler_ == nullptr) {
        return;
    }
    eventHandler_->PostTask(notifyTask, "NotifyEnableChangeTask", 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

bool ImeEnabledInfoManager::HasEnabledSwitch()
{
    return ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature
           || ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature;
}

bool ImeEnabledInfoManager::IsDefaultFullMode(int32_t userId, const std::string &bundleName)
{
    auto defaultIme = ImeInfoInquirer::GetInstance().GetDefaultImeCfgProp();
    if (defaultIme != nullptr && bundleName == defaultIme->name) {
        return true;
    }
    std::string appId;
    uint32_t versionCode;
    if (!ImeInfoInquirer::GetInstance().GetImeAppId(userId, bundleName, appId)
        || !ImeInfoInquirer::GetInstance().GetImeVersionCode(userId, bundleName, versionCode)) {
        IMSA_HILOGE("%{public}s failed to get appId and versionCode", bundleName.c_str());
        return false;
    }
    std::vector<DefaultFullImeInfo> defaultFullImeList;
    if (!SysCfgParser::ParseDefaultFullIme(defaultFullImeList)) {
        IMSA_HILOGE("failed to parse config");
        return false;
    }
    auto ime = std::find_if(defaultFullImeList.begin(), defaultFullImeList.end(),
        [&appId](const auto &ime) { return ime.appId == appId; });
    if (ime == defaultFullImeList.end()) {
        IMSA_HILOGD("not default FULL");
        return false;
    }
    bool isDefaultFull = false;
    if (ime->expirationVersionCode > 0) {
        isDefaultFull = !IsExpired(ime->expirationTime) || versionCode < ime->expirationVersionCode;
    } else {
        isDefaultFull = !IsExpired(ime->expirationTime);
    }
    IMSA_HILOGI("ime: %{public}s, isDefaultFull: %{public}d", bundleName.c_str(), isDefaultFull);
    return isDefaultFull;
}

bool ImeEnabledInfoManager::IsExpired(const std::string &expirationTime)
{
    std::istringstream expirationTimeStr(expirationTime);
    std::tm expTime = {};
    expirationTimeStr >> std::get_time(&expTime, "%Y-%m-%d %H:%M:%S");
    if (expirationTimeStr.fail()) {
        IMSA_HILOGE("get time error, expirationTime: %{public}s", expirationTime.c_str());
        return false;
    }
    auto expTimePoint = std::chrono::system_clock::from_time_t(std::mktime(&expTime));
    auto now = std::chrono::system_clock::now();
    return expTimePoint < now;
}

void ImeEnabledInfoManager::OnFullExperienceTableChanged(int32_t userId)
{
    IMSA_HILOGD("start.");
    auto defaultImeBundleName = ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName;
    if (defaultImeBundleName.empty()) {
        return;
    }
    EnabledStatus oldStatus;
    ImeEnabledInfoManager::GetInstance().GetEnabledState(userId, defaultImeBundleName, oldStatus);
    std::string content;
    int32_t ret = SettingsDataUtils::GetInstance()->GetStringValue(SETTING_URI_PROXY, SECURITY_MODE, content);
    if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        IMSA_HILOGE("read full experience table failed: %{public}d", ret);
        return;
    }
    EnabledStatus newStatus = EnabledStatus::BASIC_MODE;
    if (ret == ErrorCode::NO_ERROR) {
        std::set<std::string> bundleNames;
        ParseFullExperienceTableCfg(userId, content, bundleNames);
        auto it = std::find_if(bundleNames.begin(), bundleNames.end(),
            [&defaultImeBundleName](const std::string &bundleName) { return bundleName == defaultImeBundleName; });
        if (it != bundleNames.end()) {
            newStatus = EnabledStatus::FULL_EXPERIENCE_MODE;
        }
    }
    if (newStatus == oldStatus) {
        return;
    }
    Update(userId, defaultImeBundleName, ImeInfoInquirer::GetInstance().GetDefaultIme().extName, newStatus);
    IMSA_HILOGD("end.");
}

void ImeEnabledInfoManager::ModGlobalEnabledTable(int32_t userId, const ImeEnabledCfg &newEnabledCfg)
{
    IMSA_HILOGD("start.");
    if (userId != currentUserId_) {
        IMSA_HILOGW("[%{public}d,%{public}d] not same.", userId, currentUserId_);
        return;
    }
    std::string oldGlobalContent;
    auto ret = SettingsDataUtils::GetInstance()->GetStringValue(SETTING_URI_PROXY, ENABLE_IME, oldGlobalContent);
    if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        IMSA_HILOGW("%{public}d get global enabled table failed:%{public}d.", userId, ret);
        return;
    }
    IMSA_HILOGI("global str:%{public}s.", oldGlobalContent);
    EnabledImeCfg cfg;
    for (const auto &info : newEnabledCfg.enabledInfos) {
        if (info.enabledStatus != EnabledStatus::DISABLED) {
            cfg.userImeCfg.identities.push_back(info.bundleName);
        }
    }
    std::string newGlobalContent;
    cfg.userImeCfg.userId = std::to_string(userId);
    if (!cfg.Marshall(newGlobalContent)) {
        IMSA_HILOGE("%{public}d Unmarshall failed:%{public}s.", userId, newGlobalContent.c_str());
        return;
    }
    if (newGlobalContent == oldGlobalContent) {
        IMSA_HILOGD("[%{public}s,%{public}s] same, not deal.", newGlobalContent.c_str(), oldGlobalContent.c_str());
        return;
    }
    if (ret == ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        SettingsDataUtils::GetInstance()->SetStringValue(SETTING_URI_PROXY, ENABLE_IME, newGlobalContent);
        return;
    }
    auto globalTableUserId = GetGlobalTableUserId(oldGlobalContent);
    if (globalTableUserId.empty() || userId == atoi(globalTableUserId.c_str())) {
        SettingsDataUtils::GetInstance()->SetStringValue(SETTING_URI_PROXY, ENABLE_IME, newGlobalContent);
        return;
    }
    std::string oldGlobalUserContent;
    ret = SettingsDataUtils::GetInstance()->GetStringValue(
        SETTINGS_USER_DATA_URI + globalTableUserId + "?Proxy=true", ENABLE_IME, oldGlobalUserContent);
    if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        IMSA_HILOGW("%{public}d get user enabled table failed:%{public}d.", userId, ret);
        return;
    }
    if (ret == ErrorCode::ERROR_KEYWORD_NOT_FOUND || oldGlobalUserContent.find("version") == std::string::npos) {
        if (SettingsDataUtils::GetInstance()->SetStringValue(
                SETTINGS_USER_DATA_URI + globalTableUserId + "?Proxy=true", ENABLE_IME, oldGlobalContent)) {
            SettingsDataUtils::GetInstance()->SetStringValue(SETTING_URI_PROXY, ENABLE_IME, newGlobalContent);
        }
        return;
    }
    SettingsDataUtils::GetInstance()->SetStringValue(SETTING_URI_PROXY, ENABLE_IME, newGlobalContent);
    IMSA_HILOGD("end.");
}

std::string ImeEnabledInfoManager::GetGlobalTableUserId(const std::string &valueStr)
{
    auto root = cJSON_Parse(valueStr.c_str());
    if (root == nullptr) {
        IMSA_HILOGE("valueStr content parse failed!");
        return "";
    }
    auto subNode = Serializable::GetSubNode(root, "enableImeList");
    if (subNode == nullptr || !cJSON_IsObject(subNode)) {
        IMSA_HILOGW("subNode is null or not object");
        cJSON_Delete(root);
        return "";
    }
    if (subNode->child == nullptr) {
        IMSA_HILOGW("subNode has not child");
        cJSON_Delete(root);
        return "";
    }
    std::string userId = subNode->child->string;
    cJSON_Delete(root);
    return userId;
}
} // namespace MiscServices
} // namespace OHOS