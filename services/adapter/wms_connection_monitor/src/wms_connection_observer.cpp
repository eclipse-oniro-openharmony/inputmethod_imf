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

#include "wms_connection_observer.h"

#include "global.h"
namespace OHOS {
namespace MiscServices {
std::mutex WmsConnectionObserver::lock_;
std::set<int32_t> WmsConnectionObserver::connectedUserId_;
void WmsConnectionObserver::OnConnected(int32_t userId, int32_t screenId)
{
    IMSA_HILOGI("WMS connect, userId: %{public}d, screenId: %{public}d", userId, screenId);
    Add(userId);
    if (changeHandler_ != nullptr) {
        changeHandler_(true, userId, screenId);
    }
}

void WmsConnectionObserver::OnDisconnected(int32_t userId, int32_t screenId)
{
    IMSA_HILOGI("WMS disconnect, userId: %{public}d, screenId: %{public}d", userId, screenId);
    Remove(userId);
    if (changeHandler_ != nullptr) {
        changeHandler_(false, userId, screenId);
    }
}

void WmsConnectionObserver::Add(int32_t userId)
{
    std::lock_guard<std::mutex> lock(lock_);
    connectedUserId_.insert(userId);
}

void WmsConnectionObserver::Remove(int32_t userId)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = connectedUserId_.find(userId);
    if (it == connectedUserId_.end()) {
        return;
    }
    connectedUserId_.erase(it);
}

bool WmsConnectionObserver::IsWmsConnected(int32_t userId)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = connectedUserId_.find(userId);
    return it != connectedUserId_.end();
}
} // namespace MiscServices
} // namespace OHOS