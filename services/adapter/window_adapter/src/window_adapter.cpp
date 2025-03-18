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

#include "window_adapter.h"

#include "window.h"
#include "wm_common.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {
using namespace Rosen;
using WMError = OHOS::Rosen::WMError;

WindowAdapter::~WindowAdapter()
{
}

WindowAdapter &WindowAdapter::GetInstance()
{
    static WindowAdapter windowAdapter;
    return windowAdapter;
}

void WindowAdapter::GetFoucusInfo(OHOS::Rosen::FocusChangeInfo& focusInfo)
{
#ifdef SCENE_BOARD_ENABLE
    WindowManagerLite::GetInstance().GetFocusWindowInfo(focusInfo);
#else
    WindowManager::GetInstance().GetFocusWindowInfo(focusInfo);
#endif
}

bool WindowAdapter::GetCallingWindowInfo(const uint32_t windId, const int32_t userId,
    Rosen::CallingWindowInfo &callingWindowInfo)
{
    IMSA_HILOGD("enter, windId:%{public}d", windId);
    callingWindowInfo.windowId_ = windId;
    callingWindowInfo.userId_ = userId;
#ifdef SCENE_BOARD_ENABLE
    auto wmerr = WindowManagerLite::GetInstance().GetCallingWindowInfo(callingWindowInfo);
#else
    auto wmerr = WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
#endif
    if (wmerr != WMError::WM_OK) {
        IMSA_HILOGE("failed to get calling window info.");
        return false;
    }
    IMSA_HILOGD("callingWindowInfo:%{public}s",
        WindowDisplayChangeListener::CallingWindowInfoToString(callingWindowInfo).c_str());
    return true;
}

void WindowAdapter::RegisterCallingWindowInfoChangedListener(const WindowDisplayChangeHandler &handle)
{
    sptr<WindowDisplayChangeListener> listener = new (std::nothrow) WindowDisplayChangeListener(handle);
    if (listener == nullptr) {
        IMSA_HILOGE("failed to create listener");
        return;
    }
#ifdef SCENE_BOARD_ENABLE
    WMError ret = WindowManagerLite::GetInstance().RegisterCallingWindowDisplayChangedListener(listener);
#else
    WMError ret = WindowManager::GetInstance().RegisterCallingWindowDisplayChangedListener(listener);
#endif
    IMSA_HILOGI("register focus changed listener ret: %{public}d", ret);
}
} // namespace MiscServices
} // namespace OHOS

