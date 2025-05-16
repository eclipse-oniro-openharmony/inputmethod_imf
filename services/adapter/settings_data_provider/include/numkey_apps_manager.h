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

#ifndef INPUTMETHOD_IMF_AUTO_NUMBER_INPUT_APPS_MANAGER_H
#define INPUTMETHOD_IMF_AUTO_NUMBER_INPUT_APPS_MANAGER_H

#include "settings_data_utils.h"

namespace OHOS {
namespace MiscServices {
static constexpr int32_t BLOCK_MODE = 8;
struct NumkeyAPP : public Serializable {
    std::string name;
    bool numKey{ false };
    bool Unmarshal(cJSON *node) override
    {
        GetValue(node, GET_NAME(name), name);
        GetValue(node, GET_NAME(numKey), numKey);
        return true;
    }
};

struct NumkeyAppListCfg : public Serializable {
    std::vector<NumkeyAPP> numkeyApps;
    bool Unmarshal(cJSON *node) override
    {
        std::function<bool(NumkeyAPP)> filter = [](const NumkeyAPP &app) { return app.numKey; };
        GetValues(node, numkeyApps, filter);
        return true;
    }
};

struct UserBlockListCfg : public Serializable {
    std::vector<std::string> blockApps;
    bool Unmarshal(cJSON *node) override
    {
        std::function<bool(int32_t)> filter = [](const int32_t &mode) { return mode == BLOCK_MODE; };
        GetKeys(node, blockApps, filter);
        return true;
    }
};

class NumkeyAppsManager {
public:
    static NumkeyAppsManager &GetInstance();
    int32_t Init(int32_t userId);
    bool NeedAutoNumKeyInput(int32_t userId, const std::string &bundleName);
    int32_t OnUserSwitched(int32_t userId);
    int32_t OnUserRemoved(int32_t userId);
    int32_t RegisterUserBlockListData(int32_t userId);
    static constexpr const char *COMPATIBLE_SETTING_STRATEGY = "COMPATIBLE_SETTING_STRATEGY";
    static constexpr const char *COMPATIBLE_APP_STRATEGY = "COMPATIBLE_APP_STRATEGY";

private:
    NumkeyAppsManager() = default;
    ~NumkeyAppsManager() = default;
    NumkeyAppsManager(const NumkeyAppsManager &) = delete;
    NumkeyAppsManager(NumkeyAppsManager &&) = delete;
    NumkeyAppsManager &operator=(const NumkeyAppsManager &) = delete;
    NumkeyAppsManager &operator=(NumkeyAppsManager &&) = delete;

    int32_t InitWhiteList(int32_t userId);
    int32_t UpdateUserBlockList(int32_t userId);
    static int32_t ParseWhiteList(int32_t userId, std::unordered_set<std::string> &list);
    static int32_t ParseBlockList(int32_t userId, std::unordered_set<std::string> &list);

    bool isFeatureEnabled_{ false };

    std::atomic<bool> isListInited_{ false };
    std::mutex appListLock_;
    std::unordered_set<std::string> numKeyAppList_;

    std::mutex blockListLock_;
    std::map<int32_t, std::unordered_set<std::string>> usersBlockList_;
    std::mutex observersLock_;
    std::map<int32_t, sptr<SettingsDataObserver>> observers_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_AUTO_NUMBER_INPUT_APPS_MANAGER_H
