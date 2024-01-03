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

#ifndef IMF_WMS_CONNECTION_OBSERVER_H
#define IMF_WMS_CONNECTION_OBSERVER_H

#include <mutex>
#include <set>
#include <utility>

#include "window_manager.h"
#include "wms_connection_monitor_manager.h"
namespace OHOS {
namespace MiscServices {
class WmsConnectionObserver : public Rosen::IWMSConnectionChangedListener {
public:
    explicit WmsConnectionObserver(ChangeHandler handler) : changeHandler_(std::move(handler)){};
    ~WmsConnectionObserver() = default;
    void OnConnected(int32_t userId, int32_t screenId) override;
    void OnDisconnected(int32_t userId, int32_t screenId) override;
    static bool IsWmsConnected(int32_t userId);

private:
    ChangeHandler changeHandler_ = nullptr;
    static void Add(int32_t userId);
    static void Remove(int32_t userId);
    static std::mutex lock_;
    static std::set<int32_t> connectedUserId_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_WMS_CONNECTION_OBSERVER_H
