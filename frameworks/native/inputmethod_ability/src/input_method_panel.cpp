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
#include "input_method_ability_utils.h"
#include "ui/rs_surface_node.h"

namespace OHOS {
namespace MiscServices {
using WMError = OHOS::Rosen::WMError;
using WindowGravity = OHOS::Rosen::WindowGravity;
using WindowState = OHOS::Rosen::WindowState;
constexpr float FIXED_SOFT_KEYBOARD_PANEL_RATIO = 0.7;
constexpr float NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO = 1;
std::atomic<uint32_t> InputMethodPanel::sequenceId_{ 0 };
InputMethodPanel::~InputMethodPanel() = default;

int32_t InputMethodPanel::CreatePanel(
    const std::shared_ptr<AbilityRuntime::Context> &context, const PanelInfo &panelInfo)
{
    IMSA_HILOGD(
        "start, type/flag: %{public}d/%{public}d", static_cast<int32_t>(panelType_), static_cast<int32_t>(panelFlag_));
    panelType_ = panelInfo.panelType;
    panelFlag_ = panelInfo.panelFlag;
    winOption_ = new (std::nothrow) OHOS::Rosen::WindowOption();
    if (winOption_ == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (panelInfo.panelType == PanelType::STATUS_BAR) {
        winOption_->SetWindowType(OHOS::Rosen::WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR);
    } else {
        winOption_->SetWindowType(OHOS::Rosen::WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    }
    WMError wmError = WMError::WM_OK;
    window_ = OHOS::Rosen::Window::Create(GeneratePanelName(), winOption_, context, wmError);
    if (wmError == WMError::WM_ERROR_INVALID_PERMISSION || wmError == WMError::WM_ERROR_NOT_SYSTEM_APP) {
        IMSA_HILOGE("Create window failed, permission denied, %{public}d", wmError);
        return ErrorCode::ERROR_NOT_IME;
    }
    if (window_ == nullptr || wmError != WMError::WM_OK) {
        IMSA_HILOGE("Create window failed: %{public}d", wmError);
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    if (SetPanelProperties() != ErrorCode::NO_ERROR) {
        wmError = window_->Destroy();
        IMSA_HILOGI("Destroy window end, wmError is %{public}d.", wmError);
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    windowId_ = window_->GetWindowId();
    IMSA_HILOGI("success, type/flag/windowId: %{public}d/%{public}d/%{public}u", static_cast<int32_t>(panelType_),
        static_cast<int32_t>(panelFlag_), windowId_);
    return ErrorCode::NO_ERROR;
}

std::string InputMethodPanel::GeneratePanelName()
{
    uint32_t sequenceId = GenerateSequenceId();
    std::string windowName = panelType_ == SOFT_KEYBOARD ? "softKeyboard" + std::to_string(sequenceId)
                                                         : "statusBar" + std::to_string(sequenceId);
    IMSA_HILOGD("InputMethodPanel,  windowName = %{public}s", windowName.c_str());
    return windowName;
}

int32_t InputMethodPanel::SetPanelProperties()
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window is not exist.");
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    WindowGravity gravity = WindowGravity::WINDOW_GRAVITY_FLOAT;
    if (panelType_ == SOFT_KEYBOARD && panelFlag_ == FLG_FIXED) {
        gravity = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    } else if (panelType_ == SOFT_KEYBOARD && panelFlag_ == FLG_FLOATING) {
        window_->GetSurfaceNode()->SetFrameGravity(Rosen::Gravity::TOP_LEFT);
        Rosen::RSTransactionProxy::GetInstance()->FlushImplicitTransaction();
    } else if (panelType_ == STATUS_BAR) {
        window_->GetSurfaceNode()->SetFrameGravity(Rosen::Gravity::TOP_LEFT);
        Rosen::RSTransactionProxy::GetInstance()->FlushImplicitTransaction();
        return ErrorCode::NO_ERROR;
    }
    WMError wmError = window_->SetWindowGravity(gravity, invalidGravityPercent);
    if (wmError != WMError::WM_OK) {
        IMSA_HILOGE("SetWindowGravity failed, wmError is %{public}d, start destroy window.", wmError);
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodPanel::DestroyPanel()
{
    auto ret = HidePanel();
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("InputMethodPanel, hide panel failed, ret = %{public}d.", ret);
        return ret;
    }
    auto result = window_->Destroy();
    IMSA_HILOGI("ret = %{public}d", result);
    return result == WMError::WM_OK ? ErrorCode::NO_ERROR : ErrorCode::ERROR_OPERATE_PANEL;
}

int32_t InputMethodPanel::Resize(uint32_t width, uint32_t height)
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (!IsSizeValid(width, height)) {
        IMSA_HILOGE("invalid size");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto ret = window_->Resize(width, height);
    if (ret != WMError::WM_OK) {
        IMSA_HILOGE("failed to resize, ret: %{public}d", ret);
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    IMSA_HILOGI("success, width/height: %{public}u/%{public}u", width, height);
    {
        std::lock_guard<std::mutex> lock(heightLock_);
        panelHeight_ = height;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodPanel::MoveTo(int32_t x, int32_t y)
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (panelType_ == SOFT_KEYBOARD && panelFlag_ == FLG_FIXED) {
        IMSA_HILOGE("FLG_FIXED panel can not moveTo.");
        return ErrorCode::NO_ERROR;
    }
    auto ret = window_->MoveTo(x, y);
    IMSA_HILOGI("x/y: %{public}d/%{public}d, ret = %{public}d", x, y, ret);
    return ret == WMError::WM_OK ? ErrorCode::NO_ERROR : ErrorCode::ERROR_OPERATE_PANEL;
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
    } else {
        window_->GetSurfaceNode()->SetFrameGravity(Rosen::Gravity::TOP_LEFT);
        Rosen::RSTransactionProxy::GetInstance()->FlushImplicitTransaction();
    }
    auto ret = window_->SetWindowGravity(gravity, invalidGravityPercent);
    IMSA_HILOGI("flag: %{public}d, ret = %{public}d", panelFlag, ret);
    return ret == WMError::WM_OK ? ErrorCode::NO_ERROR : ErrorCode::ERROR_OPERATE_PANEL;
}

PanelType InputMethodPanel::GetPanelType()
{
    return panelType_;
}

PanelFlag InputMethodPanel::GetPanelFlag()
{
    return panelFlag_;
}

int32_t InputMethodPanel::ShowPanel()
{
    IMSA_HILOGD("InputMethodPanel, run in");
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (IsShowing()) {
        IMSA_HILOGI("Panel already shown.");
        return ErrorCode::NO_ERROR;
    }
    auto ret = window_->Show();
    if (ret != WMError::WM_OK) {
        IMSA_HILOGE("ShowPanel error, err = %{public}d", ret);
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    IMSA_HILOGI("success, type/flag: %{public}d/%{public}d", static_cast<int32_t>(panelType_),
        static_cast<int32_t>(panelFlag_));
    PanelStatusChange(InputWindowStatus::SHOW);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodPanel::SetTextFieldAvoidInfo(double positionY, double height)
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto ret = window_->SetTextFieldAvoidInfo(positionY, height);
    if (ret != WMError::WM_OK) {
        IMSA_HILOGE("SetTextFieldAvoidInfo error, err = %{public}d", ret);
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodPanel::HidePanel()
{
    IMSA_HILOGD("InputMethodPanel, run in");
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (IsHidden()) {
        IMSA_HILOGI("Panel already hidden.");
        return ErrorCode::NO_ERROR;
    }
    auto ret = window_->Hide();
    if (ret != WMError::WM_OK) {
        IMSA_HILOGE("HidePanel error, err = %{public}d", ret);
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    IMSA_HILOGI("success, type/flag: %{public}d/%{public}d", static_cast<int32_t>(panelType_),
        static_cast<int32_t>(panelFlag_));
    PanelStatusChange(InputWindowStatus::HIDE);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodPanel::SetCallingWindow(uint32_t windowId)
{
    IMSA_HILOGD("InputMethodPanel run in, windowId: %{public}d", windowId);
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto ret = window_->SetCallingWindow(windowId);
    IMSA_HILOGI("ret = %{public}d, windowId = %{public}u", ret, windowId);
    return ret == WMError::WM_OK ? ErrorCode::NO_ERROR : ErrorCode::ERROR_OPERATE_PANEL;
}

int32_t InputMethodPanel::SetPrivacyMode(bool isPrivacyMode)
{
    IMSA_HILOGD("isPrivacyMode: %{public}d", isPrivacyMode);
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto ret = window_->SetPrivacyMode(isPrivacyMode);
    if (ret != WMError::WM_OK) {
        IMSA_HILOGE("SetWindowPrivacyMode error, ret = %{public}d", ret);
        return static_cast<int32_t>(ret);
    }
    IMSA_HILOGI("end, isPrivacyMode: %{public}d", isPrivacyMode);
    return ErrorCode::NO_ERROR;
}

void InputMethodPanel::PanelStatusChange(const InputWindowStatus &status)
{
    if (status == InputWindowStatus::SHOW && showRegistered_ && panelStatusListener_ != nullptr) {
        IMSA_HILOGD("ShowPanel panelStatusListener_ is not nullptr");
        panelStatusListener_->OnPanelStatus(windowId_, true);
    }
    if (status == InputWindowStatus::HIDE && hideRegistered_ && panelStatusListener_ != nullptr) {
        IMSA_HILOGD("HidePanel panelStatusListener_ is not nullptr");
        panelStatusListener_->OnPanelStatus(windowId_, false);
    }
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    if (panelType_ == SOFT_KEYBOARD && (panelFlag_ == FLG_FIXED || panelFlag_ = FLG_FLOATING)) {
        auto rect = window_->GetRect();
        IMSA_HILOGD("InputMethodPanel::x:%{public}d, y:%{public}d, w:%{public}u, h:%{public}u", rect.posX_, rect.posY_,
            rect.width_, rect.height_);
        std::string name = window_->GetWindowName() + "/" + std::to_string(window_->GetWindowId());
        imsa->PanelStatusChange(status, { std::move(name), rect.posX_, rect.posY_, rect.width_, rect.height_ });
    }
}

bool InputMethodPanel::IsShowing()
{
    WindowState windowState = window_->GetWindowState();
    if (windowState == WindowState::STATE_SHOWN) {
        return true;
    }
    IMSA_HILOGD("windowState = %{public}d", static_cast<int>(windowState));
    return false;
}

bool InputMethodPanel::IsHidden()
{
    WindowState windowState = window_->GetWindowState();
    if (windowState == WindowState::STATE_HIDDEN) {
        return true;
    }
    IMSA_HILOGD("windowState = %{public}d", static_cast<int>(windowState));
    return false;
}

int32_t InputMethodPanel::SetUiContent(
    const std::string &contentInfo, napi_env env, std::shared_ptr<NativeReference> storage)
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr, can not SetUiContent.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    WMError ret = WMError::WM_OK;
    if (storage == nullptr) {
        ret = window_->NapiSetUIContent(contentInfo, env, nullptr);
    } else {
        ret = window_->NapiSetUIContent(contentInfo, env, storage->GetNapiValue());
    }
    WMError wmError = window_->SetTransparent(true);
    IMSA_HILOGI("SetTransparent ret = %{public}u", wmError);
    IMSA_HILOGI("NapiSetUIContent ret = %{public}d", ret);
    return ret == WMError::WM_OK ? ErrorCode::NO_ERROR : ErrorCode::ERROR_OPERATE_PANEL;
}

void InputMethodPanel::SetPanelStatusListener(
    std::shared_ptr<PanelStatusListener> statusListener, const std::string &type)
{
    IMSA_HILOGD("SetPanelStatusListener start.");
    if (!MarkListener(type, true)) {
        return;
    }
    if (panelStatusListener_ != nullptr) {
        IMSA_HILOGD("PanelStatusListener already set.");
        return;
    }
    panelStatusListener_ = std::move(statusListener);
    if (window_ != nullptr && IsShowing()) {
        panelStatusListener_->OnPanelStatus(windowId_, true);
    }
}

void InputMethodPanel::ClearPanelListener(const std::string &type)
{
    if (!MarkListener(type, false)) {
        return;
    }
    if (panelStatusListener_ == nullptr) {
        IMSA_HILOGD("PanelStatusListener not set, don't need to remove.");
        return;
    }
    if (showRegistered_ || hideRegistered_) {
        return;
    }
    panelStatusListener_ = nullptr;
}

bool InputMethodPanel::MarkListener(const std::string &type, bool isRegister)
{
    if (type == "show") {
        showRegistered_ = isRegister;
    } else if (type == "hide") {
        hideRegistered_ = isRegister;
    } else {
        IMSA_HILOGE("type error.");
        return false;
    }
    return true;
}

uint32_t InputMethodPanel::GenerateSequenceId()
{
    uint32_t seqId = ++sequenceId_;
    if (seqId == std::numeric_limits<uint32_t>::max()) {
        return ++sequenceId_;
    }
    return seqId;
}

bool InputMethodPanel::IsSizeValid(uint32_t width, uint32_t height)
{
    if (width > INT32_MAX || height > INT32_MAX) {
        IMSA_HILOGE("width or height over maximum");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (defaultDisplay == nullptr) {
        IMSA_HILOGE("GetDefaultDisplay failed.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    float ratio = panelType_ == PanelType::SOFT_KEYBOARD && panelFlag_ == PanelFlag::FLG_FIXED
                      ? FIXED_SOFT_KEYBOARD_PANEL_RATIO
                      : NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO;
    if (static_cast<float>(height) > defaultDisplay->GetHeight() * ratio) {
        IMSA_HILOGE("height invalid, defaultDisplay height = %{public}d, target height = %{public}u",
            defaultDisplay->GetHeight(), height);
        return false;
    }
    if (static_cast<int32_t>(width) > defaultDisplay->GetWidth()) {
        IMSA_HILOGE("width invalid, defaultDisplay width = %{public}d, target width = %{public}u",
            defaultDisplay->GetWidth(), width);
        return false;
    }
    return true;
}

uint32_t InputMethodPanel::GetHeight()
{
    std::lock_guard<std::mutex> lock(heightLock_);
    return panelHeight_;
}
} // namespace MiscServices
} // namespace OHOS
