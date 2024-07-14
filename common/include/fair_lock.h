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

#ifndef OHOS_INPUTMETHOD_FAIR_LOCK_H
#define OHOS_INPUTMETHOD_FAIR_LOCK_H
#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace OHOS {
namespace MiscServices {
class FairLock {
public:
    FairLock() : nextThreadId_(0)
    {
    }

    ~FairLock() = default;

    bool Lock(uint32_t timeout)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto threadId = nextThreadId_++;
        threadId_.push_back(threadId);
        auto ret = cv_.wait_for(
            lock, std::chrono::milliseconds(timeout), [this, threadId]() { return threadId_.front() == threadId; });
        if (!ret) {
            auto it =
                std::find_if(threadId_.begin(), threadId_.end(), [threadId](uint32_t id) { return id == threadId; });
            if (it != threadId_.end()) {
                threadId_.erase(it);
            }
            return false;
        }
        return true;
    }

    void Lock()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto threadId = nextThreadId_++;
        threadId_.push_back(threadId);
        cv_.wait(lock, [this, threadId]() { return threadId_.front() == threadId; });
    }

    void UnLock()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        threadId_.erase(threadId_.begin());
        cv_.notify_all();
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    uint32_t nextThreadId_;
    std::vector<uint32_t> threadId_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // OHOS_INPUTMETHOD_FAIR_LOCK_H
