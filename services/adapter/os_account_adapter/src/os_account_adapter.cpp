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

#include "os_account_adapter.h"

#include "global.h"
#include "os_account_manager.h"
namespace OHOS {
namespace MiscServices {
using namespace AccountSA;
bool OsAccountAdapter::IsOsAccountForeground(int32_t userId)
{
    bool isForeground = false;
    auto errCode = OsAccountManager::IsOsAccountForeground(userId, isForeground);
    if (errCode != ERR_OK) {
        IMSA_HILOGE("IsOsAccountForeground failed, userId:%{public}d.", userId);
    }
    return isForeground;
}

std::vector<int32_t> OsAccountAdapter::QueryActiveOsAccountIds()
{
    std::vector<int32_t> userIds;
    int errCode = OsAccountManager::QueryActiveOsAccountIds(userIds);
    if (errCode != ERR_OK) {
        IMSA_HILOGE("QueryActiveOsAccountIds failed.");
    }
    return userIds;
}

int32_t OsAccountAdapter::GetForegroundOsAccountLocalId()
{
    int32_t userId = MAIN_USER_ID;
    if (!BlockRetry(RETRY_INTERVAL, BLOCK_RETRY_TIMES,
        [&userId]() -> bool { return OsAccountManager::GetForegroundOsAccountLocalId(userId) == ERR_OK; })) {
        IMSA_HILOGE("get foreground userId failed!");
    }
    return userId;
}

int32_t OsAccountAdapter::GetOsAccountLocalIdFromUid(int32_t uid)
{
    int32_t userId = INVALID_USER_ID;
    auto errCode = OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
    if (errCode != ERR_OK) {
        IMSA_HILOGE("GetOsAccountLocalIdFromUid failed, uid:%{public}d", uid);
    }
    return userId;
}

int32_t OsAccountAdapter::IsOsAccountVerified(int32_t userId, bool &isUnlocked)
{
    auto errCode = OsAccountManager::IsOsAccountVerified(userId, isUnlocked);
    if (errCode != ERR_OK) {
        IMSA_HILOGE("IsOsAccountVerified failed, userId: %{public}d, errCode: %{public}d", userId, errCode);
        return ErrorCode::ERROR_OS_ACCOUNT;
    }
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS