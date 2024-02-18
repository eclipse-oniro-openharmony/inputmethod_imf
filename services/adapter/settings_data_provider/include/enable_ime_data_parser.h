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

struct EnableCfg : public Serializable {
    struct EnableData : public Serializable {
        int32_t userId{ -1 };
        std::vector<std::string> data;
        bool Unmarshal(cJSON *node) override
        {
            return Serializable::GetValue(node, std::to_string(userId), data);
        }
    };
    EnableData enableData;
    EnableCfg(std::string parseName, int32_t userId) : parseName(std::move(parseName))
    {
        enableData.userId = userId;
    }
    bool Unmarshal(cJSON *node) override
    {
        return Serializable::GetValue(node, parseName, enableData);
    }

private:
    std::string parseName;
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
    const std::map<std::string, std::string> PARSE_NAME{ { ENABLE_IME, GET_NAME(enableImeList) },
        { ENABLE_KEYBOARD, GET_NAME(enableKeyboardList) } };
    EnableImeDataParser() = default;
    ~EnableImeDataParser();

    bool ParseEnableData(const std::string &valueStr, const std::string &parseName, int32_t userId,
        std::vector<std::string> &enableVec);
    std::string GetParseName(const std::string &key);
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
