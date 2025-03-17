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

#include "file_operator.h"

#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

#include "global.h"
namespace OHOS {
namespace MiscServices {
constexpr int32_t SUCCESS = 0;
bool FileOperator::Create(const std::string &path, mode_t mode)
{
    auto ret = mkdir(path.c_str(), mode);
    if (ret != SUCCESS) {
        IMSA_HILOGE("%{public}s mkdir failed, errno:%{public}d!", path.c_str(), errno);
        return false;
    }
    return true;
}

bool FileOperator::IsExist(const std::string &path)
{
    return access(path.c_str(), F_OK) == SUCCESS;
}

bool FileOperator::Read(const std::string &path, std::string &content)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        IMSA_HILOGE("%{public}s open fail!", path.c_str());
        return false;
    }
    std::string sLine;
    while (getline(file, sLine)) {
        content.append(sLine);
    }
    return true;
}

bool FileOperator::IsValidPath(const std::string &filePath)
{
    if (filePath.find("../") != std::string::npos) {
        IMSA_HILOGE("FilePath contains '../'");
        return false;
    }
    if (filePath.find("./") != std::string::npos) {
        IMSA_HILOGE("FilePath contains './'");
        return false;
    }
    if (filePath[0] != '/') {
        IMSA_HILOGE("FilePath is not an absolute path");
        return false;
    }
    return true;
}

bool FileOperator::CheckImeCfgFilePath(const std::string &path)
{
    if (path.empty()) {
        IMSA_HILOGE("Path is empty");
        return false;
    }
    char realPath[PATH_MAX] = { 0 };
    if (realpath(path.c_str(), realPath) == nullptr) {
        IMSA_HILOGE("Path is error");
        return false;
    }
    if (!IsValidPath(realPath)) {
        IMSA_HILOGE("Path is IsValidPath");
        return false;
    }
    return true;
}

bool FileOperator::Write(const std::string &path, const std::string &content, uint32_t flags, mode_t mode)
{
    const char* fopenMode;
    if (flags & O_TRUNC) {
        fopenMode = "w";
    } else if (flags & O_APPEND) {
        fopenMode = "a";
    } else if (flags & O_CREAT) {
        fopenMode = "w";
    } else {
        fopenMode = "r+";
    }

    if (!CheckImeCfgFilePath(path)) {
        IMSA_HILOGE("path check fail");
        return false;
    }

    FILE* file = fopen(path.c_str(), fopenMode);
    if (file == nullptr) {
        IMSA_HILOGE("%{public}s open fail, errno: %{public}d", path.c_str(), errno);
        return false;
    }
    auto ret = fwrite(content.c_str(), 1, content.size(), file);
    if (ret != content.size()) {
        IMSA_HILOGE("%{public}s write fail, ret: %{public}zd, errno: %{public}d", path.c_str(), ret, errno);
        if (fclose(file) != 0) {
            IMSA_HILOGE("%{public}s close fail, errno: %{public}d", path.c_str(), errno);
        }
        return false;
    }
    if (fclose(file) != 0) {
        IMSA_HILOGE("%{public}s close fail, errno: %{public}d", path.c_str(), errno);
    }

    return true;
}

bool FileOperator::Read(const std::string &path, const std::string &key, std::string &content)
{
    if (key.empty()) {
        IMSA_HILOGE("key is empty!");
        return false;
    }
    CfgFiles *cfgFiles = GetCfgFiles(path.c_str());
    if (cfgFiles == nullptr) {
        IMSA_HILOGE("%{public}s cfgFiles is nullptr!", path.c_str());
        return false;
    }
    // parse config files, ordered by priority from high to low
    for (int32_t i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        auto realPath = GetRealPath(cfgFiles->paths[i]);
        if (realPath.empty()) {
            continue;
        }
        content = Read(realPath, key);
        if (!content.empty()) {
            break;
        }
    }
    FreeCfgFiles(cfgFiles);
    return !content.empty();
}

std::string FileOperator::Read(const std::string &path, const std::string &key)
{
    std::string content;
    bool ret = Read(path, content);
    if (!ret) {
        IMSA_HILOGE("%{public}s read failed!", path.c_str());
        return "";
    }
    if (content.find(key) == std::string::npos) {
        IMSA_HILOGD("%{public}s not contain %{public}s!", path.c_str(), key.c_str());
        return "";
    }
    return content;
}

std::string FileOperator::GetRealPath(const char *path)
{
    if (path == nullptr) {
        return "";
    }
    auto size = strnlen(path, PATH_MAX);
    if (size == 0 || size == PATH_MAX) {
        return "";
    }
    char realPath[PATH_MAX] = { 0x00 };
    if (realpath(path, realPath) == nullptr) {
        IMSA_HILOGE("failed to get realpath!");
        return "";
    }
    return std::string(realPath);
}
} // namespace MiscServices
} // namespace OHOS