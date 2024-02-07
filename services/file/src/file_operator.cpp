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

bool FileOperator::Read(mode_t mode, const std::string &path, std::string &content)
{
    auto fd = open(path.c_str(), mode);
    if (fd <= 0) {
        IMSA_HILOGE("file open failed, fd: %{public}d", fd);
        return false;
    }
    char contentTemp[MAX_FILE_LENGTH] = { 0 };
    auto ret = read(fd, contentTemp, MAX_FILE_LENGTH);
    if (ret <= 0) {
        IMSA_HILOGE("file read failed, ret: %{public}zd", ret);
        close(fd);
        return false;
    }
    close(fd);

    if (contentTemp[0] == '\0') {
        IMSA_HILOGE("content is empty");
        return false;
    }
    content = contentTemp;
    IMSA_HILOGD("content: %{public}s", content.c_str());
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

std::string FileOperator::GetContentFromSysCfgFiles(const std::string &key)
{
    std::string content;
    CfgFiles *cfgFiles = GetSysCfgFiles(IME_INPUT_TYPE_CFG_FILE_PATH);
    if (cfgFiles == nullptr) {
        IMSA_HILOGE("cfgFiles is nullptr");
        return content;
    }
    // parse config files, ordered by priority from high to low
    for (int32_t i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        auto path = cfgFiles->paths[i];
        if (path == nullptr || *path == '\0') {
            continue;
        }
        char realPath[PATH_MAX + 1] = { 0x00 };
        if (strlen(path) == 0 || strlen(path) > PATH_MAX || realpath(path, realPath) == nullptr) {
            IMSA_HILOGE("failed to get realpath");
            continue;
        }
        std::string cfgPath(realPath);
        std::string contentTemp;
        if (!GetContentFromSysCfgFile(cfgPath, key, contentTemp)) {
            continue;
        }
        content = std::move(contentTemp);
    }
    FreeCfgFiles(cfgFiles);
    IMSA_HILOGI("content: %{public}s", content.c_str());
    return content;
}

CfgFiles *FileOperator::GetSysCfgFiles(const std::string &path)
{
    return GetCfgFiles(IME_INPUT_TYPE_CFG_FILE_PATH);
}

bool FileOperator::GetContentFromSysCfgFile(const std::string &path, const std::string &key, std::string &content)
{
    std::string contentTemp;
    bool ret = Read(O_RDONLY, path, contentTemp);
    if (!ret) {
        IMSA_HILOGE("failed");
        return false;
    }
    if (contentTemp.find(key) == std::string::npos) {
        IMSA_HILOGE("not contain %{public}s", key.c_str());
        return false;
    }
    content = std::move(contentTemp);
    return true;
}
} // namespace MiscServices
} // namespace OHOS
