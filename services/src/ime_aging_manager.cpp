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

#include "ime_aging_manager.h"

#include "common_timer_errors.h"
#include "peruser_session.h"

namespace OHOS {
namespace MiscServices {
constexpr uint32_t MAX_CACHES_SIZE = 5;
constexpr uint32_t AGING_TIME = 60;
constexpr uint32_t TIMER_TASK_INTERNAL = 60000;
ImeAgingManager::ImeAgingManager() : timer_("imeCacheTimer"), timerId_(0)
{
}

ImeAgingManager &ImeAgingManager::GetInstance()
{
    static ImeAgingManager ImeAgingManager;
    return ImeAgingManager;
}

bool ImeAgingManager::Push(const std::string &bundleName, const std::shared_ptr<ImeData> &imeData)
{
    if (bundleName.empty() || imeData == nullptr || imeData->core == nullptr || imeData->agent == nullptr) {
        IMSA_HILOGE("invalid ime data");
        return false;
    }
    auto imeCache = std::make_shared<AgingIme>(*imeData, std::chrono::system_clock::now());

    std::lock_guard<std::recursive_mutex> lock(cacheMutex_);
    auto it = imeCaches_.find(bundleName);
    if (it != imeCaches_.end()) {
        it->second = imeCache;
        return true;
    }
    if (imeCaches_.empty()) {
        StartAging();
    }
    if (imeCaches_.size() == MAX_CACHES_SIZE) {
        ClearOldest();
    }
    imeCaches_.insert({ bundleName, imeCache });
    IMSA_HILOGI("push ime: %{public}s", bundleName.c_str());
    return true;
}

std::shared_ptr<ImeData> ImeAgingManager::Pop(const std::string &bundleName)
{
    std::lock_guard<std::recursive_mutex> lock(cacheMutex_);
    auto it = imeCaches_.find(bundleName);
    if (it == imeCaches_.end()) {
        return nullptr;
    }
    auto ime = it->second->data;
    if (ime.core->AsObject() != nullptr && ime.deathRecipient != nullptr) {
        ime.core->AsObject()->RemoveDeathRecipient(ime.deathRecipient);
        ime.deathRecipient = nullptr;
    }
    imeCaches_.erase(bundleName);
    if (imeCaches_.empty()) {
        StopAging();
    }
    IMSA_HILOGI("pop ime: %{public}s", bundleName.c_str());
    return std::make_shared<ImeData>(ime);
}

void ImeAgingManager::ClearOldest()
{
    std::lock_guard<std::recursive_mutex> lock(cacheMutex_);
    auto oldestIme = imeCaches_.begin();
    for (auto it = imeCaches_.begin(); it != imeCaches_.end(); it++) {
        if (it->second->timestamp < oldestIme->second->timestamp) {
            oldestIme = it;
        }
    }
    auto core = oldestIme->second->data.core;
    if (core != nullptr) {
        IMSA_HILOGI("clear ime: %{public}s", oldestIme->first.c_str());
        ClearIme(oldestIme->second);
    }
    imeCaches_.erase(oldestIme);
}

void ImeAgingManager::AgingCache()
{
    std::lock_guard<std::recursive_mutex> lock(cacheMutex_);
    for (auto it = imeCaches_.begin(); it != imeCaches_.end();) {
        // each IME can be kept for 60 seconds, and then be stopped.
        auto now = std::chrono::system_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - it->second->timestamp).count() < AGING_TIME) {
            it++;
            continue;
        }
        auto core = it->second->data.core;
        if (core != nullptr) {
            IMSA_HILOGI("clear ime: %{public}s", it->first.c_str());
            ClearIme(it->second);
        }
        it = imeCaches_.erase(it);
    }
    if (imeCaches_.empty()) {
        StopAging();
    }
}

void ImeAgingManager::ClearIme(const std::shared_ptr<AgingIme> &ime)
{
    auto imeData = ime->data;
    if (imeData.core == nullptr) {
        return;
    }
    if (imeData.core->AsObject() != nullptr && imeData.deathRecipient != nullptr) {
        imeData.core->AsObject()->RemoveDeathRecipient(imeData.deathRecipient);
    }
    imeData.core->StopInputService(true);
}

void ImeAgingManager::StartAging()
{
    std::lock_guard<std::mutex> lock(timerMutex_);
    IMSA_HILOGD("run in");
    uint32_t ret = timer_.Setup();
    if (ret != Utils::TIMER_ERR_OK) {
        IMSA_HILOGE("failed to create timer");
        return;
    }
    timerId_ = timer_.Register([this]() { AgingCache(); }, TIMER_TASK_INTERNAL, false);
}

void ImeAgingManager::StopAging()
{
    std::lock_guard<std::mutex> lock(timerMutex_);
    IMSA_HILOGD("run in");
    timer_.Unregister(timerId_);
    timer_.Shutdown(false);
}
} // namespace MiscServices
} // namespace OHOS
