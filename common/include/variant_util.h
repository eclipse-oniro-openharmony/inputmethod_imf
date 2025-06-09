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

#ifndef OHOS_VARIANT_UTIL_H
#define OHOS_VARIANT_UTIL_H
#include <cstdint>
#include <variant>

#include "global.h"
#include "input_client_info.h"
#include "input_method_utils.h"
namespace OHOS {
namespace MiscServices {
class VariantUtil {
public:
    template<typename T>
    static bool GetValue(
        const std::variant<bool, uint32_t, ImeType, ClientState, TextTotalConfig, ClientType> &input, T &output)
    {
        if (!std::holds_alternative<T>(input)) {
            return false;
        }
        output = std::get<T>(input);
        return true;
    }

    template<typename T>
    static bool GetValue(const ResponseData &input, T &output)
    {
        if (!std::holds_alternative<T>(input)) {
            return false;
        }
        output = std::get<T>(input);
        return true;
    }
};
} // namespace MiscServices
} // namespace OHOS
#endif // OHOS_VARIANT_UTIL_H
