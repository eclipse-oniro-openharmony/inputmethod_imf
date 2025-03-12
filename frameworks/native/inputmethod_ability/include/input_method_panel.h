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

#ifndef INPUT_METHOD_PANEL_H
#define INPUT_METHOD_PANEL_H

#include <cstdint>
#include <functional>
#include <map>
#include <string>

#include "calling_window_info.h"
#include "display_manager.h"
#include "input_window_info.h"
#include "native_engine/native_engine.h"
#include "panel_common.h"
#include "panel_info.h"
#include "window_change_listener_impl.h"
#include "wm_common.h"

namespace OHOS {
namespace MiscServices {
constexpr int FOLD_TOP = 0;
constexpr int FOLD_LEFT = 0;
constexpr int FOLD_RIGHT = 0;
constexpr int FOLD_BOTTOM = 606;
constexpr int UNFOLD_TOP = 0;
constexpr int UNFOLD_LEFT = 0;
constexpr int UNFOLD_RIGHT = 0;
constexpr int UNFOLD_BOTTOM = 822;
constexpr int COMMON_BOTTOM = 809;
constexpr LayoutParams RESIZE_PANEL_FOLD_PARAMS = { // FoldDefaultValue
    { FOLD_TOP, FOLD_LEFT, FOLD_RIGHT, FOLD_BOTTOM },
    { FOLD_TOP, FOLD_LEFT, FOLD_RIGHT, COMMON_BOTTOM }
};
constexpr LayoutParams RESIZE_PANEL_UNFOLD_PARAMS = { // UnfoldDefaultValue
    { UNFOLD_TOP, UNFOLD_LEFT, UNFOLD_RIGHT, UNFOLD_BOTTOM },
    { UNFOLD_TOP, UNFOLD_LEFT, UNFOLD_RIGHT, COMMON_BOTTOM }
};

class InputMethodPanel {
public:
    static constexpr uint32_t INVALID_WINDOW_ID = 0;
    using CallbackFunc = std::function<void(uint32_t, PanelFlag)>;
    InputMethodPanel() = default;
    ~InputMethodPanel();
    int32_t SetUiContent(const std::string &contentInfo, napi_env env, std::shared_ptr<NativeReference> contentStorage);
    int32_t CreatePanel(const std::shared_ptr<AbilityRuntime::Context> &context, const PanelInfo &panelInfo);
    int32_t DestroyPanel();

    int32_t Resize(uint32_t width, uint32_t height);
    int32_t MoveTo(int32_t x, int32_t y);
    int32_t StartMoving();
    int32_t GetDisplayId(uint64_t &displayId) const;
    int32_t AdjustKeyboard();
    int32_t AdjustPanelRect(const PanelFlag panelFlag, const LayoutParams &layoutParams, bool needUpdateRegion = true);
    int32_t AdjustPanelRect(PanelFlag panelFlag, EnhancedLayoutParams params, HotAreas hotAreas);
    int32_t UpdateRegion(std::vector<Rosen::Rect> region);
    int32_t GetSysPanelAdjust(std::tuple<std::vector<std::string>, std::vector<std::string>> &keys,
        FullPanelAdjustInfo &perConfigInfo);
    int32_t CalculatePanelRect(const PanelFlag panelFlag, const FullPanelAdjustInfo &perConfigInfo,
        const LayoutParams &layoutParams, const DisplaySize &displaySize,
        Rosen::KeyboardLayoutParams &keyboardLayoutParams) const;
    int32_t CalculateLandscapeRect(const DisplaySize &displaySize, const LayoutParams &layoutParams,
        const PanelAdjustInfo &lanIterValue, Rosen::KeyboardLayoutParams &keyboardLayoutParams) const;
    std::tuple<std::vector<std::string>, std::vector<std::string>> GetScreenStatus(const PanelFlag panelFlag) const;
    int32_t ChangePanelFlag(PanelFlag panelFlag);
    PanelType GetPanelType();
    PanelFlag GetPanelFlag();
    int32_t ShowPanel();
    int32_t HidePanel();
    int32_t SizeChange(const WindowSize &size);
    WindowSize GetKeyboardSize();
    bool SetPanelStatusListener(std::shared_ptr<PanelStatusListener> statusListener, const std::string &type);
    void ClearPanelListener(const std::string &type);
    int32_t SetCallingWindow(uint32_t windowId);
    int32_t GetCallingWindowInfo(CallingWindowInfo &windowInfo);
    int32_t SetPrivacyMode(bool isPrivacyMode);
    bool IsShowing();
    int32_t SetTextFieldAvoidInfo(double positionY, double height);
    void SetPanelHeightCallback(CallbackFunc heightCallback);
    int32_t IsEnhancedParamValid(PanelFlag panelFlag, EnhancedLayoutParams &params);
    int32_t SetImmersiveMode(ImmersiveMode mode);
    ImmersiveMode GetImmersiveMode();
    void SetCalingWindowDisplayId(uint64_t displayId);
    uint32_t windowId_ = INVALID_WINDOW_ID;

private:
    class KeyboardPanelInfoChangeListener : public Rosen::IKeyboardPanelInfoChangeListener {
    public:
        using ChangeHandler = std::function<void(const Rosen::KeyboardPanelInfo &keyboardPanelInfo)>;
        explicit KeyboardPanelInfoChangeListener(ChangeHandler handler) : handler_(std::move(handler)) { }
        ~KeyboardPanelInfoChangeListener() = default;
        void OnKeyboardPanelInfoChanged(const Rosen::KeyboardPanelInfo &keyboardPanelInfo) override
        {
            if (handler_ == nullptr) {
                return;
            }
            handler_(keyboardPanelInfo);
        }

