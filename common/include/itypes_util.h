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
#include "global.h"
#include "input_client_info.h"
#include "input_window_info.h"
#include "message_parcel.h"
#include "panel_info.h"
#include "sys_panel_status.h"

namespace OHOS {
namespace MiscServices {
class ITypesUtil final {
public:
    static bool Marshal(MessageParcel &data);
    static bool Unmarshal(MessageParcel &data);

    static bool Marshalling(int32_t input, MessageParcel &data);
    static bool Unmarshalling(int32_t &output, MessageParcel &data);

    static bool Marshalling(const std::string &input, MessageParcel &data);
    static bool Unmarshalling(std::string &output, MessageParcel &data);

    template<typename T, typename... Types>
    static bool Marshal(MessageParcel &parcel, const T &first, const Types &...others);
    template<typename T, typename... Types>
    static bool Unmarshal(MessageParcel &parcel, T &first, Types &...others);
};

template<typename T, typename... Types>
bool ITypesUtil::Marshal(MessageParcel &parcel, const T &first, const Types &...others)
{
    if (!Marshalling(first, parcel)) {
        return false;
    }
    return Marshal(parcel, others...);
}

template<typename T, typename... Types>
bool ITypesUtil::Unmarshal(MessageParcel &parcel, T &first, Types &...others)
{
    if (!Unmarshalling(first, parcel)) {
        return false;
    }
    return Unmarshal(parcel, others...);
}
} // namespace MiscServices
} // namespace OHOS
#endif
