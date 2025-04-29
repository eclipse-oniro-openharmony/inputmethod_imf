/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "js_util.h"

#include <iomanip>
#include <sstream>

#include "string_ex.h"

namespace OHOS {
namespace MiscServices {
constexpr int64_t JS_NUMBER_MAX_VALUE = (1LL << 53) - 1;
constexpr int32_t HEX_BYTE_WIDTH = 2;
napi_valuetype JsUtil::GetType(napi_env env, napi_value in)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, in, &valueType);
    return valueType;
}
bool JsUtil::HasProperty(napi_env env, napi_value object, const std::string &property)
{
    bool hasProperty = false;
    napi_status status = napi_has_named_property(env, object, property.c_str(), &hasProperty);
    if (status == napi_ok && hasProperty) {
        return true;
    }
    return false;
}
bool JsUtil::GetValue(napi_env env, napi_value in, std::string &out)
{
    size_t size = 0;
    auto status = napi_get_value_string_utf8(env, in, nullptr, 0, &size);
    if (status != napi_ok) {
        return false;
    }
    out.resize(size + 1, 0);
    status = napi_get_value_string_utf8(env, in, const_cast<char *>(out.data()), size + 1, &size);
    out.resize(size);
    return status == napi_ok;
}

bool JsUtil::GetValue(napi_env env, napi_value in, std::u16string &out)
{
    std::string tempOut;
    bool ret = GetValue(env, in, tempOut);
    if (ret) {
        out = Str8ToStr16(tempOut);
    }
    return ret;
}
bool JsUtil::GetValue(napi_env env, napi_value in, int32_t &out)
{
    return napi_get_value_int32(env, in, &out) == napi_ok;
}
bool JsUtil::GetValue(napi_env env, napi_value in, uint32_t &out)
{
    return napi_get_value_uint32(env, in, &out) == napi_ok;
}
bool JsUtil::GetValue(napi_env env, napi_value in, int64_t &out)
{
    return napi_get_value_int64(env, in, &out) == napi_ok;
}
bool JsUtil::GetValue(napi_env env, napi_value in, bool &out)
{
    return napi_get_value_bool(env, in, &out) == napi_ok;
}
bool JsUtil::GetValue(napi_env env, napi_value in, double &out)
{
    return napi_get_value_double(env, in, &out) == napi_ok;
}
napi_value JsUtil::GetValue(napi_env env, napi_value in)
{
    return in;
}
napi_value JsUtil::GetValue(napi_env env, const std::string &in)
{
    napi_value out = nullptr;
    napi_create_string_utf8(env, in.c_str(), in.length(), &out);
    return out;
}
napi_value JsUtil::GetValue(napi_env env, int32_t in)
{
    napi_value out = nullptr;
    napi_create_int32(env, in, &out);
    return out;
}
napi_value JsUtil::GetValue(napi_env env, uint32_t in)
{
    napi_value out = nullptr;
    napi_create_uint32(env, in, &out);
    return out;
}
napi_value JsUtil::GetValue(napi_env env, int64_t in)
{
    if (in > JS_NUMBER_MAX_VALUE) {
        // cannot exceed the range of js
        return nullptr;
    }
    napi_value out = nullptr;
    napi_create_int64(env, in, &out);
    return out;
}
napi_value JsUtil::GetValue(napi_env env, bool in)
{
    napi_value out = nullptr;
    napi_get_boolean(env, in, &out);
    return out;
}

napi_value JsUtil::GetValueU16String(napi_env env, const std::u16string &in)
{
    napi_value out = nullptr;
    napi_create_string_utf16(env, in.c_str(), in.length(), &out);
    return out;
}

std::string JsUtil::ToHex(const std::string &in)
{
    std::stringstream ss;
    if (in.size() < 1) {
        return "";
    }
    for (size_t i = 0; i < in.size(); i++) {
        ss << std::uppercase << std::hex << std::setw(sizeof(uint8_t) * HEX_BYTE_WIDTH)
        << std::setfill('0') << static_cast<uint8_t>(in.at(i));
    }
    return ss.str();
}

std::string JsUtil::ToHex(const std::u16string &in)
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

bool JsUtil::GetValueU16String(napi_env env, napi_value in, std::u16string &out)
{
    size_t size = 0;
    auto status = napi_get_value_string_utf16(env, in, nullptr, 0, &size);
    if (status != napi_ok) {
        return false;
    }
    out.resize(size + 1, 0);
    status = napi_get_value_string_utf16(env, in, out.data(), size + 1, &size);
    out.resize(size);
    return status == napi_ok;
}
} // namespace MiscServices
} // namespace OHOS