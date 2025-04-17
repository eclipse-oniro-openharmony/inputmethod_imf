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
ImeEnabledInfoManager &ImeEnabledInfoManager::GetInstance()
{
    static ImeEnabledInfoManager instance;
    return instance;
}

ImeEnabledInfoManager::~ImeEnabledInfoManager()
{
}

void ImeEnabledInfoManager::SetEnableChangedHandler(EnableChangedHandler handler)
{
    if (enableChangedHandler_ != nullptr) {
        return;
    }
    enableChangedHandler_ = std::move(handler);
}

void ImeEnabledInfoManager::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler)
{
    serviceHandler_ = eventHandler;
}

int32_t ImeEnabledInfoManager::RegularInit(const std::map<int32_t, std::vector<FullImeInfo>> &fullImeInfos)
{
    if (!HasEnabledSwitch()) {
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("run in, user num:%{public}zu.", fullImeInfos.size());
    for (const auto &fullImeInfo : fullImeInfos) {
        UpdateEnabledCfgCache(fullImeInfo.first, fullImeInfo.second);
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::Init(const std::map<int32_t, std::vector<FullImeInfo>> &fullImeInfos)
{
    if (!HasEnabledSwitch()) {
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("run in, user num:%{public}zu.", fullImeInfos.size());
    for (const auto &fullImeInfo : fullImeInfos) {
        UpdateEnabledCfgCacheIfNoCache(fullImeInfo.first, fullImeInfo.second);
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::Switch(int32_t userId, const std::vector<FullImeInfo> &imeInfos)
{
    IMSA_HILOGI("userId:%{public}d", userId);
    currentUserId_ = userId;
    ImeEnabledCfg cfg;
    auto ret = GetEnabledCache(userId, cfg);
    if (ret == ErrorCode::NO_ERROR) {
        UpdateGlobalEnabledTable(userId, cfg); // user switch, replace global table by user table
        return ret;
    }
    return UpdateEnabledCfgCache(userId, imeInfos);
}

int32_t ImeEnabledInfoManager::UpdateEnabledCfgCacheIfNoCache(int32_t userId, const std::vector<FullImeInfo> &imeInfos)
{
    ImeEnabledCfg cfg;
    auto ret = GetEnabledCache(userId, cfg);
    if (ret == ErrorCode::NO_ERROR) {
        return ret;
    }
    return UpdateEnabledCfgCache(userId, imeInfos);
}

int32_t ImeEnabledInfoManager::UpdateEnabledCfgCache(int32_t userId, const std::vector<FullImeInfo> &imeInfos)
{
    if (!HasEnabledSwitch()) {
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("run in, userId:%{public}d", userId);
    ImeEnabledCfg cfg;
    auto ret = GetEnabledCfg(userId, cfg, true, imeInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId:%{public}d get enabled cfg failed:%{public}d.", userId, ret);
        return ret;
    }
    return UpdateEnabledCfgCache(userId, cfg);
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
    if (imeInfo.prop.name == ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName) {
        IMSA_HILOGI("[%{public}d,%{public}s] is sys ime, deal in init or user add.", userId, imeInfo.prop.name.c_str());
        return ErrorCode::NO_ERROR;
    }
    ImeEnabledCfg enabledCfg;
    auto ret = GetEnabledCacheWithCorrect(userId, enabledCfg);
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
    ComputeEnabledStatus(infoTmp);
    enabledCfg.enabledInfos.emplace_back(infoTmp);
    return UpdateEnabledCfgCache(userId, enabledCfg);
}

int32_t ImeEnabledInfoManager::Delete(int32_t userId, const std::string &bundleName)
{
    if (!HasEnabledSwitch()) {
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("enable Delete run in, userId:%{public}d, bundleName:%{public}s", userId, bundleName.c_str());
    ImeEnabledCfg enabledCfg;
    auto ret = GetEnabledCacheWithCorrect(userId, enabledCfg);
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
    return UpdateEnabledCfgCache(userId, enabledCfg);
}

std::pair<int32_t, bool> ImeEnabledInfoManager::CheckUpdate(
    int32_t userId, const std::string &bundleName, const std::string &extensionName, EnabledStatus status)
{
    std::pair<int32_t, bool> checkRet{ ErrorCode::NO_ERROR, false };
    IMSA_HILOGI("update info:[%{public}d, %{public}s, %{public}s, %{public}d].", userId, bundleName.c_str(),
        extensionName.c_str(), static_cast<int32_t>(status));
    if (!HasEnabledSwitch()) {
        IMSA_HILOGE(
            "enabled feature not on, [%{public}d, %{public}s] operation not allow.", userId, bundleName.c_str());
        checkRet.first = ErrorCode::ERROR_ENABLE_IME;
        return checkRet;
    }
    if (bundleName.empty()) {
        IMSA_HILOGE("%{public}d bundleName:%{public}s abnormal.", userId, bundleName.c_str());
        checkRet.first = ErrorCode::ERROR_PARAMETER_CHECK_FAILED;
        return checkRet;
    }
    if (ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName == bundleName && status == EnabledStatus::DISABLED) {
        IMSA_HILOGW("[%{public}d,%{public}s] is sys ime, do not set DISABLED.", userId, bundleName.c_str());
        checkRet.first = ErrorCode::ERROR_DISABLE_SYSTEM_IME;
        return checkRet;
    }
    ImeEnabledCfg enabledCfg;
    auto ret = GetEnabledCache(userId, enabledCfg);
    if (ret == ErrorCode::NO_ERROR) {
        auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
            [&bundleName](const ImeEnabledInfo &info) { return bundleName == info.bundleName; });
        if (iter != enabledCfg.enabledInfos.end()) {
            return checkRet;
        }
    }
    if (!ImeInfoInquirer::GetInstance().IsInputMethod(userId, bundleName)) {
        checkRet.first = ErrorCode::ERROR_IME_NOT_FOUND;
        return checkRet;
    }
    checkRet.second = true;
    return checkRet;
}

int32_t ImeEnabledInfoManager::Update(
    int32_t userId, const std::string &bundleName, const std::string &extensionName, EnabledStatus status)
{
    auto [ret, needUpdateCache] = CheckUpdate(userId, bundleName, extensionName, status);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    if (needUpdateCache) {
        std::vector<FullImeInfo> imeInfos;
        auto ret = UpdateEnabledCfgCache(userId, imeInfos);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("user:%{public}d UpdateEnabledCfgCache failed:%{public}d.", userId, ret);
            return ErrorCode::ERROR_ENABLE_IME;
        }
    }
    ImeEnabledCfg enabledCfg;
    GetEnabledCache(userId, enabledCfg);
    auto iter = std::find_if(enabledCfg.enabledInfos.begin(), enabledCfg.enabledInfos.end(),
        [&bundleName](const ImeEnabledInfo &info) { return bundleName == info.bundleName; });
    if (iter == enabledCfg.enabledInfos.end()) {
        IMSA_HILOGE("not find [%{public}d, %{public}s, %{public}s] in ime install list.", userId, bundleName.c_str(),
            extensionName.c_str());
        return ErrorCode::ERROR_IME_NOT_FOUND;
    }
    auto oldStatus = iter->enabledStatus;
    iter->enabledStatus = status;
    iter->stateUpdateTime =
        std::to_string(duration_cast<std::chrono::milliseconds>(system_clock::now().time_since_epoch()).count());
    ComputeEnabledStatus(*iter);
    ret = UpdateEnabledCfgCache(userId, enabledCfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d UpdateEnabledCfgCache failed.", userId);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    NotifyEnableChanged(userId, bundleName, oldStatus);
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
    }
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d %{public}s get status failed %{public}d.", userId, bundleName.c_str(), ret);
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
    auto ret = GetEnabledStatesInner(userId, props);
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

int32_t ImeEnabledInfoManager::GetEnabledStateInner(
    int32_t userId, const std::string &bundleName, EnabledStatus &status)
{
    // get from cache
    ImeEnabledCfg cacheCfg;
    GetEnabledCache(userId, cacheCfg);
    auto iter = std::find_if(cacheCfg.enabledInfos.begin(), cacheCfg.enabledInfos.end(),
        [&bundleName](const ImeEnabledInfo &info) { return bundleName == info.bundleName; });
    if (iter != cacheCfg.enabledInfos.end()) {
        status = iter->enabledStatus;
        IMSA_HILOGD("[%{public}d, %{public}s] enabledStatus by cache: %{public}d.", userId, bundleName.c_str(),
            static_cast<int32_t>(status));
        return ErrorCode::NO_ERROR;
    }
    // get from table
    ImeEnabledCfg cfg;
    auto ret = GetEnabledCfg(userId, cfg);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("[%{public}d, %{public}s] get enabled cfg failed.", userId, bundleName.c_str());
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
    PostUpdateCfgCacheTask(userId);
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledStatesInner(int32_t userId, std::vector<Property> &props)
{
    // get from cache
    ImeEnabledCfg cfg;
    auto ret = GetEnabledCache(userId, cfg);
    if (ret != ErrorCode::NO_ERROR) {
        // get from table
        ret = GetEnabledCfg(userId, cfg);
        IMSA_HILOGI("%{public}d get from table ret:%{public}d.", userId, ret);
        PostUpdateCfgCacheTask(userId);
    }
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    for (auto &prop : props) {
        auto iter = std::find_if(cfg.enabledInfos.begin(), cfg.enabledInfos.end(),
            [&prop](const ImeEnabledInfo &info) { return prop.name == info.bundleName; });
        if (iter == cfg.enabledInfos.end()) {
            continue;
        }
        prop.status = iter->enabledStatus;
        IMSA_HILOGD("get [%{public}d, %{public}s,%{public}d] succeed.", userId, prop.name.c_str(),
            static_cast<int32_t>(prop.status));
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledCacheWithCorrect(int32_t userId, ImeEnabledCfg &enabledCfg)
{
    auto ret = GetEnabledCache(userId, enabledCfg);
    if (ret == ErrorCode::NO_ERROR) {
        return ret;
    }
    std::vector<FullImeInfo> imeInfos;
    ret = UpdateEnabledCfgCache(userId, imeInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("correct %{public}d failed.", userId);
        return ret;
    }
    return GetEnabledCache(userId, enabledCfg);
}

int32_t ImeEnabledInfoManager::GetEnabledCache(int32_t userId, ImeEnabledCfg &enabledCfg)
{
    std::lock_guard<std::mutex> lock(imeEnabledCfgLock_);
    auto it = imeEnabledCfg_.find(userId);
    if (it == imeEnabledCfg_.end()) {
        IMSA_HILOGE("not find %{public}d in cache.", userId);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    enabledCfg = it->second;
    IMSA_HILOGD("num %{public}zu in cache.", enabledCfg.enabledInfos.size());
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledCfg(
    int32_t userId, ImeEnabledCfg &cfg, bool needCorrectByBundleMgr, const std::vector<FullImeInfo> &imeInfos)
{
    if (!SettingsDataUtils::GetInstance()->IsDataShareReady()) {  // todo 會不會導致查不到，原來能查到，現在產不到
        IMSA_HILOGE("data share not ready.");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    auto ret = GetEnabledTableCfg(userId, cfg);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    if (needCorrectByBundleMgr) {
        ret = CorrectByBundleMgr(userId, cfg.enabledInfos, imeInfos);
        if (ret != ErrorCode::NO_ERROR) {
            return ret;
        }
    }
    ComputeEnabledStatus(cfg.enabledInfos);
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledTableCfg(int32_t userId, ImeEnabledCfg &cfg)
{
    auto ret = EnableUpgradeManager::GetInstance().Upgrade(userId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}d upgrade failed:%{public}d.", userId, ret);
        return ret;
    }
    IMSA_HILOGI("%{public}d run in.", userId);
    std::string content;
    ret = SettingsDataUtils::GetInstance()->GetStringValue(
        SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true", SettingsDataUtils::ENABLE_IME, content);
    if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        IMSA_HILOGE("%{public}d get enabled table failed:%{public}d.", userId, ret);
        return ret;
    }
    if (ret == ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        return ErrorCode::NO_ERROR;
    }
    if (!cfg.Unmarshall(content)) {
        IMSA_HILOGE("%{public}d Unmarshall failed:%{public}s.", userId, content.c_str());
        return ErrorCode::NO_ERROR;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::CorrectByBundleMgr(
    int32_t userId, std::vector<ImeEnabledInfo> &enabledInfos, const std::vector<FullImeInfo> &imeInfos)
{
    auto imeTmpInfos = imeInfos;
    if (imeTmpInfos.empty()) {
        auto ret = ImeInfoInquirer::GetInstance().QueryFullImeInfo(userId, imeTmpInfos, false);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("%{public}d QueryFullImeInfo failed.", userId);
            return ret;
        }
    }
    IMSA_HILOGI("ime size, enabled:%{public}zu, installed:%{public}zu.", enabledInfos.size(), imeTmpInfos.size());
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
                IMSA_HILOGW("%{public}s update.", info.prop.name.c_str());
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
        enabledInfos.emplace_back(infoTmp);
    }
    return ErrorCode::NO_ERROR;
}

void ImeEnabledInfoManager::ComputeEnabledStatus(ImeEnabledInfo &info)
{
    auto hasEnableSwitch = ImeInfoInquirer::GetInstance().GetSystemConfig().enableInputMethodFeature;
    auto hasFullExperienceSwitch = ImeInfoInquirer::GetInstance().GetSystemConfig().enableFullExperienceFeature;
    IMSA_HILOGI("enable cfg:[%{public}d, %{public}d].", hasEnableSwitch, hasFullExperienceSwitch);
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
    auto sysIme = ImeInfoInquirer::GetInstance().GetDefaultIme();
    if (info.bundleName != sysIme.bundleName || info.enabledStatus != EnabledStatus::DISABLED) {
        return;
    }
    info.enabledStatus = EnabledStatus::BASIC_MODE;
}

void ImeEnabledInfoManager::ComputeEnabledStatus(std::vector<ImeEnabledInfo> &infos)
{
    for (auto &info : infos) {
        ComputeEnabledStatus(info);
    }
}

int32_t ImeEnabledInfoManager::UpdateEnabledCfgCache(int32_t userId, const ImeEnabledCfg &cfg)
{
    std::string content;
    if (!cfg.Marshall(content)) {
        IMSA_HILOGE("Marshall failed");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    IMSA_HILOGI("[%{public}d, %{public}s].", userId, content.c_str());
    if (!SettingsDataUtils::GetInstance()->SetStringValue(
            SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true", SettingsDataUtils::ENABLE_IME, content)) {
        return ErrorCode::ERROR_ENABLE_IME;
    }
    {
        std::lock_guard<std::mutex> cgfLock(imeEnabledCfgLock_);
        imeEnabledCfg_.insert_or_assign(userId, cfg);
    }
    UpdateGlobalEnabledTable(userId, cfg);
    return ErrorCode::NO_ERROR;
}

void ImeEnabledInfoManager::PostUpdateCfgCacheTask(int32_t userId)
{
    if (serviceHandler_ == nullptr) {
        return;
    }
    auto task = [this, userId]() { UpdateEnabledCfgCacheIfNoCache(userId); };
    // 60000: ms, the task is time-consuming, prevent it is performed during the device boot
    serviceHandler_->PostTask(task, "UpdateEnabledCfgCacheIfNoCache", 60000, AppExecFwk::EventQueue::Priority::LOW);
}

void ImeEnabledInfoManager::NotifyEnableChanged(int32_t userId, const std::string &bundleName, EnabledStatus oldStatus)
{
    IMSA_HILOGI(
        "notify:[%{public}d,%{public}s,%{public}d].", userId, bundleName.c_str(), static_cast<int32_t>(oldStatus));
    if (serviceHandler_ == nullptr) {
        return;
    }
    auto notifyTask = [this, userId, bundleName, oldStatus]() {
        if (enableChangedHandler_ != nullptr) {
            enableChangedHandler_(userId, bundleName, oldStatus);
        }
    };
    serviceHandler_->PostTask(notifyTask, "NotifyEnableChangedTask", 0, AppExecFwk::EventQueue::Priority::VIP);
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

void ImeEnabledInfoManager::UpdateGlobalEnabledTable(int32_t userId, const ImeEnabledCfg &newEnabledCfg)
{
    IMSA_HILOGD("start.");
    if (userId != currentUserId_) {
        IMSA_HILOGW("[%{public}d,%{public}d] not same.", userId, currentUserId_);
        return;
    }
    std::vector<std::string> bundleNames;
    for (const auto &info : newEnabledCfg.enabledInfos) {
        if (info.enabledStatus != EnabledStatus::DISABLED) {
            bundleNames.emplace_back(info.bundleName);
        }
    }
    EnableUpgradeManager::GetInstance().UpdateGlobalEnabledTable(userId, bundleNames);
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
    EnabledStatus oldStatus;
    ImeEnabledInfoManager::GetInstance().GetEnabledState(userId, defaultIme.bundleName, oldStatus);
    if (newStatus == oldStatus) {
        return;
    }
    IMSA_HILOGI("%{public}d sys ime full experience changed.", userId);
    ImeEnabledInfoManager::GetInstance().Update(userId, defaultIme.bundleName, defaultIme.extName, newStatus);
}
} // namespace MiscServices
} // namespace OHOS