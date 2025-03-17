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

bool ITypesUtil::Marshalling(int64_t input, MessageParcel &data)
{
    return data.WriteInt64(input);
}

bool ITypesUtil::Unmarshalling(int64_t &output, MessageParcel &data)
{
    return data.ReadInt64(output);
}

bool ITypesUtil::Marshalling(ClientType input, MessageParcel &data)
{
    return data.WriteUint32(static_cast<uint32_t>(input));
}

bool ITypesUtil::Unmarshalling(ClientType &output, MessageParcel &data)
{
    uint32_t ret = 0;
    if (!data.ReadUint32(ret)) {
        return false;
    }
    output = static_cast<ClientType>(ret);
    return true;
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
        IMSA_HILOGE("write Property to message parcel failed.");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(Property &output, MessageParcel &data)
{
    if (!Unmarshal(data, output.name, output.id, output.label, output.labelId, output.icon, output.iconId)) {
        IMSA_HILOGE("read Property from message parcel failed.");
        return false;
    }
    return true;
}

bool ITypesUtil::Marshalling(const SubProperty &input, MessageParcel &data)
{
    if (!Marshal(data, input.label, input.labelId, input.name, input.id, input.mode, input.locale, input.language,
                 input.icon, input.iconId)) {
        IMSA_HILOGE("write SubProperty to message parcel failed.");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(SubProperty &output, MessageParcel &data)
{
    if (!Unmarshal(data, output.label, output.labelId, output.name, output.id, output.mode, output.locale,
                   output.language, output.icon, output.iconId)) {
        IMSA_HILOGE("read SubProperty from message parcel failed.");
        return false;
    }
    return true;
}

bool ITypesUtil::Marshalling(const InputAttribute &input, MessageParcel &data)
{
    if (!Marshal(data, input.inputPattern, input.enterKeyType, input.inputOption, input.isTextPreviewSupported,
        input.bundleName, input.immersiveMode, input.windowId, input.callingDisplayId)) {
        IMSA_HILOGE("write InputAttribute to message parcel failed.");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(InputAttribute &output, MessageParcel &data)
{
    if (!Unmarshal(data, output.inputPattern, output.enterKeyType, output.inputOption, output.isTextPreviewSupported,
        output.bundleName, output.immersiveMode, output.windowId, output.callingDisplayId)) {
        IMSA_HILOGE("read InputAttribute from message parcel failed.");
        return false;
    }
    return true;
}

bool ITypesUtil::Marshalling(const TextTotalConfig &input, MessageParcel &data)
{
    if (!Marshal(data, input.inputAttribute)) {
        IMSA_HILOGE("write InputAttribute to message parcel failed.");
        return false;
    }
    if (!Marshal(data, input.cursorInfo.left, input.cursorInfo.top, input.cursorInfo.height, input.cursorInfo.width)) {
        IMSA_HILOGE("write CursorInfo to message parcel failed.");
        return false;
    }
    if (!Marshal(data, input.textSelection.oldBegin, input.textSelection.oldEnd, input.textSelection.newBegin,
                 input.textSelection.newEnd)) {
        IMSA_HILOGE("write TextSelection to message parcel failed.");
        return false;
    }
    if (!Marshal(data, input.windowId)) {
        IMSA_HILOGE("write windowId to message parcel failed.");
        return false;
    }
    if (!Marshal(data, input.positionY)) {
        IMSA_HILOGE("write positionY to message parcel failed.");
        return false;
    }
    if (!Marshal(data, input.height)) {
        IMSA_HILOGE("write height to message parcel failed.");
        return false;
    }
    if (!Marshal(data, input.privateCommand)) {
        IMSA_HILOGE("write privateCommand to message parcel failed.");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(TextTotalConfig &output, MessageParcel &data)
{
    if (!Unmarshalling(output.inputAttribute, data)) {
        IMSA_HILOGE("read InputAttribute from message parcel failed.");
        return false;
    }
    if (!Unmarshal(data, output.cursorInfo.left, output.cursorInfo.top, output.cursorInfo.height,
                   output.cursorInfo.width)) {
        IMSA_HILOGE("read CursorInfo from message parcel failed.");
        return false;
    }
    if (!Unmarshal(data, output.textSelection.oldBegin, output.textSelection.oldEnd, output.textSelection.newBegin,
                   output.textSelection.newEnd)) {
        IMSA_HILOGE("read TextSelection from message parcel failed.");
        return false;
    }
    if (!Unmarshal(data, output.windowId)) {
        IMSA_HILOGE("read windowId from message parcel failed.");
        return false;
    }
    if (!Unmarshal(data, output.positionY)) {
        IMSA_HILOGE("read positionY from message parcel failed.");
        return false;
    }
    if (!Unmarshal(data, output.height)) {
        IMSA_HILOGE("read height from message parcel failed.");
        return false;
    }
    if (!Unmarshal(data, output.privateCommand)) {
        IMSA_HILOGE("read privateCommand from message parcel failed.");
        return false;
    }
    return true;
}

bool ITypesUtil::Marshalling(const InputClientInfo &input, MessageParcel &data)
{
    if (!Marshal(data, input.pid, input.uid, input.userID, input.isShowKeyboard, input.eventFlag, input.config,
                 input.state, input.isNotifyInputStart, input.needHide, input.requestKeyboardReason)) {
        IMSA_HILOGE("write InputClientInfo to message parcel failed.");
        return false;
    }
    Marshal(data, input.type, input.name);
    return true;
}

bool ITypesUtil::Unmarshalling(InputClientInfo &output, MessageParcel &data)
{
    if (!Unmarshal(data, output.pid, output.uid, output.userID, output.isShowKeyboard, output.eventFlag, output.config,
                   output.state, output.isNotifyInputStart, output.needHide, output.requestKeyboardReason)) {
        IMSA_HILOGE("read InputClientInfo from message parcel failed.");
        return false;
    }
    Unmarshal(data, output.type, output.name);
    return true;
}

bool ITypesUtil::Marshalling(const ImeWindowInfo &input, MessageParcel &data)
{
    if (!Marshal(data, static_cast<int32_t>(input.panelInfo.panelFlag),
                 static_cast<int32_t>(input.panelInfo.panelType), input.windowInfo.name, input.windowInfo.top,
                 input.windowInfo.left, input.windowInfo.width, input.windowInfo.height)) {
        IMSA_HILOGE("write InputWindowInfo to message parcel failed.");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(ImeWindowInfo &output, MessageParcel &data)
{
    int32_t panelFlag = 0;
    int32_t panelType = 0;
    InputWindowInfo windowInfo;
    if (!Unmarshal(data, panelFlag, panelType, windowInfo.name, windowInfo.top, windowInfo.left, windowInfo.width,
                   windowInfo.height)) {
        IMSA_HILOGE("read InputWindowInfo from message parcel failed.");
        return false;
    }
    output.panelInfo = { static_cast<PanelType>(panelType), static_cast<PanelFlag>(panelFlag) };
    output.windowInfo = windowInfo;
    return true;
}

bool ITypesUtil::Marshalling(const PanelStatusInfo &input, MessageParcel &data)
{
    return data.WriteInt32(static_cast<int32_t>(input.panelInfo.panelType)) &&
           data.WriteInt32(static_cast<int32_t>(input.panelInfo.panelFlag)) && data.WriteBool(input.visible) &&
           data.WriteInt32(static_cast<int32_t>(input.trigger));
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

bool ITypesUtil::Marshalling(const SysPanelStatus &input, MessageParcel &data)
{
    bool ret = data.WriteInt32(static_cast<int32_t>(input.inputType)) &&
        data.WriteInt32(input.flag) &&
        data.WriteUint32(input.width) &&
        data.WriteUint32(input.height);
        data.WriteBool(input.isMainDisplay);
    return ret;
}

bool ITypesUtil::Unmarshalling(SysPanelStatus &output, MessageParcel &data)
{
    int32_t inputType = 0;
    if (!data.ReadInt32(inputType) || !data.ReadInt32(output.flag) || !data.ReadUint32(output.width) ||
        !data.ReadUint32(output.height)) {
        return false;
    }
    output.inputType = static_cast<InputType>(inputType);
    data.ReadBool(output.isMainDisplay);
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
    IMSA_HILOGE("read ElementName from message parcel failed.");
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

bool ITypesUtil::Marshalling(const PanelInfo &input, MessageParcel &data)
{
    return data.WriteInt32(static_cast<int32_t>(input.panelType)) &&
           data.WriteInt32(static_cast<int32_t>(input.panelFlag));
}

bool ITypesUtil::Unmarshalling(PanelInfo &output, MessageParcel &data)
{
    int32_t panelType = 0;
    int32_t panelFlag = 0;
    if (!data.ReadInt32(panelType) || !data.ReadInt32(panelFlag)) {
        return false;
    }
    output.panelFlag = static_cast<PanelFlag>(panelFlag);
    output.panelType = static_cast<PanelType>(panelType);
    return true;
}

bool ITypesUtil::Marshalling(ClientState input, MessageParcel &data)
{
    return data.WriteUint32(static_cast<uint32_t>(input));
}

bool ITypesUtil::Unmarshalling(ClientState &output, MessageParcel &data)
{
    uint32_t state = 0;
    if (!data.ReadUint32(state)) {
        IMSA_HILOGE("ClientState read failed.");
        return false;
    }
    output = static_cast<ClientState>(state);
    return true;
}

bool ITypesUtil::Marshalling(SwitchTrigger input, MessageParcel &data)
{
    return data.WriteUint32(static_cast<uint32_t>(input));
}

bool ITypesUtil::Unmarshalling(SwitchTrigger &output, MessageParcel &data)
{
    uint32_t state = 0;
    if (!data.ReadUint32(state)) {
        IMSA_HILOGE("ClientState read failed.");
        return false;
    }
    output = static_cast<SwitchTrigger>(state);
    return true;
}

bool ITypesUtil::Marshalling(const PrivateDataValue &input, MessageParcel &data)
{
    size_t idx = input.index();
    if (!data.WriteInt32(static_cast<int32_t>(idx))) {
        IMSA_HILOGE("write index failed.");
        return false;
    }
    if (idx == static_cast<size_t>(PrivateDataValueType::VALUE_TYPE_STRING)) {
        auto stringValue = std::get_if<std::string>(&input);
        if (stringValue != nullptr) {
            return data.WriteString(*stringValue);
        }
    } else if (idx == static_cast<size_t>(PrivateDataValueType::VALUE_TYPE_BOOL)) {
        auto boolValue = std::get_if<bool>(&input);
        if (boolValue != nullptr) {
            return data.WriteBool(*boolValue);
        }
    } else if (idx == static_cast<size_t>(PrivateDataValueType::VALUE_TYPE_NUMBER)) {
        auto numberValue = std::get_if<int32_t>(&input);
        if (numberValue != nullptr) {
            return data.WriteInt32(*numberValue);
        }
    }
    IMSA_HILOGE("write PrivateDataValue with wrong type.");
    return false;
}

bool ITypesUtil::Unmarshalling(PrivateDataValue &output, MessageParcel &data)
{
    int32_t valueType = data.ReadInt32();
    bool res = false;
    if (valueType == static_cast<int32_t>(PrivateDataValueType::VALUE_TYPE_STRING)) {
        std::string strValue;
        res = data.ReadString(strValue);
        output.emplace<std::string>(strValue);
    } else if (valueType == static_cast<int32_t>(PrivateDataValueType::VALUE_TYPE_BOOL)) {
        bool boolValue = false;
        res = data.ReadBool(boolValue);
        output.emplace<bool>(boolValue);
    } else if (valueType == static_cast<int32_t>(PrivateDataValueType::VALUE_TYPE_NUMBER)) {
        int32_t intValue = 0;
        res = data.ReadInt32(intValue);
        output.emplace<int32_t>(intValue);
    }
    if (!res) {
        IMSA_HILOGE("read PrivateDataValue from message parcel failed.");
    }
    return res;
}

bool ITypesUtil::Marshalling(const Range &input, MessageParcel &data)
{
    if (!Marshal(data, input.start, input.end)) {
        IMSA_HILOGE("failed to write Range into message parcel.");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(Range &output, MessageParcel &data)
{
    if (!Unmarshal(data, output.start, output.end)) {
        IMSA_HILOGE("failed to read Range from message parcel.");
        return false;
    }
    return true;
}

bool ITypesUtil::Marshalling(const ArrayBuffer &input, MessageParcel &data)
{
    if (!Marshal(data, input.msgId, input.msgParam, input.jsArgc)) {
        IMSA_HILOGE("failed to write ArrayBuffer into message parcel.");
        return false;
    }
    return true;
}

bool ITypesUtil::Unmarshalling(ArrayBuffer &output, MessageParcel &data)
{
    if (!Unmarshal(data, output.msgId, output.msgParam, output.jsArgc)) {
        IMSA_HILOGE("failed to read ArrayBuffer from message parcel.");
        return false;
    }
    return true;
}

bool ITypesUtil::Marshalling(RequestKeyboardReason input, MessageParcel &data)
{
    return data.WriteInt32(static_cast<int32_t>(input));
}

bool ITypesUtil::Unmarshalling(RequestKeyboardReason &output, MessageParcel &data)
{
    int32_t ret = 0;
    if (!data.ReadInt32(ret)) {
        return false;
    }
    output = static_cast<RequestKeyboardReason>(ret);
    return true;
}
} // namespace MiscServices
} // namespace OHOS