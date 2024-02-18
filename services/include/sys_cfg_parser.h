/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef SERVICES_INCLUDE_SYS_CFG_PARSE_H
#define SERVICES_INCLUDE_SYS_CFG_PARSE_H

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "input_method_utils.h"
#include "serializable.h"
namespace OHOS {
namespace MiscServices {
struct SystemConfig : public Serializable {
    std::string systemInputMethodConfigAbility;
    std::string defaultInputMethod;
    bool enableInputMethodFeature = false;
    bool enableFullExperienceFeature = false;
    bool Unmarshal(cJSON *node) override
    {
        Serializable::GetValue(node, GET_NAME(systemInputMethodConfigAbility), systemInputMethodConfigAbility);
        Serializable::GetValue(node, GET_NAME(defaultInputMethod), defaultInputMethod);
        Serializable::GetValue(node, GET_NAME(enableInputMethodFeature), enableInputMethodFeature);
        Serializable::GetValue(node, GET_NAME(enableFullExperienceFeature), enableFullExperienceFeature);
        return true;
    }
};

struct InputTypeInfo : public Serializable {
    InputType type{ InputType::NONE };
    std::string bundleName;
    std::string subName;
    bool Unmarshal(cJSON *node) override
    {
        int32_t typeTemp = -1;
        Serializable::GetValue(node, GET_NAME(inputType), typeTemp);
        type = static_cast<InputType>(typeTemp);
        Serializable::GetValue(node, GET_NAME(bundleName), bundleName);
        Serializable::GetValue(node, GET_NAME(subtypeId), subName);
        return true;
    }
};

struct SysCfg : public Serializable {
    static constexpr const char *SUPPORTED_INPUT_TYPE_LIST = "supportedInputTypeList";
    static constexpr const char *SYSTEM_CONFIG = "systemConfig";
    std::string parseName;
    SystemConfig systemConfig;
    std::vector<InputTypeInfo> inputType;
    explicit SysCfg(std::string parseName) : parseName(std::move(parseName))
    {
    }
    bool Unmarshal(cJSON *node) override
    {
        if (parseName == SUPPORTED_INPUT_TYPE_LIST) {
            return Serializable::GetValue(node, parseName, inputType);
        } else if (parseName == SYSTEM_CONFIG) {
            return Serializable::GetValue(node, parseName, systemConfig);
        } else {
            return false;
        }
    }
};

class SysCfgParser {
public:
    static SysCfgParser &GetInstance();
    bool ParseSysCfg(SysCfg &sysCfg);

private:
    static constexpr const char *SYS_CFG_FILE_PATH = "etc/inputmethod/inputmethod_framework_config.json";
    SysCfgParser() = default;
    ~SysCfgParser() = default;
    bool ParseSysCfg(const std::string &content, SysCfg &sysCfg);
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_SYS_CFG_PARSE_H
