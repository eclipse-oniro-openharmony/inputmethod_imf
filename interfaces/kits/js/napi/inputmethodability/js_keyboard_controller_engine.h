/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef INTERFACE_KITS_JS_KEYBOARD_CONTROLLER_H
#define INTERFACE_KITS_JS_KEYBOARD_CONTROLLER_H

#include "async_call.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {
struct HideKeyboardContext : public AsyncCall::Context {
    napi_status status = napi_generic_failure;
    HideKeyboardContext() : Context(nullptr, nullptr) { };
    HideKeyboardContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)) { };

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

class JsKeyboardControllerEngine {
public:
    JsKeyboardControllerEngine() = default;
    ~JsKeyboardControllerEngine() = default;
    static napi_value Init(napi_env env, napi_value info);
    static napi_value GetKeyboardControllerInstance(napi_env env);
    static napi_value HideKeyboard(napi_env env, napi_callback_info info);
private:
    static napi_value JsConstructor(napi_env env, napi_callback_info info);
    static napi_value GetJSInputMethodProperty(napi_env env, std::vector<Property> &properties);
    static const std::string KCE_CLASS_NAME;
    static thread_local napi_ref KCERef_;
    };
}
}
#endif // INTERFACE_KITS_JS_KEYBOARD_CONTROLLER_H