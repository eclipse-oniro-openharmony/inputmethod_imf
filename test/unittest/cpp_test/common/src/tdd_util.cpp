/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "tdd_util.h"

#include <csignal>
#include <cstdint>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "accesstoken_kit.h"
#include "global.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "os_account_manager.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "window_manager.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::Security::AccessToken;
using namespace OHOS::AccountSA;
using namespace Rosen;
constexpr int32_t INVALID_USER_ID = -1;
constexpr int32_t MAIN_USER_ID = 100;
constexpr const uint16_t EACH_LINE_LENGTH = 500;
constexpr int32_t BUFF_LENGTH = 10;
constexpr const char *CMD_PIDOF_IMS = "pidof inputmethod_ser";
uint64_t TddUtil::selfTokenID_ = 0;
int64_t TddUtil::selfUid_ = -1;
int32_t TddUtil::userID_ = INVALID_USER_ID;
int32_t TddUtil::GetCurrentUserId()
{
    if (userID_ != INVALID_USER_ID) {
        return userID_;
    }
    std::vector<int32_t> userIds;
    auto ret = OsAccountManager::QueryActiveOsAccountIds(userIds);
    if (ret != ErrorCode::NO_ERROR || userIds.empty()) {
        IMSA_HILOGE("query active os account id failed");
        return MAIN_USER_ID;
    }
    return userIds[0];
}
void TddUtil::StorageSelfTokenID()
{
    selfTokenID_ = GetSelfTokenID();
}

uint64_t TddUtil::AllocTestTokenID(bool isSystemApp, bool needPermission, const std::string &bundleName)
{
    IMSA_HILOGI("bundleName: %{public}s", bundleName.c_str());
    HapInfoParams infoParams = { .userID = GetCurrentUserId(),
        .bundleName = bundleName,
        .instIndex = 0,
        .appIDDesc = bundleName,
        .isSystemApp = isSystemApp };
    PermissionStateFull permissionState = { .permissionName = "ohos.permission.CONNECT_IME_ABILITY",
        .isGeneral = true,
        .resDeviceID = { "local" },
        .grantStatus = { PermissionState::PERMISSION_GRANTED },
        .grantFlags = { 1 } };
    HapPolicyParams policyParams = { .apl = APL_NORMAL,
        .domain = bundleName,
        .permList = {},
        .permStateList = { permissionState } };
    if (!needPermission) {
        policyParams = { .apl = APL_NORMAL, .domain = bundleName, .permList = {}, .permStateList = {} };
    }
    auto tokenInfo = AccessTokenKit::AllocHapToken(infoParams, policyParams);
    return tokenInfo.tokenIDEx;
}

uint64_t TddUtil::GetTestTokenID(const std::string &bundleName)
{
    HapInfoParams infoParams = { .userID = GetUserIdByBundleName(bundleName, GetCurrentUserId()),
        .bundleName = bundleName,
        .instIndex = 0,
        .appIDDesc = "ohos.inputmethod_test.demo" };
    return AccessTokenKit::GetHapTokenID(infoParams.userID, infoParams.bundleName, infoParams.instIndex);
}

void TddUtil::DeleteTestTokenID(uint64_t tokenId)
{
    AccessTokenKit::DeleteToken(tokenId);
}

void TddUtil::SetTestTokenID(uint64_t tokenId)
{
    auto ret = SetSelfTokenID(tokenId);
    IMSA_HILOGI("SetSelfTokenID ret: %{public}d", ret);
}

void TddUtil::RestoreSelfTokenID()
{
    auto ret = SetSelfTokenID(selfTokenID_);
    IMSA_HILOGI("SetSelfTokenID ret = %{public}d", ret);
}

void TddUtil::StorageSelfUid()
{
    selfUid_ = getuid();
}

void TddUtil::SetTestUid()
{
    FocusChangeInfo info;
    WindowManager::GetInstance().GetFocusWindowInfo(info);
    IMSA_HILOGI("uid: %{public}d", info.uid_);
    setuid(info.uid_);
}
void TddUtil::RestoreSelfUid()
{
    setuid(selfUid_);
}

bool TddUtil::ExecuteCmd(const std::string &cmd, std::string &result)
{
    char buff[EACH_LINE_LENGTH] = { 0x00 };
    std::stringstream output;
    FILE *ptr = popen(cmd.c_str(), "r");
    if (ptr != nullptr) {
        while (fgets(buff, sizeof(buff), ptr) != nullptr) {
            output << buff;
        }
        pclose(ptr);
        ptr = nullptr;
    } else {
        return false;
    }
    result = output.str();
    return true;
}

pid_t TddUtil::GetImsaPid()
{
    char buff[BUFF_LENGTH] = { 0 };
    FILE *fp = popen(CMD_PIDOF_IMS, "r");
    if (fp == nullptr) {
        IMSA_HILOGI("get pid failed.");
        return -1;
    }
    fgets(buff, sizeof(buff), fp);
    pid_t pid = atoi(buff);
    pclose(fp);
    fp = nullptr;
    return pid;
}

void TddUtil::KillImsaProcess()
{
    pid_t pid = GetImsaPid();
    if (pid == -1) {
        IMSA_HILOGE("Pid of Imsa is not exist.");
        return;
    }
    auto ret = kill(pid, SIGTERM);
    if (ret != 0) {
        IMSA_HILOGE("Kill failed, ret: %{public}d", ret);
        return;
    }
    IMSA_HILOGI("Kill success.");
}

sptr<OHOS::AppExecFwk::IBundleMgr> TddUtil::GetBundleMgr()
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

int TddUtil::GetUserIdByBundleName(const std::string &bundleName, const int currentUserId)
{
    auto uid = TddUtil::GetBundleMgr()->GetUidByBundleName(bundleName, currentUserId);
    if (uid == -1) {
        IMSA_HILOGE("failed to get information and the parameters may be wrong.");
        return -1;
    }
    // 200000 means userId = uid / 200000.
    return uid/200000;
}
} // namespace MiscServices
} // namespace OHOS
