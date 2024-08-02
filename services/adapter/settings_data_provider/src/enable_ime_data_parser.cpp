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

#include "ime_info_inquirer.h"
#include "iservice_registry.h"
#include "serializable.h"
#include "settings_data_utils.h"
#include "system_ability_definition.h"

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

int32_t EnableImeDataParser::GetEnableData(const std::string &key, std::vector<std::string> &enableVec,
    const int32_t userId)
{
    if (key != std::string(ENABLE_IME) && key != std::string(ENABLE_KEYBOARD) && key != std::string(TEMP_IME)) {
        IMSA_HILOGD("invalid key: %{public}s.", key.c_str());
        return ErrorCode::ERROR_ENABLE_IME;
    }

    IMSA_HILOGD("userId: %{public}d, key: %{public}s.", userId, key.c_str());
    std::string valueStr;
    int32_t ret = SettingsDataUtils::GetInstance()->GetStringValue(key, valueStr);
    if (ret == ErrorCode::ERROR_KEYWORD_NOT_FOUND) {
        IMSA_HILOGW("no keyword exist");
        enableVec.clear();
        return ErrorCode::NO_ERROR;
    }
    if (ret != ErrorCode::NO_ERROR || valueStr.empty()) {
        IMSA_HILOGW("get value failed, or valueStr is empty.");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    auto parseRet = false;
    if (key == ENABLE_IME) {
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