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

#include "access_util.h"

#include <unistd.h>

#include <csignal>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "accesstoken_kit.h"
#include "global.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "os_account_manager.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "tdd_util.h"
#include "token_setproc.h"
namespace OHOS {
namespace MiscServices {
const std::string PERMISSION_GET_BUNDLE_INFO = "ohos.permission.GET_BUNDLE_INFO";
uint64_t AccessUtil::selfTokenID_ = 0;
uint64_t AccessUtil::GetSelfToken()
{
    return GetSelfTokenID();
}

void AccessUtil::SetSelfToken(uint64_t tokenId)
{
    auto ret = SetSelfTokenID(tokenId);
    IMSA_HILOGI("SetSelfTokenID ret: %{public}d", ret);
}

void AccessUtil::StorageSelfToken()
{
    selfTokenID_ = GetSelfTokenID();
}

void AccessUtil::RestoreSelfToken()
{
    auto ret = SetSelfTokenID(selfTokenID_);
    IMSA_HILOGI("SetSelfTokenID ret = %{public}d", ret);
}

uint64_t AccessUtil::GetTokenID(const std::string &bundleName)
{
    IMSA_HILOGI("bundleName: %{public}s", bundleName.c_str());
    HapInfoParams infoParams = { .userID = GetUserIdByBundleName(bundleName, TddUtil::GetCurrentUserId()),
        .bundleName = bundleName,
        .instIndex = 0,
        .appIDDesc = "ohos.inputmethod_test.demo" };
    return AccessTokenKit::GetHapTokenID(infoParams.userID, infoParams.bundleName, infoParams.instIndex);
}

void AccessUtil::DeleteTokenID(uint64_t tokenId)
{
    AccessTokenKit::DeleteToken(tokenId);
}

uint64_t AccessUtil::AllocTestTokenID(
    bool isSystemApp, const std::string &bundleName, const std::vector<std::string> &premission)
{
    IMSA_HILOGI("bundleName: %{public}s", bundleName.c_str());
    HapInfoParams infoParams = { .userID = TddUtil::GetCurrentUserId(),
        .bundleName = bundleName,
        .instIndex = 0,
        .appIDDesc = bundleName,
        .isSystemApp = isSystemApp };
    std::vector<PermissionStateFull> permStateList;
    for (const auto &prem : premission) {
        PermissionStateFull permissionState = { .permissionName = prem,
            .isGeneral = true,
            .resDeviceID = { "local" },
            .grantStatus = { PermissionState::PERMISSION_GRANTED },
            .grantFlags = { 1 } };
        permStateList.push_back(permissionState);
    }
    HapPolicyParams policyParams = {
        .apl = APL_NORMAL, .domain = bundleName, .permList = {}, .permStateList = permStateList
    };
    if (premission.empty()) {
        policyParams = { .apl = APL_NORMAL, .domain = bundleName, .permList = {}, .permStateList = {} };
    }
    auto tokenInfo = AccessTokenKit::AllocHapToken(infoParams, policyParams);
    return tokenInfo.tokenIDEx;
}

int32_t AccessUtil::GetUid(const std::string &bundleName)
{
    auto currentToken = GetSelfTokenID();
    GrantNativePermission(PERMISSION_GET_BUNDLE_INFO);
    auto bundleMgr = GetBundleMgr();
    if (bundleMgr == nullptr) {
        IMSA_HILOGE("bundleMgr nullptr");
        return -1;
    }
    BundleInfo bundleInfo;
    bool result =
        bundleMgr->GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, TddUtil::GetCurrentUserId());
    if (!result) {
        IMSA_HILOGE("failed to get bundle info");
        return -1;
    }
    IMSA_HILOGI("bundleName: %{public}s, uid: %{public}d", bundleName.c_str(), bundleInfo.uid);
    SetTestTokenID(currentToken);
    return bundleInfo.uid;
}

void AccessUtil::SetSelfUid(int32_t uid)
{
    setuid(uid);
    IMSA_HILOGI("set uid to: %{public}d", uid);
}

void AccessUtil::GrantNativePermissions(const std::vector<std::string> &permissions)
{
    if (permissions.empty()) {
        return;
    }
    uint32_t size = permissions.size();
    const char **perms = new const char *[size];
    for (uint32_t i = 0; i < size; i++) {
        perms[i] = permissions[i].c_str();
    }
    TokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "imf_test",
        .aplStr = "system_core",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    int res = SetSelfTokenID(tokenId);
    if (res == 0) {
        IMSA_HILOGI("SetSelfTokenID success");
    } else {
        IMSA_HILOGE("SetSelfTokenID fail");
    }
    AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

void AccessUtil::GrantNativePermission(const std::string &permission)
{
    const char **perms = new const char *[1];
    perms[0] = permission.c_str();
    TokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "imf_test",
        .aplStr = "system_core",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    int res = SetSelfTokenID(tokenId);
    if (res == 0) {
        IMSA_HILOGI("SetSelfTokenID success");
    } else {
        IMSA_HILOGE("SetSelfTokenID fail");
    }
    AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

sptr<AppExecFwk::IBundleMgr> AccessUtil::GetBundleMgr()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGE("systemAbilityManager is nullptr");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        IMSA_HILOGE("remoteObject is nullptr");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
}

int32_t AccessUtil::GetUserIdByBundleName(const std::string &bundleName, const int currentUserId)
{
    auto bundleMgr = GetBundleMgr();
    if (bundleMgr == nullptr) {
        IMSA_HILOGE("Get bundleMgr failed");
        return -1;
    }
    auto uid = bundleMgr->GetUidByBundleName(bundleName, currentUserId);
    if (uid == -1) {
        IMSA_HILOGE("failed to get information and the parameters may be wrong");
        return -1;
    }
    // 200000 means userId = uid / 200000.
    return uid / 200000;
}

} // namespace MiscServices
} // namespace OHOS