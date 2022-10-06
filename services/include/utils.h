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

#ifndef SERVICES_INCLUDE_UTILS_H
#define SERVICES_INCLUDE_UTILS_H

#include <string>

#include "input_method_property.h"
#include "input_method_status.h"
#include "string_ex.h"

namespace OHOS ::MiscServices {
class Utils {
public:
    static constexpr int USER_ID_CHANGE_VALUE = 200000;

    static std::string ToStr8(const std::u16string &str16)
    {
        return Str16ToStr8(str16);
    }

    static std::u16string ToStr16(const std::string &str)
    {
        return Str8ToStr16(str);
    }

    static std::vector<Property> ToProperty(const std::vector<InputMethodProperty> &properties)
    {
        std::vector<Property> props;
        for (const auto &property : properties) {
            props.push_back({ Str16ToStr8(property.mPackageName), Str16ToStr8(property.mAbilityName) });
        }
        return props;
    }

    static Property ToProperty(const InputMethodProperty &property)
    {
        return { Str16ToStr8(property.mPackageName), Str16ToStr8(property.mAbilityName) };
    }

    static uint32_t ToUserId(uint32_t uid)
    {
        return uid / USER_ID_CHANGE_VALUE;
    }
};
} // namespace OHOS::MiscServices

#endif // SERVICES_INCLUDE_UTILS_H
