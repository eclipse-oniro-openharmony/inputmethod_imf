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

#ifndef INTERFACE_KITS_JS_INPUT_METHOD_ENGINE_SETTING_H
#define INTERFACE_KITS_JS_INPUT_METHOD_ENGINE_SETTING_H

#include <uv.h>

#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include "async_call.h"
#include "global.h"
#include "input_method_engine_listener.h"
#include "input_method_property.h"
#include "js_callback_object.h"
#include "napi/native_api.h"

namespace OHOS {
namespace MiscServices {
class JsInputMethodEngineSetting : public InputMethodEngineListener {
public:
    JsInputMethodEngineSetting() = default;
    ~JsInputMethodEngineSetting() override = default;
    static napi_value Init(napi_env env, napi_value info);
    static napi_value GetInputMethodEngine(napi_env env, napi_callback_info info);
    static napi_value GetInputMethodAbility(napi_env env, napi_callback_info info);
    static napi_value Subscribe(napi_env env, napi_callback_info info);
    static napi_value UnSubscribe(napi_env env, napi_callback_info info);
    static napi_value MoveCursor(napi_env env, napi_callback_info info);
    void OnInputStart() override;
    void OnKeyboardStatus(bool isShow) override;
    void OnInputStop(std::string imeId) override;
    void OnSetCallingWindow(uint32_t windowId) override;
    void OnSetSubtype(const SubProperty &property) override;

private:
    static napi_value JsConstructor(napi_env env, napi_callback_info cbinfo);
    static JsInputMethodEngineSetting *GetNative(napi_env env, napi_callback_info info);
    static std::shared_ptr<JsInputMethodEngineSetting> GetInputMethodEngineSetting();
    static bool Equals(napi_env env, napi_value value, napi_ref copy, std::thread::id threadId);
    static napi_value GetJsConstProperty(napi_env env, uint32_t num);
    static napi_value GetIntJsConstProperty(napi_env env, int32_t num);
    static napi_value GetIMEInstance(napi_env env, napi_callback_info info, int flag);
    void RegisterListener(napi_value callback, std::string type,
        std::shared_ptr<JSCallbackObject> callbackObj);
    void UnRegisterListener(napi_value callback, std::string type);
    uv_work_t *GetUVwork(std::string type);
    uv_work_t *GetStopInputUVwork(std::string type, std::string imeId);
    uv_work_t *GetWindowIDUVwork(std::string type, uint32_t windowid);
    uv_work_t *GetSubtypeUVwork(std::string type, const SubProperty &property);
    static napi_value GetResultOnSetSubtype(napi_env env, const SubProperty &property);
    static std::string GetStringProperty(napi_env env, napi_value jsString);
    static constexpr int32_t MAX_VALUE_LEN = 1024;
    static const std::string IMES_CLASS_NAME;
    static thread_local napi_ref IMESRef_;
    struct UvEntry {
        std::vector<std::shared_ptr<JSCallbackObject>> vecCopy;
        std::string type;
        std::string imeid;
        uint32_t windowid = 0;
        SubProperty subProperty;
        UvEntry(std::vector<std::shared_ptr<JSCallbackObject>> cbVec, std::string type)
            : vecCopy(cbVec), type(type) {}
    };
    uv_loop_s *loop_ = nullptr;
    std::recursive_mutex mutex_;
    std::map<std::string, std::vector<std::shared_ptr<JSCallbackObject>>> jsCbMap_;
    static std::mutex engineMutex_;
    static std::shared_ptr<JsInputMethodEngineSetting> inputMethodEngine_;
};
}
}
#endif // INTERFACE_KITS_JS_INPUT_METHOD_ENGINE_SETTING_H