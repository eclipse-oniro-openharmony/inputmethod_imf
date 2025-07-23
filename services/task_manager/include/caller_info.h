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

#ifndef IMF_CALLER_INFO_H
#define IMF_CALLER_INFO_H

#include <cinttypes>
#include <string>

namespace OHOS {
namespace MiscServices {
static constexpr int32_t MAIN_USER_ID = 100;
struct CallerInfo {
    uint32_t requestId = 0;
    int32_t pid = 0;
    int32_t uid = 0;
    int32_t userId{ MAIN_USER_ID };
    uint32_t tokenId = 0;
    uint64_t fullTokenId = 0;
    std::string bundleName;
};
} // namespace MiscServices
} // namespace OHOS
#endif // IMF_CALLER_INFO_H
