/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "sys_cfg_parser.h"
#include "file_operator.h"

namespace OHOS {
namespace MiscServices {
constexpr const char *SYS_CFG_FILE_PATH = "etc/inputmethod/inputmethod_framework_config.json";
bool SysCfgParser::ParseSystemConfig(SystemConfig &systemConfig)
{
    auto content = GetSysCfgContent(GET_NAME(systemConfig));
    if (content.empty()) {
        IMSA_HILOGE("content is empty");
        return false;
    }
    ImeSystemConfig imeSysCfg;
    auto ret = imeSysCfg.Unmarshall(content);
    systemConfig = imeSysCfg.systemConfig;
    return ret;
}

bool SysCfgParser::ParseInputType(std::vector<InputTypeInfo> &inputType)
{
    auto content = GetSysCfgContent(GET_NAME(supportedInputTypeList));
    if (content.empty()) {
        IMSA_HILOGD("content is empty");
        return false;
    }
    InputTypeCfg inputTypeCfg;
    auto ret = inputTypeCfg.Unmarshall(content);
    inputType = inputTypeCfg.inputType;
    return ret;
}

bool SysCfgParser::ParsePanelAdjust(std::vector<SysPanelAdjust> &sysPanelAdjust)
{
    auto content = GetSysCfgContent(GET_NAME(sysPanelAdjust));
    if (content.empty()) {
        IMSA_HILOGE("content is empty");
        return false;
    }
    SysPanelAdjustCfg sysPanelAdjustCfg;
    auto ret = sysPanelAdjustCfg.Unmarshall(content);
    sysPanelAdjust = sysPanelAdjustCfg.panelAdjust;
    return ret;
}

bool SysCfgParser::ParseDefaultFullIme(std::vector<DefaultFullImeInfo> &defaultFullImeList)
{
    auto content = GetSysCfgContent(GET_NAME(defaultFullImeList));
    if (content.empty()) {
        IMSA_HILOGD("content is empty");
        return false;
    }
    DefaultFullImeCfg defaultFullImeCfg;
    auto ret = defaultFullImeCfg.Unmarshall(content);
    defaultFullImeList = defaultFullImeCfg.defaultFullImeList;
    return ret;
}

std::string SysCfgParser::GetSysCfgContent(const std::string &key)
{
    std::string content;
    auto ret = FileOperator::Read(SYS_CFG_FILE_PATH, key, content);
    if (!ret) {
        IMSA_HILOGD("get content by %{public}s failed.", key.c_str());
    }
    return content;
}
} // namespace MiscServices
} // namespace OHOS