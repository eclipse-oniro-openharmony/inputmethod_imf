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

#ifndef INPUTMETHOD_IMF_BIT_STATE_MANAGER_H
#define INPUTMETHOD_IMF_BIT_STATE_MANAGER_H

#include <cstdint>

#include "global.h"

namespace OHOS {
namespace MiscServices {
class BitStateManager {
public:
    explicit BitStateManager(uint32_t state) : state_(state)
    {
    }
    ~BitStateManager() = default;
    void UpdateState(uint32_t mask, bool set)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (set) {
            state_ |= mask;
        } else {
            state_ &= ~mask;
        }
    }
    bool IsMatch(uint32_t mask)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if ((state_ & mask) == mask) {
            IMSA_HILOGD("match");
            return true;
        } else {
            IMSA_HILOGE("mismatch, state: %{public}d", state_);
            return false;
        }
    }

private:
    std::mutex mutex_{};
    uint32_t state_ = 0;
};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_BIT_STATE_MANAGER_H
