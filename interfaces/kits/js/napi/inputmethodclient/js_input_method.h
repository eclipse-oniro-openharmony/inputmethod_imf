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

#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "global.h"
#include "js_context.h"

namespace OHOS {
namespace MiscServices {
struct SwitchInputMethodContext : public ContextBase {
    bool sSwitchInput = false;
    std::string packageName;
    std::string methodId;
    napi_status status = napi_generic_failure;
};
    
class JsInputMethod {
public:
    JsInputMethod() = default;
    ~JsInputMethod() = default;
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value SwitchInputMethod(napi_env env, napi_callback_info info);
private:
    static SwitchInputMethodContext *GetSwitchInputContext(napi_env env, napi_callback_info info);
    static napi_value JsConstructor(napi_env env, napi_callback_info cbinfo);
    static napi_value GetErrorCodeValue(napi_env env, int errCode);
    static void CallbackOrPromiseSwitchInput(napi_env env,
        const SwitchInputMethodContext *switchInput, napi_value err, napi_value data);
    static std::string GetStringProperty(napi_env env, napi_value obj);
    
    static thread_local napi_ref IMSRef_ ;
    static constexpr int RESULT_ERROR = 0;
    static constexpr int RESULT_DATA = 1;
    static constexpr int PARAMONE = 1;
    static constexpr int PARAMZERO = 0;
    static constexpr int RESULT_ALL = 2;
    static constexpr int RESULT_COUNT = 2;
    static constexpr std::int32_t MAX_VALUE_LEN = 4096;
    static const std::string IMS_CLASS_NAME;
};
}
}
#endif // INTERFACE_KITS_JS_INPUT_METHOD_H
