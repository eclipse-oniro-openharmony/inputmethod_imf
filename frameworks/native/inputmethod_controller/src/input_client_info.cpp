/*
* Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "input_client_info.h"

namespace OHOS {
namespace MiscServices {

bool InputClientInfoInner::ReadFromParcel(Parcel &in)
{
    pid = in.ReadInt32();
    uid = in.ReadInt32();
    userID = in.ReadInt32();
    displayId = in.ReadUint64();
    isShowKeyboard = in.ReadBool();
    int32_t bindImeTypeData = in.ReadInt32();
    bindImeType = static_cast<ImeType>(bindImeTypeData);

    std::unique_ptr<TextTotalConfigInner> configInfo(in.ReadParcelable<TextTotalConfigInner>());
    if (configInfo == nullptr) {
        return false;
    }
    config = *configInfo;

    eventFlag = in.ReadUint32();

    std::unique_ptr<InputAttributeInner> attributeInfo(in.ReadParcelable<InputAttributeInner>());
    if (attributeInfo == nullptr) {
        return false;
    }
    attribute = *attributeInfo;

    if (in.ReadBool()) {
        sptr<IRemoteObject> clientObj;
        clientObj = (static_cast<MessageParcel*>(&in))->ReadRemoteObject();
        client = iface_cast<IInputClient>(clientObj);
    }

    if (in.ReadBool()) {
        channel = (static_cast<MessageParcel*>(&in))->ReadRemoteObject();
    }
    uint32_t stateData = in.ReadUint32();
    state = static_cast<ClientState>(stateData);
    isNotifyInputStart = in.ReadBool();
    needHide = in.ReadBool();
    uiExtensionTokenId = in.ReadUint32();
    int32_t requestKeyboardReasonData = in.ReadInt32();
    requestKeyboardReason = static_cast<RequestKeyboardReason>(requestKeyboardReasonData);
    uint32_t typeData = in.ReadUint32();
    type = static_cast<ClientType>(typeData);
    name = in.ReadString();
    return true;
}

InputClientInfoInner *InputClientInfoInner::Unmarshalling(Parcel &in)
{
    InputClientInfoInner *data = new (std::nothrow) InputClientInfoInner();
    if (data && !data->ReadFromParcel(in)) {
        delete data;
        data = nullptr;
    }
    return data;
}

bool InputClientInfoInner::Marshalling(Parcel &out) const
{
    if (!out.WriteInt32(pid)) {
        return false;
    }
    if (!out.WriteInt32(uid)) {
        return false;
    }
    if (!out.WriteInt32(userID)) {
        return false;
    }
    if (!out.WriteUint64(displayId)) {
        return false;
    }
    if (!out.WriteBool(isShowKeyboard)) {
        return false;
    }
    if (!out.WriteInt32(static_cast<int32_t>(bindImeType))) {
        return false;
    }
    if (!out.WriteParcelable(&config)) {
        return false;
    }
    if (!out.WriteUint32(eventFlag)) {
        return false;
    }
    if (!out.WriteParcelable(&attribute)) {
        return false;
    }
    if (!MarshallingTwo(out)) {
        return false;
    }
    if (!MarshallingOne(out)) {
        return false;
    }
    return true;
}

bool InputClientInfoInner::MarshallingTwo(Parcel &out) const
{
    if (client == nullptr) {
        if (!out.WriteBool(false)) {
            return false;
        }
    } else {
        if (!out.WriteBool(true)) {
            return false;
        }
        if (!out.WriteRemoteObject(client->AsObject())) {
            return false;
        }
    }
    if (channel == nullptr) {
        if (!out.WriteBool(false)) {
            return false;
        }
    } else {
        if (!out.WriteBool(true)) {
            return false;
        }
        if (!out.WriteRemoteObject(channel)) {
            return false;
        }
    }
    return true;
}

bool InputClientInfoInner::MarshallingOne(Parcel &out) const
{
    if (!out.WriteUint32(static_cast<uint32_t>(state))) {
        return false;
    }
    if (!out.WriteBool(isNotifyInputStart)) {
        return false;
    }
    if (!out.WriteBool(needHide)) {
        return false;
    }
    if (!out.WriteUint32(uiExtensionTokenId)) {
        return false;
    }
    if (!out.WriteInt32(static_cast<int32_t>(requestKeyboardReason))) {
        return false;
    }
    if (!out.WriteUint32(static_cast<uint32_t>(type))) {
        return false;
    }
    if (!out.WriteString(name)) {
        return false;
    }
    return true;
}
} // namespace MiscServices
} // namespace OHOS