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

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "datashare_helper.h"
#include "global.h"
#include "input_method_property.h"
#include "serializable.h"
#include "settings_data_observer.h"
#include "uri.h"

namespace OHOS {
namespace MiscServices {
struct UserImeConfig : public Serializable {
    std::string userId;
    std::vector<std::string> identities;
    bool Unmarshal(cJSON *node) override
    {
        GetValue(node, userId, identities);
        return true;
    }
};

class SettingsDataUtils : public RefBase {
public:
    static sptr<SettingsDataUtils> GetInstance();
    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper();
    int32_t CreateAndRegisterObserver(const std::string &key, SettingsDataObserver::CallbackFunc func);
    int32_t GetStringValue(const std::string &key, std::string &value);
    bool ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper> &helper);
    Uri GenerateTargetUri(const std::string &key);
    bool EnableIme(int32_t userId, const std::string &bundleName);

private:
    SettingsDataUtils() = default;
    ~SettingsDataUtils();
    int32_t RegisterObserver(const sptr<SettingsDataObserver> &observer);
    int32_t UnregisterObserver(const sptr<SettingsDataObserver> &observer);
    sptr<IRemoteObject> GetToken();
    std::vector<std::string> split(const std::string &text, char separator);
    std::string SetSettingValues(const std::string &settingValue, const std::string &bundleName);

private:
    static std::mutex instanceMutex_;
    static sptr<SettingsDataUtils> instance_;
    std::mutex tokenMutex_;
    sptr<IRemoteObject> remoteObj_ = nullptr;
    std::mutex observerListMutex_;
    std::vector<sptr<SettingsDataObserver>> observerList_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // SETTINGS_DATA_UTILS_H
