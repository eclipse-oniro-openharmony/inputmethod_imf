/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef INTERFACE_KITS_JS_GETINPUT_METHOD_SETTING_H
#define INTERFACE_KITS_JS_GETINPUT_METHOD_SETTING_H

#include "async_call.h"
#include "global.h"
#include "input_method_controller.h"
#include "input_method_status.h"

namespace OHOS {
namespace MiscServices {
struct ListInputContext : public AsyncCall::Context {
    InputMethodStatus inputMethodStatus;
    std::vector<Property> properties;
    napi_status status = napi_generic_failure;
    ListInputContext() : Context(nullptr, nullptr) { };
    ListInputContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)) { };

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct DisplayOptionalInputMethodContext : public AsyncCall::Context {
    napi_status status = napi_generic_failure;
    DisplayOptionalInputMethodContext() : Context(nullptr, nullptr) { };
    DisplayOptionalInputMethodContext(InputAction input, OutputAction output)
        : Context(std::move(input), std::move(output)) { };

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct GetInputMethodControllerContext : public AsyncCall::Context {
    bool isStopInput;
    napi_status status = napi_generic_failure;
    GetInputMethodControllerContext() : Context(nullptr, nullptr) { };
    GetInputMethodControllerContext(InputAction input, OutputAction output)
        : Context(std::move(input), std::move(output)) { };

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            return status;
        }
        return Context::operator()(env, result);
    }
};

class JsGetInputMethodSetting {
public:
    JsGetInputMethodSetting() = default;
    ~JsGetInputMethodSetting() = default;
    static napi_value Init(napi_env env, napi_value info);
    static napi_value GetInputMethodSetting(napi_env env, napi_callback_info info);
    static napi_value ListInputMethod(napi_env env, napi_callback_info info);
    static napi_value DisplayOptionalInputMethod(napi_env env, napi_callback_info info);
private:
    static napi_value JsConstructor(napi_env env, napi_callback_info cbinfo);
    static const std::string IMS_CLASS_NAME;
    static thread_local napi_ref IMSRef_;
    };
}
}
#endif // INTERFACE_KITS_JS_GETINPUT_METHOD_SETTING_H
