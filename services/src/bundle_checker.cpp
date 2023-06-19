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
#include "bundle_checker.h"

#include <cinttypes>

#include "accesstoken_kit.h"
#include "global.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace MiscServices {
using namespace Security::AccessToken;
int64_t BundleChecker::focusedUid_ = -1;
bool BundleChecker::IsFocused(int64_t uid)
{
    IMSA_HILOGI("focusedUid_:%{public}" PRId64 ", uid:%{public}" PRId64 "", focusedUid_, uid);
    return uid == focusedUid_;
}

void BundleChecker::SetFocused(int64_t uid)
{
    IMSA_HILOGD("uid:%{public}" PRId64 "", uid);
    focusedUid_ = uid;
}

bool BundleChecker::IsSystemApp(uint64_t fullTokenID)
{
    return TokenIdKit::IsSystemAppByFullTokenID(fullTokenID);
}

bool BundleChecker::IsCurrentIme(uint32_t tokenID, const std::string &currentIme)
{
    std::string bundleName = GetBundleNameByToken(tokenID);
    if (bundleName.empty()) {
        return false;
    }
    if (bundleName != currentIme) {
        IMSA_HILOGE(
            "not current ime, caller: %{public}s, current ime: %{public}s", bundleName.c_str(), currentIme.c_str());
        return false;
    }
    IMSA_HILOGD("checked ime successfully");
    return true;
}

bool BundleChecker::CheckPermission(uint32_t tokenID, const std::string &permission)
{
    if (AccessTokenKit::VerifyAccessToken(tokenID, permission) != PERMISSION_GRANTED) {
        IMSA_HILOGE("Permission [%{public}s] not granted", permission.c_str());
        return false;
    }
    IMSA_HILOGD("verify AccessToken success");
    return true;
}

std::string BundleChecker::GetBundleNameByToken(uint32_t tokenID)
{
    auto tokenType = AccessTokenKit::GetTokenTypeFlag(tokenID);
    if (tokenType != TOKEN_HAP) {
        IMSA_HILOGE("invalid token");
        return "";
    }
    HapTokenInfo info;
    int ret = AccessTokenKit::GetHapTokenInfo(tokenID, info);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to get hap info, ret: %{public}d", ret);
        return "";
    }
    return info.bundleName;
}
} // namespace MiscServices
} // namespace OHOS
