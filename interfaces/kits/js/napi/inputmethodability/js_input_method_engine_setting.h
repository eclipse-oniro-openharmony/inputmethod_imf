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

#ifndef INTERFACE_KITS_JS_INPUT_METHOD_ENGINE_SETTING_H
#define INTERFACE_KITS_JS_INPUT_METHOD_ENGINE_SETTING_H

#include "napi/native_api.h"
#include <map>
#include <uv.h>
#include <mutex>
#include "global.h"
#include "js_context.h"

namespace OHOS {
namespace MiscServices {
class CallbackObj {
public:
    CallbackObj(napi_env env, napi_value callback);
    ~CallbackObj();
    napi_ref callback_ = nullptr;
    napi_env env_;
};

class JsInputMethodEngineSetting {
public:
    JsInputMethodEngineSetting() = default;
    ~JsInputMethodEngineSetting() = default;
    static napi_value Init(napi_env env, napi_value info);
    static napi_value GetInputMethodEngine(napi_env env, napi_callback_info info); 
    static napi_value Subscribe(napi_env env, napi_callback_info info);
    static napi_value UnSubscribe(napi_env env, napi_callback_info info);
    static napi_value MoveCursor(napi_env env, napi_callback_info info);
    void OnInputStart();
    void OnKeyboardStatus(bool isShow);
    void OnInputStop(std::string);
    void OnSetCallingWindow(uint32_t);
private:
    static napi_value JsConstructor(napi_env env, napi_callback_info cbinfo);
    static JsInputMethodEngineSetting *GetNative(napi_env env, napi_callback_info info);
    static bool Equals(napi_env env, napi_value value, napi_ref copy);
    static napi_value GetJsConstProperty(napi_env env, uint32_t num);
    void RegisterListener(napi_value callback, std::string type,
        std::shared_ptr<CallbackObj> CallbackObj);
    void UnRegisterListener(napi_value callback, std::string type);
    static std::string GetStringProperty(napi_env env, napi_value obj);
    static constexpr int32_t MAX_VALUE_LEN = 1024;
    static const std::string IMES_CLASS_NAME;
    static thread_local napi_ref IMESRef_;
    struct UvEntry {
        std::vector<std::shared_ptr<CallbackObj>> vecCopy;
        std::string type;
        UvEntry(std::vector<std::shared_ptr<CallbackObj>> cbVec, 
                std::string type) 
        : vecCopy(cbVec), type(type) {}
    };
    uv_loop_s *loop_ = nullptr;
    std::recursive_mutex mutex_;
    std::map<std::string, std::vector<std::shared_ptr<CallbackObj>>> jsCbMap_;
};
}
}
#endif // INTERFACE_KITS_JS_INPUT_METHOD_ENGINE_SETTING_H