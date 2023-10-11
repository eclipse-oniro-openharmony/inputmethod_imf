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
#include "nlohmann/json.hpp"
#include "system_ability_definition.h"
#include "uri.h"

namespace OHOS {
namespace MiscServices {
using json = nlohmann::json;
std::mutex EnableImeDataParser::instanceMutex_;
sptr<EnableImeDataParser> EnableImeDataParser::instance_ = nullptr;
namespace {
constexpr const char *SETTING_COLUMN_KEYWORD = "KEYWORD";
constexpr const char *SETTING_COLUMN_VALUE = "VALUE";
constexpr const char *SETTING_URI_PROXY = "datashare:///com.ohos.settingsdata/entry/settingsdata/"
                                          "SETTINGSDATA?Proxy=true";
constexpr const char *SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
} // namespace
EnableImeDataParser::~EnableImeDataParser()
{
    remoteObj_ = nullptr;
    if (!observerList_.empty()) {
        for (auto &iter : observerList_) {
            UnregisterObserver(iter);
        }
        observerList_.clear();
    }
}

sptr<EnableImeDataParser> EnableImeDataParser::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceMutex_);
        if (instance_ == nullptr) {
            IMSA_HILOGI("GetInstance need new EnableImeDataParser");
            instance_ = new (std::nothrow) EnableImeDataParser();
            if (instance_ == nullptr) {
                IMSA_HILOGI("instance is nullptr.");
                return instance_;
            }
        }
    }
    return instance_;
}

int32_t EnableImeDataParser::Initialize(const int32_t userId)
{
    currrentUserId_ = userId;
    enableList_.insert({ std::string(ENABLE_IME), {} });
    enableList_.insert({ std::string(ENABLE_KEYBOARD), {} });

    if (GetEnableData(ENABLE_IME, enableList_[std::string(ENABLE_IME)], userId) != ErrorCode::NO_ERROR
        || GetEnableData(ENABLE_KEYBOARD, enableList_[std::string(ENABLE_KEYBOARD)], userId) != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("get enable list failed.");
        return ErrorCode::ERROR_ENABLE_IME;
    }

    GetDefaultIme();
    return ErrorCode::NO_ERROR;
}

void EnableImeDataParser::OnUserChanged(const int32_t targetUserId)
{
    std::lock_guard<std::mutex> autoLock(listMutex_);
    IMSA_HILOGD("Current userId %{public}d, switch to %{puclic}d", currrentUserId_, targetUserId);
    currrentUserId_ = targetUserId;
    if (GetEnableData(ENABLE_IME, enableList_[std::string(ENABLE_IME)], targetUserId) != ErrorCode::NO_ERROR
        || GetEnableData(ENABLE_KEYBOARD, enableList_[std::string(ENABLE_KEYBOARD)], targetUserId)
               != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("get enable list failed.");
        return;
    }
}

