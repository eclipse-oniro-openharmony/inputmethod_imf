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

#ifndef IMF_FRAMEWORKS_RESPONSE_DATA_UTIL_H
#define IMF_FRAMEWORKS_RESPONSE_DATA_UTIL_H

#include <cstdint>
#include <variant>

#include "input_method_property.h"
#include "input_method_utils.h"

namespace OHOS {
namespace MiscServices {
inline constexpr size_t MAX_SIZE = 102400;
class ResponseDataUtil {
public:
    template<typename T> static bool Unmarshall(Parcel &in, std::vector<T> &out)
    {
        int32_t len = 0;
        if (!in.ReadInt32(len)) {
            return false;
        }
        if (len < 0) {
            return false;
        }
        auto size = static_cast<size_t>(len);
        if (size > MAX_SIZE) {
            return false;
        }
        out.clear();
        for (size_t i = 0; i < size; ++i) {
            T value;
            if (!value.ReadFromParcel(in)) {
                return false;
            }
            out.push_back(value);
        }
        return true;
    }

    template<typename T> static bool Marshall(const std::vector<T> &in, Parcel &out)
    {
        auto size = in.size();
        if (size > INT32_MAX) {
            return false;
        }
        if (!out.WriteInt32(static_cast<int32_t>(size))) {
            return false;
        }
        for (const auto &it : in) {
            if (!it.Marshalling(out)) {
                return false;
            }
        }
        return true;
    }
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_FRAMEWORKS_RESPONSE_DATA_UTIL_H
