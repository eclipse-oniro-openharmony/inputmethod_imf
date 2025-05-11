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

#ifndef SERVICES_INCLUDE_IDENTITY_CHECKER_IMPL_H
#define SERVICES_INCLUDE_IDENTITY_CHECKER_IMPL_H

#include "identity_checker.h"

namespace OHOS {
namespace MiscServices {
class IdentityCheckerImpl : public IdentityChecker {
public:
    bool IsFocused(
        int64_t callingPid, uint32_t callingTokenId, int64_t focusedPid = INVALID_PID, bool isAttach = false) override;
    bool IsSystemApp(uint64_t fullTokenId) override;
    bool IsBundleNameValid(uint32_t tokenId, const std::string &validBundleName) override;
    bool HasPermission(uint32_t tokenId, const std::string &permission) override;
    bool IsBroker(Security::AccessToken::AccessTokenID tokenId) override;
    bool IsNativeSa(Security::AccessToken::AccessTokenID tokenId) override;
    std::string GetBundleNameByToken(uint32_t tokenId) override;
    bool IsFocusedUIExtension(uint32_t callingTokenId, uint64_t displayId = DEFAULT_DISPLAY_ID) override;
    uint64_t GetDisplayIdByWindowId(int32_t callingWindowId) override;
    uint64_t GetDisplayIdByPid(int64_t callingPid) override;
    bool IsValidVirtualIme(int32_t callingUid) override;
    bool IsSpecialSaUid() override;
    bool IsDefaultImeScreen(const std::string &screenName) override;
};
} // namespace MiscServices
} // namespace OHOS

#endif // SERVICES_INCLUDE_IDENTITY_CHECKER_IMPL_H
