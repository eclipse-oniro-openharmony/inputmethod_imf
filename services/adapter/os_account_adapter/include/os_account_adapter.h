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

#ifndef SERVICES_INCLUDE_OS_ACCOUNT_ADAPTER_H
#define SERVICES_INCLUDE_OS_ACCOUNT_ADAPTER_H

#include <cstdint>
#include <vector>

namespace OHOS {
namespace MiscServices {
class OsAccountAdapter {
public:
    static constexpr int32_t MAIN_USER_ID = 100;
    static constexpr int32_t INVALID_USER_ID = -1;
    static int32_t GetOsAccountLocalIdFromUid(int32_t uid);
    static int32_t GetForegroundOsAccountLocalId();
    static std::vector<int32_t> QueryActiveOsAccountIds();
    static bool IsOsAccountForeground(int32_t userId);

private:
    static constexpr uint32_t RETRY_INTERVAL = 100;
    static constexpr uint32_t BLOCK_RETRY_TIMES = 10;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_OS_ACCOUNT_ADAPTER_H
