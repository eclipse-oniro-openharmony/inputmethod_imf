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

#include "global.h"
namespace OHOS {
namespace MiscServices {
constexpr const char *MOCK_APP_ID = "MockAppId";
ImeInfoInquirer &ImeInfoInquirer::GetInstance()
{
    static ImeInfoInquirer instance;
    return instance;
}

void ImeInfoInquirer::SetDefaultInputMethod(const std::shared_ptr<ImeInfo> &imeInfo)
{
    std::lock_guard<std::mutex> lock(defaultImeInfoLock_);
    defaultImeInfo_ = imeInfo;
}

std::shared_ptr<ImeNativeCfg> ImeInfoInquirer::GetDefaultImeCfg()
{
    std::lock_guard<std::mutex> lock(defaultImeInfoLock_);
    if (defaultImeInfo_ == nullptr || defaultImeInfo_->prop.name.empty()) {
        return nullptr;
    }
    auto imeCfg = std::make_shared<ImeNativeCfg>();
    imeCfg->bundleName = defaultImeInfo_->prop.name;
    imeCfg->extName = defaultImeInfo_->prop.id;
    imeCfg->imeId = imeCfg->bundleName + "/" + imeCfg->extName;
    return imeCfg;
}

int32_t ImeInfoInquirer::GetDefaultInputMethod(const int32_t userId, std::shared_ptr<Property> &prop, bool isBrief)
{
    std::lock_guard<std::mutex> lock(defaultImeInfoLock_);
    if (defaultImeInfo_ == nullptr || defaultImeInfo_->prop.name.empty()) {
        return ErrorCode::ERROR_IME_NOT_FOUND;
    }
    prop = std::make_shared<Property>(defaultImeInfo_->prop);
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetDefaultImeInfo(int32_t userId)
{
    std::lock_guard<std::mutex> lock(defaultImeInfoLock_);
    return defaultImeInfo_;
}

std::shared_ptr<Property> ImeInfoInquirer::GetDefaultImeCfgProp()
{
    std::lock_guard<std::mutex> lock(defaultImeInfoLock_);
    if (defaultImeInfo_ == nullptr || defaultImeInfo_->prop.name.empty()) {
        return nullptr;
    }
    return std::make_shared<Property>(defaultImeInfo_->prop);
}

bool ImeInfoInquirer::GetImeAppId(int32_t userId, const std::string &bundleName, std::string &appId)
{
    appId = MOCK_APP_ID;
    return true;
}

bool ImeInfoInquirer::GetImeVersionCode(int32_t userId, const std::string &bundleName, uint32_t &versionCode)
{
    versionCode = 0;
    return true;
}

void ImeInfoInquirer::SetDumpInfo(int32_t userId, const std::string &dumpInfo)
{
    std::lock_guard<std::mutex> lock(dumpInfosLock_);
    dumpInfos_.insert_or_assign(userId, dumpInfo);
}

std::string ImeInfoInquirer::GetDumpInfo(int32_t userId)
{
    std::lock_guard<std::mutex> lock(dumpInfosLock_);
    auto iter = dumpInfos_.find(userId);
    if (iter == dumpInfos_.end()) {
        return "";
    }
    return iter->second;
}

void ImeInfoInquirer::SetImeToStart(int32_t userId, const std::shared_ptr<ImeNativeCfg> &imeToStart)
{
    std::lock_guard<std::mutex> lock(imeToStartLock_);
    imeToStart_.insert_or_assign(userId, imeToStart);
}

std::shared_ptr<ImeNativeCfg> ImeInfoInquirer::GetImeToStart(int32_t userId)
{
    std::lock_guard<std::mutex> lock(imeToStartLock_);
    auto iter = imeToStart_.find(userId);
    if (iter == imeToStart_.end()) {
        return nullptr;
    }
    return iter->second;
}

void ImeInfoInquirer::SetImeProperty(int32_t userId, const std::shared_ptr<Property> &prop)
{
    std::lock_guard<std::mutex> lock(imePropsLock_);
    imeProps_.insert_or_assign(userId, prop);
}

std::shared_ptr<Property> ImeInfoInquirer::GetImeProperty(
    int32_t userId, const std::string &bundleName, const std::string &extName)
{
    std::lock_guard<std::mutex> lock(imePropsLock_);
    auto iter = imeProps_.find(userId);
    if (iter == imeProps_.end() || iter->second == nullptr) {
        return nullptr;
    }
    if (iter->second->name == bundleName && iter->second->id == extName) {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<Property> ImeInfoInquirer::GetCurrentInputMethod(int32_t userId)
{
    std::lock_guard<std::mutex> lock(imePropsLock_);
    auto iter = imeProps_.find(userId);
    if (iter == imeProps_.end()) {
        return nullptr;
    }
    return iter->second;
}

void ImeInfoInquirer::SetCurrentSubtype(int32_t userId, const std::shared_ptr<SubProperty> &subProp)
{
    std::lock_guard<std::mutex> lock(imeSubPropsLock_);
    imeSubProps_.insert_or_assign(userId, subProp);
}

std::shared_ptr<SubProperty> ImeInfoInquirer::GetCurrentSubtype(int32_t userId)
{
    std::lock_guard<std::mutex> lock(imeSubPropsLock_);
    auto iter = imeSubProps_.find(userId);
    if (iter == imeSubProps_.end()) {
        return nullptr;
    }
    return iter->second;
}

void ImeInfoInquirer::SetImeInfo(int32_t userId, const std::vector<std::shared_ptr<ImeInfo>> &imeInfos)
{
    std::lock_guard<std::mutex> lock(imeInfosLock_);
    imeInfos_.insert_or_assign(userId, imeInfos);
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetImeInfo(
    int32_t userId, const std::string &bundleName, const std::string &subName)
{
    std::lock_guard<std::mutex> lock(imeInfosLock_);
    auto iter = imeInfos_.find(userId);
    if (iter == imeInfos_.end()) {
        return nullptr;
    }
    auto it =
        std::find_if(iter->second.begin(), iter->second.end(), [&bundleName](const std::shared_ptr<ImeInfo> &imeInfo) {
            return imeInfo != nullptr && bundleName == imeInfo->prop.name;
        });
    if (it == iter->second.end()) {
        return nullptr;
    }
    auto imeInfo = *it;
    auto it1 = std::find_if(imeInfo->subProps.begin(), imeInfo->subProps.end(),
        [&subName](const SubProperty &subPropTmp) { return subName == subPropTmp.id; });
    if (it1 == imeInfo->subProps.end()) {
        return nullptr;
    }
    return imeInfo;
}

std::shared_ptr<SubProperty> ImeInfoInquirer::FindTargetSubtypeByCondition(
    const std::vector<SubProperty> &subProps, const Condition &condition)
{
    return nullptr;
}

int32_t ImeInfoInquirer::ListInputMethod(int32_t userId, InputMethodStatus status, std::vector<Property> &props)
{
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListInputMethodSubtype(
    int32_t userId, const std::string &bundleName, std::vector<SubProperty> &subProps)
{
    std::lock_guard<std::mutex> lock(imeInfosLock_);
    auto iter = imeInfos_.find(userId);
    if (iter == imeInfos_.end()) {
        return ErrorCode::ERROR_IME_NOT_FOUND;
    }
    auto it =
        std::find_if(iter->second.begin(), iter->second.end(), [&bundleName](const std::shared_ptr<ImeInfo> &imeInfo) {
            return imeInfo != nullptr && bundleName == imeInfo->prop.name;
        });
    if (it == iter->second.end()) {
        return ErrorCode::ERROR_IME_NOT_FOUND;
    }
    subProps = (*it)->subProps;
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListCurrentInputMethodSubtype(int32_t userId, std::vector<SubProperty> &subProps)
{
    std::string bundleName;
    {
        std::lock_guard<std::mutex> lock(imePropsLock_);
        auto iter = imeProps_.find(userId);
        if (iter == imeProps_.end() || iter->second == nullptr) {
            return ErrorCode::ERROR_IME_NOT_FOUND;
        }
        bundleName = iter->second->name;
    }
    std::lock_guard<std::mutex> lock(imeInfosLock_);
    auto iter = imeInfos_.find(userId);
    if (iter == imeInfos_.end()) {
        return ErrorCode::ERROR_IME_NOT_FOUND;
    }
    auto it =
        std::find_if(iter->second.begin(), iter->second.end(), [&bundleName](const std::shared_ptr<ImeInfo> &imeInfo) {
            return imeInfo != nullptr && bundleName == imeInfo->prop.name;
        });
    if (it == iter->second.end()) {
        return ErrorCode::ERROR_IME_NOT_FOUND;
    }
    subProps = (*it)->subProps;
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::GetSwitchInfoBySwitchCount(SwitchInfo &switchInfo, int32_t userId, uint32_t cacheCount)
{
    return ErrorCode::NO_ERROR;
}

bool ImeInfoInquirer::IsDefaultImeSet(int32_t userId)
{
    return true;
}

bool ImeInfoInquirer::IsImeInstalled(const int32_t userId, const std::string &bundleName, const std::string &extName)
{
    std::lock_guard<std::mutex> lock(imePropsLock_);
    auto iter = imeProps_.find(userId);
    if (iter == imeProps_.end() || iter->second == nullptr) {
        return false;
    }
    return iter->second->name == bundleName && iter->second->id == extName;
}

bool ImeInfoInquirer::IsEnableInputMethod()
{
    std::lock_guard<std::mutex> lock(sysCfgLock_);
    return sysCfg_.enableInputMethodFeature;
}

bool ImeInfoInquirer::IsEnableSecurityMode()
{
    std::lock_guard<std::mutex> lock(sysCfgLock_);
    return sysCfg_.enableFullExperienceFeature;
}

bool ImeInfoInquirer::IsEnableAppAgent()
{
    std::lock_guard<std::mutex> lock(sysCfgLock_);
    return sysCfg_.enableAppAgentFeature;
}

bool ImeInfoInquirer::IsEnableNumKey()
{
    std::lock_guard<std::mutex> lock(sysCfgLock_);
    return sysCfg_.enableNumKeyFeature;
}

bool ImeInfoInquirer::IsProxyIme(int32_t callingUid)
{
    std::lock_guard<std::mutex> lock(sysCfgLock_);
    return sysCfg_.proxyImeUidList.count(callingUid);
}

bool ImeInfoInquirer::IsSpecialSaUid(int32_t callingUid)
{
    std::lock_guard<std::mutex> lock(sysCfgLock_);
    return sysCfg_.specialSaUidList.count(callingUid);
}

void ImeInfoInquirer::InitSystemConfig()
{
}

void ImeInfoInquirer::SetSystemConfig(const SystemConfig &sysCfg)
{
    std::lock_guard<std::mutex> lock(sysCfgLock_);
    sysCfg_ = sysCfg;
}

SystemConfig ImeInfoInquirer::GetSystemConfig()
{
    std::lock_guard<std::mutex> lock(sysCfgLock_);
    return sysCfg_;
}

std::string ImeInfoInquirer::GetSystemSpecialIme()
{
    std::lock_guard<std::mutex> lock(sysCfgLock_);
    return sysCfg_.systemSpecialInputMethod;
}

void ImeInfoInquirer::SetRunningIme(int32_t userId, const std::vector<std::string> &ime)
{
    std::lock_guard<std::mutex> lock(runningImeLock_);
    runningIme_.insert_or_assign(userId, ime);
}

bool ImeInfoInquirer::IsRunningIme(int32_t userId, const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(runningImeLock_);
    auto iter = runningIme_.find(userId);
    if (iter == runningIme_.end()) {
        return false;
    }
    auto it = std::find_if(iter->second.begin(), iter->second.end(),
        [&bundleName](const std::string &bundleNameTmp) { return bundleName == bundleNameTmp; });
    return it != iter->second.end();
}

std::vector<std::string> ImeInfoInquirer::GetRunningIme(int32_t userId)
{
    std::lock_guard<std::mutex> lock(runningImeLock_);
    auto iter = runningIme_.find(userId);
    if (iter == runningIme_.end()) {
        return {};
    }
    return iter->second;
}

void ImeInfoInquirer::SetInputMethodExtension(pid_t pid)
{
    inputMethodExt_ = pid;
}

bool ImeInfoInquirer::IsInputMethodExtension(pid_t pid)
{
    return inputMethodExt_ == pid;
}

void ImeInfoInquirer::SetRestrictedDefaultImeDisplay(uint64_t displayId)
{
    restrictedDefaultImeDisplayId_.store(displayId);
}

bool ImeInfoInquirer::IsRestrictedDefaultImeByDisplay(uint64_t displayId)
{
    return restrictedDefaultImeDisplayId_.load() == displayId;
}

void ImeInfoInquirer::SetDynamicStartImeFlag(bool isDynamicStart)
{
    isDynamicStartIme_.store(isDynamicStart);
}

bool ImeInfoInquirer::IsDynamicStartIme()
{
    return isDynamicStartIme_.load();
}

void ImeInfoInquirer::SetDisableNumKeyAppDeviceTypes(const std::unordered_set<std::string> &types)
{
    std::lock_guard<std::mutex> lock(disableNumKeyAppDeviceTypesLock_);
    disableNumKeyAppDeviceTypes_ = types;
}

std::unordered_set<std::string> ImeInfoInquirer::GetDisableNumKeyAppDeviceTypes()
{
    std::lock_guard<std::mutex> lock(disableNumKeyAppDeviceTypesLock_);
    return disableNumKeyAppDeviceTypes_;
}

void ImeInfoInquirer::SetCapacitySupportFlag(const std::string &capacityName, bool isSupport)
{
    std::lock_guard<std::mutex> lock(capacitySupportInfosLock_);
    capacitySupportInfos_.clear();
    capacitySupportInfos_.insert_or_assign(capacityName, isSupport);
}

bool ImeInfoInquirer::IsCapacitySupport(const std::string &capacityName)
{
    std::lock_guard<std::mutex> lock(capacitySupportInfosLock_);
    auto iter = capacitySupportInfos_.find(capacityName);
    if (iter == capacitySupportInfos_.end()) {
        return false;
    }
    return iter->second;
}

void ImeInfoInquirer::SetCompatibleDeviceType(const std::string &bundleName, const std::string &compatibleDeviceType)
{
    std::lock_guard<std::mutex> lock(compatibleDeviceTypeLock_);
    compatibleDeviceType_.clear();
    compatibleDeviceType_.insert_or_assign(bundleName, compatibleDeviceType);
}

bool ImeInfoInquirer::GetCompatibleDeviceType(const std::string &bundleName, std::string &compatibleDeviceType)
{
    std::lock_guard<std::mutex> lock(compatibleDeviceTypeLock_);
    auto iter = compatibleDeviceType_.find(bundleName);
    if (iter == compatibleDeviceType_.end()) {
        return false;
    }
    compatibleDeviceType = iter->second;
    return true;
}

int32_t ImeInfoInquirer::QueryFullImeInfo(std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> &fullImeInfos) const
{
    if (!isQueryAllFullImeInfosOk_) {
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    fullImeInfos = allFullImeInfos_;
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::QueryFullImeInfo(int32_t userId, std::vector<FullImeInfo> &imeInfos, bool needBrief) const
{
    if (!isQueryFullImeInfosOk_) {
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    imeInfos = fullImeInfos_;
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::GetFullImeInfo(int32_t userId, const std::string &bundleName, FullImeInfo &imeInfo) const
{
    if (!isGetFullImeInfoOk_) {
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    imeInfo = fullImeInfo_;
    return ErrorCode::NO_ERROR;
}

void ImeInfoInquirer::SetFullImeInfo(bool isReturnOk, const FullImeInfo &imeInfo)
{
    isGetFullImeInfoOk_ = isReturnOk;
    fullImeInfo_ = imeInfo;
}

void ImeInfoInquirer::SetFullImeInfo(bool isReturnOk, const std::vector<FullImeInfo> &imeInfos)
{
    isQueryFullImeInfosOk_ = isReturnOk;
    fullImeInfos_ = imeInfos;
}

void ImeInfoInquirer::SetFullImeInfo(
    bool isReturnOk, const std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> &fullImeInfos)
{
    isQueryAllFullImeInfosOk_ = isReturnOk;
    allFullImeInfos_ = fullImeInfos;
}

ImeNativeCfg ImeInfoInquirer::GetDefaultIme()
{
    ImeNativeCfg imeCfg;
    return imeCfg;
}
} // namespace MiscServices
} // namespace OHOS