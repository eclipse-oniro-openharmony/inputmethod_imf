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

struct EnableKeyboard : public Serializable {
    int32_t userId;
    std::vector<std::string> keyboards;
    explicit EnableKeyboard(int32_t userId) : userId(userId){};
    bool Unmarshal(cJSON *node) override
    {
        return Serializable::GetValue(node, GET_NAME(userId), keyboards);
    }
};
struct EnableKeyboardCfg : public Serializable {
    EnableKeyboard enableKeyboard;
    explicit EnableKeyboardCfg(const EnableKeyboard &enableKeyboard) : enableKeyboard(std::move(enableKeyboard)){};
    bool Unmarshal(cJSON *node) override
    {
        return Serializable::GetValue(node, GET_NAME(enableKeyboardList), enableKeyboard);
    }
};

struct EnableIme : public Serializable {
    int32_t userId;
    std::vector<std::string> imes;
    explicit EnableIme(int32_t userId) : userId(userId){};
    bool Unmarshal(cJSON *node) override
    {
        Serializable::GetValue(node, GET_NAME(userId), imes);
        return true;
    }
};
struct EnableImeCfg : public Serializable {
    EnableIme enableIme;
    explicit EnableImeCfg(const EnableIme &enableIme) : enableIme(std::move(enableIme)){};
    bool Unmarshal(cJSON *node) override
    {
        return Serializable::GetValue(node, GET_NAME(enableImeList), enableIme);
    }
};

class EnableImeDataParser : public RefBase {
public:
    static sptr<EnableImeDataParser> GetInstance();
    int32_t Initialize(const int32_t userId);
    int32_t GetEnableData(const std::string &key, std::vector<std::string> &enableVec, const int32_t userId);
    // for enable list changed
    bool CheckNeedSwitch(const std::string &key, SwitchInfo &switchInfo, const int32_t userId);
    // for switch target ime
    bool CheckNeedSwitch(const SwitchInfo &info, const int32_t userId);
    void OnUserChanged(const int32_t userId);

    static constexpr const char *ENABLE_IME = "settings.inputmethod.enable_ime";
    static constexpr const char *ENABLE_KEYBOARD = "settings.inputmethod.enable_keyboard";

private:
    EnableImeDataParser() = default;
    ~EnableImeDataParser();

    bool ParseEnableIme(const std::string &valueStr, std::vector<std::string> &enableVec, int32_t userId);
    bool ParseEnableKeyboard(const std::string &valueStr, std::vector<std::string> &enableVec, int32_t userId);
    bool CheckTargetEnableName(
        const std::string &key, const std::string &targetName, std::string &nextIme, const int32_t userId);
    std::shared_ptr<Property> GetDefaultIme();

private:
    static std::mutex instanceMutex_;
    static sptr<EnableImeDataParser> instance_;
    std::mutex listMutex_;
    std::unordered_map<std::string, std::vector<std::string>> enableList_;
    std::mutex defaultImeMutex_;
    std::shared_ptr<Property> defaultImeInfo_{ nullptr };
    int32_t currrentUserId_ = 0;
};
} // namespace MiscServices
} // namespace OHOS

#endif // ENABLE_IME_DATA_PARSER_H