int32_t EnableImeDataParser::CreateAndRegisterObserver(const std::string &key, EnableImeDataObserver::CallbackFunc func)
{
    IMSA_HILOGD("key: %{public}s.", key.c_str());
    sptr<EnableImeDataObserver> observer = new (std::nothrow) EnableImeDataObserver(key, func);
    if (observer == nullptr) {
        IMSA_HILOGE("new observer is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return RegisterObserver(observer);
}

int32_t EnableImeDataParser::RegisterObserver(const sptr<EnableImeDataObserver> &observer)
{
    if (observer == nullptr) {
        IMSA_HILOGE("observer is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }

    auto uri = GenerateTargetUri(observer->GetKey());
    auto helper = CreateDataShareHelper();
    if (helper == nullptr) {
        IMSA_HILOGE("CreateDataShareHelper return nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    helper->RegisterObserver(uri, observer);
    ReleaseDataShareHelper(helper);
    IMSA_HILOGD("succeed to register observer of uri=%{public}s", uri.ToString().c_str());
    observerList_.push_back(observer);
    return ErrorCode::NO_ERROR;
}

int32_t EnableImeDataParser::UnregisterObserver(const sptr<EnableImeDataObserver> &observer)
{
    auto uri = GenerateTargetUri(observer->GetKey());
    auto helper = CreateDataShareHelper();
    if (helper == nullptr) {
        return ErrorCode::ERROR_ENABLE_IME;
    }
    helper->UnregisterObserver(uri, observer);
    ReleaseDataShareHelper(helper);
    IMSA_HILOGD("succeed to unregister observer of uri=%{public}s", uri.ToString().c_str());
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<DataShare::DataShareHelper> EnableImeDataParser::CreateDataShareHelper()
{
    auto remoteObj = GetToken();
    if (remoteObj == nullptr) {
        IMSA_HILOGE("remoteObk is nullptr.");
        return nullptr;
    }

    auto helper = DataShare::DataShareHelper::Creator(remoteObj_, SETTING_URI_PROXY, SETTINGS_DATA_EXT_URI);
    if (helper == nullptr) {
        IMSA_HILOGE("Create helper failed, uri=%{public}s", SETTING_URI_PROXY);
        return nullptr;
    }
    return helper;
}

bool EnableImeDataParser::ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper> &helper)
{
    if (helper == nullptr) {
        IMSA_HILOGW("helper is nullptr.");
        return true;
    }
    if (!helper->Release()) {
        IMSA_HILOGE("Release data share helper failed.");
        return false;
    }
    return true;
}

int32_t EnableImeDataParser::GetNextSwitchInfo(SwitchInfo &switchInfo, const int32_t userId)
{
    std::vector<std::string> enableVec;
    int32_t ret = GetEnableData(ENABLE_IME, enableVec, userId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Get enable list abnormal.");
        return ret;
    }
    auto currentIme = ImeInfoInquirer::GetInstance().GetCurrentInputMethod(userId);
    switchInfo.bundleName = GetDefaultIme()->name;
    switchInfo.subName = "";

    if (enableVec.empty()) {
        IMSA_HILOGE("Enable ime list empty.");
        return ErrorCode::NO_ERROR;
    }
    if (currentIme->name == GetDefaultIme()->name) {
        switchInfo.bundleName = enableVec[0];
        return ErrorCode::NO_ERROR;
    }

    auto iter = std::find_if(
        enableVec.begin(), enableVec.end(), [&currentIme](const std::string &ime) { return currentIme->name == ime; });
    if (iter != enableVec.end() && ++iter != enableVec.end()) {
        switchInfo.bundleName = *iter;
    }
    return ErrorCode::NO_ERROR;
}

bool EnableImeDataParser::CheckNeedSwitch(const std::string &key, SwitchInfo &switchInfo, const int32_t userId)
{
    auto currentIme = ImeInfoInquirer::GetInstance().GetCurrentInputMethod(userId);
    switchInfo.bundleName = GetDefaultIme()->name;
    switchInfo.subName = "";
    if (key == std::string(ENABLE_IME)) {
        if (currentIme->name == GetDefaultIme()->name) {
            IMSA_HILOGD("Current ime is default, do not need switch ime.");
            return false;
        }
        return CheckTargetEnableName(key, currentIme->name, switchInfo.bundleName, userId);
    } else if (key == std::string(ENABLE_KEYBOARD)) {
        if (currentIme->name != GetDefaultIme()->name || currentIme->id == GetDefaultIme()->id) {
            IMSA_HILOGD("Current ime is not default or id is default.");
            return false;
        }
        switchInfo.subName = GetDefaultIme()->id;
        return CheckTargetEnableName(key, currentIme->id, switchInfo.subName, userId);
    }
    IMSA_HILOGW("Invalid key! key: %{public}s", key.c_str());
    return false;
}

bool EnableImeDataParser::CheckNeedSwitch(const SwitchInfo &info, const int32_t userId)
{
    IMSA_HILOGD("Current userId %{public}d, target userId %{puclic}d", currrentUserId_, userId);
    std::vector<std::string> enableVec;
    std::string targetName;
    int32_t ret = 0;
    if (info.bundleName == GetDefaultIme()->name) {
        IMSA_HILOGD("Check ime keyboard.");
        if (info.subName == GetDefaultIme()->id) {
            return true;
        }
        targetName = info.subName;
        ret = GetEnableData(ENABLE_KEYBOARD, enableVec, userId);
    } else {
        IMSA_HILOGD("Check ime.");
        targetName = info.bundleName;
        ret = GetEnableData(ENABLE_IME, enableVec, userId);
    }
    if (ret != ErrorCode::NO_ERROR || enableVec.empty()) {
        IMSA_HILOGD("Get enable list failed, or enable list is empty.");
        return false;
    }

    auto iter = std::find_if(
        enableVec.begin(), enableVec.end(), [&targetName](const std::string &ime) { return targetName == ime; });
    if (iter != enableVec.end()) {
        IMSA_HILOGD("In enable list.");
        return true;
    }
    return false;
}

bool EnableImeDataParser::CheckTargetEnableName(
    const std::string &key, const std::string &targetName, std::string &nextIme, const int32_t userId)
{
    std::vector<std::string> enableVec;
    int32_t ret = GetEnableData(key, enableVec, userId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Get enable list abnormal.");
        return false;
    }

    if (enableVec.empty()) {
        IMSA_HILOGE("Enable empty, switch default ime.");
        return true;
    }
    std::lock_guard<std::mutex> autoLock(listMutex_);
    auto iter = std::find_if(
        enableVec.begin(), enableVec.end(), [&targetName](const std::string &ime) { return ime == targetName; });
    if (iter != enableVec.end()) {
        IMSA_HILOGD("Enable list has current ime, do not need switch.");
        enableList_[key].assign(enableVec.begin(), enableVec.end());
        return false;
    }

    auto it = std::find_if(enableList_[key].begin(), enableList_[key].end(),
        [&targetName](const std::string &ime) { return ime == targetName; });

    if (it != enableList_[key].end() && ++it != enableList_[key].end()) {
        auto result = std::find_first_of(it, enableList_[key].end(), enableVec.begin(), enableVec.end());
        nextIme = *result;
    }
    enableList_[key].assign(enableVec.begin(), enableVec.end());
    return true;
}

Uri EnableImeDataParser::GenerateTargetUri(const std::string &key)
{
    Uri uri(std::string(SETTING_URI_PROXY) + "&key=" + key);
    return uri;
}

int32_t EnableImeDataParser::GetEnableData(
    const std::string &key, std::vector<std::string> &enableVec, const int32_t userId)
{
    if (key != std::string(ENABLE_IME) && key != std::string(ENABLE_KEYBOARD)) {
        IMSA_HILOGD("Invalid key: %{public}s.", key.c_str());
        return ErrorCode::ERROR_ENABLE_IME;
    }

    IMSA_HILOGD("key: %{public}s.", key.c_str());
    std::string valueStr;
    int32_t ret = GetStringValue(key, valueStr);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }

    IMSA_HILOGD("valueStr: %{public}s.", valueStr.c_str());
    if (valueStr.empty()) {
        IMSA_HILOGW("valueStr is empty");
        return ErrorCode::NO_ERROR;
    }

    json jsonEnableData = json::parse(valueStr.c_str());
    if (jsonEnableData.is_null() || jsonEnableData.is_discarded()) {
        IMSA_HILOGE("json parse failed.");
        return ErrorCode::ERROR_ENABLE_IME;
    }

    if (!jsonEnableData.contains("enableImeList") || !jsonEnableData["enableImeList"].is_object()) {
        IMSA_HILOGE("enableImeList not find or abnormal");
        return ErrorCode::ERROR_ENABLE_IME;
    }

    std::string id = std::to_string(userId);
    if (!jsonEnableData["enableImeList"].contains(id) || !jsonEnableData["enableImeList"][id].is_array()) {
        IMSA_HILOGE("user id not find or abnormal");
        return ErrorCode::ERROR_ENABLE_IME;
    }

    std::vector<std::string> enableVecTemp;
    for (const auto &bundleName : jsonEnableData["enableImeList"][id]) {
        IMSA_HILOGD("enable ime string: %{public}s", std::string(bundleName).c_str());
        enableVecTemp.push_back(bundleName);
    }
    enableVec.assign(enableVecTemp.begin(), enableVecTemp.end());

    return ErrorCode::NO_ERROR;
}

int32_t EnableImeDataParser::GetStringValue(const std::string &key, std::string &value)
{
    auto helper = CreateDataShareHelper();
    if (helper == nullptr) {
        IMSA_HILOGE("CreateDataShareHelper return nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    std::vector<std::string> columns = { SETTING_COLUMN_VALUE };
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(GenerateTargetUri(key));
    auto resultSet = helper->Query(uri, predicates, columns);
    ReleaseDataShareHelper(helper);
    if (resultSet == nullptr) {
        IMSA_HILOGE("helper->Query return nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }

    int32_t count = 0;
    resultSet->GetRowCount(count);
    if (count <= 0) {
        IMSA_HILOGW("Not found keyword, key=%{public}s, count=%{public}d", key.c_str(), count);
        resultSet->Close();
        return ErrorCode::ERROR_KEYWORD_NOT_FOUND;
    }

    int32_t columIndex = 0;
    resultSet->GoToFirstRow();
    resultSet->GetColumnIndex(SETTING_COLUMN_VALUE, columIndex);
    int32_t ret = resultSet->GetString(columIndex, value);
    if (ret != NativeRdb::E_OK) {
        IMSA_HILOGE("GetString failed, ret=%{public}d", ret);
    }
    resultSet->Close();
    return ret;
}

sptr<IRemoteObject> EnableImeDataParser::GetToken()
{
    std::lock_guard<std::mutex> autoLock(tokenMutex_);
    if (remoteObj_ != nullptr) {
        return remoteObj_;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        IMSA_HILOGE("GetSystemAbilityManager return nullptr");
        return nullptr;
    }
    auto remoteObj = samgr->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID);
    if (remoteObj == nullptr) {
        IMSA_HILOGE("GetSystemAbility return nullptr");
        return nullptr;
    }
    remoteObj_ = remoteObj;
    return remoteObj_;
}

std::shared_ptr<Property> EnableImeDataParser::GetDefaultIme()
{
    if (defaultImeInfo_ != nullptr) {
        return defaultImeInfo_;
    }
    defaultImeInfo_ = std::make_shared<Property>();
    auto info = ImeInfoInquirer::GetInstance().GetDefaultImeInfo(currrentUserId_);
    defaultImeInfo_->name = info->prop.name;
    defaultImeInfo_->id = info->prop.id;
    return defaultImeInfo_;
}
} // namespace MiscServices
} // namespace OHOS