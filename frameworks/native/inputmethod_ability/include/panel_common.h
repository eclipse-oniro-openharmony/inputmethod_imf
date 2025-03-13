/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_IMF_PANEL_COMMON_H
#define INPUTMETHOD_IMF_PANEL_COMMON_H

#include <cstdint>

#include "wm_common.h"

namespace OHOS {
namespace MiscServices {
struct WindowSize {
    uint32_t width = 0;
    uint32_t height = 0;
};

struct LayoutParams {
    Rosen::Rect landscapeRect{ 0, 0, 0, 0 };
    Rosen::Rect portraitRect{ 0, 0, 0, 0 };
};

struct HotArea {
    std::vector<Rosen::Rect> keyboardHotArea;
    std::vector<Rosen::Rect> panelHotArea;
    static std::string ToString(const std::vector<Rosen::Rect> &areas)
    {
        std::string areasStr = "[";
        for (const auto area : areas) {
            areasStr.append(area.ToString());
        }
        areasStr.append("]");
        return areasStr;
    }
};

struct HotAreas {
    HotArea landscape;
    HotArea portrait;
    bool isSet{ false };
};

struct EnhancedLayoutParam {
    Rosen::Rect rect{ 0, 0, 0, 0 };
    int32_t avoidY{ 0 };
    uint32_t avoidHeight{ 0 };
    inline std::string ToString() const
    {
        std::stringstream ss;
        ss << "rect" << rect.ToString() << " avoidY " << avoidY << " avoidHeight " << avoidHeight;
        return ss.str();
    }
};

struct EnhancedLayoutParams {
    bool isFullScreen{ false };
    EnhancedLayoutParam portrait;
    EnhancedLayoutParam landscape;
};

struct DisplaySize {
    WindowSize portrait;
    WindowSize landscape;
};

struct PanelAdjustInfo {
    int32_t top{ 0 };
    int32_t left{ 0 };
    int32_t right{ 0 };
    int32_t bottom{ 0 };
    bool operator==(const PanelAdjustInfo &panelAdjust) const
    {
        return (top == panelAdjust.top && left == panelAdjust.left && right == panelAdjust.right
                && bottom == panelAdjust.bottom);
    }
};

struct FullPanelAdjustInfo {
    PanelAdjustInfo portrait;
    PanelAdjustInfo landscape;
};
} // namespace MiscServices
} // namespace OHOS
#endif //INPUTMETHOD_IMF_PANEL_COMMON_H
