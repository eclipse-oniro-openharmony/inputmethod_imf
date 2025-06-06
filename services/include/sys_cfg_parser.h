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

#include "input_method_status.h"
#include "input_method_utils.h"
#include "serializable.h"
namespace OHOS {
namespace MiscServices {
struct SystemConfig : public Serializable {
    std::string systemInputMethodConfigAbility;
    std::string defaultInputMethod;
    std::string systemSpecialInputMethod;
    bool enableInputMethodFeature = false;
    bool enableFullExperienceFeature = false;
    EnabledStatus initEnabledState{ EnabledStatus::DISABLED };
    bool enableAppAgentFeature = false;
    bool enableNumKeyFeature = false;
    std::unordered_set<int32_t> proxyImeUidList;
    std::unordered_set<int32_t> specialSaUidList;
    std::unordered_set<std::string> defaultImeScreenList;
    std::unordered_set<std::string> supportedCapacityList;
    bool Unmarshal(cJSON *node) override
    {
        GetValue(node, GET_NAME(systemInputMethodConfigAbility), systemInputMethodConfigAbility);
        GetValue(node, GET_NAME(defaultInputMethod), defaultInputMethod);
        GetValue(node, GET_NAME(systemSpecialInputMethod), systemSpecialInputMethod);
        GetValue(node, GET_NAME(enableInputMethodFeature), enableInputMethodFeature);
        GetValue(node, GET_NAME(enableFullExperienceFeature), enableFullExperienceFeature);
        auto enableState = static_cast<int32_t>(EnabledStatus::DISABLED);
        GetValue(node, GET_NAME(initEnabledState), enableState);
        initEnabledState = static_cast<EnabledStatus>(enableState);
        GetValue(node, GET_NAME(enableAppAgentFeature), enableAppAgentFeature);
        GetValue(node, GET_NAME(enableNumKeyFeature), enableNumKeyFeature);
        GetValue(node, GET_NAME(proxyImeUidList), proxyImeUidList);
        GetValue(node, GET_NAME(specialSaUidList), specialSaUidList);
        GetValue(node, GET_NAME(defaultImeScreenList), defaultImeScreenList);
        GetValue(node, GET_NAME(supportedCapacityList), supportedCapacityList);
        return true;
    }
};
struct ImeSystemConfig : public Serializable {
    SystemConfig systemConfig;
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, GET_NAME(systemConfig), systemConfig);
    }
};

struct InputTypeInfo : public Serializable {
    InputType type{ InputType::NONE };
    std::string bundleName;
    std::string subName;
    bool Unmarshal(cJSON *node) override
    {
        int32_t typeTemp = -1;
        auto ret = GetValue(node, GET_NAME(inputType), typeTemp);
        if (typeTemp <= static_cast<int32_t>(InputType::NONE) || typeTemp >= static_cast<int32_t>(InputType::END)) {
            return false;
        }
        type = static_cast<InputType>(typeTemp);
        ret = GetValue(node, GET_NAME(bundleName), bundleName) && ret;
        ret = GetValue(node, GET_NAME(subtypeId), subName) && ret;
        return ret;
    }
};
struct InputTypeCfg : public Serializable {
    std::vector<InputTypeInfo> inputType;
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, GET_NAME(supportedInputTypeList), inputType);
    }
};

struct SysPanelAdjust : public Serializable {
    std::vector<std::string> style;
    int32_t top = 0;
    int32_t left = 0;
    int32_t right = 0;
    int32_t bottom = 0;
    bool Unmarshal(cJSON *node) override
    {
        auto ret = GetValue(node, GET_NAME(style), style);
        ret = GetValue(node, GET_NAME(top), top) && ret;
        ret = GetValue(node, GET_NAME(left), left) && ret;
        ret = GetValue(node, GET_NAME(right), right) && ret;
        ret = GetValue(node, GET_NAME(bottom), bottom) && ret;
        return ret;
    }
};

struct SysPanelAdjustCfg : public Serializable {
    std::vector<SysPanelAdjust> panelAdjust;
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, GET_NAME(sysPanelAdjust), panelAdjust);
    }
};

struct DefaultFullImeInfo : public Serializable {
    std::string appId;
    std::string expirationTime;
    uint32_t expirationVersionCode{ 0 };
    bool Unmarshal(cJSON *node) override
    {
        bool ret = GetValue(node, GET_NAME(appIdentifier), appId);
        ret &= GetValue(node, GET_NAME(expirationTime), expirationTime);
        GetValue(node, GET_NAME(expirationVersionCode), expirationVersionCode);
        return ret;
    }
};

struct DefaultFullImeCfg : Serializable {
    std::vector<DefaultFullImeInfo> defaultFullImeList;
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, GET_NAME(defaultFullImeList), defaultFullImeList);
    }
};

struct IgnoreSysPanelAdjust : public Serializable {
    std::vector<int32_t> inputType;
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, GET_NAME(inputType), inputType);
    }
};

struct IgnoreSysPanelAdjustCfg : public Serializable {
    IgnoreSysPanelAdjust ignoreSysPanelAdjust;
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, GET_NAME(ignoreSysPanelAdjust), ignoreSysPanelAdjust);
    }
};

class SysCfgParser {
public:
    static bool ParseSystemConfig(SystemConfig &systemConfig);
    static bool ParseInputType(std::vector<InputTypeInfo> &inputType);
    static bool ParsePanelAdjust(std::vector<SysPanelAdjust> &sysPanelAdjust);
    static bool ParseDefaultFullIme(std::vector<DefaultFullImeInfo> &defaultFullImeList);
    static bool ParseIgnoreSysPanelAdjust(IgnoreSysPanelAdjust &ignoreSysPanelAdjust);

private:
    static std::string GetSysCfgContent(const std::string &key);
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_SYS_CFG_PARSE_H
