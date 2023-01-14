/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "imf_file_manager.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <string>

#include "global.h"

namespace OHOS {
namespace MiscServices {
using json = nlohmann::json;
FileManager &FileManager::GetInstance()
{
    static FileManager instance;
    return instance;
}

int32_t FileManager::CreateCacheFile(FileInfo &info)
{
    if (!isCachePathExit(info.path)) {
        IMSA_HILOGI("FileManager::dir = %{public}s", info.path.c_str());
        auto errCode = mkdir(info.path.c_str(), info.pathMode); //需要增加selinux权限才能创建成功
        if (errCode != SUCCESS) {
            IMSA_HILOGE("FileManager::CreateDirFailed");
            return errCode;
        }
    }

    std::string fileName = info.path + "/" + info.fileName;
    IMSA_HILOGI("FileManager::fileName = %{public}s", fileName.c_str());
    if (isCachePathExit(fileName)) {
        IMSA_HILOGI("FileManager::file: %{public}s exist", info.fileName.c_str());
        return SUCCESS;
    }
    return creat(fileName.c_str(), info.fileMode);
}

bool FileManager::isCachePathExit(std::string &path)
{
    return access(path.c_str(), F_OK) == SUCCESS;
}

bool FileManager::ReadJsonFile(const std::string &path, json &jsonCfg)
{
    std::ifstream jsonFs(path);
    if (!jsonFs.is_open()) {
        IMSA_HILOGE("ImeCfgManager:file read open failed");
        return false;
    }
    jsonCfg = json::parse(jsonFs, nullptr, false);
    if (jsonCfg.is_null() || jsonCfg.is_discarded()) {
        IMSA_HILOGE("parse failed");
        return false;
    }
    IMSA_HILOGI("ImeCfgManager::json %{public}s", jsonCfg.dump().c_str());
    return true;
}
bool FileManager::WriteJsonFile(const std::string &path, const nlohmann::json &jsonCfg)
{
    std::ofstream jsonFs(path);
    if (!jsonFs.is_open()) {
        IMSA_HILOGE("ImeCfgManager:file write open failed");
        return false;
    }
    constexpr int32_t weight = 2;
    jsonFs << std::setw(weight) << jsonCfg << std::endl;
    return true;
}
} // namespace MiscServices
} // namespace OHOS
