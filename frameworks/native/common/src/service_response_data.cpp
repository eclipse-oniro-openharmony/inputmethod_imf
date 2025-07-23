/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "service_response_data.h"

namespace OHOS {
namespace MiscServices {
const std::unordered_map<int32_t, UnmarshalFunc> ServiceResponseDataInner::UNMARSHAL_FUNCTION_MAP = {
    { static_cast<int32_t>(TYPE_MONOSTATE), [](Parcel &in, ServiceResponseData &out) { out = std::monostate{}; } },
    { static_cast<int32_t>(TYPE_BOOL), [](Parcel &in, ServiceResponseData &out) { out = in.ReadBool(); } },
    { static_cast<int32_t>(TYPE_INT32), [](Parcel &in, ServiceResponseData &out) { out = in.ReadInt32(); } },
    { static_cast<int32_t>(TYPE_UINT32), [](Parcel &in, ServiceResponseData &out) { out = in.ReadUint32(); } },
    { static_cast<int32_t>(TYPE_INT64), [](Parcel &in, ServiceResponseData &out) { out = in.ReadInt64(); } },
    { static_cast<int32_t>(TYPE_UINT64), [](Parcel &in, ServiceResponseData &out) { out = in.ReadUint64(); } },
    { static_cast<int32_t>(TYPE_REMOTE_OBJECT),
        [](Parcel &in, ServiceResponseData &out) { out = static_cast<MessageParcel *>(&in)->ReadRemoteObject(); } },
    { static_cast<int32_t>(TYPE_PROPERTY),
        [](Parcel &in, ServiceResponseData &out) {
            Property value;
            value.ReadFromParcel(in);
            out = value;
        } },
    { static_cast<int32_t>(TYPE_PROPERTIES),
        [](Parcel &in, ServiceResponseData &out) {
            std::vector<Property> value;
            ResponseDataUtil::Unmarshall(in, value);
            out = value;
        } },
    { static_cast<int32_t>(TYPE_SUB_PROPERTY),
        [](Parcel &in, ServiceResponseData &out) {
            SubProperty value;
            value.ReadFromParcel(in);
            out = value;
        } },
    { static_cast<int32_t>(TYPE_SUB_PROPERTIES),
        [](Parcel &in, ServiceResponseData &out) {
            std::vector<SubProperty> value;
            ResponseDataUtil::Unmarshall(in, value);
            out = value;
        } },
    { static_cast<int32_t>(TYPE_START_INPUT_RESPONSE),
        [](Parcel &in, ServiceResponseData &out) {
            StartInputResponse value;
            value.ReadFromParcel(in);
            out = value;
        } },
    { static_cast<int32_t>(TYPE_INPUT_START_INFO),
        [](Parcel &in, ServiceResponseData &out) {
            InputStartInfo value;
            value.ReadFromParcel(in);
            out = value;
        } },
    { static_cast<int32_t>(TYPE_AMS_ELEMENT_NAME),
        [](Parcel &in, ServiceResponseData &out) {
            AppExecFwk::ElementName value;
            value.ReadFromParcel(in);
            out = value;
        } },
};
bool ServiceResponseDataInner::ReadFromParcel(Parcel &in)
{
    int32_t valueType = 0;
    if (!in.ReadInt32(valueType)) {
        return false;
    }
    if (valueType < static_cast<int32_t>(ServiceDataType::TYPE_MONOSTATE)
        || valueType >= static_cast<int32_t>(ServiceDataType::TYPE_END)) {
        IMSA_HILOGE("invalid value type");
        return false;
    }
    auto iter = UNMARSHAL_FUNCTION_MAP.find(valueType);
    if (iter == UNMARSHAL_FUNCTION_MAP.end()) {
        return false;
    }
    auto handler = iter->second;
    if (handler == nullptr) {
        return false;
    }
    handler(in, data);
    return true;
}

bool ServiceResponseDataInner::Marshalling(Parcel &out) const
{
    int32_t valueType = static_cast<int32_t>(data.index());
    if (valueType < static_cast<int32_t>(ServiceDataType::TYPE_MONOSTATE)
        || valueType >= static_cast<int32_t>(ServiceDataType::TYPE_END)) {
        IMSA_HILOGE("invalid value type");
        return false;
    }
    if (!out.WriteInt32(valueType)) {
        IMSA_HILOGE("failed to write valueType");
        return false;
    }
    if (valueType == static_cast<int32_t>(ServiceDataType::TYPE_MONOSTATE)) {
        return true;
    }
    ServiceResponseWriter writer{ out };
    std::visit(writer, data);
    if (!writer.result) {
        IMSA_HILOGE("failed to write response data");
        return false;
    }
    return true;
}

ServiceResponseDataInner *ServiceResponseDataInner::Unmarshalling(Parcel &in)
{
    auto data = new (std::nothrow) ServiceResponseDataInner();
    if (data && !data->ReadFromParcel(in)) {
        delete data;
        data = nullptr;
    }
    return data;
}

void StartInputResponse::Set(sptr<IRemoteObject> imeAgent, int64_t imePid, const std::string &imeBundleName)
{
    agent = imeAgent;
    pid = imePid;
    bundleName = imeBundleName;
}

bool StartInputResponse::ReadFromParcel(Parcel &in)
{
    agent = static_cast<MessageParcel *>(&in)->ReadRemoteObject();
    if (agent == nullptr) {
        return false;
    }
    if (!in.ReadInt64(pid)) {
        return false;
    }
    if (!in.ReadString(bundleName)) {
        return false;
    }
    return true;
}

bool StartInputResponse::Marshalling(Parcel &out) const
{
    if (!static_cast<MessageParcel *>(&out)->WriteRemoteObject(agent)) {
        return false;
    }
    if (!out.WriteInt64(pid)) {
        return false;
    }
    if (!out.WriteString(bundleName)) {
        return false;
    }
    return true;
}

StartInputResponse *StartInputResponse::Unmarshalling(Parcel &in)
{
    auto data = new (std::nothrow) StartInputResponse();
    if (data == nullptr) {
        return nullptr;
    }
    if (!data->ReadFromParcel(in)) {
        delete data;
        return nullptr;
    }
    return data;
}

void InputStartInfo::Set(bool inputStart, uint32_t id, int32_t reason)
{
    isInputStart = inputStart;
    callingWindowId = id;
    requestKeyboardReason = reason;
}

bool InputStartInfo::ReadFromParcel(Parcel &in)
{
    if (!in.ReadBool(isInputStart)) {
        return false;
    }
    if (!in.ReadUint32(callingWindowId)) {
        return false;
    }
    if (!in.ReadInt32(requestKeyboardReason)) {
        return false;
    }
    return true;
}

bool InputStartInfo::Marshalling(Parcel &out) const
{
    if (!out.WriteBool(isInputStart)) {
        return false;
    }
    if (!out.WriteUint32(callingWindowId)) {
        return false;
    }
    if (!out.WriteInt32(requestKeyboardReason)) {
        return false;
    }
    return true;
}

InputStartInfo *InputStartInfo::Unmarshalling(Parcel &in)
{
    auto data = new (std::nothrow) InputStartInfo();
    if (data == nullptr) {
        return nullptr;
    }
    if (!data->ReadFromParcel(in)) {
        delete data;
        return nullptr;
    }
    return data;
}
} // namespace MiscServices
} // namespace OHOS