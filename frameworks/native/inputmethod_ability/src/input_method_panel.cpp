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

#include <tuple>

#include "dm_common.h"
#include "global.h"
#include "input_method_ability_utils.h"
#include "scene_board_judgement.h"
#include "sys_cfg_parser.h"
#include "ui/rs_surface_node.h"

namespace OHOS {
namespace MiscServices {
using WMError = OHOS::Rosen::WMError;
using WindowGravity = OHOS::Rosen::WindowGravity;
using WindowState = OHOS::Rosen::WindowState;
using namespace Rosen;
constexpr float FIXED_SOFT_KEYBOARD_PANEL_RATIO = 0.7;
constexpr float NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO = 1;
constexpr int32_t NUMBER_ZERO = 0;
constexpr int32_t NUMBER_TWO = 2;
constexpr int32_t CUTOUTINFO = 100;
constexpr float DPI_CALCULATION_RATIO = 160.0;
std::atomic<uint32_t> InputMethodPanel::sequenceId_{ 0 };
InputMethodPanel::~InputMethodPanel() = default;

int32_t InputMethodPanel::CreatePanel(const std::shared_ptr<AbilityRuntime::Context> &context,
    const PanelInfo &panelInfo)
{
    IMSA_HILOGD("start, type/flag: %{public}d/%{public}d", static_cast<int32_t>(panelType_),
        static_cast<int32_t>(panelFlag_));
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
    if (panelInfo.panelType == SOFT_KEYBOARD) {
        isScbEnable_ = Rosen::SceneBoardJudgement::IsSceneBoardEnabled();
        if (isScbEnable_) {
            RegisterKeyboardPanelInfoChangeListener();
        }
    }
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
    if (panelType_ == SOFT_KEYBOARD) {
        UnregisterKeyboardPanelInfoChangeListener();
    }
    auto result = window_->Destroy();
    IMSA_HILOGI("ret = %{public}d", result);
    return ErrorCode::NO_ERROR;
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
    return ret == WMError::WM_ERROR_INVALID_PARAM ? ErrorCode::ERROR_PARAMETER_CHECK_FAILED : ErrorCode::NO_ERROR;
}

int32_t InputMethodPanel::AdjustPanelRect(const PanelFlag panelFlag, const LayoutParams &layoutParams)
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window is nullptr");
        return ErrorCode::ERROR_WINDOW_MANAGER;
    }
    if (layoutParams.portraitRect.posX_ < 0 || layoutParams.portraitRect.posY_ < 0 ||
        layoutParams.landscapeRect.posX_ < 0 || layoutParams.landscapeRect.posY_ < 0) {
        IMSA_HILOGE("posX_ and posY_ cannot be less than 0");
        return ErrorCode::ERROR_PARAMETER_CHECK_FAILED;
    }
    if (!CheckSize(panelFlag, layoutParams.portraitRect.width_, layoutParams.portraitRect.height_, true)) {
        IMSA_HILOGE("Portrait invalid size");
        return ErrorCode::ERROR_PARAMETER_CHECK_FAILED;
    }
    if (!CheckSize(panelFlag, layoutParams.landscapeRect.width_, layoutParams.landscapeRect.height_, false)) {
        IMSA_HILOGE("Landscape invalid size");
        return ErrorCode::ERROR_PARAMETER_CHECK_FAILED;
    }
    auto result = ParsePanelRect(panelFlag, layoutParams);
    if (result != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("ParsePanelRect failed result = %{public}d", result);
        return ErrorCode::ERROR_WINDOW_MANAGER;
    }
    auto ret = window_->AdjustKeyboardLayout(keyboardLayoutParams_);
    if (ret != WMError::WM_OK) {
        IMSA_HILOGE("AdjustPanelRect error, err = %{public}d", ret);
        return ErrorCode::ERROR_WINDOW_MANAGER;
    }
    IMSA_HILOGI("success, type/flag: %{public}d/%{public}d", static_cast<int32_t>(panelType_),
        static_cast<int32_t>(panelFlag_));
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodPanel::ParsePanelRect(const PanelFlag panelFlag, const LayoutParams &layoutParams)
{
    std::vector<SysPanelAdjust> configs;
    auto isSuccess = SysCfgParser::ParsePanelAdjust(configs);
    if (isSuccess) {
        std::lock_guard<std::mutex> lk(panelAdjustLock_);
        panelAdjust_.clear();
        for (const auto &config : configs) {
            panelAdjust_.insert({ config.style, { config.top, config.left, config.right, config.bottom } });
        }
    } else {
        IMSA_HILOGE("There is no configuration file.");
        auto ret = CalculateNoConfigRect(panelFlag, layoutParams);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("CalculateNoConfigRect failed, err = %{public}d", ret);
            return ret;
        }
        return ErrorCode::NO_ERROR;
    }
    std::tuple<std::vector<std::string>, std::vector<std::string>> keys = GetScreenStatus(panelFlag);
    auto ret = GetSysPanelAdjust(panelFlag, keys, layoutParams);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("GetSysPanelAdjust failed");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodPanel::CalculateNoConfigRect(const PanelFlag panelFlag, const LayoutParams &layoutParams)
{
    if (panelFlag == PanelFlag::FLG_FIXED) {
        keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
        WindowSize portraitDisplaySize;
        if (!GetDisplaySize(true, portraitDisplaySize)) {
            IMSA_HILOGE("GetPortraitDisplaySize failed.");
            return ErrorCode::ERROR_WINDOW_MANAGER;
        }
        keyboardLayoutParams_.PortraitPanelRect_.width_ = portraitDisplaySize.width;
        keyboardLayoutParams_.PortraitPanelRect_.height_ = layoutParams.portraitRect.height_;
        keyboardLayoutParams_.PortraitPanelRect_.posY_ =
            static_cast<int32_t>(portraitDisplaySize.height - keyboardLayoutParams_.PortraitPanelRect_.height_);
        keyboardLayoutParams_.PortraitPanelRect_.posX_ = NUMBER_ZERO;
        //fixed Portraitkeyboard
        keyboardLayoutParams_.PortraitKeyboardRect_.width_ = keyboardLayoutParams_.PortraitPanelRect_.width_;
        keyboardLayoutParams_.PortraitKeyboardRect_.height_ = keyboardLayoutParams_.PortraitPanelRect_.height_;
        keyboardLayoutParams_.PortraitKeyboardRect_.posY_ = keyboardLayoutParams_.PortraitPanelRect_.posY_;
        keyboardLayoutParams_.PortraitKeyboardRect_.posX_ = keyboardLayoutParams_.PortraitPanelRect_.posX_;

        WindowSize landscapeDisplaySize;
        if (!GetDisplaySize(false, landscapeDisplaySize)) {
            IMSA_HILOGE("GetLandscapeDisplaySize failed.");
            return ErrorCode::ERROR_WINDOW_MANAGER;
        }
        keyboardLayoutParams_.LandscapePanelRect_.width_ = landscapeDisplaySize.width;
        keyboardLayoutParams_.LandscapePanelRect_.height_ = layoutParams.landscapeRect.height_;
        keyboardLayoutParams_.LandscapePanelRect_.posY_ =
            static_cast<int32_t>(landscapeDisplaySize.height - keyboardLayoutParams_.LandscapePanelRect_.height_);
        keyboardLayoutParams_.LandscapePanelRect_.posX_ = NUMBER_ZERO;
        //Landscapekeyboard
        keyboardLayoutParams_.LandscapeKeyboardRect_.width_ = keyboardLayoutParams_.LandscapePanelRect_.width_;
        keyboardLayoutParams_.LandscapeKeyboardRect_.height_ = keyboardLayoutParams_.LandscapePanelRect_.height_;
        keyboardLayoutParams_.LandscapeKeyboardRect_.posY_ = keyboardLayoutParams_.LandscapePanelRect_.posY_;
        keyboardLayoutParams_.LandscapeKeyboardRect_.posX_ = keyboardLayoutParams_.LandscapePanelRect_.posX_;
    } else {
        keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
        keyboardLayoutParams_.LandscapeKeyboardRect_ = layoutParams.landscapeRect;
        keyboardLayoutParams_.PortraitKeyboardRect_ = layoutParams.portraitRect;
        keyboardLayoutParams_.LandscapePanelRect_ = layoutParams.landscapeRect;
        keyboardLayoutParams_.PortraitPanelRect_ = layoutParams.portraitRect;
    }
    return ErrorCode::NO_ERROR;
}

std::tuple<std::vector<std::string>, std::vector<std::string>> InputMethodPanel::GetScreenStatus(
    const PanelFlag panelFlag)
{
    std::lock_guard<std::mutex> lock(panelAdjustLock_);
    std::string flag;
    std::string foldStatus = "default";
    if (panelFlag == PanelFlag::FLG_FIXED) {
        flag = "fix";
        keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    } else {
        flag = "floating";
        keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    }
    if (Rosen::DisplayManager::GetInstance().IsFoldable() &&
        Rosen::DisplayManager::GetInstance().GetFoldStatus() != Rosen::FoldStatus::FOLDED) {
        foldStatus = "foldable";
    }
    std::vector<std::string> lanPanel = { flag, foldStatus, "landscape" };
    std::vector<std::string> porPanel = { flag, foldStatus, "portrait" };
    return std::make_tuple(lanPanel, porPanel);
}

int32_t InputMethodPanel::GetSysPanelAdjust(const PanelFlag panelFlag,
    std::tuple<std::vector<std::string>, std::vector<std::string>> &keys, const LayoutParams &layoutParams)
{
    std::lock_guard<std::mutex> lock(panelAdjustLock_);
    auto lanPanel = std::get<0>(keys);
    auto porPanel = std::get<1>(keys);
    auto lanIter = panelAdjust_.find(lanPanel);
    auto porIter = panelAdjust_.find(porPanel);
    if (lanIter == panelAdjust_.end() || porIter == panelAdjust_.end()) {
        IMSA_HILOGE("lanIter or porIter not supported");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto lanIterValue = lanIter->second;
    auto porIterValue = porIter->second;
    keyboardLayoutParams_.LandscapeKeyboardRect_ = layoutParams.landscapeRect;
    keyboardLayoutParams_.PortraitKeyboardRect_ = layoutParams.portraitRect;
    keyboardLayoutParams_.LandscapePanelRect_ = layoutParams.landscapeRect;
    keyboardLayoutParams_.PortraitPanelRect_ = layoutParams.portraitRect;
    return CalculatePanelRect(panelFlag, lanIterValue, porIterValue, layoutParams);
}

int32_t InputMethodPanel::CalculatePanelRect(const PanelFlag panelFlag, PanelAdjustInfo &lanIterValue,
    PanelAdjustInfo &porIterValue, const LayoutParams &layoutParams)
{
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (defaultDisplay == nullptr) {
        IMSA_HILOGE("GetDefaultDisplay failed.");
        return ErrorCode::ERROR_EX_SERVICE_SPECIFIC;
    }
    auto densityDpi = defaultDisplay->GetDpi();
    if (panelFlag == PanelFlag::FLG_FIXED) {
        //fixed PortraitPanel
        WindowSize portraitDisplaySize;
        if (!GetDisplaySize(true, portraitDisplaySize)) {
            IMSA_HILOGE("GetDisplaySize failed.");
            return ErrorCode::ERROR_WINDOW_MANAGER;
        }
        keyboardLayoutParams_.PortraitPanelRect_.width_ = portraitDisplaySize.width;
        keyboardLayoutParams_.PortraitPanelRect_.height_ =
            layoutParams.portraitRect.height_ +
            static_cast<uint32_t>((porIterValue.top + porIterValue.bottom) * (densityDpi / DPI_CALCULATION_RATIO));
        if (keyboardLayoutParams_.PortraitPanelRect_.height_ >
            portraitDisplaySize.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO) {
            keyboardLayoutParams_.PortraitPanelRect_.height_ =
                portraitDisplaySize.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO;
        }
        keyboardLayoutParams_.PortraitPanelRect_.posY_ =
            static_cast<int32_t>(portraitDisplaySize.height - keyboardLayoutParams_.PortraitPanelRect_.height_);
        keyboardLayoutParams_.PortraitPanelRect_.posX_ = NUMBER_ZERO;
        //fixed Portraitkeyboard
        keyboardLayoutParams_.PortraitKeyboardRect_.width_ =
            keyboardLayoutParams_.PortraitPanelRect_.width_ -
            static_cast<uint32_t>((porIterValue.left + porIterValue.right) * (densityDpi / DPI_CALCULATION_RATIO));
        keyboardLayoutParams_.PortraitKeyboardRect_.height_ =
            keyboardLayoutParams_.PortraitPanelRect_.height_ -
            static_cast<uint32_t>((porIterValue.top + porIterValue.bottom) * (densityDpi / DPI_CALCULATION_RATIO));
        keyboardLayoutParams_.PortraitKeyboardRect_.posY_ =
            keyboardLayoutParams_.PortraitPanelRect_.posY_ + porIterValue.top * (densityDpi / DPI_CALCULATION_RATIO);
        keyboardLayoutParams_.PortraitKeyboardRect_.posX_ =
            keyboardLayoutParams_.PortraitPanelRect_.posX_ + porIterValue.left * (densityDpi / DPI_CALCULATION_RATIO);
        return CalculateLandscapeRect(defaultDisplay, layoutParams, lanIterValue, densityDpi);
    }
    CalculateFloatRect(layoutParams, lanIterValue, porIterValue, densityDpi);
    return ErrorCode::NO_ERROR;
}

void InputMethodPanel::CalculateFloatRect(const LayoutParams &layoutParams, PanelAdjustInfo &lanIterValue,
    PanelAdjustInfo &porIterValue, int densityDpi)
{
    //portrait floating keyboard
    keyboardLayoutParams_.PortraitKeyboardRect_.width_ = layoutParams.portraitRect.width_;
    keyboardLayoutParams_.PortraitKeyboardRect_.height_ = layoutParams.portraitRect.height_;
    keyboardLayoutParams_.PortraitKeyboardRect_.posY_ = layoutParams.portraitRect.posY_;
    keyboardLayoutParams_.PortraitKeyboardRect_.posX_ = layoutParams.portraitRect.posX_;
    //portrait floating panel
    keyboardLayoutParams_.PortraitPanelRect_.width_ =
        keyboardLayoutParams_.PortraitKeyboardRect_.width_ +
        static_cast<uint32_t>((porIterValue.left + porIterValue.right) * (densityDpi / DPI_CALCULATION_RATIO));
    keyboardLayoutParams_.PortraitPanelRect_.height_ =
        keyboardLayoutParams_.PortraitKeyboardRect_.height_ +
        static_cast<uint32_t>((porIterValue.top + porIterValue.bottom) * (densityDpi / DPI_CALCULATION_RATIO));
    keyboardLayoutParams_.PortraitPanelRect_.posY_ =
        keyboardLayoutParams_.PortraitKeyboardRect_.posY_ - porIterValue.top * (densityDpi / DPI_CALCULATION_RATIO);
    keyboardLayoutParams_.PortraitPanelRect_.posX_ =
        keyboardLayoutParams_.PortraitKeyboardRect_.posX_ - porIterValue.left * (densityDpi / DPI_CALCULATION_RATIO);

    //landscape floating keyboard
    keyboardLayoutParams_.LandscapeKeyboardRect_.width_ = layoutParams.landscapeRect.width_;
    keyboardLayoutParams_.LandscapeKeyboardRect_.height_ = layoutParams.landscapeRect.height_;
    keyboardLayoutParams_.LandscapeKeyboardRect_.posY_ = layoutParams.landscapeRect.posY_;
    keyboardLayoutParams_.LandscapeKeyboardRect_.posX_ = layoutParams.landscapeRect.posX_;
    //landscape floating panel
    keyboardLayoutParams_.LandscapePanelRect_.width_ =
        keyboardLayoutParams_.LandscapeKeyboardRect_.width_ +
        static_cast<uint32_t>((lanIterValue.left + lanIterValue.right) * (densityDpi / DPI_CALCULATION_RATIO));
    keyboardLayoutParams_.LandscapePanelRect_.height_ =
        keyboardLayoutParams_.LandscapeKeyboardRect_.height_ +
        static_cast<uint32_t>((lanIterValue.top + lanIterValue.bottom) * (densityDpi / DPI_CALCULATION_RATIO));
    keyboardLayoutParams_.LandscapePanelRect_.posY_ =
        keyboardLayoutParams_.LandscapeKeyboardRect_.posY_ - lanIterValue.top * (densityDpi / DPI_CALCULATION_RATIO);
    keyboardLayoutParams_.LandscapePanelRect_.posX_ =
        keyboardLayoutParams_.LandscapeKeyboardRect_.posX_ - lanIterValue.left * (densityDpi / DPI_CALCULATION_RATIO);
}

int32_t InputMethodPanel::CalculateLandscapeRect(sptr<OHOS::Rosen::Display> &defaultDisplay,
    const LayoutParams &layoutParams, PanelAdjustInfo &lanIterValue, int densityDpi)
{
    //LandscapePanel
    WindowSize landscapeDisplaySize;
    if (!GetDisplaySize(false, landscapeDisplaySize)) {
        IMSA_HILOGE("GetDisplaySize failed.");
        return ErrorCode::ERROR_WINDOW_MANAGER;
    }
    keyboardLayoutParams_.LandscapePanelRect_.width_ = landscapeDisplaySize.width;
    keyboardLayoutParams_.LandscapePanelRect_.height_ =
        layoutParams.landscapeRect.height_ +
        static_cast<uint32_t>((lanIterValue.top + lanIterValue.bottom) * (densityDpi / DPI_CALCULATION_RATIO));
    if (keyboardLayoutParams_.LandscapePanelRect_.height_ >
        landscapeDisplaySize.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO) {
        keyboardLayoutParams_.LandscapePanelRect_.height_ =
            landscapeDisplaySize.height * FIXED_SOFT_KEYBOARD_PANEL_RATIO;
    }
    keyboardLayoutParams_.LandscapePanelRect_.posY_ =
        static_cast<int32_t>(landscapeDisplaySize.height - keyboardLayoutParams_.LandscapePanelRect_.height_);
    keyboardLayoutParams_.LandscapePanelRect_.posX_ = NUMBER_ZERO;
    //Landscapekeyboard
    keyboardLayoutParams_.LandscapeKeyboardRect_.width_ =
        keyboardLayoutParams_.LandscapePanelRect_.width_ -
        static_cast<uint32_t>((lanIterValue.left + lanIterValue.right) * (densityDpi / DPI_CALCULATION_RATIO));
    keyboardLayoutParams_.LandscapeKeyboardRect_.height_ =
        keyboardLayoutParams_.LandscapePanelRect_.height_ -
        static_cast<uint32_t>((lanIterValue.top + lanIterValue.bottom) * (densityDpi / DPI_CALCULATION_RATIO));
    keyboardLayoutParams_.LandscapeKeyboardRect_.posY_ =
        keyboardLayoutParams_.LandscapePanelRect_.posY_ + lanIterValue.top * (densityDpi / DPI_CALCULATION_RATIO);
    keyboardLayoutParams_.LandscapeKeyboardRect_.posX_ =
        keyboardLayoutParams_.LandscapePanelRect_.posX_ + lanIterValue.left * (densityDpi / DPI_CALCULATION_RATIO);
    sptr<Rosen::CutoutInfo> cutoutInfo = defaultDisplay->GetCutoutInfo();
    if (cutoutInfo != nullptr) {
        if (Rosen::DisplayManager::GetInstance().IsFoldable() &&
            Rosen::DisplayManager::GetInstance().GetFoldStatus() != Rosen::FoldStatus::FOLDED) {
            return ErrorCode::NO_ERROR;
        }
        keyboardLayoutParams_.LandscapeKeyboardRect_.width_ = keyboardLayoutParams_.LandscapeKeyboardRect_.width_ -
            (CUTOUTINFO - lanIterValue.left) * NUMBER_TWO  * (densityDpi / DPI_CALCULATION_RATIO);
        keyboardLayoutParams_.LandscapeKeyboardRect_.posX_ = keyboardLayoutParams_.LandscapeKeyboardRect_.posX_ +
            (CUTOUTINFO - lanIterValue.left) * (densityDpi / DPI_CALCULATION_RATIO);
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
    WindowGravity gravity = WindowGravity::WINDOW_GRAVITY_FLOAT;
    if (panelFlag == FLG_FIXED) {
        gravity = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    } else {
        window_->GetSurfaceNode()->SetFrameGravity(Rosen::Gravity::TOP_LEFT);
        Rosen::RSTransactionProxy::GetInstance()->FlushImplicitTransaction();
    }
    auto ret = window_->SetWindowGravity(gravity, invalidGravityPercent);
    if (ret == WMError::WM_OK) {
        panelFlag_ = panelFlag;
    }
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
    if (!isScbEnable_) {
        PanelStatusChangeToImc(InputWindowStatus::SHOW, window_->GetRect());
    }
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
    if (!isScbEnable_) {
        PanelStatusChangeToImc(InputWindowStatus::HIDE, { 0, 0, 0, 0 });
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodPanel::SetCallingWindow(uint32_t windowId)
{
    IMSA_HILOGD("InputMethodPanel run in, windowId: %{public}d", windowId);
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr.");
        return ErrorCode::ERROR_PANEL_NOT_FOUND;
    }
    auto ret = window_->SetCallingWindow(windowId);
    IMSA_HILOGI("ret = %{public}d, windowId = %{public}u", ret, windowId);
    return ret == WMError::WM_OK ? ErrorCode::NO_ERROR : ErrorCode::ERROR_WINDOW_MANAGER;
}

int32_t InputMethodPanel::GetCallingWindowInfo(CallingWindowInfo &windowInfo)
{
    IMSA_HILOGD("InputMethodPanel in");
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ is nullptr");
        return ErrorCode::ERROR_PANEL_NOT_FOUND;
    }
    auto ret = window_->GetCallingWindowWindowStatus(windowInfo.status);
    if (ret != WMError::WM_OK) {
        IMSA_HILOGE("get status failed, ret: %{public}d", ret);
        return ErrorCode::ERROR_WINDOW_MANAGER;
    }
    ret = window_->GetCallingWindowRect(windowInfo.rect);
    if (ret != WMError::WM_OK) {
        IMSA_HILOGE("get rect failed, ret: %{public}d", ret);
        return ErrorCode::ERROR_WINDOW_MANAGER;
    }
    IMSA_HILOGI("status: %{public}d, rect[x/y/w/h]: [%{public}d/%{public}d/%{public}d/%{public}d]",
        static_cast<uint32_t>(windowInfo.status), windowInfo.rect.posX_, windowInfo.rect.posY_, windowInfo.rect.width_,
        windowInfo.rect.height_);
    return ErrorCode::NO_ERROR;
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
}

void InputMethodPanel::PanelStatusChangeToImc(const InputWindowStatus &status, const Rosen::Rect &rect)
{
    ImeWindowInfo info;
    info.panelInfo = { panelType_, panelFlag_ };
    if (info.panelInfo.panelType != SOFT_KEYBOARD || info.panelInfo.panelFlag == FLG_CANDIDATE_COLUMN) {
        IMSA_HILOGD("no need to deal");
        return;
    }
    auto imsa = ImaUtils::GetImsaProxy();
    if (imsa == nullptr) {
        IMSA_HILOGE("imsa is nullptr");
        return;
    }
    std::string name = window_->GetWindowName() + "/" + std::to_string(window_->GetWindowId());
    info.windowInfo = { std::move(name), rect.posX_, rect.posY_, rect.width_, rect.height_ };
    IMSA_HILOGD("rect[%{public}d,%{public}d,%{public}u,%{public}u], status:%{public}d, "
                "panelFlag:%{public}d",
        rect.posX_, rect.posY_, rect.width_, rect.height_, status, info.panelInfo.panelFlag);
    imsa->PanelStatusChange(status, info);
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

int32_t InputMethodPanel::SetUiContent(const std::string &contentInfo, napi_env env,
    std::shared_ptr<NativeReference> storage)
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
    return ret == WMError::WM_ERROR_INVALID_PARAM ? ErrorCode::ERROR_PARAMETER_CHECK_FAILED : ErrorCode::NO_ERROR;
}

bool InputMethodPanel::SetPanelStatusListener(std::shared_ptr<PanelStatusListener> statusListener,
    const std::string &type)
{
    if (!MarkListener(type, true)) {
        return false;
    }
    IMSA_HILOGD("type: %{public}s", type.c_str());
    if (type == "show" || type == "hide") {
        if (panelStatusListener_ == nullptr) {
            IMSA_HILOGD("panelStatusListener_ need to be set");
            panelStatusListener_ = std::move(statusListener);
        }
        if (window_ != nullptr) {
            if (type == "show" && IsShowing()) {
                panelStatusListener_->OnPanelStatus(windowId_, true);
            }
            if (type == "hide" && IsHidden()) {
                panelStatusListener_->OnPanelStatus(windowId_, false);
            }
        }
    }
    if (panelType_ == PanelType::SOFT_KEYBOARD &&
        (panelFlag_ == PanelFlag::FLG_FIXED || panelFlag_ == PanelFlag::FLG_FLOATING) && type == "sizeChange") {
        if (panelStatusListener_ == nullptr && statusListener != nullptr) {
            panelStatusListener_ = std::move(statusListener);
        }
        std::lock_guard<std::mutex> lock(windowListenerLock_);
        if (windowChangedListener_ != nullptr) {
            IMSA_HILOGD("windowChangedListener already registered");
            return true;
        }
        windowChangedListener_ = new (std::nothrow)
            WindowChangeListenerImpl([this](WindowSize windowSize) { SizeChange(windowSize); });
        if (windowChangedListener_ == nullptr || window_ == nullptr) {
            IMSA_HILOGE("observer or window_ is nullptr");
            return false;
        }
        auto ret = window_->RegisterWindowChangeListener(windowChangedListener_);
        if (ret != WMError::WM_OK) {
            IMSA_HILOGE("RegisterWindowChangeListener error, err = %{public}d", ret);
            return false;
        }
    }
    return true;
}

void InputMethodPanel::ClearPanelListener(const std::string &type)
{
    if (!MarkListener(type, false)) {
        return;
    }
    IMSA_HILOGD("type: %{public}s", type.c_str());
    if (type == "sizeChange" && windowChangedListener_ != nullptr && window_ != nullptr) {
        auto ret = window_->UnregisterWindowChangeListener(windowChangedListener_);
        IMSA_HILOGI("UnregisterWindowChangeListener ret = %{public}d", ret);
        windowChangedListener_ = nullptr;
    }
    if (panelStatusListener_ == nullptr) {
        IMSA_HILOGD("PanelStatusListener not set, don't need to remove.");
        return;
    }
    if (showRegistered_ || hideRegistered_ || sizeChangeRegistered_) {
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
    } else if (type == "sizeChange") {
        sizeChangeRegistered_ = isRegister;
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
        return false;
    }
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (defaultDisplay == nullptr) {
        IMSA_HILOGE("GetDefaultDisplay failed.");
        return false;
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

int32_t InputMethodPanel::SizeChange(const WindowSize &size)
{
    IMSA_HILOGD("InputMethodPanel, run in");
    IMSA_HILOGI("type/flag: %{public}d/%{public}d, width/height: %{public}d/%{public}d",
        static_cast<int32_t>(panelType_), static_cast<int32_t>(panelFlag_), static_cast<int32_t>(size.width),
        static_cast<int32_t>(size.height));
    panelStatusListener_->OnSizeChange(windowId_, size);
    return ErrorCode::NO_ERROR;
}

void InputMethodPanel::RegisterKeyboardPanelInfoChangeListener()
{
    kbPanelInfoListener_ = new (std::nothrow)
        KeyboardPanelInfoChangeListener([this](const KeyboardPanelInfo &keyboardPanelInfo) {
            if (panelHeightCallback_ != nullptr) {
                panelHeightCallback_(keyboardPanelInfo.rect_.height_, panelFlag_);
            }
            HandleKbPanelInfoChange(keyboardPanelInfo);
        });
    if (kbPanelInfoListener_ == nullptr) {
        return;
    }
    if (window_ == nullptr) {
        return;
    }
    auto ret = window_->RegisterKeyboardPanelInfoChangeListener(kbPanelInfoListener_);
    IMSA_HILOGD("ret: %{public}d", ret);
}

void InputMethodPanel::UnregisterKeyboardPanelInfoChangeListener()
{
    if (window_ == nullptr) {
        return;
    }
    auto ret = window_->UnregisterKeyboardPanelInfoChangeListener(kbPanelInfoListener_);
    kbPanelInfoListener_ = nullptr;
    IMSA_HILOGD("ret: %{public}d", ret);
}

void InputMethodPanel::HandleKbPanelInfoChange(const KeyboardPanelInfo &keyboardPanelInfo)
{
    IMSA_HILOGD("run in");
    InputWindowStatus status = InputWindowStatus::HIDE;
    if (keyboardPanelInfo.isShowing_) {
        status = InputWindowStatus::SHOW;
    }
    PanelStatusChangeToImc(status, keyboardPanelInfo.rect_);
}

bool InputMethodPanel::GetDisplaySize(bool isPortrait, WindowSize &size)
{
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (defaultDisplay == nullptr) {
        IMSA_HILOGE("GetDefaultDisplay failed.");
        return false;
    }
    auto width = defaultDisplay->GetWidth();
    auto height = defaultDisplay->GetHeight();
    bool isDisplayPortrait = width < height;
    if (isPortrait != isDisplayPortrait) {
        size = { .width = height, .height = width };
    } else {
        size = { .width = width, .height = height };
    }
    return true;
}

bool InputMethodPanel::CheckSize(PanelFlag panelFlag, uint32_t width, uint32_t height, bool isDataPortrait)
{
    WindowSize displaySize;
    if (!GetDisplaySize(isDataPortrait, displaySize)) {
        IMSA_HILOGE("GetDisplaySize failed.");
        return false;
    }
    return IsSizeValid(panelFlag, width, height, displaySize.width, displaySize.height);
}

bool InputMethodPanel::IsSizeValid(PanelFlag panelFlag, uint32_t width, uint32_t height, int32_t displayWidth,
    int32_t displayHeight)
{
    if (width > INT32_MAX || height > INT32_MAX) {
        IMSA_HILOGE("width or height over maximum");
        return false;
    }
    float ratio = panelType_ == PanelType::SOFT_KEYBOARD && panelFlag == PanelFlag::FLG_FIXED
                      ? FIXED_SOFT_KEYBOARD_PANEL_RATIO
                      : NON_FIXED_SOFT_KEYBOARD_PANEL_RATIO;
    if (static_cast<float>(height) > displayHeight * ratio) {
        IMSA_HILOGE("height invalid, defaultDisplay height = %{public}d, target height = %{public}u", displayHeight,
            height);
        return false;
    }
    if (static_cast<int32_t>(width) > displayWidth) {
        IMSA_HILOGE("width invalid, defaultDisplay width = %{public}d, target width = %{public}u", displayWidth, width);
        return false;
    }
    return true;
}

void InputMethodPanel::SetPanelHeightCallback(CallbackFunc heightCallback)
{
    panelHeightCallback_ = std::move(heightCallback);
}
} // namespace MiscServices
} // namespace OHOS
