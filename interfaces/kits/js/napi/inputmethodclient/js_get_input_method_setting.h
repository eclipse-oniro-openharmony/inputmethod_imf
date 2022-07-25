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
#include "napi/native_api.h"
#include "global.h"
#include "input_method_property.h"
#include "js_context.h"

namespace OHOS {
namespace MiscServices {
constexpr int RESULT_ERROR = 0;
constexpr int RESULT_DATA = 1;
constexpr int PARAZERO = 1;
constexpr int PARAMONE = 1;
constexpr int RESULT_ALL = 2;
constexpr int RESULT_COUNT = 2;
const std::string IMS_CLASS_NAME = "InputMethodSetting";
static thread_local napi_ref IMSRef_ = nullptr;
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
    static void GetResult(napi_env env, std::vector<InputMethodProperty*> &properties, napi_value &result);
    static void ProcessCallbackOrPromiseCBArray(napi_env env,ContextBase *ctxt);
    static void ProcessCallbackOrPromise(napi_env env, ContextBase *ctxt);
    };
}
} 
#endif // INTERFACE_KITS_JS_GETINPUT_METHOD_SETTING_H
