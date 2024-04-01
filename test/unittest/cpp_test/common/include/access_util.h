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

#ifndef INPUTMETHOD_IMF_ACCESS_UTIL_H
#define INPUTMETHOD_IMF_ACCESS_UTIL_H

#include <string>

#include "bundle_mgr_interface.h"
namespace OHOS {
namespace MiscServices {
class AccessUtil {
public:
    static uint64_t GetSelfToken();
    static void SetSelfToken(uint64_t tokenId);
    static void StorageSelfToken();
    static void RestoreSelfToken();
    static uint64_t GetTokenID(const std::string &bundleName);
    static void DeleteTokenID(uint64_t tokenId);
    static uint64_t AllocTestTokenID(
        bool isSystemApp, const std::string &bundleName, const std::vector<std::string> &premission = {});
    static int32_t GetUid(const std::string &bundleName);
    static void SetSelfUid(int32_t uid);
    static void GrantNativePermissions(const std::vector<std::string> &permissions);
    static void GrantNativePermission(const std::string &permission);

private:
    static sptr<AppExecFwk::IBundleMgr> GetBundleMgr();
    static int32_t GetUserIdByBundleName(const std::string &bundleName, int32_t currentUserId);
    static uint64_t selfTokenID_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_IMF_ACCESS_UTIL_H
