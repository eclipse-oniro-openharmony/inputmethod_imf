/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "itypes_util.h"

#include "global.h"
#include "iremote_object.h"

namespace OHOS {
namespace MiscServices {
bool ITypesUtil::Marshal(MessageParcel &data)
{
    return true;
}

bool ITypesUtil::Unmarshal(MessageParcel &data)
{
    return true;
}

bool ITypesUtil::Marshalling(bool input, MessageParcel &data)
{
    return data.WriteBool(input);
}

bool ITypesUtil::Unmarshalling(bool &output, MessageParcel &data)
{
    return data.ReadBool(output);
}

bool ITypesUtil::Marshalling(uint32_t input, MessageParcel &data)
{
    return data.WriteUint32(input);
}

bool ITypesUtil::Unmarshalling(uint32_t &output, MessageParcel &data)
{
    return data.ReadUint32(output);
}

bool ITypesUtil::Marshalling(int32_t input, MessageParcel &data)
{
    return data.WriteInt32(input);
}

bool ITypesUtil::Unmarshalling(int32_t &output, MessageParcel &data)
{
    return data.ReadInt32(output);
}

bool ITypesUtil::Marshalling(uint64_t input, MessageParcel &data)
{
    return data.WriteUint64(input);
}

bool ITypesUtil::Unmarshalling(uint64_t &output, MessageParcel &data)
{
    return data.ReadUint64(output);
}

bool ITypesUtil::Marshalling(const std::string &input, MessageParcel &data)
{
    return data.WriteString(input);
}

bool ITypesUtil::Unmarshalling(std::string &output, MessageParcel &data)
{
    return data.ReadString(output);
}

bool ITypesUtil::Marshalling(const std::u16string &input, MessageParcel &data)
{
    return data.WriteString16(input);
}

bool ITypesUtil::Unmarshalling(std::u16string &output, MessageParcel &data)
{
    return data.ReadString16(output);
}

bool ITypesUtil::Marshalling(const std::vector<uint8_t> &input, MessageParcel &data)
{
    return data.WriteUInt8Vector(input);
}

bool ITypesUtil::Unmarshalling(std::vector<uint8_t> &output, MessageParcel &data)
{
    return data.ReadUInt8Vector(&output);
}

bool ITypesUtil::Marshalling(const sptr<IRemoteObject> &input, MessageParcel &data)
{
    return data.WriteRemoteObject(input);
}

bool ITypesUtil::Unmarshalling(sptr<IRemoteObject> &output, MessageParcel &data)
{
    output = data.ReadRemoteObject();
    return true;
}
bool ITypesUtil::Marshalling(const Property &input, MessageParcel &data)
{
    if (!Marshal(data, input.name, input.id, input.label, input.icon, input.iconId)) {
        IMSA_HILOGE("ITypesUtil::write Property to message parcel failed");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(Property &output, MessageParcel &data)
{
    if (!Unmarshal(data, output.name, output.id, output.label, output.icon, output.iconId)) {
        IMSA_HILOGE("ITypesUtil::read Property from message parcel failed");
        return false;
    }
    return true;
}

bool ITypesUtil::Marshalling(const SubProperty &input, MessageParcel &data)
{
    if (!Marshal(data, input.label, input.name, input.id, input.mode, input.locale, input.language, input.icon,
            input.iconId)) {
        IMSA_HILOGE("ITypesUtil::write SubProperty to message parcel failed");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(SubProperty &output, MessageParcel &data)
{
    if (!Unmarshal(data, output.label, output.name, output.id, output.mode, output.locale, output.language,
            output.icon, output.iconId)) {
        IMSA_HILOGE("ITypesUtil::read SubProperty from message parcel failed");
        return false;
    }
    return true;
}
} // namespace MiscServices
} // namespace OHOS