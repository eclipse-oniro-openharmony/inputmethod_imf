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

#ifndef IMF_FRAMEWORKS_SERVICE_RESPONSE_DATA_H
#define IMF_FRAMEWORKS_SERVICE_RESPONSE_DATA_H

#include <cstdint>
#include <future>
#include <unordered_map>
#include <variant>

#include "element_name.h"
#include "input_method_property.h"
#include "input_method_utils.h"
#include "response_data_util.h"

namespace OHOS {
namespace MiscServices {
using RequestId = uint32_t;
using RequestFunc = std::function<int32_t(RequestId)>;

struct StartInputResponse : public Parcelable {
    sptr<IRemoteObject> agent{ nullptr };
    int64_t pid{ 0 };
    std::string bundleName;
    void Set(sptr<IRemoteObject> imeAgent, int64_t imePid, const std::string &imeBundleName);
    bool ReadFromParcel(Parcel &in);
    bool Marshalling(Parcel &out) const override;
    static StartInputResponse *Unmarshalling(Parcel &in);
};

struct InputStartInfo : public Parcelable {
    bool isInputStart{ false };
    uint32_t callingWindowId{ 0 };
    int32_t requestKeyboardReason{ 0 };
    void Set(bool inputStart, uint32_t id, int32_t reason);
    bool ReadFromParcel(Parcel &in);
    bool Marshalling(Parcel &out) const override;
    static InputStartInfo *Unmarshalling(Parcel &in);
};

enum ServiceDataType : int32_t {
    TYPE_MONOSTATE = 0,

    // basic types
    TYPE_BOOL = 1,
    TYPE_INT32 = 2,
    TYPE_UINT32 = 3,
    TYPE_INT64 = 4,
    TYPE_UINT64 = 5,
    TYPE_REMOTE_OBJECT = 6,

    // inner types
    TYPE_PROPERTY = 7,
    TYPE_PROPERTIES = 8,
    TYPE_SUB_PROPERTY = 9,
    TYPE_SUB_PROPERTIES = 10,
    TYPE_START_INPUT_RESPONSE = 11,
    TYPE_INPUT_START_INFO = 12,

    // external types
    TYPE_AMS_ELEMENT_NAME = 13,

    TYPE_END,
};

using ServiceResponseData = std::variant<std::monostate, bool, int32_t, uint32_t, int64_t, uint64_t,
    sptr<IRemoteObject>, Property, std::vector<Property>, SubProperty, std::vector<SubProperty>, StartInputResponse,
    InputStartInfo, AppExecFwk::ElementName>;

struct ServiceResponse {
    int32_t result{ 0 };
    ServiceResponseData responseData{ std::monostate{} };
};

struct PendingRequest {
    std::promise<ServiceResponse> promise;
};

using UnmarshalFunc = std::function<void(Parcel &, ServiceResponseData &)>;
struct ServiceResponseDataInner : public Parcelable {
public:
    bool ReadFromParcel(Parcel &in);
    bool Marshalling(Parcel &out) const override;
    static ServiceResponseDataInner *Unmarshalling(Parcel &in);
    ServiceResponseData data;

private:
    static const std::unordered_map<int32_t, UnmarshalFunc> UNMARSHAL_FUNCTION_MAP;
};

struct ServiceResponseWriter {
    Parcel &out;
    bool result = true;

    explicit ServiceResponseWriter(Parcel &parcel) : out(parcel)
    {
    }

    void operator()(std::monostate val)
    {
        IMSA_HILOGD("no need to marshal");
        result = true;
    }
    void operator()(bool val)
    {
        result = out.WriteBool(val);
    }
    void operator()(int32_t val)
    {
        result = out.WriteInt32(val);
    }
    void operator()(uint32_t val)
    {
        result = out.WriteUint32(val);
    }
    void operator()(int64_t val)
    {
        result = out.WriteInt64(val);
    }
    void operator()(uint64_t val)
    {
        result = out.WriteUint64(val);
    }
    void operator()(sptr<IRemoteObject> val)
    {
        result = static_cast<MessageParcel *>(&out)->WriteRemoteObject(val);
    }
    void operator()(const Property &val)
    {
        result = val.Marshalling(out);
    }
    void operator()(const std::vector<Property> &val)
    {
        result = ResponseDataUtil::Marshall(val, out);
    }
    void operator()(const SubProperty &val)
    {
        result = val.Marshalling(out);
    }
    void operator()(const std::vector<SubProperty> &val)
    {
        result = ResponseDataUtil::Marshall(val, out);
    }
    void operator()(const StartInputResponse &val)
    {
        result = val.Marshalling(out);
    }
    void operator()(const InputStartInfo &val)
    {
        result = val.Marshalling(out);
    }
    void operator()(const AppExecFwk::ElementName &val)
    {
        result = val.Marshalling(out);
    }
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_FRAMEWORKS_SERVICE_RESPONSE_DATA_H
