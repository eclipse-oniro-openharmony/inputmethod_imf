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

#include "string_utils.h"

#include <iomanip>
#include <sstream>

#include "unicode/ustring.h"
#include "unicode/unistr.h"

#include "global.h"

namespace OHOS {
namespace MiscServices {
constexpr int32_t HEX_BYTE_WIDTH = 2;

std::string StringUtils::ToHex(const std::string &in)
{
    std::stringstream ss;
    if (in.size() < 1) {
        return "";
    }
    for (size_t i = 0; i < in.size(); i++) {
        ss << std::uppercase << std::hex << std::setw(sizeof(uint8_t) * HEX_BYTE_WIDTH)
        << std::setfill('0') << static_cast<char16_t>(in.at(i));
    }
    return ss.str();
}

std::string StringUtils::ToHex(const std::u16string &in)
{
    std::stringstream ss;
    if (in.size() < 1) {
        return "";
    }
    for (size_t i = 0; i < in.size(); i++) {
        ss << std::uppercase << std::hex << std::setw(sizeof(char16_t) * HEX_BYTE_WIDTH)
        << std::setfill('0') << in.at(i);
    }
    return ss.str();
}

int32_t StringUtils::CountUtf16Chars(const std::u16string &in)
{
    int32_t ret = u_countChar32(in.data(), in.size());
    IMSA_HILOGD("size:%{public}zu,ret:%{public}d", in.size(), ret);
    return ret;
}

void StringUtils::TruncateUtf16String(std::u16string &in, int32_t maxChars)
{
    const UChar* src = in.data();
    size_t srcLen = in.size();
    size_t offset = 0;
    int32_t count = 0;
    if (maxChars < 0  || srcLen <= maxChars) {
        IMSA_HILOGD("srcLen:%{public}zu,maxChars:%{public}d", srcLen, maxChars);
        return;
    }
    while (offset < srcLen && count < maxChars) {
        UChar32 c;
        U16_NEXT(src, offset, srcLen, c);
        if (c == U_SENTINEL) {
            break;
        }
        count++;
    }
    IMSA_HILOGD("srcLen:%{public}zu,maxChars:%{public}d,resultLen:%{public}zu,count:%{public}d",
        srcLen, maxChars, offset, count);
    if (offset < srcLen) {
        IMSA_HILOGI("chars length exceeds limit,maxChars:%{public}d,offset:%{public}zu", maxChars, offset);
        in.resize(offset);
    }
    return;
}
} // namespace MiscServices
} // namespace OHOS