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

#ifndef INPUTMETHOD_IMF_TEST_UNITTEST_COMMON_SCOPE_UTILS_H
#define INPUTMETHOD_IMF_TEST_UNITTEST_COMMON_SCOPE_UTILS_H

#include "global.h"
#include "tdd_util.h"
namespace OHOS {
namespace MiscServices {
constexpr int32_t ROOT_UID = 0;
class AccessScope {
public:
    AccessScope(uint64_t tokenId, int32_t uid)
    {
        IMSA_HILOGI("enter");
        if (tokenId > 0) {
            originalTokenId_ = TddUtil::GetCurrentTokenID();
            TddUtil::SetTestTokenID(tokenId);
        }
        if (uid > 0) {
            TddUtil::SetSelfUid(uid);
        }
    }
    ~AccessScope()
    {
        if (originalTokenId_ > 0) {
            TddUtil::SetTestTokenID(originalTokenId_);
        }
        TddUtil::SetSelfUid(ROOT_UID);
        IMSA_HILOGI("exit");
    }

private:
    uint64_t originalTokenId_{ 0 };
};

class TokenScope {
public:
    explicit TokenScope(uint64_t tokenId)
    {
        IMSA_HILOGI("enter");
        originalTokenId_ = TddUtil::GetCurrentTokenID();
        TddUtil::SetTestTokenID(tokenId);
    }
    ~TokenScope()
    {
        TddUtil::SetTestTokenID(originalTokenId_);
        IMSA_HILOGI("exit");
    }

private:
    uint64_t originalTokenId_{ 0 };
};

class UidScope {
public:
    explicit UidScope(int32_t uid)
    {
        IMSA_HILOGI("enter, uid: %{public}d", uid);
        TddUtil::SetSelfUid(uid);
    }
    ~UidScope()
    {
        TddUtil::SetSelfUid(ROOT_UID);
        IMSA_HILOGI("exit");
    }
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_IMF_TEST_UNITTEST_COMMON_SCOPE_UTILS_H
