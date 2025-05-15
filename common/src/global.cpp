/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "global.h"

#include <cstdio>
#include <thread>

namespace OHOS {
namespace MiscServices {
bool BlockRetry(uint32_t interval, uint32_t maxRetryTimes, Function func)
{
    IMSA_HILOGD("retry start");
    uint32_t times = 0;
    do {
        times++;
        if (func()) {
            IMSA_HILOGD("success, retry times is: %{public}d", times);
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    } while (times < maxRetryTimes);
    IMSA_HILOGI("retry failed");
    return false;
}
} // namespace MiscServices
} // namespace OHOS