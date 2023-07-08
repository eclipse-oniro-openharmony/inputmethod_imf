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

#ifndef OHOS_INPUTMETHOD_QUEUE_H
#define OHOS_INPUTMETHOD_QUEUE_H
#include <condition_variable>
#include <mutex>
#include <queue>

namespace OHOS {
namespace MiscServices {
template<typename T> class Queue {
public:
    explicit Queue(uint32_t interval) : INTERVAL(interval)
    {
    }

    ~Queue()
    {
    }

    void Pop()
    {
        std::lock_guard<std::mutex> lock(queuesMutex_);
        queues_.pop();
        execCv_.notify_all();
    }

    void Push(const T &data)
    {
        std::lock_guard<std::mutex> lock(queuesMutex_);
        queues_.push(data);
    }

    void WaitExec(const T &data)
    {
        if (!IsReadyToExec(data)) {
            std::unique_lock<std::mutex> lock(execMutex_);
            execCv_.wait_for(
                lock, std::chrono::milliseconds(INTERVAL), [&data, this]() { return IsReadyToExec(data); });
        }
    }

    bool IsReadyToExec(const T &data)
    {
        std::lock_guard<std::mutex> lock(queuesMutex_);
        return data == queues_.front();
    }

private:
    const uint32_t INTERVAL;
    std::mutex queuesMutex_;
    std::queue<T> queues_;
    std::mutex execMutex_;
    std::condition_variable execCv_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // OHOS_INPUTMETHOD_QUEUE_H
