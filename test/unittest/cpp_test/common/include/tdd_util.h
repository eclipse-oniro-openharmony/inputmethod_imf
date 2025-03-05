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

#include "block_data.h"
#include "bundle_mgr_interface.h"
#include "foundation/window/window_manager/interfaces/innerkits/wm/window.h"
#include "window_manager.h"
#include "window_option.h"
#include "wm_common.h"

namespace OHOS {
namespace MiscServices {
class FocusChangedListenerTestImpl : public Rosen::IFocusChangedListener {
public:
    FocusChangedListenerTestImpl() = default;
    ~FocusChangedListenerTestImpl() = default;
    void OnFocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo) override;
    void OnUnfocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo) override;
    bool getFocus_ = false;
    static std::shared_ptr<BlockData<bool>> isFocused_;
    static std::shared_ptr<BlockData<bool>> unFocused_;
};
class TddUtil {
public:
    static int32_t GetCurrentUserId();
    static void StorageSelfTokenID();
    static uint64_t AllocTestTokenID(
        bool isSystemApp, const std::string &bundleName, const std::vector<std::string> &premission = {});
    static uint64_t GetTestTokenID(const std::string &bundleName);
    static void DeleteTestTokenID(uint64_t tokenId);
    static void SetTestTokenID(uint64_t tokenId);
    static void RestoreSelfTokenID();
    static uint64_t GetCurrentTokenID();
    static int32_t GetUid(const std::string &bundleName);
    static void SetSelfUid(int32_t uid);
    static bool ExecuteCmd(const std::string &cmd, std::string &result);
    static pid_t GetImsaPid();
    static bool KillImsaProcess();
    static void PushEnableImeValue(const std::string &key, const std::string &value);
    static void GrantNativePermission();
    static void DeleteGlobalTable(const std::string &key);
    static void DeleteUserTable(int32_t userId, const std::string &key);
    static void GenerateGlobalTable(const std::string &key, const std::string &content);
    static void GenerateUserTable(int32_t userId, const std::string &key, const std::string &content);
    static int32_t GetEnableData(std::string &value);
    static void InitWindow(bool isShow);
    static void DestroyWindow();
    static bool GetFocused();
    static bool GetUnfocused();
    static void InitCurrentImePermissionInfo();
    static std::string currentBundleNameMock_;
    class WindowManager {
    public:
        static void CreateWindow();
        static void ShowWindow();
        static void HideWindow();
        static void DestroyWindow();
        static void RegisterFocusChangeListener();
        static int32_t currentWindowId_;

    private:
        static sptr<Rosen::Window> window_;
        static uint64_t windowTokenId_;
    };

private:
    static sptr<OHOS::AppExecFwk::IBundleMgr> GetBundleMgr();
    static int GetUserIdByBundleName(const std::string &bundleName, const int currentUserId);
    static void DeleteTable(const std::string &key, const std::string &uriProxy);
    static void GenerateTable(const std::string &key, const std::string &uriProxy, const std::string &content);
    static uint64_t selfTokenID_;
    static uint64_t testTokenID_;
    static int32_t userID_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_UNITTEST_UTIL_H