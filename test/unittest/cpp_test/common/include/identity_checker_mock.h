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

#ifndef IMF_TEST_IDENTITY_CHECKER_MOCK_H
#define IMF_TEST_IDENTITY_CHECKER_MOCK_H

#include "identity_checker.h"

namespace OHOS {
namespace MiscServices {
class IdentityCheckerMock : public IdentityChecker {
public:
    IdentityCheckerMock() = default;
    virtual ~IdentityCheckerMock() = default;
    bool IsFocused(int64_t callingPid, uint32_t callingTokenId, int64_t focusedPid = INVALID_PID) override
    {
        return isFocused_;
    }
    bool IsSystemApp(uint64_t fullTokenId) override
    {
        return isSystemApp_;
    }
    bool IsBundleNameValid(uint32_t tokenId, const std::string &validBundleName) override
    {
        return isBundleNameValid_;
    }
    bool HasPermission(uint32_t tokenId, const std::string &permission) override
    {
        return hasPermission_;
    }
    bool IsBroker(Security::AccessToken::AccessTokenID tokenId) override
    {
        return isBroker_;
    }
    bool IsNativeSa(Security::AccessToken::AccessTokenID tokenId) override
    {
        return isNativeSa_;
    }
    std::string GetBundleNameByToken(uint32_t tokenId) override
        {
            return bundleName_;
    }
    uint64_t GetDisplayIdByWindowId(int32_t callingWindowId) override
    {
        return 0;
    }
    bool IsStylusSa() override
    {
        return isStylusSa_;
    }
    static void ResetParam()
    {
        isFocused_ = false;
        isSystemApp_ = false;
        isBundleNameValid_ = false;
        hasPermission_ = false;
        isBroker_ = false;
        isNativeSa_ = false;
        isStylusSa_ = false;
        bundleName_ = "";
    }
    static void SetFocused(bool isFocused)
    {
        isFocused_ = isFocused;
    }
    static void SetSystemApp(bool isSystemApp)
    {
        isSystemApp_ = isSystemApp;
    }
    static void SetBundleNameValid(bool isBundleNameValid)
    {
        isBundleNameValid_ = isBundleNameValid;
    }
    static void SetBroker(bool isBroker)
    {
        isBroker_ = isBroker;
    }
    static void SetNativeSa(bool isNativeSa)
    {
        isNativeSa_ = isNativeSa;
    }
    static void SetPermission(bool hasPermission)
    {
        hasPermission_ = hasPermission;
    }

    static void SetBundleName(const std::string &bundleName)
    {
        bundleName_ = bundleName;
    }
    static void SetStylusSa(bool isStylusSa)
    {
        isStylusSa_ = isStylusSa;
    }

private:
    static bool isFocused_;
    static bool isSystemApp_;
    static bool isBundleNameValid_;
    static bool hasPermission_;
    static bool isBroker_;
    static bool isNativeSa_;
    static bool isStylusSa_;
    static std::string bundleName_;
};
bool IdentityCheckerMock::isFocused_ { false };
bool IdentityCheckerMock::isSystemApp_ { false };
bool IdentityCheckerMock::isBundleNameValid_ { false };
bool IdentityCheckerMock::hasPermission_ { false };
bool IdentityCheckerMock::isBroker_ { false };
bool IdentityCheckerMock::isNativeSa_ { false };
bool IdentityCheckerMock::isStylusSa_ { false };
std::string IdentityCheckerMock::bundleName_;
} // namespace MiscServices
} // namespace OHOS
#endif // IMF_TEST_IDENTITY_CHECKER_MOCK_H
