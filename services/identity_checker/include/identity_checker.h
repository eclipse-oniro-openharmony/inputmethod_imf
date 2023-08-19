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

#ifndef SERVICES_INCLUDE_IDENTITY_CHECKER_H
#define SERVICES_INCLUDE_IDENTITY_CHECKER_H

#include <string>

#include "access_token.h"
namespace OHOS {
namespace MiscServices {
class IdentityChecker {
public:
    static constexpr int64_t INVALID_PID = -1;
    virtual ~IdentityChecker() = default;
    virtual bool IsFocused(int64_t callingPid, uint32_t callingTokenId, int64_t focusedPid = INVALID_PID) = 0;
    virtual bool IsSystemApp(uint64_t fullTokenId) = 0;
    virtual bool IsCurrentIme(uint32_t tokenId, const std::string &currentBundleName) = 0;
    virtual bool HasPermission(uint32_t tokenId, const std::string &permission) = 0;
    virtual bool IsBroker(Security::AccessToken::AccessTokenID tokenId) = 0;
};
} // namespace MiscServices
} // namespace OHOS

#endif // SERVICES_INCLUDE_IDENTITY_CHECKER_H
