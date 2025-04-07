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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_WINDOW_INFO_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_WINDOW_INFO_H

#include <cstdint>
#include <string>
#include "parcel.h"

#include "panel_info.h"
namespace OHOS {
namespace MiscServices {
enum class InputWindowStatus : uint32_t {
    SHOW,
    HIDE,
    NONE
};

struct InputWindowInfo : public Parcelable {
    std::string name;      // the name of inputWindow
    int32_t left { 0 };    // the abscissa of the upper-left vertex of inputWindow
    int32_t top { 0 };     // the ordinate of the upper-left vertex of inputWindow
    uint32_t width { 0 };  // the width of inputWindow
    uint32_t height { 0 }; // the height of inputWindow

    bool ReadFromParcel(Parcel &in)
    {
        name = in.ReadString();
        left = in.ReadInt32();
        top = in.ReadInt32();
        width = in.ReadUint32();
        height = in.ReadUint32();
        return true;
    }

    bool Marshalling(Parcel &out) const
    {
        if (!out.WriteString(name)) {
            return false;
        }
        if (!out.WriteInt32(left)) {
            return false;
        }
        if (!out.WriteInt32(top)) {
            return false;
        }
        if (!out.WriteUint32(width)) {
            return false;
        }
        if (!out.WriteUint32(height)) {
            return false;
        }
        return true;
    }

    static InputWindowInfo *Unmarshalling(Parcel &in)
    {
        InputWindowInfo *data = new (std::nothrow) InputWindowInfo();
        if (data && !data->ReadFromParcel(in)) {
            delete data;
            data = nullptr;
        }
        return data;
    }
};

struct ImeWindowInfo : public Parcelable {
    PanelInfo panelInfo;
    InputWindowInfo windowInfo;

    bool ReadFromParcel(Parcel &in)
    {
        std::unique_ptr<PanelInfo> pInfo(in.ReadParcelable<PanelInfo>());
        if (pInfo == nullptr) {
            return false;
        }
        panelInfo = *pInfo;

        std::unique_ptr<InputWindowInfo> wInfo(in.ReadParcelable<InputWindowInfo>());
        if (wInfo == nullptr) {
            return false;
        }
        windowInfo = *wInfo;
        return true;
    }

    bool Marshalling(Parcel &out) const
    {
        if (!out.WriteParcelable(&panelInfo)) {
            return false;
        }
        if (!out.WriteParcelable(&windowInfo)) {
            return false;
        }
        return true;
    }
    static ImeWindowInfo *Unmarshalling(Parcel &in)
    {
        ImeWindowInfo *data = new (std::nothrow) ImeWindowInfo();
        if (data && !data->ReadFromParcel(in)) {
            delete data;
            data = nullptr;
        }
    return data;
    }
};
} // namespace MiscServices
} // namespace OHOS

#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_WINDOW_INFO_H
