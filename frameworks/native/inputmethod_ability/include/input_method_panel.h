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

#include "js_runtime_utils.h"
#include "panel_status_listener.h"
#include "window.h"
#include "window_option.h"
#include "wm_common.h"

namespace OHOS {
using namespace OHOS::Rosen;
namespace MiscServices {
enum PanelType {
    SOFT_KEYBOARD = 0,
    STATUS_BAR,
};

enum PanelFlag {
    FLG_FIXED = 0,
    FLG_FLOATING,
};

struct PanelInfo {
    PanelType panelType = SOFT_KEYBOARD;
    PanelFlag panelFlag = FLG_FIXED;
};

class InputMethodPanel {
public:
    InputMethodPanel() = default;
    ~InputMethodPanel();
    int32_t SetUiContent(const std::string &contentInfo, NativeEngine &engine, std::shared_ptr<NativeReference> &contentStorage);
    int32_t CreatePanel(const std::shared_ptr<AbilityRuntime::Context> &context, const PanelInfo &panelInfo);
    int32_t DestroyPanel();

    int32_t Resize(uint32_t width, uint32_t height);
    int32_t MoveTo(int32_t x, int32_t y);
    int32_t ChangePanelFlag(PanelFlag panelFlag);
    PanelType GetPanelType();
    int32_t ShowPanel();
    int32_t HidePanel();
    void SetPanelStatusListener(std::shared_ptr<PanelStatusListener> statusListener);
    void RemovePanelListener(const std::string &type);
    uint32_t windowId_ = 0;

private:
    bool IsShowing();
    bool IsHidden();
    static uint32_t GenerateSequenceId();

    sptr<Window> window_ = nullptr;
    sptr<WindowOption> winOption_ = nullptr;
    PanelType panelType_ = PanelType::SOFT_KEYBOARD;
    PanelFlag panelFlag_ = PanelFlag::FLG_FIXED;
    bool showRegistered_ = true;
    bool hideRegistered_ = true;
    uint32_t invalidGravityPercent = 0;
    std::shared_ptr<PanelStatusListener> panelStatusListener_ = nullptr;

    static std::atomic<uint32_t> sequenceId_;
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUT_METHOD_PANEL_H
