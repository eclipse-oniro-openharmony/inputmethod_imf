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

#include "enable_ime_data_parser.h"

#include <algorithm>

#include "ime_info_inquirer.h"

namespace OHOS {
namespace MiscServices {
std::mutex EnableImeDataParser::instanceMutex_;
sptr<EnableImeDataParser> EnableImeDataParser::instance_ = nullptr;
EnableImeDataParser::~EnableImeDataParser()
{
}

sptr<EnableImeDataParser> EnableImeDataParser::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceMutex_);
        if (instance_ == nullptr) {
            IMSA_HILOGI("need to create instance.");
            instance_ = new (std::nothrow) EnableImeDataParser();
            if (instance_ == nullptr) {
                IMSA_HILOGE("instance is nullptr!");
                return instance_;
            }
        }
    }
    return instance_;
}

int32_t EnableImeDataParser::Initialize(const int32_t userId)
{
    currentUserId_ = userId;
    {
        std::lock_guard<std::mutex> autoLock(listMutex_);
        enableList_.insert({ std::string(ENABLE_IME), {} });
        enableList_.insert({ std::string(ENABLE_KEYBOARD), {} });
    }
    UpdateEnableData(userId, ENABLE_IME);
    UpdateEnableData(userId, ENABLE_KEYBOARD);
    GetDefaultIme();
    return ErrorCode::NO_ERROR;
}

void EnableImeDataParser::OnUserChanged(const int32_t targetUserId)
{
    std::lock_guard<std::mutex> lock(userIdLock_);
    IMSA_HILOGI("run in %{public}d}.", targetUserId);
    currentUserId_ = targetUserId;
    UpdateEnableData(targetUserId, ENABLE_IME);
    UpdateEnableData(targetUserId, ENABLE_KEYBOARD);
}

bool EnableImeDataParser::CheckNeedSwitch(const std::string &key, SwitchInfo &switchInfo, const int32_t userId)
{
    IMSA_HILOGD("start, data changed.");
    auto currentIme = ImeInfoInquirer::GetInstance().GetCurrentInputMethod(userId);
    auto defaultIme = GetDefaultIme();
    if (defaultIme == nullptr) {
        IMSA_HILOGE("defaultIme is nullptr!");
        return false;
    }
    switchInfo.bundleName = defaultIme->name;
    switchInfo.subName = "";
    if (currentIme == nullptr) {
        IMSA_HILOGE("currentIme is nullptr!");
        return true;
    }
    if (key == std::string(ENABLE_IME)) {
        if (currentIme->name == defaultIme->name) {
            std::lock_guard<std::mutex> autoLock(listMutex_);
            GetEnableData(key, enableList_[key], userId);
            IMSA_HILOGD("current ime is default, do not need switch ime.");
            return false;
        }
        return CheckTargetEnableName(key, currentIme->name, switchInfo.bundleName, userId);
    } else if (key == std::string(ENABLE_KEYBOARD)) {
        if (currentIme->name != defaultIme->name || currentIme->id == defaultIme->id) {
            IMSA_HILOGD("current ime is not default or id is default.");
            std::lock_guard<std::mutex> autoLock(listMutex_);
            GetEnableData(key, enableList_[key], userId);
            return false;
        }
        switchInfo.subName = defaultIme->id;
        return CheckTargetEnableName(key, currentIme->id, switchInfo.subName, userId);
    }
    IMSA_HILOGW("invalid key: %{public}s.", key.c_str());
    return false;
}

bool EnableImeDataParser::CheckNeedSwitch(const SwitchInfo &info, const int32_t userId)
{
    IMSA_HILOGD("current userId: %{public}d, target userId: %{public}d, check bundleName: %{public}s", currentUserId_,
        userId, info.bundleName.c_str());
    if (info.bundleName == GetDefaultIme()->name) {
        IMSA_HILOGD("default ime, permit to switch");
        return true;
    }
    IMSA_HILOGD("check ime.");
    std::vector<std::string> tempVec;
    int32_t tempRet = GetEnableData(TEMP_IME, tempVec, userId);
    if (tempRet != ErrorCode::NO_ERROR || tempVec.empty()) {
        IMSA_HILOGD("get tempVec list failed, or tempVec list is empty.");
    } else {
        auto iter = std::find_if(
            tempVec.begin(), tempVec.end(), [&info](const std::string &ime) { return info.bundleName == ime; });
        if (iter != tempVec.end()) {
            IMSA_HILOGD("In tempVec list.");
            return true;
        }
    }
    std::vector<std::string> enableVec;
    int32_t ret = GetEnableData(ENABLE_IME, enableVec, userId);
    if (ret != ErrorCode::NO_ERROR || enableVec.empty()) {
        IMSA_HILOGD("get enable list failed, or enable list is empty.");
        return false;
    }

    auto iter = std::find_if(enableVec.begin(), enableVec.end(),
        [&info](const std::string &ime) { return info.bundleName == ime; });
    if (iter != enableVec.end()) {
        IMSA_HILOGD("in enable list.");
        return true;
    }
    return false;
}

