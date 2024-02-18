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

#include <sys/stat.h>
#include <unistd.h>

#include <fstream>

#include "global.h"
namespace OHOS {
namespace MiscServices {
int32_t FileOperator::Create(mode_t mode, const std::string &path)
{
    return mkdir(path.c_str(), mode);
}

bool FileOperator::IsExist(const std::string &path)
{
    return access(path.c_str(), F_OK) == 0;
}

bool FileOperator::Read(const std::string &path, std::string &content)
{
    std::ifstream infile;
    std::string sLine;
    infile.open(path);
    if (!infile.is_open()) {
        IMSA_HILOGE("path: %s Readfile fail", path.c_str());
        return false;
    }
    while (getline(infile, sLine)) {
        content.append(sLine);
    }
    infile.close();
    return true;
}

bool FileOperator::Write(int32_t flags, const std::string &path, const std::string &content, mode_t mode)
{
    IMSA_HILOGD("content: %{public}s", content.c_str());
    auto fd = open(path.c_str(), flags, mode);
    if (fd <= 0) {
        IMSA_HILOGE("file open failed, fd: %{public}d", fd);
        return false;
    }
    auto ret = write(fd, content.c_str(), content.size());
    if (ret <= 0) {
        IMSA_HILOGE("file write failed, ret: %{public}zd", ret);
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

bool FileOperator::Read(const std::string &path, const std::string &key, std::string &content)
{
    if (key.empty()) {
        IMSA_HILOGE("key is empty");
        return false;
    }
    CfgFiles *cfgFiles = GetCfgFiles(path.c_str());
    if (cfgFiles == nullptr) {
        IMSA_HILOGE("cfgFiles is nullptr");
        return false;
    }
    // parse config files, ordered by priority from high to low
    for (int32_t i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        auto pathTemp = cfgFiles->paths[i];
        if (pathTemp == nullptr || *pathTemp == '\0') {
            continue;
        }
        char realPath[PATH_MAX + 1] = { 0x00 };
        if (strlen(pathTemp) == 0 || strlen(pathTemp) > PATH_MAX || realpath(pathTemp, realPath) == nullptr) {
            IMSA_HILOGE("failed to get realpath");
            continue;
        }
        std::string cfgPath(realPath);
        auto contentTemp = Read(cfgPath, key);
        if (!contentTemp.empty()) {
            content = std::move(contentTemp);
            break;
        }
    }
    FreeCfgFiles(cfgFiles);
    if (content.empty()) {
        return false;
    }
    IMSA_HILOGI("content: %{public}s", content.c_str());
    return true;
}

std::string FileOperator::Read(const std::string &path, const std::string &key)
{
    std::string content;
    bool ret = Read(path, content);
    if (!ret) {
        IMSA_HILOGE("failed");
        return "";
    }
    if (content.find(key) == std::string::npos) {
        IMSA_HILOGE("not contain %{public}s", key.c_str());
        return "";
    }
    return content;
}
} // namespace MiscServices
} // namespace OHOS
