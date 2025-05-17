/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef SERVICES_INCLUDE_INPUT_ATTRIBUTE_H
#define SERVICES_INCLUDE_INPUT_ATTRIBUTE_H

#include <cstdint>
#include <sstream>

#include "parcel.h"

namespace OHOS {
namespace MiscServices {

struct InputAttribute {
    static const int32_t PATTERN_TEXT = 0x00000001;
    static const int32_t PATTERN_PASSWORD = 0x00000007;
    static const int32_t PATTERN_PASSWORD_NUMBER = 0x00000008;
    static const int32_t PATTERN_PASSWORD_SCREEN_LOCK = 0x00000009;
    static const int32_t PATTERN_NEWPASSWORD = 0x0000000b;
    static const int32_t PATTERN_ONE_TIME_CODE = 0x0000000d;
    int32_t inputPattern = 0;
    int32_t enterKeyType = 0;
    int32_t inputOption = 0;
    bool isTextPreviewSupported { false };
    std::string bundleName { "" };
    int32_t immersiveMode = 0;
    uint32_t windowId = 0; // for transfer
    uint64_t callingDisplayId = 0;
    std::u16string placeholder { u"" };
    std::u16string abilityName { u"" };
    bool needAutoInputNumkey { false }; // number keys need to be automatically handled by imf

    bool GetSecurityFlag() const
    {
        return inputPattern == PATTERN_PASSWORD || inputPattern == PATTERN_PASSWORD_SCREEN_LOCK ||
            PATTERN_PASSWORD_NUMBER == inputPattern || PATTERN_NEWPASSWORD == inputPattern;
    }

    bool IsOneTimeCodeFlag() const
    {
        return inputPattern == PATTERN_ONE_TIME_CODE;
    }

    bool IsSecurityImeFlag() const
    {
        return GetSecurityFlag() || IsOneTimeCodeFlag();
    }

    bool operator==(const InputAttribute &info) const
    {
        return inputPattern == info.inputPattern && enterKeyType == info.enterKeyType &&
            inputOption == info.inputOption && isTextPreviewSupported == info.isTextPreviewSupported;
    }

    inline std::string ToString() const
    {
        std::stringstream ss;
        ss << "[" << "inputPattern:" << inputPattern
        << "enterKeyType:" << enterKeyType << "inputOption:" << inputOption
        << "isTextPreviewSupported:" << isTextPreviewSupported << "bundleName:" << bundleName
        << "immersiveMode:" << immersiveMode << "windowId:" << windowId
        << "callingDisplayId:" << callingDisplayId
        << "needNumInput: " << needAutoInputNumkey
        << "]";
        return ss.str();
    }
};

struct InputAttributeInner : public Parcelable {
    static const int32_t PATTERN_TEXT = 0x00000001;
    static const int32_t PATTERN_PASSWORD = 0x00000007;
    static const int32_t PATTERN_PASSWORD_NUMBER = 0x00000008;
    static const int32_t PATTERN_PASSWORD_SCREEN_LOCK = 0x00000009;
    static const int32_t PATTERN_NEWPASSWORD = 0x0000000b;
    int32_t inputPattern = 0;
    int32_t enterKeyType = 0;
    int32_t inputOption = 0;
    bool isTextPreviewSupported { false };
    std::string bundleName { "" };
    int32_t immersiveMode = 0;
    uint32_t windowId = 0; // for transfer
    uint64_t callingDisplayId = 0;
    std::u16string placeholder { u"" };
    std::u16string abilityName { u"" };
    bool needAutoInputNumkey { false }; // number keys need to be automatically handled by imf

    bool ReadFromParcel(Parcel &in)
    {
        inputPattern = in.ReadInt32();
        enterKeyType = in.ReadInt32();
        inputOption = in.ReadInt32();
        isTextPreviewSupported = in.ReadBool();
        bundleName = in.ReadString();
        immersiveMode = in.ReadInt32();
        windowId = in.ReadUint32();
        callingDisplayId = in.ReadUint64();
        placeholder = in.ReadString16();
        abilityName = in.ReadString16();
        needAutoInputNumkey = in.ReadBool();
        return true;
    }

    bool Marshalling(Parcel &out) const
    {
        if (!out.WriteInt32(inputPattern)) {
            return false;
        }
        if (!out.WriteInt32(enterKeyType)) {
            return false;
        }
        if (!out.WriteInt32(inputOption)) {
            return false;
        }
        if (!out.WriteBool(isTextPreviewSupported)) {
            return false;
        }
        if (!out.WriteString(bundleName)) {
            return false;
        }
        if (!out.WriteInt32(immersiveMode)) {
            return false;
        }
        if (!out.WriteUint32(windowId)) {
            return false;
        }
        if (!out.WriteUint64(callingDisplayId)) {
            return false;
        }
        auto ret = out.WriteString16(placeholder) && out.WriteString16(abilityName);
        ret = ret && out.WriteBool(needAutoInputNumkey);
        return ret;
    }

    static InputAttributeInner *Unmarshalling(Parcel &in)
    {
        InputAttributeInner *data = new (std::nothrow) InputAttributeInner();
        if (data && !data->ReadFromParcel(in)) {
            delete data;
            data = nullptr;
        }
        return data;
    }

    bool operator==(const InputAttribute &info) const
    {
        return inputPattern == info.inputPattern && enterKeyType == info.enterKeyType &&
            inputOption == info.inputOption && isTextPreviewSupported == info.isTextPreviewSupported;
    }

    bool GetSecurityFlag() const
    {
        return inputPattern == PATTERN_PASSWORD || inputPattern == PATTERN_PASSWORD_SCREEN_LOCK ||
            PATTERN_PASSWORD_NUMBER == inputPattern || PATTERN_NEWPASSWORD == inputPattern;
    }
};
} // namespace MiscServices
} // namespace OHOS

#endif // SERVICES_INCLUDE_INPUT_ATTRIBUTE_H
