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

#include <string>

#include "bundle_mgr_interface.h"
#include "window.h"
#include "window_option.h"
#include "wm_common.h"

namespace OHOS {
namespace MiscServices {
class TddUtil {
public:
    static void StorageSelfTokenID();
    static uint64_t AllocTestTokenID(bool isSystemApp, bool needPermission, const std::string &bundleName);
    static uint64_t GetTestTokenID(const std::string &bundleName);
    static void DeleteTestTokenID(uint64_t tokenId);
    static void SetTestTokenID(uint64_t tokenId);
    static void RestoreSelfTokenID();
    static bool ExecuteCmd(const std::string &cmd, std::string &result);
    static pid_t GetImsaPid();
    static void KillImsaProcess();
    static void SetFocusWindow();
    static void RestoreFocusWindow();

private:
    static int32_t GetCurrentUserId();
    static sptr<OHOS::AppExecFwk::IBundleMgr> GetBundleMgr();
    static int GetUserIdByBundleName(const std::string &bundleName, const int currentUserId);
    static uint64_t selfTokenID_;
    static uint64_t testTokenID_;
    static int32_t userID_;
    static sptr<Rosen::Window> window_;
    static sptr<Rosen::WindowOption> winOption_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_UNITTEST_UTIL_H