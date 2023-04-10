/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "input_method_panel.h"

#include "display_manager.h"
#include "global.h"
#include "window.h"
#include "wm_common.h"

namespace OHOS {
namespace MiscServices {
using WMError = OHOS::Rosen::WMError;
using WindowGravity = OHOS::Rosen::WindowGravity;
using WindowState = OHOS::Rosen::WindowState;
std::atomic<uint32_t> InputMethodPanel::sequenceId_{ 0 };
InputMethodPanel::~InputMethodPanel() = default;

int32_t InputMethodPanel::CreatePanel(
    const std::shared_ptr<AbilityRuntime::Context> &context, const PanelInfo &panelInfo)
{
    IMSA_HILOGD("InputMethodPanel start to create panel.");
    panelType_ = panelInfo.panelType;
    panelFlag_ = panelInfo.panelFlag;
    winOption_ = new (std::nothrow) OHOS::Rosen::WindowOption();
    if (winOption_ == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    winOption_->SetWindowType(OHOS::Rosen::WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    WMError wmError = WMError::WM_OK;
    WindowGravity gravity = WindowGravity::WINDOW_GRAVITY_FLOAT;
    if (panelType_ == SOFT_KEYBOARD && panelFlag_ == FLG_FIXED) {
        gravity = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    }
    uint32_t sequenceId = GenerateSequenceId();
    std::string windowName = panelType_ == SOFT_KEYBOARD ? "softKeyboard" + std::to_string(sequenceId)
                                                         : "statusBar" + std::to_string(sequenceId);
    IMSA_HILOGD("InputMethodPanel,  windowName = %{public}s", windowName.c_str());
    window_ = OHOS::Rosen::Window::Create(windowName, winOption_, context, wmError);
    if (wmError == WMError::WM_ERROR_INVALID_PERMISSION) {
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    if (window_ == nullptr || wmError != WMError::WM_OK) {
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    windowId_ = window_->GetWindowId();
    IMSA_HILOGD("GetWindowId, windowId = %{public}u", windowId_);
    wmError = window_->SetWindowGravity(gravity, invalidGravityPercent);
    if (wmError == WMError::WM_OK) {
        return ErrorCode::NO_ERROR;
    }
    wmError = window_->Destroy();
    return wmError == WMError::WM_OK ? ErrorCode::NO_ERROR : ErrorCode::ERROR_OPERATE_PANEL;
}

int32_t InputMethodPanel::DestroyPanel()
{
    auto ret = HidePanel();
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGI("InputMethodPanel, hide panel failed, ret = %{public}d.", ret);
        return ret;
    }
    auto result = window_->Destroy();
    if (result != WMError::WM_OK) {
        IMSA_HILOGE("InputMethodPanel, destroy panel error, ret = %{public}d", result);
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodPanel::Resize(uint32_t width, uint32_t height)
{
    if (window_ == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (defaultDisplay == nullptr) {
        IMSA_HILOGE("GetDefaultDisplay failed.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (width > INT32_MAX || height > INT32_MAX) {
        IMSA_HILOGE("width or height over maximum");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    // the resize width can not exceed the width of display width of device
    // the resize height can not exceed half of the display height of device
    if (static_cast<int32_t>(width) > defaultDisplay->GetWidth()
        || static_cast<int32_t>(height) > defaultDisplay->GetHeight() / 2) {
        IMSA_HILOGD("GetDefaultDisplay, defaultDisplay->width = %{public}d, defaultDisplay->height = %{public}d, "
                    "width = %{public}u, height = %{public}u",
            defaultDisplay->GetWidth(), defaultDisplay->GetHeight(), width, height);
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto ret = window_->Resize(width, height);
    return ret == WMError::WM_OK ? ErrorCode::NO_ERROR : ErrorCode::ERROR_OPERATE_PANEL;
}

int32_t InputMethodPanel::MoveTo(int32_t x, int32_t y)
{
    if (panelType_ == SOFT_KEYBOARD && panelFlag_ == FLG_FIXED) {
        IMSA_HILOGE("FLG_FIXED panel can not moveTo.");
        return ErrorCode::NO_ERROR;
    }
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto ret = window_->MoveTo(x, y);
    if (ret != WMError::WM_OK) {
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodPanel::ChangePanelFlag(PanelFlag panelFlag)
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (panelFlag_ == panelFlag) {
        return ErrorCode::NO_ERROR;
    }
    if (panelType_ == STATUS_BAR) {
        IMSA_HILOGE("STATUS_BAR cannot ChangePanelFlag.");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    panelFlag_ = panelFlag;
    WindowGravity gravity = WindowGravity::WINDOW_GRAVITY_FLOAT;
    if (panelFlag == FLG_FIXED) {
        gravity = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    }
    auto ret = window_->SetWindowGravity(gravity, invalidGravityPercent);
    return ret == WMError::WM_OK ? ErrorCode::NO_ERROR : ErrorCode::ERROR_OPERATE_PANEL;
}

PanelType InputMethodPanel::GetPanelType()
{
    return panelType_;
}

int32_t InputMethodPanel::ShowPanel()
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (IsShowing()) {
        IMSA_HILOGE("Panel already shown.");
        return ErrorCode::NO_ERROR;
    }
    auto ret = window_->Show();
    if (ret != WMError::WM_OK) {
        IMSA_HILOGE("ShowPanel error, err = %{public}d, ERROR_SHOW_PANEL", ret);
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    if (showRegistered_ && panelStatusListener_ != nullptr) {
        IMSA_HILOGE("InputMethodPanel::ShowPanel panelStatusListener_ is not nullptr");
        panelStatusListener_->OnPanelStatus(windowId_, true);
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodPanel::HidePanel()
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (IsHidden()) {
        IMSA_HILOGE("Panel already hidden.");
        return ErrorCode::NO_ERROR;
    }
    auto ret = window_->Hide();
    if (ret != WMError::WM_OK) {
        IMSA_HILOGE("HidePanel error, err = %{public}d, ERROR_HIDE_PANEL", ret);
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    if (hideRegistered_ && panelStatusListener_ != nullptr) {
        IMSA_HILOGE("InputMethodPanel::HidePanel panelStatusListener_ is not nullptr");
        panelStatusListener_->OnPanelStatus(windowId_, false);
    }
    return ErrorCode::NO_ERROR;
}

bool InputMethodPanel::IsShowing()
{
    WindowState windowState = window_->GetWindowState();
    if (windowState == WindowState::STATE_SHOWN) {
        return true;
    }
    IMSA_HILOGD("InputMethodPanel windowState = %{public}d", static_cast<int>(windowState));
    return false;
}

bool InputMethodPanel::IsHidden()
{
    WindowState windowState = window_->GetWindowState();
    if (windowState == WindowState::STATE_HIDDEN) {
        return true;
    }
    IMSA_HILOGD("InputMethodPanel windowState = %{public}d", static_cast<int>(windowState));
    return false;
}

int32_t InputMethodPanel::SetUiContent(const std::string &contentInfo, NativeEngine *engine, NativeValue *storage)
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr, can not SetUiContent.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto ret = window_->SetUIContent(contentInfo, engine, storage);
    if (ret != WMError::WM_OK) {
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    return ErrorCode::NO_ERROR;
}

void InputMethodPanel::SetPanelStatusListener(std::shared_ptr<PanelStatusListener> statusListener)
{
    IMSA_HILOGD("SetPanelStatusListener start.");
    if (panelStatusListener_ != nullptr) {
        IMSA_HILOGE("PanelStatusListener already set.");
        return;
    }
    panelStatusListener_ = std::move(statusListener);
}

void InputMethodPanel::RemovePanelListener(const std::string &type)
{
    if (type == "show") {
        showRegistered_ = false;
    } else if (type == "hide") {
        hideRegistered_ = false;
    } else {
        IMSA_HILOGE("type error.");
        return;
    }
    if (panelStatusListener_ == nullptr) {
        IMSA_HILOGE("PanelStatusListener not set, don't need to remove.");
        return;
    }
    if (showRegistered_ || hideRegistered_) {
        return;
    }
    panelStatusListener_ = nullptr;
}

uint32_t InputMethodPanel::GenerateSequenceId()
{
    uint32_t seqId = ++sequenceId_;
    if (seqId == std::numeric_limits<uint32_t>::max()) {
        return ++sequenceId_;
    }
    return seqId;
}
} // namespace MiscServices
} // namespace OHOS
