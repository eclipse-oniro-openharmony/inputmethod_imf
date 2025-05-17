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

#include <cinttypes>

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "global.h"
#include "ime_info_inquirer.h"
#include "tokenid_kit.h"
#include "window_adapter.h"

namespace OHOS {
namespace MiscServices {
using namespace Rosen;
using namespace Security::AccessToken;
using namespace OHOS::AAFwk;
bool IdentityCheckerImpl::IsFocused(int64_t callingPid, uint32_t callingTokenId, int64_t focusedPid, bool isAttach)
{
    if (focusedPid != INVALID_PID && callingPid == focusedPid) {
        IMSA_HILOGD("focused app, pid: %{public}" PRId64 "", callingPid);
        return true;
    }
    auto displayId = WindowAdapter::GetDisplayIdByPid(callingPid);
    if (focusedPid == INVALID_PID) {
        FocusChangeInfo focusInfo;
        WindowAdapter::GetFocusInfo(focusInfo, displayId);
        focusedPid = focusInfo.pid_;
        if (callingPid == focusedPid) {
            IMSA_HILOGD("focused app, pid: %{public}" PRId64 ", display: %{public}" PRIu64 "", callingPid, displayId);
            return true;
        }
    }
    if (isAttach && ImeInfoInquirer::GetInstance().IsInputMethodExtension(callingPid)) {
        return false;
    }
    bool isFocused = IsFocusedUIExtension(callingTokenId, displayId);
    if (!isFocused) {
        IMSA_HILOGE("not focused, focusedPid: %{public}" PRId64 ", callerPid: %{public}" PRId64 ", callerToken: "
                    "%{public}d",
            focusedPid, callingPid, callingTokenId);
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

bool IdentityCheckerImpl::IsFormShell(AccessTokenID tokenId)
{
    return AccessTokenKit::GetTokenTypeFlag(tokenId) == TypeATokenTypeEnum::TOKEN_SHELL;
}

bool IdentityCheckerImpl::IsFocusedUIExtension(uint32_t callingTokenId, uint64_t displayId)
{
    bool isFocused = false;
    auto ret = AbilityManagerClient::GetInstance()->CheckUIExtensionIsFocused(callingTokenId, isFocused);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to CheckUIExtensionIsFocused, ret: %{public}d", ret);
        return false;
    }
    IMSA_HILOGD("tokenId: %{public}d, displayId: %{public}" PRIu64 ", isFocused: %{public}d", callingTokenId,
        displayId, isFocused);
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

uint64_t IdentityCheckerImpl::GetDisplayIdByWindowId(int32_t callingWindowId)
{
    return WindowAdapter::GetDisplayIdByWindowId(callingWindowId);
}

uint64_t IdentityCheckerImpl::GetDisplayIdByPid(int64_t callingPid)
{
    return WindowAdapter::GetDisplayIdByPid(callingPid);
}

bool IdentityCheckerImpl::IsValidVirtualIme(int32_t callingUid)
{
    return ImeInfoInquirer::GetInstance().IsVirtualProxyIme(callingUid);
}

bool IdentityCheckerImpl::IsSpecialSaUid()
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    return ImeInfoInquirer::GetInstance().IsSpecialSaUid(callingUid);
}

bool IdentityCheckerImpl::IsDefaultImeScreen(const std::string &screenName)
{
    return ImeInfoInquirer::GetInstance().IsDefaultImeScreen(screenName);
}
} // namespace MiscServices
} // namespace OHOS