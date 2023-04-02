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
#ifndef INTERFACE_KITS_JS_GET_INPUT_METHOD_CONTROLLER_H
#define INTERFACE_KITS_JS_GET_INPUT_METHOD_CONTROLLER_H

#include "async_call.h"
#include "controller_listener.h"
#include "global.h"
#include "js_callback_object.h"
#include "js_input_method.h"

namespace OHOS {
namespace MiscServices {
struct HandleContext : public AsyncCall::Context {
    bool isHandle = false;
    napi_status status = napi_generic_failure;
    HandleContext() : Context(nullptr, nullptr){};
    HandleContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

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

class JsGetInputMethodController : public ControllerListener {
public:
    JsGetInputMethodController() = default;
    ~JsGetInputMethodController() = default;
    static napi_value Init(napi_env env, napi_value info);
    static napi_value GetController(napi_env env, napi_callback_info cbInfo);
    static napi_value GetInputMethodController(napi_env env, napi_callback_info cbInfo);
    static std::shared_ptr<JsGetInputMethodController> GetInstance();
    static napi_value HandleSoftKeyboard(napi_env env, napi_callback_info info, std::function<int32_t()> callback,
        bool isOutput, bool needThrowException);
    static napi_value HideSoftKeyboard(napi_env env, napi_callback_info info);
    static napi_value ShowSoftKeyboard(napi_env env, napi_callback_info info);
    static napi_value StopInputSession(napi_env env, napi_callback_info info);
    static napi_value StopInput(napi_env env, napi_callback_info info);
    static napi_value Subscribe(napi_env env, napi_callback_info info);
    static napi_value UnSubscribe(napi_env env, napi_callback_info info);
    void OnSelectByRange(int32_t start, int32_t end) override;
    void OnSelectByMovement(int32_t direction) override;

private:
    static napi_value JsConstructor(napi_env env, napi_callback_info cbinfo);
    static napi_value GetIMController(napi_env env, napi_callback_info cbInfo, bool needThrowException);
    static napi_value CreateSelectRange(napi_env env, int32_t start, int32_t end);
    static napi_value CreateSelectMovement(napi_env env, int32_t direction);
    void RegisterListener(napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj);
    void UnRegisterListener(std::string type);
    struct UvEntry {
        std::vector<std::shared_ptr<JSCallbackObject>> vecCopy;
        std::string type;
        int32_t start = 0;
        int32_t end = 0;
        int32_t direction = 0;
        explicit UvEntry(const std::vector<std::shared_ptr<JSCallbackObject>> &cbVec, const std::string &type)
            : vecCopy(cbVec), type(type)
        {
        }
    };
    using EntrySetter = std::function<void(UvEntry &)>;
    uv_work_t *GetUVwork(const std::string &type, EntrySetter entrySetter = nullptr);
    uv_loop_s *loop_ = nullptr;
    std::recursive_mutex mutex_;
    std::map<std::string, std::vector<std::shared_ptr<JSCallbackObject>>> jsCbMap_;
    static std::mutex controllerMutex_;
    static std::shared_ptr<JsGetInputMethodController> controller_;
    static const std::string IMC_CLASS_NAME;
    static thread_local napi_ref IMCRef_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INTERFACE_KITS_JS_GET_INPUT_METHOD_CONTROLLER_H
