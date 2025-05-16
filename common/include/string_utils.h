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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_STRING_UTILS_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_STRING_UTILS_H

#include <string>

namespace OHOS {
namespace MiscServices {
class StringUtils final {
public:
    static std::string ToHex(const std::string &in);
    static std::string ToHex(const std::u16string &in);
    static int32_t CountUtf16Chars(const std::u16string &in);
    static void TruncateUtf16String(std::u16string &in, int32_t maxChars);
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_STRING_UTILS_H