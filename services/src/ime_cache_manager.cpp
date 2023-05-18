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

#include "ime_cache_manager.h"

#include "common_timer_errors.h"
#include "peruser_session.h"

namespace OHOS {
namespace MiscServices {
constexpr uint32_t MAX_CACHES_SIZE = 5;
constexpr uint32_t TIME_OUT = 60;
constexpr uint32_t TIMER_TASK_INTERNAL = 60;
ImeCacheManager::ImeCacheManager() : timer_("imeCacheTimer"), timerId_(0)
{
}

ImeCacheManager &ImeCacheManager::GetInstance()
{
    static ImeCacheManager imeCacheManager;
    return imeCacheManager;
}

bool ImeCacheManager::Push(const std::string &imeName, const std::shared_ptr<ImeCache> &info)
{
    if (imeName.empty() || info == nullptr) {
        return false;
    }
    std::lock_guard<std::recursive_mutex> lock(cacheMutex_);
    auto it = imeCaches_.find(imeName);
    if (it != imeCaches_.end()) {
        it->second = info;
        return true;
    }
    if (imeCaches_.empty()) {
        StartTimer();
    }
    if (imeCaches_.size() == MAX_CACHES_SIZE) {
        ClearOldest();
    }
    info->timeStamp = time(nullptr);
    imeCaches_.insert({ imeName, info });
    return true;
}

std::shared_ptr<ImeCache> ImeCacheManager::Pop(const std::string &imeName)
{
    std::lock_guard<std::recursive_mutex> lock(cacheMutex_);
    auto it = imeCaches_.find(imeName);
    if (it == imeCaches_.end()) {
        return nullptr;
    }
    auto cache = it->second;
    imeCaches_.erase(imeName);
    if (imeCaches_.empty()) {
        StopTimer();
    }
    return cache;
}

void ImeCacheManager::ClearOldest()
{
    std::lock_guard<std::recursive_mutex> lock(cacheMutex_);
    auto oldestIme = imeCaches_.begin();
    for (auto it = imeCaches_.begin(); it != imeCaches_.end();) {
        if (it->second->timeStamp < oldestIme->second->timeStamp) {
            oldestIme = it;
        }
    }
    imeCaches_.erase(oldestIme);
}

void ImeCacheManager::CheckTimeOut()
{
    std::lock_guard<std::recursive_mutex> lock(cacheMutex_);
    for (auto it = imeCaches_.begin(); it != imeCaches_.end();) {
        auto now = time(nullptr);
        if (difftime(now, it->second->timeStamp) < TIME_OUT) {
            it++;
            continue;
        }
        auto core = it->second->core;
        if (core != nullptr) {
            core->StopInputService(it->first);
        }
        it = imeCaches_.erase(it);
    }
    if (imeCaches_.empty()) {
        StopTimer();
    }
}

void ImeCacheManager::StartTimer()
{
    uint32_t ret = timer_.Setup();
    if (ret != Utils::TIMER_ERR_OK) {
        IMSA_HILOGE("failed to create timer");
        return;
    }
    timerId_ = timer_.Register([this]() { CheckTimeOut(); }, TIMER_TASK_INTERNAL, false);
}

void ImeCacheManager::StopTimer()
{
    timer_.Unregister(timerId_);
    timer_.Shutdown();
}
} // namespace MiscServices
} // namespace OHOS
