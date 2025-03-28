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

#include <cinttypes>

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

void WindowAdapter::GetFocusInfo(OHOS::Rosen::FocusChangeInfo &focusInfo, uint64_t displayId)
{
#ifdef SCENE_BOARD_ENABLE
    WindowManagerLite::GetInstance().GetFocusWindowInfo(focusInfo, displayId);
#else
    WindowManager::GetInstance().GetFocusWindowInfo(focusInfo, displayId);
#endif
}

bool WindowAdapter::GetCallingWindowInfo(
    const uint32_t windId, const int32_t userId, CallingWindowInfo &callingWindowInfo)
{
#ifdef SCENE_BOARD_ENABLE
    IMSA_HILOGD("[%{public}d,%{public}d] run in.", userId, windId);
    callingWindowInfo.windowId_ = static_cast<int32_t>(windId);
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

bool WindowAdapter::ListWindowInfo(std::vector<sptr<OHOS::Rosen::WindowInfo>> &windowInfos)
{
#ifdef SCENE_BOARD_ENABLE
    WindowInfoOption option;
    WMError ret = WindowManagerLite::GetInstance().ListWindowInfo(option, windowInfos);
    if (ret != WMError::WM_OK) {
        IMSA_HILOGE("ListWindowInfo failed, ret: %{public}d", ret);
        return false;
    }
    return true;
#else
    IMSA_HILOGE("capability not supported");
    return false;
#endif
}

uint64_t WindowAdapter::GetDisplayIdByWindowId(int32_t callingWindowId)
{
#ifdef SCENE_BOARD_ENABLE
    if (callingWindowId == DEFAULT_DISPLAY_ID) {
        FocusChangeInfo info;
        WindowManagerLite::GetInstance().GetFocusWindowInfo(info);
        callingWindowId = info.windowId_;
    }
    std::vector<sptr<WindowInfo>> windowInfos;
    if (!ListWindowInfo(windowInfos)) {
        return DEFAULT_DISPLAY_ID;
    }
    auto iter = std::find_if(windowInfos.begin(), windowInfos.end(), [&callingWindowId](const auto &windowInfo) {
        if (windowInfo == nullptr) {
            return false;
        }
        return windowInfo->windowMetaInfo.windowId == callingWindowId;
    });
    if (iter == windowInfos.end()) {
        IMSA_HILOGE("not found window info with windowId: %{public}d", callingWindowId);
        return DEFAULT_DISPLAY_ID;
    }
    auto callingDisplayId = (*iter)->windowDisplayInfo.displayId;
    IMSA_HILOGD("window windowId: %{public}d, displayId: %{public}" PRIu64 "", callingWindowId, callingDisplayId);
    return callingDisplayId;
#else
    IMSA_HILOGI("capability not supported");
    return DEFAULT_DISPLAY_ID;
#endif
}

uint64_t WindowAdapter::GetDisplayIdByPid(int64_t callingPid)
{
#ifdef SCENE_BOARD_ENABLE
    std::vector<sptr<WindowInfo>> windowInfos;
    if (!ListWindowInfo(windowInfos)) {
        return DEFAULT_DISPLAY_ID;
    }
    auto iter = std::find_if(windowInfos.begin(), windowInfos.end(), [&callingPid](const auto &windowInfo) {
        if (windowInfo == nullptr) {
            return false;
        }
        return windowInfo->windowMetaInfo.pid == callingPid;
    });
    if (iter == windowInfos.end()) {
        IMSA_HILOGE("not found window info with pid: %{public}" PRId64 "", callingPid);
        return DEFAULT_DISPLAY_ID;
    }
    auto callingDisplayId = (*iter)->windowDisplayInfo.displayId;
    IMSA_HILOGD("window pid: %{public}" PRId64 ", displayId: %{public}" PRIu64 "", callingPid, callingDisplayId);
    return callingDisplayId;
#else
    IMSA_HILOGI("capability not supported");
    return DEFAULT_DISPLAY_ID;
#endif
}
} // namespace MiscServices
} // namespace OHOS
