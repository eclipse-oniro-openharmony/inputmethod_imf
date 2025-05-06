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

#include "enable_upgrade_manager.h"
#include "ime_info_inquirer.h"
namespace OHOS {
namespace MiscServices {
using namespace std::chrono;
constexpr const char *SYSTEM_SPECIAL_IME = "";
ImeEnabledInfoManager &ImeEnabledInfoManager::GetInstance()
{
    static ImeEnabledInfoManager instance;
    return instance;
}

ImeEnabledInfoManager::~ImeEnabledInfoManager()
{
}

void ImeEnabledInfoManager::SetCurrentImeStatusChangedHandler(CurrentImeStatusChangedHandler handler)
{
    if (currentImeStatusChangedHandler_ != nullptr) {
        return;
    }
    currentImeStatusChangedHandler_ = std::move(handler);
}

void ImeEnabledInfoManager::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler)
{
    serviceHandler_ = eventHandler;
}

int32_t ImeEnabledInfoManager::Init(const std::map<int32_t, std::vector<FullImeInfo>> &fullImeInfos)
{
    std::lock_guard<std::mutex> lock(operateLock_);
    if (!SettingsDataUtils::GetInstance().IsDataShareReady()) {
        IMSA_HILOGE("data share not ready.");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    IMSA_HILOGI("run in, user num:%{public}zu.", fullImeInfos.size());
    for (const auto &fullImeInfo : fullImeInfos) {
        auto cfg = GetEnabledCache(fullImeInfo.first);
        if (!cfg.enabledInfos.empty()) {
            continue;
        }
        UpdateEnabledCfgCache(fullImeInfo.first, fullImeInfo.second);
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::Switch(int32_t userId, const std::vector<FullImeInfo> &imeInfos)
{
    std::lock_guard<std::mutex> lock(operateLock_);
    IMSA_HILOGI("userId:%{public}d", userId);
    auto cfg = GetEnabledCache(userId);
    if (!cfg.enabledInfos.empty()) {
        UpdateGlobalEnabledTable(userId, cfg); // user switch, replace global table by user table
        return ErrorCode::NO_ERROR;
    }
    if (!SettingsDataUtils::GetInstance().IsDataShareReady()) {
        IMSA_HILOGE("data share not ready.");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    return UpdateEnabledCfgCache(userId, imeInfos);
}

int32_t ImeEnabledInfoManager::UpdateEnabledCfgCache(int32_t userId, const std::vector<FullImeInfo> &imeInfos)
{
    IMSA_HILOGI("run in, userId:%{public}d", userId);
    ImeEnabledCfg cfg;
    auto ret = GetEnabledCfg(userId, imeInfos, cfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId:%{public}d get enabled cfg failed:%{public}d.", userId, ret);
        return ret;
    }
    return UpdateEnabledCfgCache(userId, cfg);
}

int32_t ImeEnabledInfoManager::Delete(int32_t userId)
{
    ClearEnabledCache(userId);
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::Add(int32_t userId, const FullImeInfo &imeInfo)
{
    std::lock_guard<std::mutex> lock(operateLock_);
    IMSA_HILOGI("userId:%{public}d, bundleName:%{public}s", userId, imeInfo.prop.name.c_str());
    if (imeInfo.prop.name == ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName) {
        IMSA_HILOGI("[%{public}d,%{public}s] is sys ime, deal in init or user add.", userId, imeInfo.prop.name.c_str());
        return ErrorCode::NO_ERROR;
    }
    ImeEnabledCfg enabledCfg;
    auto ret = GetEnabledCacheWithCorrect(userId, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
        [&imeInfo](const ImeEnabledInfo &info) { return imeInfo.prop.name == info.bundleName; });
    if (iter != enabledCfg.enabledInfos.end()) {
        IMSA_HILOGI("%{public}d/%{public}s install after uninstall", userId, imeInfo.prop.name.c_str());
        if (iter->extensionName.empty()) {
            IMSA_HILOGW(
                "%{public}d/%{public}s uninstall before upgrade install again.", userId, imeInfo.prop.name.c_str());
            iter->extensionName = imeInfo.prop.id;
            return UpdateEnabledCfgCache(userId, enabledCfg);
        }
        return ErrorCode::NO_ERROR;
    }
    enabledCfg.enabledInfos.emplace_back(imeInfo.prop.name, imeInfo.prop.id,
        ComputeEnabledStatus(imeInfo.prop.name, ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState));
    return UpdateEnabledCfgCache(userId, enabledCfg);
}

int32_t ImeEnabledInfoManager::Delete(int32_t userId, const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(operateLock_);
    IMSA_HILOGI("enable Delete run in, userId:%{public}d, bundleName:%{public}s", userId, bundleName.c_str());
    ImeEnabledCfg enabledCfg;
    auto ret = GetEnabledCacheWithCorrect(userId, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("user:%{public}d get enabledCfg failed:%{public}d.", userId, ret);
        return ret;
    }
    if (!IsCurrentIme(bundleName, enabledCfg.enabledInfos)) {
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGW("current ime %{public}d/%{public}s uninstall", userId, bundleName.c_str());
    ModCurrentIme(enabledCfg.enabledInfos);
    ret = UpdateEnabledCfgCache(userId, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d update enable info failed:%{public}d.", userId, ret);
    }
    NotifyCurrentImeStatusChanged(userId, bundleName, EnabledStatus::DISABLED);
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::CheckUpdate(
    int32_t userId, const std::string &bundleName, const std::string &extensionName, EnabledStatus status)
{
    IMSA_HILOGI("update info:[%{public}d, %{public}s, %{public}s, %{public}d].", userId, bundleName.c_str(),
        extensionName.c_str(), static_cast<int32_t>(status));
    if (bundleName.empty()) {
        IMSA_HILOGE("%{public}d bundleName:%{public}s abnormal.", userId, bundleName.c_str());
        return ErrorCode::ERROR_PARAMETER_CHECK_FAILED;
    }
    if (ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName == bundleName && status == EnabledStatus::DISABLED) {
        IMSA_HILOGW("[%{public}d,%{public}s] is sys ime, do not set DISABLED.", userId, bundleName.c_str());
        return ErrorCode::ERROR_DISABLE_SYSTEM_IME;
    }
    if (extensionName.empty()) {
        if (!ImeInfoInquirer::GetInstance().IsInputMethod(userId, bundleName)) {
            IMSA_HILOGE(
                "[%{public}d, %{public}s, %{public}s] not an ime.", userId, bundleName.c_str(), extensionName.c_str());
            return ErrorCode::ERROR_IME_NOT_FOUND;
        }
    } else {
        if (!ImeInfoInquirer::GetInstance().IsImeInstalled(userId, bundleName, extensionName)) {
            IMSA_HILOGE(
                "[%{public}d, %{public}s, %{public}s] not an ime.", userId, bundleName.c_str(), extensionName.c_str());
            return ErrorCode::ERROR_IME_NOT_FOUND;
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::Update(
    int32_t userId, const std::string &bundleName, const std::string &extensionName, EnabledStatus status)
{
    std::lock_guard<std::mutex> lock(operateLock_);
    auto ret = CheckUpdate(userId, bundleName, extensionName, status);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    ImeEnabledCfg enabledCfg;
    ret = GetEnabledCacheWithCorrect(userId, bundleName, extensionName, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        return ErrorCode::ERROR_ENABLE_IME;
    }
    auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
        [&bundleName](const ImeEnabledInfo &info) { return bundleName == info.bundleName; });
    if (iter == enabledCfg.enabledInfos.end()) {
        IMSA_HILOGE("[%{public}d, %{public}s] not find.", userId, bundleName.c_str());
        return ErrorCode::ERROR_IME_NOT_FOUND;
    }
    if (iter->enabledStatus == status) {
        return ErrorCode::NO_ERROR;
    }
    iter->enabledStatus = status;
    iter->stateUpdateTime =
        std::to_string(duration_cast<std::chrono::milliseconds>(system_clock::now().time_since_epoch()).count());
    auto isCurrentIme = IsCurrentIme(bundleName, enabledCfg.enabledInfos);
    if (isCurrentIme && status == EnabledStatus::DISABLED) {
        ModCurrentIme(enabledCfg.enabledInfos);
    }
    ret = UpdateEnabledCfgCache(userId, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d update enable info failed:%{public}d.", userId, ret);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    if (isCurrentIme) {
        NotifyCurrentImeStatusChanged(userId, bundleName, iter->enabledStatus);
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledState(int32_t userId, const std::string &bundleName, EnabledStatus &status)
{
    std::lock_guard<std::mutex> lock(operateLock_);
    IMSA_HILOGD("[%{public}d, %{public}s] start.", userId, bundleName.c_str());
    if (bundleName.empty()) {
        IMSA_HILOGW("%{public}d bundleName is empty.", userId);
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    if (!HasEnabledSwitch()) {
        status = EnabledStatus::FULL_EXPERIENCE_MODE;
        return ErrorCode::NO_ERROR;
    }
    if (bundleName == SYSTEM_SPECIAL_IME) {
        status = EnabledStatus::FULL_EXPERIENCE_MODE;
        return ErrorCode::NO_ERROR;
    }
    auto ret = GetEnabledStateInner(userId, bundleName, status);
    if (bundleName == ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName &&
        (ret != ErrorCode::NO_ERROR || status == EnabledStatus::DISABLED)) {
        IMSA_HILOGI("mod sys ime enabledStatus.");
        status = EnabledStatus::BASIC_MODE;
    }
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d %{public}s get status failed %{public}d.", userId, bundleName.c_str(), ret);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledStates(int32_t userId, std::vector<Property> &props)
{
    std::lock_guard<std::mutex> lock(operateLock_);
    if (props.empty()) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    if (!HasEnabledSwitch()) {
        for (auto &prop : props) {
            prop.status = EnabledStatus::FULL_EXPERIENCE_MODE;
        }
        return ErrorCode::NO_ERROR;
    }
    auto ret = GetEnabledStatesInner(userId, props);
    for (auto &prop : props) {
        if (prop.name == ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName &&
            prop.status == EnabledStatus::DISABLED) {
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

int32_t ImeEnabledInfoManager::GetEnabledStateInner(
    int32_t userId, const std::string &bundleName, EnabledStatus &status)
{
    IMSA_HILOGD("%{public}d/%{public}s get enabledStatus start.", userId, bundleName.c_str());
    ImeEnabledCfg enabledCfg;
    auto ret = GetEnabledCacheWithCorrect(userId, bundleName, "", enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        return ErrorCode::ERROR_ENABLE_IME;
    }
    auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
        [&bundleName](const ImeEnabledInfo &info) { return bundleName == info.bundleName; });
    if (iter == enabledCfg.enabledInfos.end()) {
        IMSA_HILOGE("not find [%{public}d, %{public}s] in enableCfg.", userId, bundleName.c_str());
        return ErrorCode::ERROR_ENABLE_IME;
    }
    status = iter->enabledStatus;
    IMSA_HILOGD("%{public}d/%{public}s get enabledStatus end:%{public}d.", userId, bundleName.c_str(),
        static_cast<int32_t>(status));
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledStatesInner(int32_t userId, std::vector<Property> &props)
{
    IMSA_HILOGD("%{public}d/%{public}zu get enabledStatus start.", userId, props.size());
    auto cfg = GetEnabledCache(userId);
    if (cfg.enabledInfos.empty()) {
        auto ret = UpdateEnabledCfgCache(userId);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("%{public}d update enable info failed:%{public}d.", userId, ret);
            return ret;
        }
        cfg = GetEnabledCache(userId);
    }
    for (auto &prop : props) {
        auto iter = std::find_if(cfg.enabledInfos.begin(), cfg.enabledInfos.end(),
            [&prop](const ImeEnabledInfo &info) { return prop.name == info.bundleName; });
        if (iter == cfg.enabledInfos.end()) {
            IMSA_HILOGW("%{public}d/%{public}s enable info abnormal.", userId, prop.name.c_str());
            continue;
        }
        prop.status = iter->enabledStatus;
        IMSA_HILOGD("%{public}d/%{public}s get succeed:%{public}d.", userId, prop.name.c_str(),
            static_cast<int32_t>(prop.status));
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledCacheWithCorrect(int32_t userId, ImeEnabledCfg &enabledCfg)
{
    enabledCfg = GetEnabledCache(userId);
    if (!enabledCfg.enabledInfos.empty()) {
        return ErrorCode::NO_ERROR;
    }
    auto ret = UpdateEnabledCfgCache(userId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d update enable info failed:%{public}d.", userId, ret);
        return ret;
    }
    enabledCfg = GetEnabledCache(userId);
    return enabledCfg.enabledInfos.empty() ? ErrorCode::ERROR_ENABLE_IME : ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledCacheWithCorrect(
    int32_t userId, const std::string &bundleName, const std::string &extensionName, ImeEnabledCfg &enabledCfg)
{
    enabledCfg = GetEnabledCache(userId);
    auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
        [&bundleName](const ImeEnabledInfo &info) { return bundleName == info.bundleName; });
    if (iter != enabledCfg.enabledInfos.end()) {
        return ErrorCode::NO_ERROR;
    }
    auto ret = UpdateEnabledCfgCache(userId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d update enable info failed:%{public}d.", userId, ret);
        return ret;
    }
    enabledCfg = GetEnabledCache(userId);
    return enabledCfg.enabledInfos.empty() ? ErrorCode::ERROR_ENABLE_IME : ErrorCode::NO_ERROR;
}

bool ImeEnabledInfoManager::IsInEnabledCache(
    int32_t userId, const std::string &bundleName, const std::string &extensionName)
{
    auto cfg = GetEnabledCache(userId);
    auto iter = std::find_if(cfg.enabledInfos.begin(), cfg.enabledInfos.end(),
        [&bundleName](const ImeEnabledInfo &info) { return bundleName == info.bundleName; });
    return iter != cfg.enabledInfos.end();
}

void ImeEnabledInfoManager::SetEnabledCache(int32_t userId, const ImeEnabledCfg &cfg)
{
    std::lock_guard<std::mutex> cgfLock(imeEnabledCfgLock_);
    imeEnabledCfg_.insert_or_assign(userId, cfg);
}

ImeEnabledCfg ImeEnabledInfoManager::GetEnabledCache(int32_t userId)
{
    std::lock_guard<std::mutex> lock(imeEnabledCfgLock_);
    auto it = imeEnabledCfg_.find(userId);
    if (it == imeEnabledCfg_.end()) {
        IMSA_HILOGE("not find %{public}d in cache.", userId);
        return {};
    }
    IMSA_HILOGD("num %{public}zu in cache.", it->second.enabledInfos.size());
    return it->second;
}

void ImeEnabledInfoManager::ClearEnabledCache(int32_t userId)
{
    std::lock_guard<std::mutex> cfgLock(imeEnabledCfgLock_);
    imeEnabledCfg_.erase(userId);
}

int32_t ImeEnabledInfoManager::GetEnabledCfg(
    int32_t userId, const std::vector<FullImeInfo> &imeInfos, ImeEnabledCfg &cfg)
{
    std::string userContent;
    auto ret = SettingsDataUtils::GetInstance().GetStringValue(
        SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true", SettingsDataUtils::ENABLE_IME, userContent);
    if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        IMSA_HILOGE("%{public}d get enabled table failed:%{public}d.", userId, ret);
        return ret;
    }
    // system reboot
    if (userContent.find("version") != std::string::npos) {
        cfg.Unmarshall(userContent);
        return CorrectByBundleMgr(userId, imeInfos, cfg.enabledInfos);
    }
    // the system version upgrade or the system starts for the first time
    ret = EnableUpgradeManager::GetInstance().Upgrade(userId, imeInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d upgrade failed:%{public}d.", userId, ret);
        return ret;
    }
    ret = SettingsDataUtils::GetInstance().GetStringValue(
        SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true", SettingsDataUtils::ENABLE_IME, userContent);
    if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        IMSA_HILOGE("%{public}d get enabled table failed after upgrade:%{public}d.", userId, ret);
        return ret;
    }
    cfg.Unmarshall(userContent);
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::CorrectByBundleMgr(
    int32_t userId, const std::vector<FullImeInfo> &imeInfos, std::vector<ImeEnabledInfo> &enabledInfos)
{
    auto finalImeInfos = imeInfos;
    if (finalImeInfos.empty()) {
        auto ret = ImeInfoInquirer::GetInstance().QueryFullImeInfo(userId, finalImeInfos, true);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("%{public}d QueryFullImeInfo failed.", userId);
            return ret;
        }
    }
    IMSA_HILOGI("ime size, enabled:%{public}zu, installed:%{public}zu.", enabledInfos.size(), finalImeInfos.size());
    for (const auto &enabledInfo : enabledInfos) {
        if (!enabledInfo.extraInfo.isDefaultIme) {
            continue;
        }
        auto iter = std::find_if(finalImeInfos.begin(), finalImeInfos.end(),
            [&enabledInfo](const FullImeInfo &imeInfoTmp) { return enabledInfo.bundleName == imeInfoTmp.prop.name; });
        if (iter == finalImeInfos.end()) {
            IMSA_HILOGW("current ime %{public}d/%{public}s uninstall when imsa abnormal", userId,
                enabledInfo.bundleName.c_str());
            ModCurrentIme(enabledInfos);
            break;
        }
    }
    for (const auto &imeInfo : finalImeInfos) {
        auto iter = std::find_if(enabledInfos.begin(), enabledInfos.end(),
            [&imeInfo](const auto &enabledInfoTmp) { return enabledInfoTmp.bundleName == imeInfo.prop.name; });
        if (iter != enabledInfos.end()) {
            if (iter->extensionName.empty()) {
                IMSA_HILOGW("%{public}d/%{public}s uninstall before upgrade install again when imsa abnormal", userId,
                    imeInfo.prop.name.c_str());
                iter->extensionName = imeInfo.prop.id;
            }
            continue;
        }
        IMSA_HILOGW("%{public}d/%{public}s first install when imsa abnormal", userId, imeInfo.prop.name.c_str());
        enabledInfos.emplace_back(imeInfo.prop.name, imeInfo.prop.id,
            ComputeEnabledStatus(imeInfo.prop.name, ImeInfoInquirer::GetInstance().GetSystemConfig().initEnabledState));
    }
    return ErrorCode::NO_ERROR;
}

EnabledStatus ImeEnabledInfoManager::ComputeEnabledStatus(const std::string &bundleName, EnabledStatus initStatus)
{
    if (!HasEnabledSwitch()) {
        return EnabledStatus::FULL_EXPERIENCE_MODE;
    }
    auto sysIme = ImeInfoInquirer::GetInstance().GetDefaultIme();
    if (bundleName == sysIme.bundleName && initStatus == EnabledStatus::DISABLED) {
        return EnabledStatus::BASIC_MODE;
    }
    return initStatus;
}

int32_t ImeEnabledInfoManager::UpdateEnabledCfgCache(int32_t userId, const ImeEnabledCfg &cfg)
{
    std::string content;
    if (!cfg.Marshall(content)) {
        IMSA_HILOGE("Marshall failed");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    IMSA_HILOGD("[%{public}d, %{public}s].", userId, content.c_str());
    if (!SettingsDataUtils::GetInstance().SetStringValue(
        SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true", SettingsDataUtils::ENABLE_IME, content)) {
        IMSA_HILOGE("%{public}d SetStringValue:%{public}s failed.", userId, content.c_str());
        return ErrorCode::ERROR_ENABLE_IME;
    }
    SetEnabledCache(userId, cfg);
    UpdateGlobalEnabledTable(userId, cfg);
    return ErrorCode::NO_ERROR;
}

void ImeEnabledInfoManager::NotifyCurrentImeStatusChanged(
    int32_t userId, const std::string &bundleName, EnabledStatus newStatus)
{
    if (serviceHandler_ == nullptr) {
        return;
    }
    auto notifyTask = [this, userId, bundleName, newStatus]() {
        if (currentImeStatusChangedHandler_ != nullptr) {
            currentImeStatusChangedHandler_(userId, bundleName, newStatus);
        }
    };
    serviceHandler_->PostTask(notifyTask, "NotifyCurrentImeStatusChanged", 0, AppExecFwk::EventQueue::Priority::VIP);
}

bool ImeEnabledInfoManager::HasEnabledSwitch()
{
    auto sysCfg = ImeInfoInquirer::GetInstance().GetSystemConfig();
    return sysCfg.enableInputMethodFeature || sysCfg.enableFullExperienceFeature;
}

bool ImeEnabledInfoManager::IsDefaultFullMode(int32_t userId, const std::string &bundleName)
{
    auto defaultIme = ImeInfoInquirer::GetInstance().GetDefaultImeCfgProp();
    if (defaultIme != nullptr && bundleName == defaultIme->name) {
        return true;
    }
    std::string appId;
    uint32_t versionCode;
    if (!ImeInfoInquirer::GetInstance().GetImeAppId(userId, bundleName, appId) ||
        !ImeInfoInquirer::GetInstance().GetImeVersionCode(userId, bundleName, versionCode)) {
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

void ImeEnabledInfoManager::UpdateGlobalEnabledTable(int32_t userId, const ImeEnabledCfg &newEnabledCfg)
{
    IMSA_HILOGD("start.");
    if (serviceHandler_ == nullptr) {
        return;
    }
    auto task = [userId, newEnabledCfg]() {
        EnableUpgradeManager::GetInstance().UpdateGlobalEnabledTable(userId, newEnabledCfg);
    };
    serviceHandler_->PostTask(task, "UpdateGlobalEnabledTable", 0, AppExecFwk::EventQueue::Priority::LOW);
    IMSA_HILOGD("end.");
}

void ImeEnabledInfoManager::OnFullExperienceTableChanged(int32_t userId)
{
    auto defaultIme = ImeInfoInquirer::GetInstance().GetDefaultIme();
    if (defaultIme.bundleName.empty()) {
        return;
    }
    std::set<std::string> bundleNames;
    auto ret = EnableUpgradeManager::GetInstance().GetFullExperienceTable(userId, bundleNames);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId:%{public}d read full experience table failed: %{public}d", userId, ret);
        return;
    }
    auto newStatus = EnabledStatus::BASIC_MODE;
    auto it = std::find_if(bundleNames.begin(), bundleNames.end(),
        [&defaultIme](const std::string &bundleName) { return bundleName == defaultIme.bundleName; });
    if (it != bundleNames.end()) {
        newStatus = EnabledStatus::FULL_EXPERIENCE_MODE;
    }
    IMSA_HILOGI("%{public}d full experience table changed.", userId);
    ImeEnabledInfoManager::GetInstance().Update(userId, defaultIme.bundleName, defaultIme.extName, newStatus);
}

int32_t ImeEnabledInfoManager::SetCurrentIme(
    int32_t userId, const std::string &imeId, const std::string &subName, bool isSetByUser)
{
    std::lock_guard<std::mutex> lock(operateLock_);
    if (imeId.empty()) {
        return ErrorCode::NO_ERROR;
    }
    auto [bundleName, extName] = SplitImeId(imeId);
    ImeEnabledCfg enabledCfg;
    auto ret = GetEnabledCacheWithCorrect(userId, bundleName, extName, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d get enable info failed:%{public}d.", userId, ret);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
        [](const auto &info) { return info.extraInfo.isTmpIme; });
    if (iter != enabledCfg.enabledInfos.end()) {
        IMSA_HILOGI("%{public}d has tmp ime:%{public}s, not mod.", userId, iter->bundleName.c_str());
        return ErrorCode::NO_ERROR;
    }
    for (auto &info : enabledCfg.enabledInfos) {
        info.extraInfo.currentSubName = "";
        info.extraInfo.isDefaultImeSet = false;
        info.extraInfo.isDefaultIme = false;
        if (info.bundleName == bundleName) {
            info.extensionName = extName;
            info.extraInfo.currentSubName = subName;
            info.extraInfo.isDefaultImeSet = isSetByUser;
            info.extraInfo.isDefaultIme = true;
        }
    }
    ret = UpdateEnabledCfgCache(userId, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d update enable info failed:%{public}d.", userId, ret);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::SetTmpIme(int32_t userId, const std::string &imeId)
{
    std::lock_guard<std::mutex> lock(operateLock_);
    IMSA_HILOGI("%{public}d set tmp ime:%{public}s.", userId, imeId.c_str());
    ImeEnabledCfg enabledCfg;
    if (imeId.empty()) {
        auto ret = GetEnabledCacheWithCorrect(userId, enabledCfg);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("%{public}d get enable info failed:%{public}d.", userId, ret);
            return ret;
        }
        auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
            [](const auto &info) { return info.extraInfo.isTmpIme; });
        if (iter == enabledCfg.enabledInfos.end()) {
            return ErrorCode::NO_ERROR;
        }
        for (auto &info : enabledCfg.enabledInfos) {
            info.extraInfo.isTmpIme = false;
        }
    } else {
        auto [bundleName, extName] = SplitImeId(imeId);
        auto ret = GetEnabledCacheWithCorrect(userId, bundleName, extName, enabledCfg);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("%{public}d/%{public}s get enable info failed:%{public}d.", userId, bundleName.c_str(), ret);
            return ret;
        }
        auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
            [&tmpBundleName = bundleName](
                const auto &info) { return info.extraInfo.isTmpIme && info.bundleName == tmpBundleName; });
        if (iter != enabledCfg.enabledInfos.end()) {
            IMSA_HILOGI("%{public}d set ime same with tmp ime:%{public}s, not mod.", userId, iter->bundleName.c_str());
            return ErrorCode::NO_ERROR;
        }
        for (auto &info : enabledCfg.enabledInfos) {
            info.extraInfo.isTmpIme = false;
            if (info.bundleName == bundleName) {
                info.extraInfo.isTmpIme = true;
            }
        }
    }
    auto ret = UpdateEnabledCfgCache(userId, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d update enable info failed:%{public}d.", userId, ret);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<ImeNativeCfg> ImeEnabledInfoManager::GetCurrentImeCfg(int32_t userId)
{
    std::lock_guard<std::mutex> lock(operateLock_);
    ImeEnabledCfg enabledCfg;
    auto ret = GetEnabledCacheWithCorrect(userId, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d get enable info failed:%{public}d.", userId, ret);
        return std::make_shared<ImeNativeCfg>();
    }
    auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
        [](const auto &info) { return info.extraInfo.isTmpIme; });
    if (iter != enabledCfg.enabledInfos.end()) {
        IMSA_HILOGI("%{public}d has tmp default ime:%{public}s.", userId, iter->bundleName.c_str());
        ImeNativeCfg nativeInfo;
        nativeInfo.imeId = iter->bundleName + "/" + iter->extensionName;
        nativeInfo.bundleName = iter->bundleName;
        nativeInfo.extName = iter->extensionName;
        return std::make_shared<ImeNativeCfg>(nativeInfo);
    }
    iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
        [](const auto &info) { return info.extraInfo.isDefaultIme; });
    if (iter != enabledCfg.enabledInfos.end()) {
        IMSA_HILOGI("%{public}d has default ime:%{public}s.", userId, iter->bundleName.c_str());
        ImeNativeCfg nativeInfo;
        nativeInfo.imeId = iter->bundleName + "/" + iter->extensionName;
        nativeInfo.bundleName = iter->bundleName;
        nativeInfo.extName = iter->extensionName;
        nativeInfo.subName = iter->extraInfo.currentSubName;
        return std::make_shared<ImeNativeCfg>(nativeInfo);
    }
    IMSA_HILOGI("%{public}d not set default ime.", userId);
    return std::make_shared<ImeNativeCfg>();
}

bool ImeEnabledInfoManager::IsDefaultImeSet(int32_t userId)
{
    std::lock_guard<std::mutex> lock(operateLock_);
    ImeEnabledCfg enabledCfg;
    auto ret = GetEnabledCacheWithCorrect(userId, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d get enable info failed:%{public}d.", userId, ret);
        return false;
    }
    for (auto &info : enabledCfg.enabledInfos) {
        if (info.extraInfo.isDefaultIme) {
            return info.extraInfo.isDefaultImeSet;
        }
    }
    return false;
}

std::pair<std::string, std::string> ImeEnabledInfoManager::SplitImeId(const std::string &imeId)
{
    std::string bundleName;
    std::string extName;
    auto pos = imeId.find('/');
    if (pos != std::string::npos && pos + 1 < imeId.size()) {
        bundleName = imeId.substr(0, pos);
        extName = imeId.substr(pos + 1);
    }
    return std::make_pair(bundleName, extName);
}

void ImeEnabledInfoManager::ModCurrentIme(std::vector<ImeEnabledInfo> &enabledInfos)
{
    std::string oldBundleName;
    auto oldIter = std::find_if(enabledInfos.begin(), enabledInfos.end(),
        [](const ImeEnabledInfo &enabledInfoTmp) { return enabledInfoTmp.extraInfo.isDefaultIme; });
    auto newIter = std::find_if(enabledInfos.begin(), enabledInfos.end(), [](const ImeEnabledInfo &enabledInfoTmp) {
        return enabledInfoTmp.bundleName == ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName;
    });
    if (newIter == enabledInfos.end()) {
        newIter = std::find_if(enabledInfos.begin(), enabledInfos.end(), [](const ImeEnabledInfo &enabledInfoTmp) {
            return enabledInfoTmp.enabledStatus != EnabledStatus::DISABLED;
        });
    }
    if (newIter == enabledInfos.end()) {
        return;
    }
    newIter->extraInfo.isDefaultIme = true;
    if (oldIter != enabledInfos.end()) {
        oldIter->extraInfo.isDefaultIme = false;
        oldIter->extraInfo.isDefaultImeSet = false;
        oldIter->extraInfo.isTmpIme = false;
        oldIter->extraInfo.currentSubName = "";
    }
}

bool ImeEnabledInfoManager::IsCurrentIme(const std::string &bundleName, const std::vector<ImeEnabledInfo> &enabledInfos)
{
    auto iter = std::find_if(enabledInfos.begin(), enabledInfos.end(),
        [&bundleName](const ImeEnabledInfo &info) { return info.extraInfo.isDefaultIme; });
    return iter != enabledInfos.end() && iter->bundleName == bundleName;
}
} // namespace MiscServices
} // namespace OHOS