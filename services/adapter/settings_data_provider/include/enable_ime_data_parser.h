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

#ifndef ENABLE_IME_DATA_PARSER_H
#define ENABLE_IME_DATA_PARSER_H

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "datashare_helper.h"
#include "global.h"
#include "input_method_property.h"
#include "input_method_status.h"
#include "serializable.h"
#include "settings_data_utils.h"

namespace OHOS {
namespace MiscServices {
struct SwitchInfo {
    std::chrono::system_clock::time_point timestamp{};
    std::string bundleName;
    std::string subName;
    bool operator==(const SwitchInfo &info) const
    {
        return (timestamp == info.timestamp && bundleName == info.bundleName && subName == info.subName);
    }
};

struct EnableKeyBoardCfg : public Serializable {
    UserImeConfig userImeCfg;
    bool Unmarshal(cJSON *node) override
    {
        GetValue(node, GET_NAME(enableKeyboardList), userImeCfg);
        return true;
    }
};

struct EnableImeCfg : public Serializable {
    UserImeConfig userImeCfg;
    bool Unmarshal(cJSON *node) override
    {
        GetValue(node, GET_NAME(enableImeList), userImeCfg);
        return true;
    }
    bool Marshal(cJSON *node) const override
    {
        SetValue(node, GET_NAME(enableImeList), userImeCfg);
        return true;
    }
};

struct TempImeCfg : public Serializable {
    UserImeConfig tempImeList;
    bool Unmarshal(cJSON *node) override
    {
        GetValue(node, GET_NAME(tempImeList), tempImeList);
        return true;
    }
};

class EnableImeDataParser : public RefBase {
public:
    static sptr<EnableImeDataParser> GetInstance();
    int32_t Initialize(const int32_t userId);
    int32_t GetEnableData(const std::string &key, std::vector<std::string> &enableVec, const int32_t userId);
    int32_t GetEnableIme(int32_t userId, std::vector<std::string> &enableVec);
    // for enable list changed
    bool CheckNeedSwitch(const std::string &key, SwitchInfo &switchInfo, const int32_t userId);
    // for switch target ime
    bool CheckNeedSwitch(const SwitchInfo &info, const int32_t userId);
    void OnUserChanged(const int32_t userId);
    void OnConfigChanged(int32_t userId, const std::string &key);
    void OnPackAdded(int32_t userId, const std::string &bundleName);
    int32_t GetImeEnablePattern(int32_t userId, const std::string &bundleName, EnabledStatus &status);
    
    static constexpr const char *ENABLE_IME = "settings.inputmethod.enable_ime";
    static constexpr const char *ENABLE_KEYBOARD = "settings.inputmethod.enable_keyboard";
    static constexpr const char *TEMP_IME = "settings.inputmethod.temp_ime";

private:
    EnableImeDataParser() = default;
    ~EnableImeDataParser();
    int32_t UpdateEnableData(int32_t userId, const std::string &key);
    void CoverGlobalEnableTable(const std::string &valueStr);
    std::string GetUserEnableTable(int32_t userId);
    std::string GetEanbleIme(int32_t userId, const std::string &globalStr);
    std::string GetGlobalTableUserId(const std::string &valueStr);
    int32_t GetEnableImeFromCache(std::vector<std::string> &enableVec);
    bool ParseEnableIme(const std::string &valueStr, int32_t userId, std::vector<std::string> &enableVec);
    bool ParseEnableKeyboard(const std::string &valueStr, int32_t userId, std::vector<std::string> &enableVec);
    bool ParseTempIme(const std::string &valueStr, int32_t userId, std::vector<std::string> &tempVector);
    bool CheckTargetEnableName(const std::string &key, const std::string &targetName, std::string &nextIme,
        const int32_t userId);
    std::shared_ptr<Property> GetDefaultIme();
    void OnPackAddedBackGround(int32_t userId, const std::string &bundleName, const std::string &globalContent);
    void OnPackAddedForeGround(int32_t userId, const std::string &bundleName, const std::string &globalContent);
    int32_t AddToUserEnabledTable(int32_t userId, const std::string &bundleName, std::string &userContent);
    int32_t AddToGlobalEnabledTable(int32_t userId, const std::string &bundleName, std::string &globalContent);
    int32_t AddToEnabledTable(
        int32_t userId, const std::string &bundleName, const std::string &uriProxy, std::string &tableContent);
    int32_t CoverUserEnabledTable(int32_t userId, const std::string &userContent);

private:
    static std::mutex instanceMutex_;
    static sptr<EnableImeDataParser> instance_;
    std::mutex listMutex_;
    std::unordered_map<std::string, std::vector<std::string>> enableList_;
    std::mutex defaultImeMutex_;
    std::shared_ptr<Property> defaultImeInfo_{ nullptr };
    int32_t currentUserId_ = 0;
    bool isEnableImeInit_{ false };
};
} // namespace MiscServices
} // namespace OHOS

#endif // ENABLE_IME_DATA_PARSER_H