    private:
        ChangeHandler handler_ = nullptr;
    };
    void RegisterKeyboardPanelInfoChangeListener();
    void UnregisterKeyboardPanelInfoChangeListener();
    void HandleKbPanelInfoChange(const Rosen::KeyboardPanelInfo &keyboardPanelInfo);
    bool IsHidden();
    int32_t SetPanelProperties(Rosen::KeyboardLayoutParams &keyboardLayoutParams);
    std::string GeneratePanelName();
    void NotifyPanelStatus();
    void PanelStatusChange(const InputWindowStatus &status);
    void PanelStatusChangeToImc(const InputWindowStatus &status, const Rosen::Rect &rect);
    bool MarkListener(const std::string &type, bool isRegister);
    bool SetPanelSizeChangeListener(std::shared_ptr<PanelStatusListener> statusListener);
    std::shared_ptr<PanelStatusListener> GetPanelListener();
    static uint32_t GenerateSequenceId();
    bool IsSizeValid(uint32_t width, uint32_t height, const DisplaySize &displaySize) const;
    bool IsSizeValid(PanelFlag panelFlag, uint32_t width, uint32_t height, int32_t displayWidth,
        int32_t displayHeight) const;
    bool IsRectValid(const Rosen::Rect &rect, const WindowSize &displaySize) const;
    bool CheckSize(PanelFlag panelFlag, const WindowSize &input, const DisplaySize &displaySize,
        bool isDataPortrait) const;
    bool GetDisplaySize(bool isPortrait, WindowSize &size);
    int32_t CalculateFloatRect(const LayoutParams &layoutParams, const FullPanelAdjustInfo &perConfigInfo,
        Rosen::KeyboardLayoutParams &keyboardLayoutParams) const;
    int32_t CalculateNoConfigRect(const PanelFlag panelFlag, const LayoutParams &layoutParams,
        const DisplaySize &displaySize, Rosen::KeyboardLayoutParams &keyboardLayoutParams) const;
    int32_t GetResizeParams(Rosen::Rect &portrait, Rosen::Rect &landscape, const WindowSize &input,
        const DisplaySize &displaySize);

    int32_t ParseEnhancedParams(PanelFlag panelFlag, const FullPanelAdjustInfo &adjustInfo,
        EnhancedLayoutParams &params, const DisplaySize &display) const;
    int32_t RectifyRect(bool isFullScreen, EnhancedLayoutParam &layoutParam, const WindowSize &displaySize,
        PanelFlag panelFlag, const PanelAdjustInfo &adjustInfo) const;
    int32_t CalculateAvoidHeight(EnhancedLayoutParam &layoutParam, const WindowSize &displaySize, PanelFlag panelFlag,
        const PanelAdjustInfo &adjustInfo) const;

    void CalculateHotAreas(const EnhancedLayoutParams &enhancedParams, const Rosen::KeyboardLayoutParams &params,
        const FullPanelAdjustInfo &adjustInfo, HotAreas &hotAreas);
    void CalculateDefaultHotArea(
        const Rosen::Rect &keyboard, const Rosen::Rect &panel, const PanelAdjustInfo &adjustInfo, HotArea &hotArea);
    void CalculateHotArea(
        const Rosen::Rect &keyboard, const Rosen::Rect &panel, const PanelAdjustInfo &adjustInfo, HotArea &hotArea);
    void CalculateEnhancedHotArea(
        const EnhancedLayoutParam &layout, const PanelAdjustInfo &adjustInfo, HotArea &hotArea);
    void RectifyAreas(const std::vector<Rosen::Rect> availableAreas, std::vector<Rosen::Rect> &areas);
    Rosen::Rect GetRectIntersection(Rosen::Rect a, Rosen::Rect b);
    uint32_t SafeSubtract(uint32_t minuend, uint32_t subtrahend);

    int32_t ResizePanel(uint32_t width, uint32_t height, const DisplaySize &displaySize);
    int32_t ResizeWithoutAdjust(uint32_t width, uint32_t height, const DisplaySize &displaySize);
    int32_t ResizeEnhancedPanel(uint32_t width, uint32_t height, const DisplaySize &displaySize);
    void UpdateRectParams(Rosen::Rect &portrait, Rosen::Rect &landscape,
        const WindowSize &input, const LayoutParams &currParams, const DisplaySize &displaySize) const;
    int32_t MovePanelRect(int32_t x, int32_t y, const DisplaySize &displaySize);
    int32_t MoveEnhancedPanelRect(int32_t x, int32_t y, const DisplaySize &displaySize);

