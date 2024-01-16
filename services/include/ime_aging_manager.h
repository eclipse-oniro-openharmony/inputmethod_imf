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

#ifndef SERVICES_INCLUDE_IME_AGING_MANAGER_H
#define SERVICES_INCLUDE_IME_AGING_MANAGER_H

#include <chrono>
#include <map>
#include <utility>

#include "global.h"
#include "i_input_method_agent.h"
#include "i_input_method_core.h"
#include "input_death_recipient.h"
#include "peruser_session.h"
#include "timer.h"

namespace OHOS {
namespace MiscServices {
struct ImeCache {
    ImeData data;
    std::chrono::system_clock::time_point timestamp{};
    ImeCache(ImeData data, std::chrono::system_clock::time_point timestamp)
        : data(std::move(data)), timestamp(timestamp)
    {
    }
};

class ImeAgingManager {
public:
    static ImeAgingManager &GetInstance();
    bool Push(const std::string &imeName, const std::shared_ptr<ImeData> &imeCache);
    std::shared_ptr<ImeData> Pop(const std::string &imeName);

private:
    ImeAgingManager();
    void ClearOldest();
    void AgingCache();
    void StartAging();
    void StopAging();
    void StopIme(const std::shared_ptr<ImeCache> &ime);

    std::mutex timerMutex_;
    Utils::Timer timer_;
    uint32_t timerId_;
    std::recursive_mutex cacheMutex_;
    std::map<std::string, std::shared_ptr<ImeCache>> imeCaches_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // SERVICES_INCLUDE_IME_AGING_MANAGER_H
