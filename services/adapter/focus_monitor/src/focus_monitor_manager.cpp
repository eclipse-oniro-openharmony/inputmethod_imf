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

#include "focus_monitor_manager.h"

#include "focus_change_listener.h"
#include "global.h"
#include "window_manager.h"

namespace OHOS {
namespace MiscServices {
using namespace Rosen;
FocusMonitorManager &FocusMonitorManager::GetInstance()
{
    static FocusMonitorManager focusMonitorManager;
    return focusMonitorManager;
}

void FocusMonitorManager::RegisterFocusChangedListener(const FocusHandle &handle)
{
    sptr<IFocusChangedListener> listener = new (std::nothrow) FocusChangedListener(handle);
    if (listener == nullptr) {
        IMSA_HILOGE("failed to create listener");
        return;
    }
    WMError ret = WindowManager::GetInstance().RegisterFocusChangedListener(listener);
    IMSA_HILOGI("register focus changed listener ret: %{public}d", ret);
}
} // namespace MiscServices
} // namespace OHOS