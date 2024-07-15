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

#ifndef OHOS_INPUTMETHOD_FAIR_LOCK_GUARD_H
#define OHOS_INPUTMETHOD_FAIR_LOCK_GUARD_H
#include <algorithm>
#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>

#include "fair_lock.h"

namespace OHOS {
namespace MiscServices {
class FairLockGuard {
public:
    explicit FairLockGuard(FairLock &lock) : lock_(lock)
    {
        lock_.Lock();
    }

    FairLockGuard(FairLock &lock, uint32_t timeout, bool &isTimeout) : lock_(lock)
    {
        isTimeout = lock_.Lock(timeout);
        isTimeout_ = isTimeout;
    }

    ~FairLockGuard()
    {
        if (isTimeout_) {
            return;
        }
        lock_.UnLock();
    }

private:
    FairLock &lock_;
    bool isTimeout_{ false };
};
} // namespace MiscServices
} // namespace OHOS
#endif // OHOS_INPUTMETHOD_FAIR_LOCK_GUARD_H
