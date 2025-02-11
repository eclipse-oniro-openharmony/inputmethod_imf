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
#include "js_runtime_utils.h"
#include "panel_info.h"
#include "panel_status_listener.h"
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

struct LayoutParams {
    Rosen::Rect landscapeRect;
    Rosen::Rect portraitRect;
};

struct PanelAdjustInfo {
    int32_t top;
    int32_t left;
    int32_t right;
    int32_t bottom;
    bool operator==(const PanelAdjustInfo &panelAdjust) const
    {
        return (top == panelAdjust.top && left == panelAdjust.left && right == panelAdjust.right &&
                bottom == panelAdjust.bottom);
    }
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
    int32_t GetDisplayId(uint64_t &displayId);
    int32_t AdjustPanelRect(const PanelFlag panelFlag, const LayoutParams &layoutParams);
    int32_t ParsePanelRect(const PanelFlag panelFlag, const LayoutParams &layoutParams);
    int32_t GetSysPanelAdjust(const PanelFlag panelFlag,
        std::tuple<std::vector<std::string>, std::vector<std::string>> &keys, const LayoutParams &layoutParams);
    int32_t CalculatePanelRect(const PanelFlag panelFlag, PanelAdjustInfo &lanIterValue, PanelAdjustInfo &porIterValue,
        const LayoutParams &layoutParams);
    int32_t CalculateLandscapeRect(sptr<OHOS::Rosen::Display> &defaultDisplay, const LayoutParams &layoutParams,
        PanelAdjustInfo &lanIterValue, float densityDpi);
    std::tuple<std::vector<std::string>, std::vector<std::string>> GetScreenStatus(const PanelFlag panelFlag);
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
    bool IsDisplayPortrait();
    int32_t SetTextFieldAvoidInfo(double positionY, double height);
    void SetPanelHeightCallback(CallbackFunc heightCallback);
    uint32_t windowId_ = INVALID_WINDOW_ID;

private:
    class KeyboardPanelInfoChangeListener : public Rosen::IKeyboardPanelInfoChangeListener {
    public:
        using ChangeHandler = std::function<void(const Rosen::KeyboardPanelInfo &keyboardPanelInfo)>;
        explicit KeyboardPanelInfoChangeListener(ChangeHandler handler) : handler_(std::move(handler))
        {
        }
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
    int32_t SetPanelProperties();
    std::string GeneratePanelName();
    void PanelStatusChange(const InputWindowStatus &status);
    void PanelStatusChangeToImc(const InputWindowStatus &status, const Rosen::Rect &rect);
    bool MarkListener(const std::string &type, bool isRegister);
    static uint32_t GenerateSequenceId();
    bool IsSizeValid(uint32_t width, uint32_t height);
    bool IsSizeValid(PanelFlag panelFlag, uint32_t width, uint32_t height, int32_t displayWidth, int32_t displayHeight);
    bool CheckSize(PanelFlag panelFlag, uint32_t width, uint32_t height, bool isDataPortrait);
    bool GetDisplaySize(bool isPortrait, WindowSize &size);
    int32_t CalculateFloatRect(const LayoutParams &layoutParams, PanelAdjustInfo &lanIterValue,
        PanelAdjustInfo &porIterValue, float densityDpi);
    int32_t CalculateNoConfigRect(const PanelFlag panelFlag, const LayoutParams &layoutParams);
    void SetResizeParams(uint32_t width, uint32_t height);
    LayoutParams GetResizeParams();

    sptr<OHOS::Rosen::Window> window_ = nullptr;
    sptr<OHOS::Rosen::WindowOption> winOption_ = nullptr;
    sptr<WindowChangeListenerImpl> windowChangeListenerImpl_ = nullptr;
    PanelType panelType_ = PanelType::SOFT_KEYBOARD;
    PanelFlag panelFlag_ = PanelFlag::FLG_FIXED;
    bool showRegistered_ = false;
    bool hideRegistered_ = false;
    bool sizeChangeRegistered_ = false;
    uint32_t invalidGravityPercent = 0;
    std::shared_ptr<PanelStatusListener> panelStatusListener_ = nullptr;

    static std::atomic<uint32_t> sequenceId_;
    sptr<Rosen::IKeyboardPanelInfoChangeListener> kbPanelInfoListener_{ nullptr };
    bool isScbEnable_{ false };

    std::mutex panelAdjustLock_;
    std::map<std::vector<std::string>, PanelAdjustInfo> panelAdjust_;

    std::mutex keyboardSizeLock_;
    WindowSize keyboardSize_{ 0, 0 };
    Rosen::KeyboardLayoutParams keyboardLayoutParams_;
    std::mutex windowListenerLock_;
    sptr<Rosen::IWindowChangeListener> windowChangedListener_ = nullptr;
    CallbackFunc panelHeightCallback_ = nullptr;

    LayoutParams adjustPanelRectLayoutParams_;

    LayoutParams resizePanelFoldParams_ { // FoldDefaultValue
        {FOLD_TOP, FOLD_LEFT, FOLD_RIGHT, FOLD_BOTTOM},
        {FOLD_TOP, FOLD_LEFT, FOLD_RIGHT, COMMON_BOTTOM}
    };
    LayoutParams resizePanelUnfoldParams_ { // UnfoldDefaultValue
        {UNFOLD_TOP, UNFOLD_LEFT, UNFOLD_RIGHT, UNFOLD_BOTTOM},
        {UNFOLD_TOP, UNFOLD_LEFT, UNFOLD_RIGHT, COMMON_BOTTOM}
    };
    std::atomic<bool> isWaitSetUiContent_{true};
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUT_METHOD_PANEL_H