    bool IsDisplayPortrait();
    bool IsDisplayUnfolded();
    int32_t GetDisplaySize(DisplaySize &size);
    int32_t GetDensityDpi(float &densityDpi);
    int32_t InitAdjustInfo(const DisplaySize &displaySize);
    int32_t GetAdjustInfo(PanelFlag panelFlag, FullPanelAdjustInfo &fullPanelAdjustInfo,
        const DisplaySize &displaySize);

    void UpdateResizeParams(const DisplaySize &displaySize);
    void UpdateHotAreas(const DisplaySize &displaySize);
    void UpdateLayoutInfo(PanelFlag panelFlag,  const LayoutParams &params, const DisplaySize &displaySize);
    void UpdateEnhancedLayoutInfo(PanelFlag panelFlag, const EnhancedLayoutParams &enhancedParams,
        const  Rosen::KeyboardLayoutParams &wmsParams, const DisplaySize &displaySize);
    Rosen::KeyboardLayoutParams ConvertToWMSParam(PanelFlag panelFlag, const EnhancedLayoutParams &layoutParams) const;
    Rosen::KeyboardTouchHotAreas ConvertToWMSHotArea(const HotAreas &hotAreas) const;
    void OnPanelHeightChange(const Rosen::KeyboardPanelInfo &keyboardPanelInfo);
    int32_t GetKeyboardArea(PanelFlag panelFlag, const WindowSize &size, PanelAdjustInfo &keyboardArea);
    int32_t GetWindowOrientation(PanelFlag panelFlag, uint32_t windowWidth, bool &isPortrait,
        const DisplaySize &displaySize);

    void SetHotAreas(const HotAreas &hotAreas, const DisplaySize &displaySize);
    HotAreas GetHotAreas(const DisplaySize &displaySize);

    sptr<Rosen::Display> GetCurDisplay() const;
    static std::string KeyboardLayoutParamsToString(const Rosen::KeyboardLayoutParams &input);
    int32_t CalculateLayoutRect(const PanelFlag panelFlag, const FullPanelAdjustInfo &perConfigInfo,
        const LayoutParams &layoutParams, const DisplaySize &displaySize,
        Rosen::KeyboardLayoutParams &keyboardLayoutParams) const;
    void SetEnhancedLayoutParams(const EnhancedLayoutParams &enhancedLayout, const DisplaySize &displaySize);
    EnhancedLayoutParams GetEnhancedLayoutParams(const DisplaySize &displaySize);
    void SetKeyboardLayoutParams(const Rosen::KeyboardLayoutParams &params, const DisplaySize &displaySize);
    Rosen::KeyboardLayoutParams GetKeyboardLayoutParams(const DisplaySize &displaySize);

private:
    sptr<OHOS::Rosen::Window> window_ = nullptr;
    sptr<OHOS::Rosen::WindowOption> winOption_ = nullptr;
    PanelType panelType_ = PanelType::SOFT_KEYBOARD;
    PanelFlag panelFlag_ = PanelFlag::FLG_FIXED;
    bool showRegistered_ = false;
    bool hideRegistered_ = false;
    bool sizeChangeRegistered_ = false;
    bool sizeUpdateRegistered_ = false;
    uint32_t invalidGravityPercent = 0;
    std::shared_ptr<PanelStatusListener> panelStatusListener_ = nullptr;

    static std::atomic<uint32_t> sequenceId_;
    sptr<Rosen::IKeyboardPanelInfoChangeListener> kbPanelInfoListener_ { nullptr };
    bool isScbEnable_ { false };

    std::mutex panelAdjustLock_;
    std::map<std::vector<std::string>, PanelAdjustInfo> panelAdjust_;
    std::mutex adjustInfoInitLock_;
    std::atomic<bool> isAdjustInfoInitialized_{ false };

    PanelParamsCache<OHOS::Rosen::DisplayId, HotAreas> hotAreas_;
    PanelParamsCache<OHOS::Rosen::DisplayId, EnhancedLayoutParams> enhancedLayoutParams_;
    PanelParamsCache<OHOS::Rosen::DisplayId, Rosen::KeyboardLayoutParams> keyboardLayoutParams_;

    std::mutex keyboardSizeLock_;
    WindowSize keyboardSize_ { 0, 0 };
    std::mutex windowListenerLock_;
    sptr<Rosen::IWindowChangeListener> windowChangedListener_ = nullptr;
    CallbackFunc panelHeightCallback_ = nullptr;
    LayoutParams resizePanelFoldParams_ = RESIZE_PANEL_FOLD_PARAMS;
    LayoutParams resizePanelUnfoldParams_ = RESIZE_PANEL_UNFOLD_PARAMS;
    std::atomic<bool> isWaitSetUiContent_ { true };
    std::atomic<bool> isInEnhancedAdjust_{ false };
    ImmersiveMode immersiveMode_ { ImmersiveMode::NONE_IMMERSIVE };
    uint64_t callingWindowDisplayId_ { 0 };
    uint64_t defDisplayId_ { OHOS::Rosen::DISPLAY_ID_INVALID };
};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUT_METHOD_PANEL_H
