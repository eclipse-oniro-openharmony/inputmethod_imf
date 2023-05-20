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

#ifndef OHOS_JS_UTIL_H
#define OHOS_JS_UTIL_H
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
namespace OHOS {
namespace MiscServices {
class JsUtil {
public:
    static napi_valuetype GetType(napi_env env, napi_value in);
    // js to native
    static bool GetValue(napi_env env, napi_value in, std::string &out);
    static bool GetValue(napi_env env, napi_value in, int32_t &out);
    static bool GetValue(napi_env env, napi_value in, uint32_t &out);
    static bool GetValue(napi_env env, napi_value in, int64_t &out);
    static bool GetValue(napi_env env, napi_value in, bool &out);
    template<typename T> static bool GetValue(napi_env env, napi_value in, std::vector<T> &items)
    {
        uint32_t len = 0;
        napi_get_array_length(env, in, &len);
        items.resize(len);
        for (uint32_t i = 0; i < len; i++) {
            napi_value item = nullptr;
            auto status = napi_get_element(env, in, i, &item);
            T buff{};
            if (status != napi_ok || !GetValue(env, item, buff)) {
                return false;
            }
            items[i] = std::move(buff);
        }
        return true;
    }

    // native to js
    static napi_value GetValue(napi_env env, const std::string &in);
    static napi_value GetValue(napi_env env, int32_t in);
    static napi_value GetValue(napi_env env, uint32_t in);
    static napi_value GetValue(napi_env env, int64_t in);
    static napi_value GetValue(napi_env env, bool in);
    template<typename T> static napi_value GetValue(napi_env env, const std::vector<T> &items)
    {
        napi_value array = nullptr;
        auto status = napi_create_array(env, &array);
        if (status != napi_ok) {
            return nullptr;
        }
        uint32_t index = 0;
        for (const T &item : items) {
            auto itemValue = GetValue(env, item);
            if (itemValue == nullptr) {
                return nullptr;
            }
            status = napi_set_element(env, array, index++, itemValue);
            if (status != napi_ok) {
                return nullptr;
            }
        }
        return array;
    }
    class Object {
    public:
        template<typename T>
        static bool WriteProperty(napi_env env, napi_value object, const std::string &property, const T &value)
        {
            return napi_set_named_property(env, object, property.c_str(), GetValue(env, value)) == napi_ok;
        }
        template<typename T>
        static bool ReadProperty(napi_env env, napi_value object, const std::string &property, T &value)
        {
            napi_value propValue = nullptr;
            napi_get_named_property(env, object, property.c_str(), &propValue);
            return GetValue(env, propValue, value);
        }
    };
    class Const {
    public:
        static napi_value Null(napi_env env)
        {
            napi_value value = nullptr;
            napi_get_null(env, &value);
            return value;
        }
        static napi_value Undefined(napi_env env)
        {
            napi_value value = nullptr;
            napi_get_undefined(env, &value);
            return value;
        }
    };
    class ScopeGuard {
    public:
        explicit ScopeGuard(napi_env env) : env_(env), scope_(nullptr)
        {
            napi_open_handle_scope(env_, &scope_);
        }
        ~ScopeGuard()
        {
            napi_close_handle_scope(env_, scope_);
        }

    private:
        napi_env env_;
        napi_handle_scope scope_;
    };
};
} // namespace MiscServices
} // namespace OHOS
#endif // OHOS_JS_UTIL_H
