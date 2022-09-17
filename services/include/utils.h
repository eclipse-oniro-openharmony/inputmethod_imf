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

/*! \file utils.h */

#ifndef SERVICES_INCLUDE_UTILS_H
#define SERVICES_INCLUDE_UTILS_H

#include <codecvt>
#include <iostream>
#include <locale>
#include <string>

#include "input_method_controller.h"
#include "input_method_property.h"
#include "string_ex.h"

namespace OHOS {
namespace MiscServices {
    class Utils {
    public:
        static constexpr int USER_ID_CHANGE_VALUE = 200000;
        static std::string to_utf8(std::u16string str16)
        {
            return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.to_bytes(str16);
        }
        static std::u16string to_utf16(std::string str)
        {
            return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(str);
        }
        static std::vector<Property> GetProperty(const std::vector<InputMethodProperty> &properties)
        {
            std::vector<Property> props;
            for (const auto &property : properties) {
                props.push_back({ Str16ToStr8(property.mPackageName), Str16ToStr8(property.mAbilityName) });
            }
            return props;
        }
        static uint32_t GetUserId(uint32_t uid)
        {
            return uid / USER_ID_CHANGE_VALUE;
        }
    };
} // namespace MiscServices
} // namespace OHOS

#endif // SERVICES_INCLUDE_UTILS_H
