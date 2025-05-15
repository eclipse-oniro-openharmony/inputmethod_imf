/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_IMF_INPUT_METHOD_PROPERTY_H
#define INPUTMETHOD_IMF_INPUT_METHOD_PROPERTY_H
#include <mutex>
#include <thread>
#include <vector>
#include "input_method_status.h"
#include "parcel.h"

namespace OHOS {
namespace MiscServices {
struct Property : public Parcelable {
    std::string name;     // the bundleName of inputMethod
    std::string id;       // the extensionName of inputMethod
    std::string label;    // the label of inputMethod
    uint32_t labelId = 0; // the labelId of inputMethod
    std::string icon;     // the icon of inputMethod
    uint32_t iconId = 0;  // the icon id of inputMethod
    EnabledStatus status { EnabledStatus::DISABLED };  // the enabled status of inputMethod

    bool ReadFromParcel(Parcel &in)
    {
        name = in.ReadString();
        id = in.ReadString();
        label = in.ReadString();
        labelId = in.ReadUint32();
        icon = in.ReadString();
        iconId = in.ReadUint32();
        status = static_cast<EnabledStatus>(in.ReadInt32());
        return true;
    }

    bool Marshalling(Parcel &out) const
    {
        if (!out.WriteString(name)) {
            return false;
        }
        if (!out.WriteString(id)) {
            return false;
        }
        if (!out.WriteString(label)) {
            return false;
        }
        if (!out.WriteUint32(labelId)) {
            return false;
        }
        if (!out.WriteString(icon)) {
            return false;
        }
        if (!out.WriteUint32(iconId)) {
            return false;
        }
        if (!out.WriteInt32(static_cast<int32_t>(status))) {
            return false;
        }
        return true;
    }

    static Property *Unmarshalling(Parcel &in)
    {
        Property *data = new (std::nothrow) Property();
        if (data && !data->ReadFromParcel(in)) {
            delete data;
            data = nullptr;
        }
        return data;
    }
};

struct SubProperty : public Parcelable {
    std::string label;    // the label of subtype
    uint32_t labelId = 0; // the labelId of subtype
    std::string name;     // the bundleName of inputMethod
    std::string id;       // the name of subtype
    std::string mode;     // the mode of subtype, containing "upper" and "lower"
    std::string locale;   // the tongues of subtype, such as "zh_CN", "en_US", etc.
    std::string language; // the language of subtype
    std::string icon;     // the icon of subtype
    uint32_t iconId = 0;  // the icon id of subtype

    bool ReadFromParcel(Parcel &in)
    {
        label = in.ReadString();
        labelId = in.ReadUint32();
        name = in.ReadString();
        id = in.ReadString();
        mode = in.ReadString();
        locale = in.ReadString();
        language = in.ReadString();
        icon = in.ReadString();
        iconId = in.ReadUint32();
        return true;
    }

    bool Marshalling(Parcel &out) const
    {
        if (!out.WriteString(label)) {
            return false;
        }
        if (!out.WriteUint32(labelId)) {
            return false;
        }
        if (!out.WriteString(name)) {
            return false;
        }
        if (!out.WriteString(id)) {
            return false;
        }
        if (!out.WriteString(mode)) {
            return false;
        }
        if (!out.WriteString(locale)) {
            return false;
        }
        if (!out.WriteString(language)) {
            return false;
        }
        if (!out.WriteString(icon)) {
            return false;
        }
        if (!out.WriteUint32(iconId)) {
            return false;
        }
        return true;
    }
    static SubProperty *Unmarshalling(Parcel &in)
    {
        SubProperty *data = new (std::nothrow) SubProperty();
        if (data && !data->ReadFromParcel(in)) {
            delete data;
            data = nullptr;
        }
        return data;
    }
};

struct FullImeInfo {
    bool isNewIme { false };
    uint32_t tokenId { 0 };
    std::string appId;
    uint32_t versionCode;
    Property prop;
    std::vector<SubProperty> subProps;
};

struct ImeInfo : public FullImeInfo {
    SubProperty subProp;
    bool isSpecificSubName { true };
};

struct SwitchInfo {
    std::chrono::system_clock::time_point timestamp{};
    std::string bundleName;
    std::string subName;
    bool operator==(const SwitchInfo &info) const
    {
        return (timestamp == info.timestamp && bundleName == info.bundleName && subName == info.subName);
    }
};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_INPUT_METHOD_PROPERTY_H
