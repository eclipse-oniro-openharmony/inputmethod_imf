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
#include <sstream>
#include "settings_data_utils.h"

#include "ime_info_inquirer.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
std::mutex SettingsDataUtils::instanceMutex_;
sptr<SettingsDataUtils> SettingsDataUtils::instance_ = nullptr;
SettingsDataUtils::~SettingsDataUtils()
{
    {
        std::lock_guard<std::mutex> autoLock(remoteObjMutex_);
        remoteObj_ = nullptr;
    }
    std::lock_guard<decltype(observerListMutex_)> lock(observerListMutex_);
    if (!observerList_.empty()) {
        for (auto &iter : observerList_) {
            UnregisterObserver(iter);
        }
        observerList_.clear();
    }
}

sptr<SettingsDataUtils> SettingsDataUtils::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceMutex_);
        if (instance_ == nullptr) {
            IMSA_HILOGI("GetInstance need new SettingsDataUtils.");
            instance_ = new (std::nothrow) SettingsDataUtils();
            if (instance_ == nullptr) {
                IMSA_HILOGE("instance is nullptr!");
                return instance_;
            }
        }
    }
    return instance_;
}

int32_t SettingsDataUtils::CreateAndRegisterObserver(const std::string &key, SettingsDataObserver::CallbackFunc func)
{
    IMSA_HILOGD("key: %{public}s.", key.c_str());
    sptr<SettingsDataObserver> observer = new (std::nothrow) SettingsDataObserver(key, func);
    if (observer == nullptr) {
        IMSA_HILOGE("observer is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return RegisterObserver(observer);
}

int32_t SettingsDataUtils::RegisterObserver(const sptr<SettingsDataObserver> &observer)
{
    if (observer == nullptr) {
        IMSA_HILOGE("observer is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }

    auto uri = GenerateTargetUri(std::string(SETTING_URI_PROXY), observer->GetKey());
    auto helper = SettingsDataUtils::CreateDataShareHelper(std::string(SETTING_URI_PROXY));
    if (helper == nullptr) {
        IMSA_HILOGE("helper is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    helper->RegisterObserver(uri, observer);
    ReleaseDataShareHelper(helper);
    IMSA_HILOGD("succeed to register observer of uri: %{public}s.", uri.ToString().c_str());

    std::lock_guard<decltype(observerListMutex_)> lock(observerListMutex_);
    observerList_.push_back(observer);
    return ErrorCode::NO_ERROR;
}

int32_t SettingsDataUtils::UnregisterObserver(const sptr<SettingsDataObserver> &observer)
{
    auto uri = GenerateTargetUri(std::string(SETTING_URI_PROXY), observer->GetKey());
    auto helper = SettingsDataUtils::CreateDataShareHelper(std::string(SETTING_URI_PROXY));
    if (helper == nullptr) {
        return ErrorCode::ERROR_ENABLE_IME;
    }
    helper->UnregisterObserver(uri, observer);
    ReleaseDataShareHelper(helper);
    IMSA_HILOGD("succeed to unregister observer of uri: %{public}s.", uri.ToString().c_str());
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<DataShare::DataShareHelper> SettingsDataUtils::CreateDataShareHelper(const std::string &uriProxy)
{
    auto remoteObj = GetToken();
    if (remoteObj == nullptr) {
        IMSA_HILOGE("remoteObk is nullptr!");
        return nullptr;
    }

    auto helper = DataShare::DataShareHelper::Creator(remoteObj_, uriProxy, std::string(SETTINGS_DATA_EXT_URI));
    if (helper == nullptr) {
        IMSA_HILOGE("create helper failed, uri: %{public}s!", uriProxy.c_str());
        return nullptr;
    }
    return helper;
}

bool SettingsDataUtils::ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper> &helper)
{
    if (helper == nullptr) {
        IMSA_HILOGW("helper is nullptr.");
        return true;
    }
    if (!helper->Release()) {
        IMSA_HILOGE("release data share helper failed.");
        return false;
    }
    return true;
}

Uri SettingsDataUtils::GenerateTargetUri(const std::string &uriProxy, const std::string &key)
{
    Uri uri(std::string(uriProxy) + "&key=" + key);
    return uri;
}

bool SettingsDataUtils::SetStringValue(const std::string &uriProxy, const std::string &key, const std::string &value)
{
    IMSA_HILOGD("start.");
    auto helper = CreateDataShareHelper(uriProxy);
    if (helper == nullptr) {
        IMSA_HILOGE("helper is nullptr.");
        return false;
    }
    DataShare::DataShareValueObject keyObj(key);
    DataShare::DataShareValueObject valueObj(value);
    DataShare::DataShareValuesBucket bucket;
    bucket.Put(SETTING_COLUMN_KEYWORD, keyObj);
    bucket.Put(SETTING_COLUMN_VALUE, valueObj);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(GenerateTargetUri(uriProxy, key));
    if (helper->Update(uri, predicates, bucket) <= 0) {
        int index = helper->Insert(uri, bucket);
        IMSA_HILOGI("no data exists, insert ret index: %{public}d", index);
    } else {
        IMSA_HILOGI("data exits");
    }
    bool ret = ReleaseDataShareHelper(helper);
    IMSA_HILOGI("ReleaseDataShareHelper isSuccess: %{public}d", ret);
    return ret;
}

int32_t SettingsDataUtils::GetStringValue(const std::string &uriProxy, const std::string &key, std::string &value)
{
    IMSA_HILOGD("start.");
    auto helper = CreateDataShareHelper(uriProxy);
    if (helper == nullptr) {
        IMSA_HILOGE("helper is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    std::vector<std::string> columns = { SETTING_COLUMN_VALUE };
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(GenerateTargetUri(uriProxy, key));
    auto resultSet = helper->Query(uri, predicates, columns);
    ReleaseDataShareHelper(helper);
    if (resultSet == nullptr) {
        IMSA_HILOGE("resultSet is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }

    int32_t count = 0;
    resultSet->GetRowCount(count);
    if (count <= 0) {
        IMSA_HILOGW("not found keyword, key: %{public}s, count: %{public}d.", key.c_str(), count);
        resultSet->Close();
        return ErrorCode::ERROR_KEYWORD_NOT_FOUND;
    }

    int32_t columIndex = 0;
    resultSet->GoToFirstRow();
    resultSet->GetColumnIndex(SETTING_COLUMN_VALUE, columIndex);
    int32_t ret = resultSet->GetString(columIndex, value);
    if (ret != DataShare::E_OK) {
        IMSA_HILOGE("failed to GetString, ret: %{public}d!", ret);
    }
    resultSet->Close();
    return ret;
}

sptr<IRemoteObject> SettingsDataUtils::GetToken()
{
    std::lock_guard<std::mutex> autoLock(remoteObjMutex_);
    if (remoteObj_ != nullptr) {
        return remoteObj_;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        IMSA_HILOGE("system ability manager is nullptr!");
        return nullptr;
    }
    auto remoteObj = samgr->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID);
    if (remoteObj == nullptr) {
        IMSA_HILOGE("system ability is nullptr!");
        return nullptr;
    }
    remoteObj_ = remoteObj;
    return remoteObj_;
}

bool SettingsDataUtils::EnableIme(int32_t userId, const std::string &bundleName)
{
    const int32_t mainUserId = 100;
    if (userId != mainUserId) {
        IMSA_HILOGE("user is not main.");
        return false;
    }
    const char *settingKey = "settings.inputmethod.enable_ime";
    std::string settingValue = "";
    GetStringValue(std::string(SETTING_URI_PROXY), settingKey, settingValue);
    IMSA_HILOGI("settingValue: %{public}s", settingValue.c_str());
    std::string value = "";
    if (settingValue == "") {
        value = "{\"enableImeList\" : {\"100\" : [\"" + bundleName + "\"]}}";
    } else {
        value = SetSettingValues(settingValue, bundleName);
    }
    IMSA_HILOGI("value: %{public}s", value.c_str());
    return SetStringValue(std::string(SETTING_URI_PROXY), settingKey, value);
}
 
std::vector<std::string> SettingsDataUtils::split(const std::string &text, char delim)
{
    std::vector<std::string> tokens;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            tokens.push_back(item);
        }
    }
    return tokens;
}
 
std::string SettingsDataUtils::SetSettingValues(const std::string &settingValue, const std::string &bundleName)
{
    std::string value = "";
    std::vector<std::string> settingValues = split(settingValue, ']');
    for (uint32_t i = 0; i < settingValues.size(); ++i) {
        if (i == 0) {
            if (settingValues[0].back() == '[') {
                value += settingValues[i] + "\"" + bundleName + "\"" + "]";
            } else {
                value += settingValues[i] + ",\"" + bundleName + "\"" + "]";
            }
        } else {
            value += settingValues[i];
        }
    }
    return value;
}
} // namespace MiscServices
} // namespace OHOS