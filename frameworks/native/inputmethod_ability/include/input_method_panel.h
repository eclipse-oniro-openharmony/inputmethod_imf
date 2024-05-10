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

struct LayoutParams {
    Rosen::Rect landscapeRect;
    Rosen::Rect portraitRect;
};

struct PanelAdjust {
    int32_t top;
    int32_t left;
    uint32_t right;
    uint32_t bottom;
    bool operator==(const PanelAdjust &panelAdjust) const
    {
        return (top == panelAdjust.top && left == panelAdjust.left && right == panelAdjust.right &&
                bottom == panelAdjust.bottom);
    }
};

class InputMethodPanel {
public:
    static constexpr uint32_t INVALID_WINDOW_ID = 0;
    InputMethodPanel() = default;
    ~InputMethodPanel();
    int32_t SetUiContent(const std::string &contentInfo, napi_env env, std::shared_ptr<NativeReference> contentStorage);
    int32_t CreatePanel(const std::shared_ptr<AbilityRuntime::Context> &context, const PanelInfo &panelInfo);
    int32_t DestroyPanel();

    int32_t Resize(uint32_t width, uint32_t height);
    int32_t MoveTo(int32_t x, int32_t y);
    int32_t AdjustPanelRect(PanelFlag &panelFlag, LayoutParams &layoutParams);
    int32_t ParsePanelRect(PanelFlag &panelFlag, LayoutParams &layoutParams);
    int32_t GetSysPanelAdjust(PanelFlag &panelFlag,
        std::tuple<std::vector<std::string>, std::vector<std::string>> &keys, LayoutParams &layoutParams);
    int32_t CalculatePanelRect(PanelFlag &panelFlag, PanelAdjust &lanIterValue, PanelAdjust &porIterValue,
        LayoutParams &layoutParams);
    int32_t CalculateLandscapeRect(sptr<OHOS::Rosen::Display> &defaultDisplay, LayoutParams &layoutParams,
        PanelAdjust &lanIterValue, int densityDpi);
    std::tuple<std::vector<std::string>, std::vector<std::string>> GetScreenStatus(PanelFlag panelFlag);
    int32_t ChangePanelFlag(PanelFlag panelFlag);
    PanelType GetPanelType();
    PanelFlag GetPanelFlag();
    int32_t ShowPanel();
    int32_t HidePanel();
    int32_t SizeChange(const WindowSize &size);
    void SetPanelStatusListener(std::shared_ptr<PanelStatusListener> statusListener, const std::string &type);
    void ClearPanelListener(const std::string &type);
    int32_t SetCallingWindow(uint32_t windowId);
    int32_t GetCallingWindowInfo(CallingWindowInfo &windowInfo);
    int32_t SetPrivacyMode(bool isPrivacyMode);
    bool IsShowing();
    int32_t SetTextFieldAvoidInfo(double positionY, double height);
    uint32_t GetHeight();
    uint32_t windowId_ = INVALID_WINDOW_ID;

private:
    bool IsHidden();
    int32_t SetPanelProperties();
    std::string GeneratePanelName();
    void PanelStatusChange(const InputWindowStatus &status);
    void PanelStatusChangeToImc(const InputWindowStatus &status);
    bool MarkListener(const std::string &type, bool isRegister);
    static uint32_t GenerateSequenceId();
    bool IsSizeValid(uint32_t width, uint32_t height);

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
    std::mutex heightLock_;
    uint32_t panelHeight_ = 0;

    std::mutex panelAdjustLock_;
    std::mutex panelAdjustListLock_;
    std::map<std::vector<std::string>, PanelAdjust> panelAdjust_;

    std::vector<std::string> lanPanel_;
    std::vector<std::string> porPanel_;

    Rosen::KeyboardLayoutParams keyboardLayoutParams_;
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUT_METHOD_PANEL_H
