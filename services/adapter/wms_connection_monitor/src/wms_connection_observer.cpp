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
std::unordered_map<int32_t, bool> WmsConnectionObserver::wmsConnectionStatus_;
bool WmsConnectionObserver::IsWmsConnected(int32_t userId)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = wmsConnectionStatus_.find(userId);
    if (it == wmsConnectionStatus_.end()) {
        return false;
    }
    return it->second;
}

void WmsConnectionObserver::UpdateWmsConnectionStatus(int32_t userId, bool isConnected)
{
    std::lock_guard<std::mutex> lock(lock_);
    wmsConnectionStatus_.insert_or_assign(userId, isConnected);
}

void WmsConnectionObserver::OnConnected(int32_t userId, int32_t screenId)
{
    IMSA_HILOGI("WMS connect, userId: %{public}d, screenId: %{public}d", userId, screenId);
    UpdateWmsConnectionStatus(userId, true);
    if (changeHandler_ != nullptr) {
        changeHandler_(userId, screenId);
    }
}

void WmsConnectionObserver::OnDisconnected(int32_t userId, int32_t screenId)
{
    IMSA_HILOGI("WMS connect, userId: %{public}d, screenId: %{public}d", userId, screenId);
    UpdateWmsConnectionStatus(userId, false);
}
} // namespace MiscServices
} // namespace OHOS