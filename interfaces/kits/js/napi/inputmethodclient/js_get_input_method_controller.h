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

#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "global.h"
#include "js_context.h"
#include "js_input_method.h"

namespace OHOS {
namespace MiscServices {
struct StopInputContext : public ContextBase {
    bool sStopInput = false; 
    napi_status status = napi_generic_failure;
};
class JsGetInputMethodController {
public:
    JsGetInputMethodController() = default;
    ~JsGetInputMethodController() = default;
    static napi_value Init(napi_env env, napi_value info);
    static napi_value GetInputMethodController(napi_env env, napi_callback_info info);
    static napi_value StopInput(napi_env env, napi_callback_info Info);
private:
    static StopInputContext *GetStopInputContext(napi_env env, napi_callback_info info);
    static napi_value JsConstructor(napi_env env, napi_callback_info cbinfo);
    static napi_value GetErrorCodeValue(napi_env env, ErrCode errCode);
    static void CBOrPromiseStopInput(napi_env env,
        const StopInputContext *stopInput, napi_value err, napi_value data);
    static const std::string IMC_CLASS_NAME;
    static thread_local napi_ref IMSRef_;
    static constexpr int RESULT_ERROR = 0;
    static constexpr int RESULT_DATA = 1;
    static constexpr int PARAMONE = 1;
    static constexpr int PARAMZERO = 0;
    static constexpr int RESULT_ALL = 2;
    static constexpr int RESULT_COUNT = 2;
};
}
}
#endif // INTERFACE_KITS_JS_GETINPUT_METHOD_CCONTROLLER_H
