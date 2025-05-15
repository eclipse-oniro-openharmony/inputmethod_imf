/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
namespace OHOS {
namespace MiscServices {
bool OsAccountAdapter::IsOsAccountForeground(int32_t userId)
{
    return true;
}

std::vector<int32_t> OsAccountAdapter::QueryActiveOsAccountIds()
{
    std::vector<int32_t> userIds;
    userIds.push_back(100);
    return userIds;
}

int32_t OsAccountAdapter::GetForegroundOsAccountLocalId()
{
    int32_t userId = MAIN_USER_ID;
    return userId;
}

int32_t OsAccountAdapter::GetOsAccountLocalIdFromUid(int32_t uid)
{
    int32_t userId = INVALID_USER_ID;
    return userId;
}

int32_t OsAccountAdapter::IsOsAccountVerified(int32_t userId, bool &isUnlocked)
{
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS