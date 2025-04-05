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

#ifndef INPUTMETHOD_IMF_PANEL_INFO_H
#define INPUTMETHOD_IMF_PANEL_INFO_H

#include "parcel.h"

namespace OHOS {
namespace MiscServices {
enum PanelType {
    SOFT_KEYBOARD = 0,
    STATUS_BAR,
};

enum PanelFlag {
    FLG_FIXED = 0,
    FLG_FLOATING,
    FLG_CANDIDATE_COLUMN,
};

struct PanelInfo : public Parcelable {
    PanelType panelType = SOFT_KEYBOARD;
    PanelFlag panelFlag = FLG_FIXED;

    bool ReadFromParcel(Parcel &in)
    {
        int32_t panelTypeData = in.ReadInt32();
        int32_t panelFlagData = in.ReadInt32();
        panelType = static_cast<PanelType>(panelTypeData);
        panelFlag = static_cast<PanelFlag>(panelFlagData);
        return true;
    }
    bool Marshalling(Parcel &out) const
    {
        if (!out.WriteInt32(static_cast<int32_t>(panelType))) {
            return false;
        }
        if (!out.WriteInt32(static_cast<int32_t>(panelFlag))) {
            return false;
        }
        return true;
    }
    static PanelInfo *Unmarshalling(Parcel &in)
    {
        PanelInfo *data = new (std::nothrow) PanelInfo();
        if (data && !data->ReadFromParcel(in)) {
            delete data;
            data = nullptr;
        }
        return data;
    }
};

enum class ImmersiveMode : int32_t {
    NONE_IMMERSIVE = 0,
    IMMERSIVE = 1,
    LIGHT_IMMERSIVE = 2,
    DARK_IMMERSIVE = 3,
    END,
};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_PANEL_INFO_H