bool EnableImeDataParser::CheckTargetEnableName(const std::string &key, const std::string &targetName,
    std::string &nextIme, const int32_t userId)
{
    IMSA_HILOGD("start.");
    std::vector<std::string> enableVec;
    int32_t ret = GetEnableData(key, enableVec, userId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("get enable list abnormal.");
        return false;
    }

    if (enableVec.empty()) {
        IMSA_HILOGE("enable empty, switch default ime.");
        return true;
    }
    std::lock_guard<std::mutex> autoLock(listMutex_);
    auto iter = std::find_if(enableVec.begin(), enableVec.end(),
        [&targetName](const std::string &ime) { return ime == targetName; });
    if (iter != enableVec.end()) {
        IMSA_HILOGD("enable list has current ime, do not need switch.");
        enableList_[key].assign(enableVec.begin(), enableVec.end());
        return false;
    }

    auto it = std::find_if(enableList_[key].begin(), enableList_[key].end(),
        [&targetName](const std::string &ime) { return ime == targetName; });
    if (it == enableList_[key].end()) {
        enableList_[key].assign(enableVec.begin(), enableVec.end());
        return true;
    }

    std::rotate(enableList_[key].begin(), it, enableList_[key].end());
    auto result =
        std::find_first_of(enableList_[key].begin(), enableList_[key].end(), enableVec.begin(), enableVec.end());
    if (result != enableList_[key].end()) {
        IMSA_HILOGD("found the next cached ime in enable ime list.");
        nextIme = *result;
    }
    enableList_[key].assign(enableVec.begin(), enableVec.end());
    return true;
}

void EnableImeDataParser::CoverGlobalEnableTable(const std::string &valueStr)
{
    SettingsDataUtils::GetInstance()->SetStringValue(SETTING_URI_PROXY, ENABLE_IME, valueStr);
}

