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

#include <map>

#include "file_operator.h"
#include "global.h"
namespace OHOS {
namespace MiscServices {
SysCfgParser &SysCfgParser::GetInstance()
{
    static SysCfgParser instance;
    return instance;
}

bool SysCfgParser::ParseSysCfg(SysCfg &sysCfg)
{
    std::string content;
    auto ret = FileOperator::Read(SYS_CFG_FILE_PATH, sysCfg.parseName, content);
    if (!ret) {
        IMSA_HILOGE("get content by %{public}s failed", sysCfg.parseName.c_str());
        return false;
    }
    return ParseSysCfg(content, sysCfg);
}

bool SysCfgParser::ParseSysCfg(const std::string &content, SysCfg &sysCfg)
{
    return sysCfg.Unmarshall(content);
}
} // namespace MiscServices
} // namespace OHOS
