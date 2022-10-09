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
#ifndef INTERFACE_KITS_JS_GETINPUT_METHOD_CCONTROLLER_H
#define INTERFACE_KITS_JS_GETINPUT_METHOD_CCONTROLLER_H

#include "async_call.h"
#include "global.h"
#include "js_input_method.h"

namespace OHOS {
namespace MiscServices {
struct HandleContext : public AsyncCall::Context {
    bool isHandle = false;
    napi_status status = napi_generic_failure;
    HandleContext() : Context(nullptr, nullptr) { };
    HandleContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)) { };

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

class JsGetInputMethodController {
public:
    JsGetInputMethodController() = default;
    ~JsGetInputMethodController() = default;
    static napi_value Init(napi_env env, napi_value info);
    static napi_value GetController(napi_env env, napi_callback_info cbInfo);
    static napi_value HandleSoftKeyboard(
        napi_env env, napi_callback_info info, std::function<int32_t()> callback, bool isOutput, bool v9Flag);
    static napi_value HideSoftKeyboard(napi_env env, napi_callback_info info);
    static napi_value ShowSoftKeyboard(napi_env env, napi_callback_info info);
    static napi_value StopInputSession(napi_env env, napi_callback_info info);
    static napi_value StopInput(napi_env env, napi_callback_info info);

private:
    static napi_value JsConstructor(napi_env env, napi_callback_info cbinfo);
    static const std::string IMC_CLASS_NAME;
    static thread_local napi_ref IMCRef_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INTERFACE_KITS_JS_GETINPUT_METHOD_CCONTROLLER_H
