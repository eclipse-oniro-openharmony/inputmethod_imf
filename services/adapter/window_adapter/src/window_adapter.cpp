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

#include "global.h"
#include "window.h"
#include "wm_common.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::Rosen;
using WMError = OHOS::Rosen::WMError;

WindowAdapter::~WindowAdapter()
{
}

WindowAdapter &WindowAdapter::GetInstance()
{
    static WindowAdapter windowAdapter;
    return windowAdapter;
}

void WindowAdapter::GetFoucusInfo(FocusChangeInfo &focusInfo)
{
#ifdef SCENE_BOARD_ENABLE
    WindowManagerLite::GetInstance().GetFocusWindowInfo(focusInfo);
#else
    WindowManager::GetInstance().GetFocusWindowInfo(focusInfo);
#endif
}

bool WindowAdapter::GetCallingWindowInfo(
    const uint32_t windId, const int32_t userId, CallingWindowInfo &callingWindowInfo)
{
#ifdef SCENE_BOARD_ENABLE
    IMSA_HILOGD("[%{public}d,%{public}d] run in.", userId, windId);
    callingWindowInfo.windowId_ = windId;
    callingWindowInfo.userId_ = userId;
    auto wmErr = WindowManagerLite::GetInstance().GetCallingWindowInfo(callingWindowInfo);
    if (wmErr != WMError::WM_OK) {
        IMSA_HILOGE("[%{public}d,%{public}d,%{public}d] failed to get calling window info.", userId, windId, wmErr);
        return false;
    }
    IMSA_HILOGD("callingWindowInfo:%{public}s",
        WindowDisplayChangeListener::CallingWindowInfoToString(callingWindowInfo).c_str());
    return true;
#else
    IMSA_HILOGE("capability not supported");
    return false;
#endif
}

void WindowAdapter::RegisterCallingWindowInfoChangedListener(const WindowDisplayChangeHandler &handle)
{
#ifdef SCENE_BOARD_ENABLE
    sptr<WindowDisplayChangeListener> listener = new (std::nothrow) WindowDisplayChangeListener(handle);
    if (listener == nullptr) {
        IMSA_HILOGE("failed to create listener");
        return;
    }
    auto wmErr = WMError::WM_OK;
    wmErr = WindowManagerLite::GetInstance().RegisterCallingWindowDisplayChangedListener(listener);
    IMSA_HILOGI("register focus changed listener ret: %{public}d", wmErr);
#endif
}
} // namespace MiscServices
} // namespace OHOS
