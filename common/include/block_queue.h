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

#ifndef OHOS_INPUTMETHOD_BLOCK_QUEUE_H
#define OHOS_INPUTMETHOD_BLOCK_QUEUE_H
#include <condition_variable>
#include <mutex>
#include <queue>

namespace OHOS {
namespace MiscServices {
template<typename T>
class BlockQueue {
public:
    explicit BlockQueue(uint32_t timeout) : timeout_(timeout)
    {
    }

    ~BlockQueue() = default;

    void Pop()
    {
        std::unique_lock<std::mutex> lock(queuesMutex_);
        queues_.pop();
        cv_.notify_all();
    }

    void Push(const T &data)
    {
        std::unique_lock<std::mutex> lock(queuesMutex_);
        queues_.push(data);
    }

    void Wait(const T &data)
    {
        std::unique_lock<std::mutex> lock(queuesMutex_);
        cv_.wait_for(lock, std::chrono::milliseconds(timeout_), [&data, this]() { return data == queues_.front(); });
    }

    bool IsReady(const T &data)
    {
        std::unique_lock<std::mutex> lock(queuesMutex_);
        return data == queues_.front();
    }

    bool GetFront(T &data)
    {
        std::unique_lock<std::mutex> lock(queuesMutex_);
        if (queues_.empty()) {
            return false;
        }
        data = queues_.front();
        return true;
    }

private:
    const uint32_t timeout_;
    std::mutex queuesMutex_;
    std::queue<T> queues_;
    std::condition_variable cv_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // OHOS_INPUTMETHOD_BLOCK_QUEUE_H
