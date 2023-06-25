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

#include <unistd.h>

#include <cstdint>
#include <string>
#include <vector>

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "bundle_mgr_client_impl.h"
#include "global.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "os_account_manager.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::Security::AccessToken;
using namespace OHOS::AccountSA;
constexpr int32_t INVALID_USER_ID = -1;
constexpr int32_t MAIN_USER_ID = 100;
constexpr const uint16_t EACH_LINE_LENGTH = 500;
uint64_t TddUtil::selfTokenID_ = 0;
uint64_t TddUtil::testTokenID_ = 0;
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
        userIds[0] = MAIN_USER_ID;
    }
    return userIds[0];
}
void TddUtil::StorageSelfTokenID()
{
    selfTokenID_ = GetSelfTokenID();
}

void TddUtil::AllocTestTokenID(const std::string &bundleName)
{
    IMSA_HILOGI("bundleName: %{public}s", bundleName.c_str());
    HapInfoParams infoParams = { .userID = GetCurrentUserId(),
        .bundleName = bundleName,
        .instIndex = 0,
        .appIDDesc = "ohos.inputmethod_test.demo",
        .isSystemApp = true };
    PermissionStateFull permissionState = { .permissionName = "ohos.permission.CONNECT_IME_ABILITY",
        .isGeneral = true,
        .resDeviceID = { "local" },
        .grantStatus = { PermissionState::PERMISSION_GRANTED },
        .grantFlags = { 1 } };
    HapPolicyParams policyParams = {
        .apl = APL_NORMAL, .domain = "test.domain.inputmethod", .permList = {}, .permStateList = { permissionState }
    };

    auto tokenInfo = AccessTokenKit::AllocHapToken(infoParams, policyParams);
    testTokenID_ = tokenInfo.tokenIDEx;
}

void TddUtil::DeleteTestTokenID()
{
    AccessTokenKit::DeleteToken(testTokenID_);
}

void TddUtil::SetTestTokenID()
{
    auto ret = SetSelfTokenID(testTokenID_);
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
    auto bundleName = AAFwk::AbilityManagerClient::GetInstance()->GetTopAbility().GetBundleName();

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGE("systemAbilityManager is nullptr");
        return;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        IMSA_HILOGE("remoteObject is nullptr");
        return;
    }
    sptr<AppExecFwk::IBundleMgr> iBundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (iBundleMgr == nullptr) {
        IMSA_HILOGE("iBundleMgr is nullptr");
        return;
    }
    auto uid = iBundleMgr->GetUidByBundleName(bundleName, GetCurrentUserId());
    IMSA_HILOGI("uid: %{public}d", uid);
    setuid(uid);
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
} // namespace MiscServices
} // namespace OHOS
