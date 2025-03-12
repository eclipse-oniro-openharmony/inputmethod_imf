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
    inline std::string ToString() const
    {
        std::stringstream ss;
        ss << "width:" << width
        << "height:" <<  height;
        return ss.str();
    }

    bool operator==(const WindowSize &other)
    {
        return (width == other.width && height == other.height);
    }
};

struct LayoutParams {
    Rosen::Rect landscapeRect{ 0, 0, 0, 0 };
    Rosen::Rect portraitRect{ 0, 0, 0, 0 };
    inline WindowSize GetWindowSize(bool isPortrait) const
    {
        WindowSize ret;
        if (isPortrait) {
            ret.width = portraitRect.width_;
            ret.height = portraitRect.height_;
        } else {
            ret.width = landscapeRect.width_;
            ret.height = landscapeRect.height_;
        }
        return ret;
    }

    inline std::string ToString() const
    {
        std::stringstream ss;
        ss << "landscapeRect:" << landscapeRect.ToString()
        << " portraitRect:" <<  portraitRect.ToString();
        return ss.str();
    }
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

    inline std::string ToString() const
    {
        std::stringstream ss;
        ss << "keyboardHotArea:" << HotArea::ToString(keyboardHotArea)
        << "panelHotArea:" <<  HotArea::ToString(panelHotArea);
        return ss.str();
    }
};

struct HotAreas {
    HotArea landscape;
    HotArea portrait;
    bool isSet{ false };
    inline std::string ToString() const
    {
        std::stringstream ss;
        ss << "landscape:" << landscape.ToString()<< "portrait:"
        << portrait.ToString() << "isSet:" << isSet;
        return ss.str();
    }
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
    inline std::string ToString() const
    {
        std::stringstream ss;
        ss << "portrait:" << portrait.ToString() << "landscape:"
        << landscape.ToString() << "isFullScreen:" << isFullScreen;
        return ss.str();
    }
};

struct DisplaySize {
    WindowSize portrait;
    WindowSize landscape;
    OHOS::Rosen::DisplayId displayId = OHOS::Rosen::DISPLAY_ID_INVALID;
    bool isPortrait = false;
    float densityDpi = 0.0f;
    inline std::string ToString() const
    {
        std::stringstream ss;
        ss << "portrait:" << portrait.ToString() << "landscape:"
        << landscape.ToString() << "displayId:" << displayId
        << "isPortrait:" << isPortrait << "densityDpi:" << densityDpi;
        return ss.str();
    }

    inline WindowSize GetCurActualitySize() const
    {
        if (isPortrait) {
            return portrait;
        }
        return landscape;
    }
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

    inline std::string ToString() const
    {
        std::stringstream ss;
        ss << "top:" << top
        << "left:" << left
        << "bottom:" << bottom
        << "right:" << right;
        return ss.str();
    }
};

struct FullPanelAdjustInfo {
    PanelAdjustInfo portrait;
    PanelAdjustInfo landscape;
    inline std::string ToString() const
    {
        std::stringstream ss;
        ss << "portrait:" << portrait.ToString()
        << "landscape:" << landscape.ToString()
        << "isExitConifg:" << isExitConifg;
        return ss.str();
    }
    bool isExitConifg = false;
};

template <typename _Key, typename _Value>
class PanelParamsCache {
public:
    PanelParamsCache()
    {
        std::lock_guard<std::mutex> lock(lock_);
        last_ = new (std::nothrow) _Value();
    }

    ~PanelParamsCache()
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (last_) {
            delete last_;
            last_ = nullptr;
        }
    }

    void addOrUpdateData(const _Key &key, const _Value &value)
    {
        std::lock_guard<std::mutex> lock(lock_);
        caches_[key] = value;
        *last_ = value;
    }

    void getData(const _Key &key,  _Value &value)
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (caches_.find(key) == caches_.end()) {
            value = *last_;
        } else {
            value = caches_[key];
        }
    }
private:
    std::mutex  lock_;
    _Value *last_ = nullptr;
    std::map<_Key, _Value> caches_;
};
} // namespace MiscServices
} // namespace OHOS
#endif //INPUTMETHOD_IMF_PANEL_COMMON_H
