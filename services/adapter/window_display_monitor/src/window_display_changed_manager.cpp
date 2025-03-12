/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "window_display_changed_manager.h"

#include "window.h"
#include "wm_common.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {
#ifdef SCENE_BOARD_ENABLE
using namespace Rosen;
using WMError = OHOS::Rosen::WMError;

WindowDisplayChangedManager::~WindowDisplayChangedManager()
{
}

WindowDisplayChangedManager &WindowDisplayChangedManager::GetInstance()
{
    static WindowDisplayChangedManager displayMonitorManager;
    return displayMonitorManager;
}

void WindowDisplayChangedManager::GetFoucusInfo(OHOS::Rosen::FocusChangeInfo& focusInfo)
{
    WindowManagerLite::GetInstance().GetFocusWindowInfo(focusInfo);
}

bool WindowDisplayChangedManager::GetCallingWindowInfo(const uint32_t windId, const int32_t userId,
    Rosen::CallingWindowInfo &callingWindowInfo)
{
    IMSA_HILOGD("enter, windId:%{public}d", windId);
    callingWindowInfo.windowId_ = windId;
    callingWindowInfo.userId_ = userId;
    auto wmerr = WindowManagerLite::GetInstance().GetCallingWindowInfo(callingWindowInfo);
    if (wmerr != WMError::WM_OK) {
        IMSA_HILOGE("failed to get calling window info.");
        return false;
    }
    IMSA_HILOGI("callingWindowInfo:%{public}s",
        WindowDisplayChangeListener::CallingWindowInfoToString(callingWindowInfo).c_str());
    return true;
}

void  WindowDisplayChangedManager::RegisterCallingWindowInfoChangedListener(const WindowDisplayChangeHandler &handle)
{
    sptr<WindowDisplayChangeListener> listener = new (std::nothrow) WindowDisplayChangeListener(handle);
    if (listener == nullptr) {
        IMSA_HILOGE("failed to create listener");
        return;
    }
    WMError ret = WindowManagerLite::GetInstance().RegisterCallingWindowDisplayChangedListener(listener);
    IMSA_HILOGI("register focus changed listener ret: %{public}d", ret);
}
#endif
} // namespace MiscServices
} // namespace OHOS

