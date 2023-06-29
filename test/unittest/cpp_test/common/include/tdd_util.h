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

#ifndef INPUTMETHOD_UNITTEST_UTIL_H
#define INPUTMETHOD_UNITTEST_UTIL_H

#include <memory>
#include <string>

#include "key_event.h"

namespace OHOS {
namespace MiscServices {
class TddUtil {
public:
    static void StorageSelfTokenID();
    static void AllocTestTokenID(const std::string &bundleName);
    static void DeleteTestTokenID();
    static void SetTestTokenID();
    static void RestoreSelfTokenID();
    static void StorageSelfUid();
    static void SetTestUid();
    static void RestoreSelfUid();
    static bool ExecuteCmd(const std::string &cmd, std::string &result);

    static bool CheckCurrentProp(const std::string &bundleName, const std::string &extName);
    static bool CheckCurrentSubProp(const std::string &bundleName, const std::string &extName);
    static bool CheckCurrentSubProps(uint32_t subTypeNum, const std::string &bundleName,
        const std::vector<std::string> extNames, const std::vector<std::string> &languages);

private:
    static int32_t GetCurrentUserId();
    static int64_t GetNanoTime();
    static uint64_t selfTokenID_;
    static uint64_t testTokenID_;
    static int32_t userID_;
    static int64_t selfUid_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_UNITTEST_UTIL_H