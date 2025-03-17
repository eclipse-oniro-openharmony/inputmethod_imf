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

#ifndef IMF_SERVICES_INCLUDE_FILE_OPERATOR_H
#define IMF_SERVICES_INCLUDE_FILE_OPERATOR_H

#include <fcntl.h>
#include <string>

#include "config_policy_utils.h"
namespace OHOS {
namespace MiscServices {
class FileOperator {
public:
    static bool Create(const std::string &path, mode_t mode);
    static bool IsExist(const std::string &path);
    static bool Read(const std::string &path, std::string &content);
    static bool Read(const std::string &path, const std::string &key, std::string &content);
    static bool Write(const std::string &path, const std::string &content, uint32_t flags,
        mode_t mode = S_IRUSR | S_IWUSR);
    static std::string GetRealPath(const char *path);

private:
    static std::string Read(const std::string &path, const std::string &key);
    static bool IsValidPath(const std::string &filePath);
    static bool CheckImeCfgFilePath(const std::string &path);
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_SERVICES_INCLUDE_FILE_OPERATOR_H
