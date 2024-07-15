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
#include <list>
#include <mutex>
#include <thread>

namespace OHOS {
namespace MiscServices {
class FairLock {
public:
    FairLock() = default;
    ~FairLock() = default;

    bool Lock(uint32_t timeout)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto threadId = std::this_thread::get_id();
        threadId_.push_back(threadId);
        auto ret = cv_.wait_for(
            lock, std::chrono::milliseconds(timeout), [this, threadId]() { return threadId_.front() == threadId; });
        if (!ret) {
            auto it = std::find(threadId_.begin(), threadId_.end(), threadId);
            if (it != threadId_.end()) {
                threadId_.erase(it);
            }
            return true;
        }
        return false;
    }

    void Lock()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto threadId = std::this_thread::get_id();
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
    std::list<std::thread::id> threadId_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // OHOS_INPUTMETHOD_FAIR_LOCK_H
