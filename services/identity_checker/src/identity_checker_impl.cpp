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
#include "identity_checker_impl.h"

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "global.h"
#include "tokenid_kit.h"
#include <cinttypes>
#ifdef SCENE_BOARD_ENABLE
#include "window_manager_lite.h"
#else
#include "window_manager.h"
#endif

namespace OHOS {
namespace MiscServices {
using namespace Rosen;
using namespace Security::AccessToken;
bool IdentityCheckerImpl::IsFocused(int64_t callingPid, uint32_t callingTokenId, int64_t focusedPid)
{
    if (focusedPid != INVALID_PID && callingPid == focusedPid) {
        IMSA_HILOGD("focused app, pid: %{public}" PRId64 "", callingPid);
        return true;
    }
    if (focusedPid == INVALID_PID) {
        WMError ret = WMError::WM_OK;
        std::vector<FocusChangeInfo> focusInfos;
#ifdef SCENE_BOARD_ENABLE
        ret = WindowManagerLite::GetInstance().GetAllFocusWindowInfo(focusInfos);
#else
        ret = WindowManager::GetInstance().GetAllFocusWindowInfo(focusInfos);
#endif
        if (ret != WMError::WM_OK) {
            IMSA_HILOGE("GetAllFocusWindowInfo failed, ret: %{public}d", ret);
            return false;
        }
        auto iter = std::find_if(focusInfos.begin(), focusInfos.end(),
            [&callingPid](const auto &focusInfo) { return focusInfo.pid_ == callingPid; });
        if (iter != focusInfos.end()) {
            IMSA_HILOGD("focused app, pid: %{public}" PRId64 "", callingPid);
            return true;
        }
    }
    // zll: 此处遗留，有待确认UIExtension是否支持查找非主设备屏幕的UIExtension窗口
    bool isFocused = IsFocusedUIExtension(callingTokenId);
    if (!isFocused) {
        IMSA_HILOGE("not focused, focusedPid: %{public}" PRId64 ", callerPid: %{public}" PRId64 ", callerToken: "
                    "%{public}d",
            realFocusedPid, callingPid, callingTokenId);
    }
    return isFocused;
}

bool IdentityCheckerImpl::IsSystemApp(uint64_t fullTokenId)
{
    return TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
}

bool IdentityCheckerImpl::IsBundleNameValid(uint32_t tokenId, const std::string &validBundleName)
{
    std::string bundleName = GetBundleNameByToken(tokenId);
    if (bundleName.empty()) {
        return false;
    }
    if (bundleName != validBundleName) {
        IMSA_HILOGE("bundleName is invalid, caller: %{public}s, current: %{public}s", bundleName.c_str(),
            validBundleName.c_str());
        return false;
    }
    IMSA_HILOGD("checked successfully.");
    return true;
}

bool IdentityCheckerImpl::HasPermission(uint32_t tokenId, const std::string &permission)
{
    if (AccessTokenKit::VerifyAccessToken(tokenId, permission) != PERMISSION_GRANTED) {
        IMSA_HILOGE("Permission [%{public}s] not granted!", permission.c_str());
        return false;
    }
    IMSA_HILOGD("verify AccessToken success.");
    return true;
}

bool IdentityCheckerImpl::IsBroker(AccessTokenID tokenId)
{
    if (!IsNativeSa(tokenId)) {
        return false;
    }
    NativeTokenInfo nativeTokenInfoRes;
    AccessTokenKit::GetNativeTokenInfo(tokenId, nativeTokenInfoRes);
    return nativeTokenInfoRes.processName == "broker";
}

bool IdentityCheckerImpl::IsNativeSa(AccessTokenID tokenId)
{
    return AccessTokenKit::GetTokenTypeFlag(tokenId) == TypeATokenTypeEnum::TOKEN_NATIVE;
}

bool IdentityCheckerImpl::IsFocusedUIExtension(uint32_t callingTokenId)
{
    bool isFocused = false;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->CheckUIExtensionIsFocused(callingTokenId, isFocused);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to CheckUIExtensionIsFocused, ret: %{public}d", ret);
        return false;
    }
    IMSA_HILOGD("tokenId: %{public}d, check result: %{public}d, isFocused: %{public}d", callingTokenId, ret, isFocused);
    return isFocused;
}

std::string IdentityCheckerImpl::GetBundleNameByToken(uint32_t tokenId)
{
    auto tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType != TOKEN_HAP) {
        IMSA_HILOGE("invalid token!");
        return "";
    }
    HapTokenInfo info;
    int ret = AccessTokenKit::GetHapTokenInfo(tokenId, info);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to get hap info, ret: %{public}d!", ret);
        return "";
    }
    return info.bundleName;
}

bool IdentityCheckerImpl::IsTargetSa(int32_t callingUid, int32_t validUid)
{
    return callingUid == validUid;
}

uint64_t IdentityCheckerImpl::GetCallingDisplayId(int64_t callingPid)
{
    WindowInfoOption option;
    std::vector<sptr<WindowInfo>> windowInfos;
    WMError ret = WindowManager::GetInstance().ListWindowInfo(option, windowInfos);
    if (ret != WMError::WM_OK) {
        IMSA_HILOGE("ListWindowInfo failed, ret: %{public}d", ret);
        return INVALID_DISPLAY_ID;
    }
    auto iter = std::find_if(windowInfos.begin(), windowInfos.end(), [&callingPid](const auto &windowInfo) {
        if (windowInfo == nullptr) {
            return false;
        }
        return windowInfo->pid == callingPid;
    });
    if (iter == windowInfos.end()) {
        IMSA_HILOGE("not found window info with pid: %{public}" PRId64 "", callingPid);
        return INVALID_DISPLAY_ID;
    }
    auto callingDisplayId = (*iter)->windowDisplayInfo.displayId;
    IMSA_HILOGD("window pid: %{public}" PRId64 ", displayId: %{public} " PRIu64 "", callingPid, callingDisplayId);
    return callingDisplayId;
}
} // namespace MiscServices
} // namespace OHOS