std::string EnableImeDataParser::GetUserEnableTable(int32_t userId)
{
    std::string valueStr;
    int32_t ret = SettingsDataUtils::GetInstance()->GetStringValue(
        SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true", ENABLE_IME, valueStr);
    IMSA_HILOGI("get user enable table, userId = %{public}d, ret = %{public}d, valurStr = %{public}s",
        userId, ret, valueStr.c_str());
    if (valueStr.empty()) {
        auto defaultIme = GetDefaultIme();
        if (defaultIme != nullptr) {
            valueStr =
              "{\"enableImeList\" : {\"" + std::to_string(userId) + "\" : [\"" + defaultIme->name + "\"]}}";
        }
    }
    return valueStr;
}

std::string EnableImeDataParser::GetEanbleIme(int32_t userId, const std::string &globalStr)
{
    std::string enableStr = globalStr;
    std::string globaleUserId;
    if (enableStr.empty()) {
        enableStr = GetUserEnableTable(currentUserId_);
        CoverGlobalEnableTable(enableStr);
        globaleUserId = std::to_string(currentUserId_);
    } else {
        globaleUserId = GetGlobalTableUserId(enableStr);
    }
    SettingsDataUtils::GetInstance()->SetStringValue(SETTINGS_USER_DATA_URI + globaleUserId + "?Proxy=true",
        ENABLE_IME, enableStr);
    if (globaleUserId != std::to_string(currentUserId_)) {
        enableStr = GetUserEnableTable(currentUserId_);
        CoverGlobalEnableTable(enableStr);
    }
    if (currentUserId_ != userId) {
        enableStr = GetUserEnableTable(userId);
    }
    return enableStr;
}

int32_t EnableImeDataParser::GetEnableData(
    const std::string &key, std::vector<std::string> &enableVec, const int32_t userId)
{
    if (key != std::string(ENABLE_IME) && key != std::string(ENABLE_KEYBOARD) && key != std::string(TEMP_IME)) {
        IMSA_HILOGD("invalid key: %{public}s.", key.c_str());
        return ErrorCode::ERROR_ENABLE_IME;
    }
    IMSA_HILOGD("userId: %{public}d, key: %{public}s.", userId, key.c_str());
    std::string valueStr;
    int32_t ret = SettingsDataUtils::GetInstance()->GetStringValue(SETTING_URI_PROXY, key, valueStr);
    if (ret == ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        IMSA_HILOGW("no keyword exist");
        enableVec.clear();
        return ErrorCode::NO_ERROR;
    }
    if (key != ENABLE_IME && (ret != ErrorCode::NO_ERROR || valueStr.empty())) {
        IMSA_HILOGW("get value failed, or valueStr is empty.");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    auto parseRet = false;
    if (key == ENABLE_IME) {
        valueStr = GetEanbleIme(userId, valueStr);
        if (valueStr.empty()) {
            IMSA_HILOGW("valueStr is empty, userId = %{public}d", userId);
            return ErrorCode::NO_ERROR;
        }
        parseRet = ParseEnableIme(valueStr, userId, enableVec);
    }
    if (key == ENABLE_KEYBOARD) {
        parseRet = ParseEnableKeyboard(valueStr, userId, enableVec);
    }
    if (key == TEMP_IME) {
        parseRet = ParseTempIme(valueStr, userId, enableVec);
    }
    return parseRet ? ErrorCode::NO_ERROR : ErrorCode::ERROR_ENABLE_IME;
}

std::string EnableImeDataParser::GetGlobalTableUserId(const std::string &valueStr)
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

bool EnableImeDataParser::ParseTempIme(const std::string &valueStr, int32_t userId,
    std::vector<std::string> &tempVector)
{
    TempImeCfg tempIme;
    tempIme.tempImeList.userId = std::to_string(userId);
    auto ret = tempIme.Unmarshall(valueStr);
    if (!ret) {
        return ret;
    }
    tempVector = tempIme.tempImeList.identities;
    return true;
}

bool EnableImeDataParser::ParseEnableIme(const std::string &valueStr, int32_t userId,
    std::vector<std::string> &enableVec)
{
    EnableImeCfg enableIme;
    enableIme.userImeCfg.userId = std::to_string(userId);
    auto ret = enableIme.Unmarshall(valueStr);
    if (!ret) {
        return ret;
    }
    enableVec = enableIme.userImeCfg.identities;
    return true;
}

bool EnableImeDataParser::ParseEnableKeyboard(const std::string &valueStr, int32_t userId,
    std::vector<std::string> &enableVec)
{
    EnableKeyBoardCfg enableKeyboard;
    enableKeyboard.userImeCfg.userId = std::to_string(userId);
    auto ret = enableKeyboard.Unmarshall(valueStr);
    if (!ret) {
        return ret;
    }
    enableVec = enableKeyboard.userImeCfg.identities;
    return true;
}

std::shared_ptr<Property> EnableImeDataParser::GetDefaultIme()
{
    std::lock_guard<std::mutex> lock(defaultImeMutex_);
    if (defaultImeInfo_ == nullptr) {
        defaultImeInfo_ = std::make_shared<Property>();
    }
    if (!defaultImeInfo_->name.empty() && !defaultImeInfo_->id.empty()) {
        IMSA_HILOGD("defaultImeInfo_ has cached default time: %{public}s.", defaultImeInfo_->name.c_str());
        return defaultImeInfo_;
    }

    auto defaultIme = ImeInfoInquirer::GetInstance().GetDefaultImeCfgProp();
    if (defaultIme == nullptr) {
        IMSA_HILOGE("defaultIme is nullptr!");
        return defaultImeInfo_;
    }
    defaultImeInfo_->name = defaultIme->name;
    defaultImeInfo_->id = defaultIme->id;
    return defaultImeInfo_;
}

void EnableImeDataParser::OnConfigChanged(int32_t userId, const std::string &key)
{
    UpdateEnableData(userId, key);
}

void EnableImeDataParser::OnPackageAdded(int32_t userId, const std::string &bundleName)
{
    IMSA_HILOGI("run in:%{public}d,%{public}s.", userId, bundleName.c_str());
    auto initEnabledState = ImeInfoInquirer::GetInstance().GetSystemInitEnabledState();
    if (initEnabledState != EnabledStatus::BASIC_MODE) {
        IMSA_HILOGI("init enabled state is %{public}d.", static_cast<int32_t>(initEnabledState));
        return;
    }
    if (bundleName == ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName) {
        IMSA_HILOGI("default ime not deal:%{public}d,%{public}s.", userId, bundleName.c_str());
        return;
    }
    auto settingInstance = SettingsDataUtils::GetInstance();
    if (settingInstance == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(userIdLock_);
    std::string globalStr;
    int32_t ret = settingInstance->GetStringValue(SETTING_URI_PROXY, ENABLE_IME, globalStr);
    if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        IMSA_HILOGE("get global table failed:%{public}d.", ret);
        return;
    }
    if (userId == currentUserId_) {
        OnForegroundPackageAdded(userId, bundleName, globalStr);
        return;
    }
    OnBackgroundPackageAdded(userId, bundleName, globalStr);
}

void EnableImeDataParser::OnBackgroundPackageAdded(
    int32_t userId, const std::string &bundleName, const std::string &globalContent)
{
    IMSA_HILOGI("run in:%{public}d,%{public}s,%{public}s.", userId, bundleName.c_str(), globalContent.c_str());
    auto globalUserId = GetGlobalTableUserId(globalContent);
    if (!globalUserId.empty() && globalUserId == std::to_string(userId)) {
        IMSA_HILOGW("background add, but globalUserId same with userId:[%{public}d,%{public}d,%{public}s].",
            currentUserId_, userId, bundleName.c_str());
    }
    std::string finalUserContent;
    AddToUserEnableTable(userId, bundleName, finalUserContent);
}

void EnableImeDataParser::OnForegroundPackageAdded(
    int32_t userId, const std::string &bundleName, const std::string &globalContent)
{
    IMSA_HILOGI("run in:%{public}d,%{public}s,%{public}s.", userId, bundleName.c_str(), globalContent.c_str());
    auto globalUserId = GetGlobalTableUserId(globalContent);
    if (globalUserId.empty() || globalUserId == std::to_string(currentUserId_)) {
        std::string finalGlobalContent;
        auto ret = AddToGlobalEnableTable(userId, bundleName, finalGlobalContent);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("add failed:[%{public}d,%{public}s,%{public}s,%{public}d].", userId, globalUserId.c_str(),
                bundleName.c_str(), ret);
            return;
        }
        CoverUserEnableTable(userId, finalGlobalContent);
        return;
    }
    IMSA_HILOGW("globalUserId not same with currentUserId:%{public}d,%{public}s,%{public}s.", userId,
        bundleName.c_str(), globalContent.c_str());
    auto ret = CoverUserEnableTable(atoi(globalUserId.c_str()), globalContent);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("cover failed:[%{public}d,%{public}s,%{public}s,%{public}d].", userId, globalUserId.c_str(),
            bundleName.c_str(), ret);
        return;
    }
    std::string finalUserContent;
    ret = AddToUserEnableTable(userId, bundleName, finalUserContent);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId not same, add failed:[%{public}d,%{public}s,%{public}s,%{public}d].", userId,
            globalUserId.c_str(), bundleName.c_str(), ret);
        return;
    }
    CoverGlobalEnableTable(finalUserContent);
}

