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

#include "input_method_config_parser.h"

#include <algorithm>
#include <cstdio>
#include <fcntl.h>
#include <ios>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>

#include "file_ex.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {
namespace {
    constexpr const char *IME_INPUT_TYPE_CFG_FILE_PATH = "etc/inputmethod/inputmethod_framework_config.json";
    using json = nlohmann::json;
} // namespace

CfgFiles* ImeConfigParse::ParseFromCustom()
{
    return GetCfgFiles(IME_INPUT_TYPE_CFG_FILE_PATH);
}

bool ImeConfigParse::parseJson(const std::string &cfgPath,  const std::string &parseKey, nlohmann::json &jsonCfg)
{
    IMSA_HILOGD("in");
    std::string jsonStr = ImeConfigParse::ReadFile(cfgPath);
    if (jsonStr.empty()) {
        IMSA_HILOGE("json config size is invalid");
        return false;
    }
    jsonCfg = json::parse(jsonStr, nullptr, false);
    if (jsonCfg.is_discarded()) {
        IMSA_HILOGE("jsonStr parse failed");
        return false;
    }
    if (!jsonCfg.contains(parseKey)) {
        IMSA_HILOGE("%{public}s not find or abnormal", parseKey.c_str());
        return false;
    }
    IMSA_HILOGD("get json: %{public}s", jsonCfg.dump().c_str());
    return true;
}

std::string ImeConfigParse::ReadFile(const std::string &path)
{
    std::ifstream infile;
    std::string sLine;
    std::string sAll = "";
    infile.open(path);
    if (!infile.is_open()) {
        IMSA_HILOGE("path: %s Readfile fail", path.c_str());
        return sAll;
    }

    while (getline(infile, sLine)) {
        sAll.append(sLine);
    }
    infile.close();
    return sAll;
}
} // namespace MiscServices
} // namespace OHOS