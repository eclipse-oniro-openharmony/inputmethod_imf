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

#ifndef OHOS_INPUTMETHOD_FFRT_UTILS
#define OHOS_INPUTMETHOD_FFRT_UTILS

#include <functional>

#include "block_data.h"
#include "ffrt_inner.h"

namespace OHOS {
namespace MiscServices {
class FFRTUtils {
public:
    static bool SubmitBlockTask(const std::shared_ptr<std::function<void()>> &task, uint32_t timeout);

    template<class T>
    static bool SubmitBlockTask(const std::shared_ptr<std::function<T()>> &task, uint32_t timeout, T &taskResult);
};

bool FFRTUtils::SubmitBlockTask(const std::shared_ptr<std::function<void()>> &task, uint32_t timeout)
{
    auto result = std::make_shared<BlockData<bool>>(timeout, false);
    auto blockTask = [task, result]() {
        (*task)();
        bool ret = true;
        result->SetValue(ret);
    };
    ffrt::submit(blockTask);
    return result->GetValue();
}

template<class T>
bool FFRTUtils::SubmitBlockTask(const std::shared_ptr<std::function<T()>> &task, uint32_t timeout, T &taskResult)
{
    auto result = std::make_shared<BlockData<T>>(timeout);
    auto blockTask = [task, result]() {
        T ret = (*task)();
        result->SetValue(ret);
    };
    ffrt::submit(blockTask);
    return result->GetValue(taskResult);
}
} // namespace MiscServices
} // namespace OHOS

#endif // OHOS_INPUTMETHOD_FFRT_UTILS
