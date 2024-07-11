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

#include "settings_data_utils.h"

#include "ime_info_inquirer.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
std::mutex SettingsDataUtils::instanceMutex_;
sptr<SettingsDataUtils> SettingsDataUtils::instance_ = nullptr;
constexpr const char *SETTING_COLUMN_KEYWORD = "KEYWORD";
constexpr const char *SETTING_COLUMN_VALUE = "VALUE";
constexpr const char *SETTING_URI_PROXY = "datashare:///com.ohos.settingsdata/entry/settingsdata/"
                                          "SETTINGSDATA?Proxy=true";
constexpr const char *SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
SettingsDataUtils::~SettingsDataUtils()
{
    remoteObj_ = nullptr;
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

    auto uri = GenerateTargetUri(observer->GetKey());
    auto helper = SettingsDataUtils::CreateDataShareHelper();
    if (helper == nullptr) {
        IMSA_HILOGE("helper is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    helper->RegisterObserver(uri, observer);
    ReleaseDataShareHelper(helper);
    IMSA_HILOGD("succeed to register observer of uri: %{public}s.", uri.ToString().c_str());
    observerList_.push_back(observer);
    return ErrorCode::NO_ERROR;
}

int32_t SettingsDataUtils::UnregisterObserver(const sptr<SettingsDataObserver> &observer)
{
    auto uri = GenerateTargetUri(observer->GetKey());
    auto helper = SettingsDataUtils::CreateDataShareHelper();
    if (helper == nullptr) {
        return ErrorCode::ERROR_ENABLE_IME;
    }
    helper->UnregisterObserver(uri, observer);
    ReleaseDataShareHelper(helper);
    IMSA_HILOGD("succeed to unregister observer of uri: %{public}s.", uri.ToString().c_str());
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<DataShare::DataShareHelper> SettingsDataUtils::CreateDataShareHelper()
{
    auto remoteObj = GetToken();
    if (remoteObj == nullptr) {
        IMSA_HILOGE("remoteObk is nullptr!");
        return nullptr;
    }

    auto helper = DataShare::DataShareHelper::Creator(remoteObj_, SETTING_URI_PROXY, SETTINGS_DATA_EXT_URI);
    if (helper == nullptr) {
        IMSA_HILOGE("create helper failed, uri: %{public}s!", SETTING_URI_PROXY);
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

Uri SettingsDataUtils::GenerateTargetUri(const std::string &key)
{
    Uri uri(std::string(SETTING_URI_PROXY) + "&key=" + key);
    return uri;
}

int32_t SettingsDataUtils::GetStringValue(const std::string &key, std::string &value)
{
    IMSA_HILOGD("start.");
    auto helper = CreateDataShareHelper();
    if (helper == nullptr) {
        IMSA_HILOGE("helper is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    std::vector<std::string> columns = { SETTING_COLUMN_VALUE };
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(GenerateTargetUri(key));
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
    std::lock_guard<std::mutex> autoLock(tokenMutex_);
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
} // namespace MiscServices
} // namespace OHOS