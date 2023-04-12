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

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
namespace OHOS {
constexpr size_t MAX_ARGC = 2;
struct Property {
    std::string name;
    std::string id;
    std::string icon;
    int32_t iconId = 0;
};
struct JsProperty {
    static napi_value Write(napi_env env, const std::shared_ptr<Property> &nativeObject)
    {
        if (nativeObject == nullptr) {
            return JsUtil::Const::Null(env);
        }
        return Write(env, *nativeObject);
    }
    static napi_value Write(napi_env env, const Property &nativeObject)
    {
        napi_value object = nullptr;
        napi_create_object(env, &object);
        auto ret = JsUtil::Object::WriteProperty(env, object, "name", nativeObject.name);
        ret = ret && JsUtil::Object::WriteProperty(env, object, "id", nativeObject.id);
        ret = ret && JsUtil::Object::WriteProperty(env, object, "icon", nativeObject.icon);
        ret = ret && JsUtil::Object::WriteProperty(env, object, "iconId", nativeObject.iconId);
        return ret ? object : JsUtil::Const::Null(env);
    }
    static bool Read(napi_env env, napi_value object, Property &nativeObject)
    {
        auto ret = JsUtil::Object::ReadProperty(env, object, "name", nativeObject.name);
        ret = ret && JsUtil::Object::ReadProperty(env, object, "id", nativeObject.id);
        ret = ret && JsUtil::Object::ReadProperty(env, object, "icon", nativeObject.icon);
        ret = ret && JsUtil::Object::ReadProperty(env, object, "iconId", nativeObject.iconId);
        return ret;
    }
};

using Action = std::function<napi_value(napi_value in)>;
static napi_value Handle(napi_env env, napi_callback_info info, const Action &action)
{
    size_t argc = MAX_ARGC;
    napi_value args[MAX_ARGC];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    return action(args[0]);
}
static napi_value GetInt32(napi_env env, napi_callback_info info)
{
    return Handle(env, info, [env](napi_value in) -> napi_value {
        int32_t test = 0;
        JsUtil::GetValue(env, in, test);
        return JsUtil::GetValue(env, test);
    });
}
static napi_value GetInt64(napi_env env, napi_callback_info info)
{
    return Handle(env, info, [env](napi_value in) -> napi_value {
        int64_t test = 0;
        JsUtil::GetValue(env, in, test);
        return JsUtil::GetValue(env, test);
    });
}
static napi_value GetUint32(napi_env env, napi_callback_info info)
{
    return Handle(env, info, [env](napi_value in) -> napi_value {
        uint32_t test = 0;
        JsUtil::GetValue(env, in, test);
        return JsUtil::GetValue(env, test);
    });
}
static napi_value GetBool(napi_env env, napi_callback_info info)
{
    return Handle(env, info, [env](napi_value in) -> napi_value {
        bool test = false;
        JsUtil::GetValue(env, in, test);
        return JsUtil::GetValue(env, test);
    });
}
static napi_value GetString(napi_env env, napi_callback_info info)
{
    return Handle(env, info, [env](napi_value in) -> napi_value {
        std::string test;
        JsUtil::GetValue(env, in, test);
        return JsUtil::GetValue(env, test);
    });
}
static napi_value GetObject(napi_env env, napi_callback_info info)
{
    return Handle(env, info, [env](napi_value in) -> napi_value {
        Property prop{};
        JsProperty::Read(env, in, prop);
        return JsProperty::Write(env, prop);
    });
}
static napi_value GetArrayString(napi_env env, napi_callback_info info)
{
    return Handle(env, info, [env](napi_value in) -> napi_value {
        std::vector<std::string> test;
        JsUtil::GetValue(env, in, test);
        return JsUtil::GetValue(env, test);
    });
}
static napi_value GetArrayInt32(napi_env env, napi_callback_info info)
{
    return Handle(env, info, [env](napi_value in) -> napi_value {
        std::vector<std::int32_t> test;
        JsUtil::GetValue(env, in, test);
        return JsUtil::GetValue(env, test);
    });
}
static napi_value GetArrayBool(napi_env env, napi_callback_info info)
{
    return Handle(env, info, [env](napi_value in) -> napi_value {
        std::vector<bool> test;
        JsUtil::GetValue(env, in, test);
        return JsUtil::GetValue(env, test);
    });
}
static napi_value GetNull(napi_env env, napi_callback_info info)
{
    return JsUtil::Const::Null(env);
}
static napi_value GetUndefined(napi_env env, napi_callback_info info)
{
    return JsUtil::Const::Undefined(env);
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getInt32", GetInt32),
        DECLARE_NAPI_FUNCTION("getUint32", GetUint32),
        DECLARE_NAPI_FUNCTION("getInt64", GetInt64),
        DECLARE_NAPI_FUNCTION("getBool", GetBool),
        DECLARE_NAPI_FUNCTION("getString", GetString),
        DECLARE_NAPI_FUNCTION("getObject", GetObject),
        DECLARE_NAPI_FUNCTION("getArrayString", GetArrayString),
        DECLARE_NAPI_FUNCTION("getArrayInt32", GetArrayInt32),
        DECLARE_NAPI_FUNCTION("getArrayBool", GetArrayBool),
        DECLARE_NAPI_FUNCTION("getNull", GetNull),
        DECLARE_NAPI_FUNCTION("getUndefined", GetUndefined),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(*properties), properties));

    return exports;
}
EXTERN_C_END

/*
 * module define
 */
static napi_module _module = { .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "ohos_js_util",
    .nm_priv = ((void *)0),
    .reserved = { 0 } };
/*
 * module register
 */
extern "C" __attribute__((constructor)) void Register()
{
    napi_module_register(&_module);
}
} // namespace OHOS