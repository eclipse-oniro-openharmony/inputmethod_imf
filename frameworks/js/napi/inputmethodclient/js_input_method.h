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
#ifndef INTERFACE_KITS_JS_INPUT_METHOD_H
#define INTERFACE_KITS_JS_INPUT_METHOD_H

#include "async_call.h"
#include "global.h"
#include "input_method_controller.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"

namespace OHOS {
namespace MiscServices {
struct SwitchInputMethodContext : public AsyncCall::Context {
    bool isSwitchInput = false;
    std::string packageName;
    std::string methodId;
    std::string name;
    std::string id;
    napi_status status = napi_generic_failure;
    SwitchInputMethodContext() : Context(nullptr, nullptr){};
    SwitchInputMethodContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            output_ = nullptr;
            return status;
        }
        return Context::operator()(env, result);
    }
};

class JsInputMethod {
public:
    JsInputMethod() = default;
    ~JsInputMethod() = default;
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value SwitchInputMethod(napi_env env, napi_callback_info info);
    static napi_value SwitchCurrentInputMethodSubtype(napi_env env, napi_callback_info info);
    static napi_value SwitchCurrentInputMethodAndSubtype(napi_env env, napi_callback_info info);
    static napi_value GetCurrentInputMethodSubtype(napi_env env, napi_callback_info info);
    static napi_value GetCurrentInputMethod(napi_env env, napi_callback_info info);
    static int32_t GetNumberProperty(napi_env env, napi_value obj);
    static napi_value GetJsInputMethodProperty(napi_env env, const Property &property);
    static napi_value GetJSInputMethodSubProperties(napi_env env, const std::vector<SubProperty> &subProperties);
    static napi_value GetJSInputMethodProperties(napi_env env, const std::vector<Property> &properties);
    static std::string GetStringProperty(napi_env env, napi_value obj);
    static napi_value GetJsInputMethodSubProperty(napi_env env, const SubProperty &subProperty);

private:
    static napi_status GetInputMethodProperty(
        napi_env env, napi_value argv, std::shared_ptr<SwitchInputMethodContext> ctxt);
    static napi_status GetInputMethodSubProperty(
        napi_env env, napi_value argv, std::shared_ptr<SwitchInputMethodContext> ctxt);
    static constexpr std::int32_t MAX_VALUE_LEN = 4096;
    static constexpr size_t PARAM_POS_TWO = 2;
    static constexpr size_t PARAM_POS_ONE = 1;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INTERFACE_KITS_JS_INPUT_METHOD_H
