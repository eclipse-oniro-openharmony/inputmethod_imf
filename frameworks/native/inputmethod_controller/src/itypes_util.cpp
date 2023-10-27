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
#include "input_client_proxy.h"
#include "input_data_channel_proxy.h"
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

bool ITypesUtil::Marshalling(double input, MessageParcel &data)
{
    return data.WriteDouble(input);
}

bool ITypesUtil::Unmarshalling(double &output, MessageParcel &data)
{
    return data.ReadDouble(output);
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
    if (!Marshal(data, input.name, input.id, input.label, input.labelId, input.icon, input.iconId)) {
        IMSA_HILOGE("ITypesUtil::write Property to message parcel failed");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(Property &output, MessageParcel &data)
{
    if (!Unmarshal(data, output.name, output.id, output.label, output.labelId, output.icon, output.iconId)) {
        IMSA_HILOGE("ITypesUtil::read Property from message parcel failed");
        return false;
    }
    return true;
}

bool ITypesUtil::Marshalling(const SubProperty &input, MessageParcel &data)
{
    if (!Marshal(data, input.label, input.labelId, input.name, input.id, input.mode, input.locale, input.language,
            input.icon, input.iconId)) {
        IMSA_HILOGE("ITypesUtil::write SubProperty to message parcel failed");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(SubProperty &output, MessageParcel &data)
{
    if (!Unmarshal(data, output.label, output.labelId, output.name, output.id, output.mode, output.locale,
        output.language, output.icon, output.iconId)) {
        IMSA_HILOGE("ITypesUtil::read SubProperty from message parcel failed");
        return false;
    }
    return true;
}

bool ITypesUtil::Marshalling(const InputAttribute &input, MessageParcel &data)
{
    if (!Marshal(data, input.inputPattern, input.enterKeyType, input.inputOption)) {
        IMSA_HILOGE("write InputAttribute to message parcel failed");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(InputAttribute &output, MessageParcel &data)
{
    if (!Unmarshal(data, output.inputPattern, output.enterKeyType, output.inputOption)) {
        IMSA_HILOGE("read InputAttribute from message parcel failed");
        return false;
    }
    return true;
}

bool ITypesUtil::Marshalling(const TextTotalConfig &input, MessageParcel &data)
{
    if (!Marshal(data, input.inputAttribute.inputPattern, input.inputAttribute.enterKeyType,
        input.inputAttribute.inputOption)) {
        IMSA_HILOGE("write InputAttribute to message parcel failed");
        return false;
    }
    if (!Marshal(data, input.cursorInfo.left, input.cursorInfo.top, input.cursorInfo.height, input.cursorInfo.width)) {
        IMSA_HILOGE("write CursorInfo to message parcel failed");
        return false;
    }
    if (!Marshal(data, input.textSelection.oldBegin, input.textSelection.oldEnd, input.textSelection.newBegin,
        input.textSelection.newEnd)) {
        IMSA_HILOGE("write TextSelection to message parcel failed");
        return false;
    }
    if (!Marshal(data, input.windowId)) {
        IMSA_HILOGE("write windowId to message parcel failed");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(TextTotalConfig &output, MessageParcel &data)
{
    if (!Unmarshalling(output.inputAttribute, data)) {
        IMSA_HILOGE("read InputAttribute from message parcel failed");
        return false;
    }
    if (!Unmarshal(data, output.cursorInfo.left, output.cursorInfo.top,
        output.cursorInfo.height, output.cursorInfo.width)) {
        IMSA_HILOGE("read CursorInfo from message parcel failed");
        return false;
    }
    if (!Unmarshal(data, output.textSelection.oldBegin, output.textSelection.oldEnd,
        output.textSelection.newBegin, output.textSelection.newEnd)) {
        IMSA_HILOGE("read TextSelection from message parcel failed");
        return false;
    }
    if (!Unmarshal(data, output.windowId)) {
        IMSA_HILOGE("read windowId from message parcel failed");
        return false;
    }
    return true;
}

bool ITypesUtil::Marshalling(const InputClientInfo &input, MessageParcel &data)
{
    if (!Marshal(data, input.pid, input.uid, input.userID, input.isShowKeyboard, input.eventFlag, input.config,
            input.client->AsObject(), input.channel->AsObject())) {
        IMSA_HILOGE("write InputClientInfo to message parcel failed");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(InputClientInfo &output, MessageParcel &data)
{
    if (!Unmarshal(
        data, output.pid, output.uid, output.userID, output.isShowKeyboard, output.eventFlag, output.config)) {
        IMSA_HILOGE("read InputClientInfo from message parcel failed");
        return false;
    }
    auto client = data.ReadRemoteObject();
    auto channel = data.ReadRemoteObject();
    if (client == nullptr || channel == nullptr) {
        IMSA_HILOGE("read remote object failed");
        return false;
    }
    output.client = iface_cast<IInputClient>(client);
    output.channel = iface_cast<IInputDataChannel>(channel);
    return true;
}

bool ITypesUtil::Marshalling(const InputWindowInfo &input, MessageParcel &data)
{
    if (!Marshal(data, input.name, input.top, input.left, input.width, input.height)) {
        IMSA_HILOGE("write InputWindowInfo to message parcel failed");
        return false;
    }
    return true;
}
bool ITypesUtil::Unmarshalling(InputWindowInfo &output, MessageParcel &data)
{
    if (!Unmarshal(data, output.name, output.top, output.left, output.width, output.height)) {
        IMSA_HILOGE("read InputWindowInfo from message parcel failed");
        return false;
    }
    return true;
}

bool ITypesUtil::Marshalling(const PanelStatusInfo &input, MessageParcel &data)
{
    return data.WriteInt32(static_cast<int32_t>(input.panelInfo.panelType))
           && data.WriteInt32(static_cast<int32_t>(input.panelInfo.panelFlag)) && data.WriteBool(input.visible)
           && data.WriteInt32(static_cast<int32_t>(input.trigger));
}

bool ITypesUtil::Unmarshalling(PanelStatusInfo &output, MessageParcel &data)
{
    int32_t type = -1;
    int32_t flag = -1;
    bool visible = false;
    int32_t trigger = -1;
    if (!data.ReadInt32(type) || !data.ReadInt32(flag) || !data.ReadBool(visible) || !data.ReadInt32(trigger)) {
        return false;
    }
    output = { { static_cast<PanelType>(type), static_cast<PanelFlag>(flag) }, visible, static_cast<Trigger>(trigger) };
    return true;
}

bool ITypesUtil::Marshalling(EventType input, MessageParcel &data)
{
    return data.WriteUint32(static_cast<uint32_t>(input));
}

bool ITypesUtil::Unmarshalling(EventType &output, MessageParcel &data)
{
    auto ret = data.ReadUint32();
    output = static_cast<EventType>(ret);
    return true;
}

bool ITypesUtil::Marshalling(const OHOS::AppExecFwk::ElementName &input, MessageParcel &data)
{
    return data.WriteString(input.GetBundleName().c_str()) && data.WriteString(input.GetModuleName().c_str()) &&
           data.WriteString(input.GetAbilityName().c_str());
}

bool ITypesUtil::Unmarshalling(OHOS::AppExecFwk::ElementName &output, MessageParcel &data)
{
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    if (data.ReadString(bundleName) && data.ReadString(moduleName) && data.ReadString(abilityName)) {
        output.SetBundleName(bundleName);
        output.SetModuleName(moduleName);
        output.SetAbilityName(abilityName);
        return true;
    }
    IMSA_HILOGE("read ElementName from message parcel failed");
    return false;
}

bool ITypesUtil::Marshalling(InputType input, MessageParcel &data)
{
    return data.WriteInt32(static_cast<int32_t>(input));
}

bool ITypesUtil::Unmarshalling(InputType &output, MessageParcel &data)
{
    int32_t ret = 0;
    if (!data.ReadInt32(ret)) {
        return false;
    }
    output = static_cast<InputType>(ret);
    return true;
}
} // namespace MiscServices
} // namespace OHOS