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

#ifndef SETTINGS_DATA_UTILS_H
#define SETTINGS_DATA_UTILS_H

#include "datashare_helper.h"
#include "input_method_property.h"
#include "serializable.h"
#include "settings_data_observer.h"

namespace OHOS {
namespace MiscServices {
constexpr const char *SETTING_COLUMN_KEYWORD = "KEYWORD";
constexpr const char *SETTING_COLUMN_VALUE = "VALUE";
constexpr const char *SETTING_URI_PROXY = "datashare:///com.ohos.settingsdata/entry/settingsdata/"
                                          "SETTINGSDATA?Proxy=true";
constexpr const char *SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
constexpr const char *SETTINGS_USER_DATA_URI = "datashare:///com.ohos.settingsdata/"
                                               "entry/settingsdata/USER_SETTINGSDATA_";
struct UserImeConfig : public Serializable {
    std::string userId;
    std::vector<std::string> identities;
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, userId, identities);
    }
    bool Marshal(cJSON *node) const override
    {
        return SetValue(node, userId, identities);
    }
};

class SettingsDataUtils : public RefBase {
public:
    static sptr<SettingsDataUtils> GetInstance();
    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(const std::string &uriProxy);
    int32_t CreateAndRegisterObserver(
        const std::string &uriProxy, const std::string &key, SettingsDataObserver::CallbackFunc func);
    int32_t GetStringValue(const std::string &uriProxy, const std::string &key, std::string &value);
    bool SetStringValue(const std::string &uriProxy, const std::string &key, const std::string &value);
    bool ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper> &helper);
    Uri GenerateTargetUri(const std::string &uriProxy, const std::string &key);
    bool EnableIme(int32_t userId, const std::string &bundleName);
    void NotifyDataShareReady();
    bool IsDataShareReady();

private:
    SettingsDataUtils() = default;
    ~SettingsDataUtils();
    int32_t RegisterObserver(const std::string &uriProxy, const sptr<SettingsDataObserver> &observer);
    int32_t UnregisterObserver(const std::string &uriProxy, const sptr<SettingsDataObserver> &observer);
    sptr<IRemoteObject> GetToken();
    std::vector<std::string> Split(const std::string &text, char separator);
    std::string SetSettingValues(const std::string &settingValue, const std::string &bundleName);

private:
    static std::mutex instanceMutex_;
    static sptr<SettingsDataUtils> instance_;
    std::mutex remoteObjMutex_;
    sptr<IRemoteObject> remoteObj_ = nullptr;
    std::mutex observerListMutex_;
    std::vector<std::pair<std::string, sptr<SettingsDataObserver>>> observerList_;
    std::atomic<bool> isDataShareReady_{ false };
};
} // namespace MiscServices
} // namespace OHOS

#endif // SETTINGS_DATA_UTILS_H
