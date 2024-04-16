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
#include "input_window_info.h"
#include "js_runtime_utils.h"
#include "panel_info.h"
#include "panel_status_listener.h"
#include "window.h"
#include "wm_common.h"
namespace OHOS {
namespace MiscServices {
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
    int32_t ChangePanelFlag(PanelFlag panelFlag);
    PanelType GetPanelType();
    PanelFlag GetPanelFlag();
    int32_t ShowPanel();
    int32_t HidePanel();
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
    class WindowChangedListener : public Rosen::IWindowChangeListener {
    public:
        using ChangeHandler = std::function<void(const Rosen::Rect &rect)>;
        explicit WindowChangedListener(ChangeHandler handler) : handler_(std::move(handler))
        {
        }
        ~WindowChangedListener() = default;
        void OnSizeChange(Rosen::Rect Rect, Rosen::WindowSizeChangeReason reason,
            const std::shared_ptr<Rosen::RSTransaction> &rsTransaction = nullptr) override;

    private:
        ChangeHandler handler_ = nullptr;
    };
    class KeyboardPanelInfoChangeListener : public Rosen::IKeyboardPanelInfoChangeListener {
    public:
        using ChangeHandler = std::function<void(const Rosen::KeyboardPanelInfo &keyboardPanelInfo)>;
        explicit KeyboardPanelInfoChangeListener(ChangeHandler handler) : handler_(std::move(handler))
        {
        }
        ~KeyboardPanelInfoChangeListener() = default;
        void OnKeyboardPanelInfoChanged(const Rosen::KeyboardPanelInfo &keyboardPanelInfo) override;

    private:
        ChangeHandler handler_ = nullptr;
    };
    void RegisterKeyboardPanelInfoChangeListener();
    void UnregisterKeyboardPanelInfoChangeListener();
    void HandleKbPanelInfoChange(const Rosen::KeyboardPanelInfo &keyboardPanelInfo);
    void RegisterWindowChangeListener();
    void UnregisterWindowChangeListener();
    void HandleSizeChange(const Rosen::Rect &rect);
    bool IsHidden();
    int32_t SetPanelProperties();
    std::string GeneratePanelName();
    void PanelStatusChange(const InputWindowStatus &status);
    void PanelStatusChangeToImc(const InputWindowStatus &status, const Rosen::Rect &rect);
    bool MarkListener(const std::string &type, bool isRegister);
    static uint32_t GenerateSequenceId();
    bool IsSizeValid(uint32_t width, uint32_t height);

    sptr<OHOS::Rosen::Window> window_ = nullptr;
    sptr<OHOS::Rosen::WindowOption> winOption_ = nullptr;
    PanelType panelType_ = PanelType::SOFT_KEYBOARD;
    PanelFlag panelFlag_ = PanelFlag::FLG_FIXED;
    bool showRegistered_ = false;
    bool hideRegistered_ = false;
    uint32_t invalidGravityPercent = 0;
    std::shared_ptr<PanelStatusListener> panelStatusListener_ = nullptr;

    static std::atomic<uint32_t> sequenceId_;
    std::mutex heightLock_;
    uint32_t panelHeight_ = 0;
    sptr<Rosen::IWindowChangeListener> windowChangeListener_{ nullptr };
    sptr<Rosen::IKeyboardPanelInfoChangeListener> kbPanelInfoListener_{ nullptr };
    bool isScbEnable_{ false };
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUT_METHOD_PANEL_H
