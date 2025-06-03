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
 
#include <cinttypes>
#include "input_method_utils.h"
#include "input_method_tools.h"
namespace OHOS {
namespace MiscServices {

bool PanelStatusInfoInner::ReadFromParcel(Parcel &in)
{
    std::unique_ptr<PanelInfo> panelDataInfo(in.ReadParcelable<PanelInfo>());
    if (panelDataInfo == nullptr) {
        return false;
    }
    panelInfo = *panelDataInfo;

    visible = in.ReadBool();
    int32_t triggerData = in.ReadInt32();
    trigger = static_cast<Trigger>(triggerData);
    sessionId = in.ReadUint32();
    return true;
}

bool TextSelectionInner::ReadFromParcel(Parcel &in)
{
    oldBegin = in.ReadInt32();
    oldEnd = in.ReadInt32();
    newBegin = in.ReadInt32();
    newEnd = in.ReadInt32();
    return true;
}

bool Value::ReadFromParcel(Parcel &in)
{
    uint32_t size = 0;
    size = in.ReadUint32();

    valueMap.clear();
    if (size == 0) {
        IMSA_HILOGE("size is zero!");
        return true;
    }

    if (size > MAX_VALUE_MAP_COUNT) {
        IMSA_HILOGE("size is invalid!");
        return false;
    }

    for (uint32_t index = 0; index < size; index++) {
        std::string key = in.ReadString();
        int32_t valueType = in.ReadInt32();
        if (valueType == static_cast<int32_t>(PrivateDataValueType::VALUE_TYPE_STRING)) {
            std::string strValue = in.ReadString();
            valueMap.insert(std::make_pair(key, strValue));
        } else if (valueType == static_cast<int32_t>(PrivateDataValueType::VALUE_TYPE_BOOL)) {
            bool boolValue = false;
            boolValue = in.ReadBool();
            valueMap.insert(std::make_pair(key, boolValue));
        } else if (valueType == static_cast<int32_t>(PrivateDataValueType::VALUE_TYPE_NUMBER)) {
            int32_t intValue = 0;
            intValue = in.ReadInt32();
            valueMap.insert(std::make_pair(key, intValue));
        }
    }
    return true;
}

bool TextTotalConfigInner::ReadFromParcel(Parcel &in)
{
    std::unique_ptr<InputAttributeInner> inputAttributeInfo(in.ReadParcelable<InputAttributeInner>());
    if (inputAttributeInfo == nullptr) {
        return false;
    }
    inputAttribute = *inputAttributeInfo;

    std::unique_ptr<CursorInfoInner> cInfo(in.ReadParcelable<CursorInfoInner>());
    if (cInfo == nullptr) {
        return false;
    }
    cursorInfo = *cInfo;

    std::unique_ptr<TextSelectionInner> textSelectionInfo(in.ReadParcelable<TextSelectionInner>());
    if (textSelectionInfo == nullptr) {
        return false;
    }
    textSelection = *textSelectionInfo;

    windowId = in.ReadUint32();
    positionY = in.ReadDouble();
    height = in.ReadDouble();

    std::unique_ptr<Value> commandValueInfo(in.ReadParcelable<Value>());
        if (commandValueInfo == nullptr) {
            return false;
        }
    commandValue = *commandValueInfo;
    requestKeyboardReason = static_cast<RequestKeyboardReason>(in.ReadInt32());
    abilityToken = (static_cast<MessageParcel*>(&in))->ReadRemoteObject();
    return true;
}

bool ArrayBuffer::ReadFromParcel(Parcel &in)
{
    uint64_t jsArgcData = in.ReadUint64();
    jsArgc = static_cast<size_t>(jsArgcData);
    msgId = in.ReadString();

    if (!in.ReadUInt8Vector(&msgParam)) {
        return false;
    }
    return true;
}

PanelStatusInfoInner *PanelStatusInfoInner::Unmarshalling(Parcel &in)
{
    PanelStatusInfoInner *data = new (std::nothrow) PanelStatusInfoInner();
    if (data && !data->ReadFromParcel(in)) {
        delete data;
        data = nullptr;
    }
    return data;
}

TextSelectionInner *TextSelectionInner::Unmarshalling(Parcel &in)
{
    TextSelectionInner *data = new (std::nothrow) TextSelectionInner();
    if (data && !data->ReadFromParcel(in)) {
        delete data;
        data = nullptr;
    }
    return data;
}

Value *Value::Unmarshalling(Parcel &in)
{
    Value *data = new (std::nothrow) Value();
    if (data && !data->ReadFromParcel(in)) {
        delete data;
        data = nullptr;
    }
    return data;
}

KeyEventValue *KeyEventValue::Unmarshalling(Parcel &in)
{
    KeyEventValue *data = new (std::nothrow) KeyEventValue;
    if (data == nullptr) {
        return data;
    }
    data->event = MMI::KeyEvent::Create();
    if (data->event == nullptr) {
        delete data;
        return nullptr;
    }
    if (!data->event->ReadFromParcel(in)) {
        delete data;
        data = nullptr;
    }
    return data;
}

TextTotalConfigInner *TextTotalConfigInner::Unmarshalling(Parcel &in)
{
    TextTotalConfigInner *data = new (std::nothrow) TextTotalConfigInner();
    if (data && !data->ReadFromParcel(in)) {
        delete data;
        data = nullptr;
    }
    return data;
}

ArrayBuffer *ArrayBuffer::Unmarshalling(Parcel &in)
{
    ArrayBuffer *data = new (std::nothrow) ArrayBuffer();
    if (data && !data->ReadFromParcel(in)) {
        delete data;
        data = nullptr;
    }
    return data;
}

bool PanelStatusInfoInner::Marshalling(Parcel &out) const
{
    if (!out.WriteParcelable(&panelInfo)) {
        return false;
    }

    if (!out.WriteBool(visible)) {
        return false;
    }

    if (!out.WriteInt32(static_cast<int32_t>(trigger))) {
        return false;
    }

    if (!out.WriteUint32(sessionId)) {
        return false;
    }
    return true;
}

bool TextSelectionInner::Marshalling(Parcel &out) const
{
    if (!out.WriteInt32(oldBegin)) {
        return false;
    }

    if (!out.WriteInt32(oldEnd)) {
        return false;
    }

    if (!out.WriteInt32(newBegin)) {
        return false;
    }

    if (!out.WriteInt32(newEnd)) {
        return false;
    }
    return true;
}

bool Value::Marshalling(Parcel &out) const
{
    if (!out.WriteUint32(valueMap.size())) {
    return false;
    }
    if (valueMap.size() == 0) {
        IMSA_HILOGE("valueMap size is zero!");
        return true;
    }
    for (auto& it : valueMap) {
        std::string key = it.first;
        if (!out.WriteString(key)) {
            return false;
        }
        auto value = it.second;
        bool ret = false;
        int32_t valueType = static_cast<int32_t>(value.index());
        if (!out.WriteInt32(valueType)) {
            return false;
        }
        if (valueType == static_cast<int32_t>(PrivateDataValueType::VALUE_TYPE_STRING)) {
            auto stringValue = std::get_if<std::string>(&value);
            if (stringValue != nullptr) {
                ret = out.WriteString(*stringValue);
            }
        } else if (valueType == static_cast<int32_t>(PrivateDataValueType::VALUE_TYPE_BOOL)) {
            auto boolValue = std::get_if<bool>(&value);
            if (boolValue != nullptr) {
                ret = out.WriteBool(*boolValue);
            }
        } else if (valueType == static_cast<int32_t>(PrivateDataValueType::VALUE_TYPE_NUMBER)) {
            auto numberValue = std::get_if<int32_t>(&value);
            if (numberValue != nullptr) {
                ret = out.WriteInt32(*numberValue);
            }
        }
        if (ret == false) {
            return ret;
        }
    }
    return true;
}

bool KeyEventValue::Marshalling(Parcel &out) const
{
    return event->WriteToParcel(out);
}

bool TextTotalConfigInner::Marshalling(Parcel &out) const
{
    if (!out.WriteParcelable(&inputAttribute)) {
        return false;
    }

    if (!out.WriteParcelable(&cursorInfo)) {
        return false;
    }

    if (!out.WriteParcelable(&textSelection)) {
        return false;
    }

    if (!out.WriteUint32(windowId)) {
        return false;
    }

    if (!out.WriteDouble(positionY)) {
        return false;
    }

    if (!out.WriteDouble(height)) {
        return false;
    }

    if (!out.WriteParcelable(&commandValue)) {
        return false;
    }

    if (!out.WriteInt32(static_cast<int32_t>(requestKeyboardReason))) {
        return false;
    }

    if (abilityToken != nullptr && !out.WriteRemoteObject(abilityToken)) {
        return false;
    }
    return true;
}

bool ArrayBuffer::Marshalling(Parcel &out) const
{
    if (!out.WriteUint64(static_cast<uint64_t>(jsArgc))) {
        return false;
    }
    if (!out.WriteString(msgId)) {
        return false;
    }
    if (!out.WriteUInt8Vector(msgParam)) {
        return false;
    }
    return true;
}

bool ResponseDataInner::ReadFromParcel(Parcel &in)
{
    uint64_t index = in.ReadUint64();
    switch (index) {
        case static_cast<uint64_t>(ResponseDataType::NALL_TYPE):
            rspData = std::monostate{};
            break;
        case static_cast<uint64_t>(ResponseDataType::STRING_TYPE):
            rspData = in.ReadString();
            break;
        case static_cast<uint64_t>(ResponseDataType::INT32_TYPE):
            rspData = in.ReadInt32();
            break;
        case static_cast<uint64_t>(ResponseDataType::CONFIG_TYPE): {
            TextTotalConfigInner *config = TextTotalConfigInner::Unmarshalling(in);
            if (config == nullptr) {
                IMSA_HILOGE("TextTotalConfigInner::Unmarshal is NULL");
                return false;
            }
            rspData = InputMethodTools::GetInstance().InnerToTextTotalConfig(*config);
            delete config;
            config = nullptr;
            break;
        }
        default:
            IMSA_HILOGE("bad parameter indxe: %{public}" PRIu64 "", index);
            return false;
    }
    return true;
}

bool ResponseDataInner::Marshalling(Parcel &out) const
{
    uint64_t index = static_cast<uint64_t>(rspData.index());
    if (!out.WriteUint64(index)) {
        return false;
    }
    switch (index) {
        case static_cast<uint64_t>(ResponseDataType::NALL_TYPE):
            return true;
        case static_cast<uint64_t>(ResponseDataType::STRING_TYPE): {
            if (!std::holds_alternative<std::string>(rspData)) {
                return false;
            }
            return out.WriteString(std::get<std::string>(rspData));
        }
        case static_cast<uint64_t>(ResponseDataType::INT32_TYPE): {
            if (!std::holds_alternative<int32_t>(rspData)) {
                return false;
            }
            return out.WriteInt32(std::get<int32_t>(rspData));
        }
        case static_cast<uint64_t>(ResponseDataType::CONFIG_TYPE): {
            if (!std::holds_alternative<TextTotalConfig>(rspData)) {
                return false;
            }
            TextTotalConfigInner textConfigInner =
                InputMethodTools::GetInstance().TextTotalConfigToInner(std::get<TextTotalConfig>(rspData));
            return textConfigInner.Marshalling(out);
        }
        default:
            return false;
    }
}
} // namespace MiscServices
} // namespace OHOS