int32_t EnableImeDataParser::AddToUserEnableTable(
    int32_t userId, const std::string &bundleName, std::string &userContent)
{
    return AddToEnableTable(
        userId, bundleName, SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true", userContent);
}

int32_t EnableImeDataParser::AddToGlobalEnableTable(
    int32_t userId, const std::string &bundleName, std::string &globalContent)
{
    return AddToEnableTable(userId, bundleName, SETTING_URI_PROXY, globalContent);
}

int32_t EnableImeDataParser::AddToEnableTable(
    int32_t userId, const std::string &bundleName, const std::string &uriProxy, std::string &tableContent)
{
    auto settingInstance = SettingsDataUtils::GetInstance();
    if (settingInstance == nullptr) {
        return ErrorCode::ERROR_ENABLE_IME;
    }
    std::string valueStr;
    int32_t ret = settingInstance->GetStringValue(uriProxy, ENABLE_IME, valueStr);
    if (ret != ErrorCode::NO_ERROR && ret != ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        IMSA_HILOGE("get enable table failed:[%{public}d, %{public}s, %{public}s, %{public}d].", userId,
            bundleName.c_str(), uriProxy.c_str(), ret);
        return ret;
    }
    IMSA_HILOGI("start:%{public}d,%{public}s,%{public}s,%{public}s.", userId, bundleName.c_str(), uriProxy.c_str(),
        valueStr.c_str());
    EnableImeCfg imeCfg;
    imeCfg.userImeCfg.userId = std::to_string(userId);
    imeCfg.Unmarshall(valueStr);
    auto it = std::find_if(imeCfg.userImeCfg.identities.begin(), imeCfg.userImeCfg.identities.end(),
        [&bundleName](const std::string &bundleNameTmp) { return bundleNameTmp == bundleName; });
    if (it == imeCfg.userImeCfg.identities.end()) {
        imeCfg.userImeCfg.identities.push_back(bundleName);
    }
    imeCfg.Marshall(tableContent);
    if (tableContent.empty()) {
        IMSA_HILOGE(
            "Marshall failed:[%{public}d, %{public}s, %{public}s].", userId, bundleName.c_str(), uriProxy.c_str());
        return ErrorCode::ERROR_ENABLE_IME;
    }
    settingInstance->SetStringValue(uriProxy, ENABLE_IME, tableContent);
    IMSA_HILOGI("end:%{public}d,%{public}s,%{public}s,%{public}s.", userId, bundleName.c_str(), uriProxy.c_str(),
        tableContent.c_str());
    return ErrorCode::NO_ERROR;
}

int32_t EnableImeDataParser::CoverUserEnableTable(int32_t userId, const std::string &userContent)
{
    auto settingInstance = SettingsDataUtils::GetInstance();
    if (settingInstance == nullptr) {
        return ErrorCode::ERROR_ENABLE_IME;
    }
    auto ret = settingInstance->SetStringValue(
        SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true", ENABLE_IME, userContent);
    if (!ret) {
        return ErrorCode::ERROR_ENABLE_IME;
    }
    return ErrorCode::NO_ERROR;
}

int32_t EnableImeDataParser::GetImeEnablePattern(int32_t userId, const std::string &bundleName, EnabledStatus &status)
{
    if (ImeInfoInquirer::GetInstance().GetDefaultIme().bundleName == bundleName) {
        status = EnabledStatus::BASIC_MODE;
        return ErrorCode::NO_ERROR;
    }
    std::vector<std::string> bundleNames;
    auto ret = GetEnableIme(userId, bundleNames);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("[%{public}d, %{public}s] GetEnableIme failed:%{public}d.", userId, bundleName.c_str(), ret);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    auto it = std::find_if(bundleNames.begin(), bundleNames.end(),
        [&bundleName](const std::string &bundleNameTmp) { return bundleNameTmp == bundleName; });
    if (it == bundleNames.end()) {
        status = EnabledStatus::DISABLED;
        return ErrorCode::NO_ERROR;
    }
    status = EnabledStatus::BASIC_MODE;
    return ErrorCode::NO_ERROR;
}

int32_t EnableImeDataParser::UpdateEnableData(int32_t userId, const std::string &key)
{
    std::lock_guard<std::mutex> autoLock(listMutex_);
    if (key == ENABLE_IME) {
        isEnableImeInit_ = false;
    }
    std::vector<std::string> enableData;
    IMSA_HILOGD("update userId: %{public}d %{public}s", userId, key.c_str());
    auto ret = GetEnableData(key, enableData, userId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d get enable %{public}s list failed!", userId, key.c_str());
        return ret;
    }
    enableList_.insert_or_assign(key, enableData);
    if (key == ENABLE_IME) {
        isEnableImeInit_ = true;
    }
    return ErrorCode::NO_ERROR;
}

int32_t EnableImeDataParser::GetEnableIme(int32_t userId, std::vector<std::string> &enableVec)
{
    if (userId != currentUserId_) {
        return GetEnableData(ENABLE_IME, enableVec, userId);
    }
    auto ret = GetEnableImeFromCache(enableVec);
    if (ret == ErrorCode::NO_ERROR) {
        return ErrorCode::NO_ERROR;
    }
    ret = UpdateEnableData(userId, ENABLE_IME);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    return GetEnableImeFromCache(enableVec);
}

int32_t EnableImeDataParser::GetEnableImeFromCache(std::vector<std::string> &enableVec)
{
    std::lock_guard<std::mutex> autoLock(listMutex_);
    if (isEnableImeInit_) {
        auto it = enableList_.find(ENABLE_IME);
        if (it != enableList_.end()) {
            IMSA_HILOGD("GetEnableIme from cache.");
            enableVec = it->second;
            return ErrorCode::NO_ERROR;
        }
    }
    return ErrorCode::ERROR_ENABLE_IME;
}
} // namespace MiscServices
} // namespace OHOS