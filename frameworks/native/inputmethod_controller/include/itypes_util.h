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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_ITYPES_UTIL_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_ITYPES_UTIL_H

#include <climits>
#include <map>
#include <memory>

#include "element_name.h"
#include "event_status_manager.h"
#include "global.h"
#include "input_client_info.h"
#include "input_window_info.h"
#include "message_parcel.h"
#include "panel_info.h"

namespace OHOS {
namespace MiscServices {
class ITypesUtil final {
public:
    static bool Marshal(MessageParcel &data);
    static bool Unmarshal(MessageParcel &data);

    static bool Marshalling(bool input, MessageParcel &data);
    static bool Unmarshalling(bool &output, MessageParcel &data);

    static bool Marshalling(uint32_t input, MessageParcel &data);
    static bool Unmarshalling(uint32_t &output, MessageParcel &data);

    static bool Marshalling(int32_t input, MessageParcel &data);
    static bool Unmarshalling(int32_t &output, MessageParcel &data);

    static bool Marshalling(uint64_t input, MessageParcel &data);
    static bool Unmarshalling(uint64_t &output, MessageParcel &data);

    static bool Marshalling(double input, MessageParcel &data);
    static bool Unmarshalling(double &output, MessageParcel &data);

    static bool Marshalling(const std::u16string &input, MessageParcel &data);
    static bool Unmarshalling(std::u16string &output, MessageParcel &data);

    static bool Marshalling(const std::string &input, MessageParcel &data);
    static bool Unmarshalling(std::string &output, MessageParcel &data);

    static bool Marshalling(const std::vector<uint8_t> &input, MessageParcel &data);
    static bool Unmarshalling(std::vector<uint8_t> &output, MessageParcel &data);

    static bool Marshalling(const sptr<IRemoteObject> &input, MessageParcel &data);
    static bool Unmarshalling(sptr<IRemoteObject> &output, MessageParcel &data);

    static bool Marshalling(const Property &input, MessageParcel &data);
    static bool Unmarshalling(Property &output, MessageParcel &data);

    static bool Marshalling(const SubProperty &input, MessageParcel &data);
    static bool Unmarshalling(SubProperty &output, MessageParcel &data);

    static bool Marshalling(const InputAttribute &input, MessageParcel &data);
    static bool Unmarshalling(InputAttribute &output, MessageParcel &data);

    static bool Marshalling(const InputClientInfo &input, MessageParcel &data);
    static bool Unmarshalling(InputClientInfo &output, MessageParcel &data);

    static bool Marshalling(const InputWindowInfo &input, MessageParcel &data);
    static bool Unmarshalling(InputWindowInfo &output, MessageParcel &data);

    static bool Marshalling(const TextTotalConfig &input, MessageParcel &data);
    static bool Unmarshalling(TextTotalConfig &output, MessageParcel &data);

    static bool Marshalling(const PanelStatusInfo &info, MessageParcel &data);
    static bool Unmarshalling(PanelStatusInfo &info, MessageParcel &data);

    static bool Marshalling(EventType input, MessageParcel &data);
    static bool Unmarshalling(EventType &output, MessageParcel &data);

    static bool Marshalling(InputType input, MessageParcel &data);
    static bool Unmarshalling(InputType &output, MessageParcel &data);

    static bool Marshalling(const OHOS::AppExecFwk::ElementName &input, MessageParcel &data);
    static bool Unmarshalling(OHOS::AppExecFwk::ElementName &output, MessageParcel &data);

    static bool Marshalling(const PanelInfo &input, MessageParcel &data);
    static bool Unmarshalling(PanelInfo &output, MessageParcel &data);

    static bool Marshalling(ClientState input, MessageParcel &data);
    static bool Unmarshalling(ClientState &output, MessageParcel &data);

    template<class T>
    static bool Marshalling(const std::vector<T> &val, MessageParcel &parcel);
    template<class T>
    static bool Unmarshalling(std::vector<T> &val, MessageParcel &parcel);

    template<class K, class V>
    static bool Marshalling(const std::map<K, V> &val, MessageParcel &parcel);
    template<class K, class V>
    static bool Unmarshalling(std::map<K, V> &val, MessageParcel &parcel);

    template<typename T, typename... Types>
    static bool Marshal(MessageParcel &parcel, const T &first, const Types &... others);
    template<typename T, typename... Types>
    static bool Unmarshal(MessageParcel &parcel, T &first, Types &... others);
};

template<class T>
bool ITypesUtil::Marshalling(const std::vector<T> &val, MessageParcel &parcel)
{
    if (val.size() > INT_MAX) {
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(val.size()))) {
        return false;
    }

    for (auto &v : val) {
        if (!Marshalling(v, parcel)) {
            return false;
        }
    }
    return true;
}

template<class T>
bool ITypesUtil::Unmarshalling(std::vector<T> &val, MessageParcel &parcel)
{
    int32_t len = parcel.ReadInt32();
    if (len < 0) {
        return false;
    }

    size_t readAbleSize = parcel.GetReadableBytes();
    size_t size = static_cast<size_t>(len);
    if ((size > readAbleSize) || (size > val.max_size())) {
        return false;
    }

    val.resize(size);
    if (val.size() < size) {
        return false;
    }

    for (auto &v : val) {
        if (!Unmarshalling(v, parcel)) {
            return false;
        }
    }

    return true;
}

template<typename T, typename... Types>
bool ITypesUtil::Marshal(MessageParcel &parcel, const T &first, const Types &... others)
{
    if (!Marshalling(first, parcel)) {
        return false;
    }
    return Marshal(parcel, others...);
}

template<typename T, typename... Types>
bool ITypesUtil::Unmarshal(MessageParcel &parcel, T &first, Types &... others)
{
    if (!Unmarshalling(first, parcel)) {
        return false;
    }
    return Unmarshal(parcel, others...);
}

template<class K, class V>
bool ITypesUtil::Marshalling(const std::map<K, V> &result, MessageParcel &parcel)
{
    if (!parcel.WriteInt32(static_cast<int32_t>(result.size()))) {
        return false;
    }
    for (const auto &entry : result) {
        if (!Marshalling(entry.first, parcel)) {
            return false;
        }
        if (!Marshalling(entry.second, parcel)) {
            return false;
        }
    }
    return true;
}

template<class K, class V>
bool ITypesUtil::Unmarshalling(std::map<K, V> &val, MessageParcel &parcel)
{
    int32_t size = 0;
    if (!parcel.ReadInt32(size)) {
        return false;
    }
    if (size < 0) {
        return false;
    }

    size_t readAbleSize = parcel.GetReadableBytes();
    size_t len = static_cast<size_t>(size);
    if ((len > readAbleSize) || len > val.max_size()) {
        return false;
    }

    for (int32_t i = 0; i < size; i++) {
        K key;
        if (!Unmarshalling(key, parcel)) {
            return false;
        }
        V value;
        if (!Unmarshalling(value, parcel)) {
            return false;
        }
        val.insert({ key, value });
    }
    return true;
}
} // namespace MiscServices
} // namespace OHOS
#endif
