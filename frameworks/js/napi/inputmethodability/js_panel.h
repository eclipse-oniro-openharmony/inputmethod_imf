/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_IMF_JSPANEL_H
#define INPUTMETHOD_IMF_JSPANEL_H

#include <uv.h>

#include "async_call.h"
#include "input_method_panel.h"
#include "js_callback_object.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"

namespace OHOS {
namespace MiscServices {
class JsPanel {
public:
    JsPanel() = default;
    ~JsPanel();
    static napi_value Constructor(napi_env env);
    static napi_value SetUiContent(napi_env env, napi_callback_info info);
    static napi_value Resize(napi_env env, napi_callback_info info);
    static napi_value MoveTo(napi_env env, napi_callback_info info);
    static napi_value Show(napi_env env, napi_callback_info info);
    static napi_value Hide(napi_env env, napi_callback_info info);
    static napi_value ChangeFlag(napi_env env, napi_callback_info info);
    static napi_value Subscribe(napi_env env, napi_callback_info info);
    static napi_value UnSubscribe(napi_env env, napi_callback_info info);
    void SetNative(const std::shared_ptr<InputMethodPanel> &panel);
    std::shared_ptr<InputMethodPanel> &GetNative();
    static thread_local napi_ref panelConstructorRef_;
private:
    struct PanelContentContext : public AsyncCall::Context {
        std::string path = "";
        uint32_t width = 0;
        uint32_t height = 0;
        int32_t x = 0;
        int32_t y = 0;
        void *native = nullptr;
        std::shared_ptr<NativeReference> contentStorage = nullptr;
        PanelContentContext(napi_env env, napi_callback_info info) : Context(nullptr, nullptr)
        {
            napi_value self = nullptr;
            napi_status status = napi_get_cb_info(env, info, 0, nullptr, &self, nullptr);
            status = napi_unwrap(env, self, &native);
        };
        PanelContentContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};
        napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
        {
            NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
            return Context::operator()(env, argc, argv, self);
        }
        napi_status operator()(napi_env env, napi_value *result) override
        {
            if (status_ != napi_ok) {
                output_ = nullptr;
                return status_;
            }
            return Context::operator()(env, result);
        }
    };
    static napi_value JsNew(napi_env env, napi_callback_info info);
    static std::shared_ptr<InputMethodPanel> UnwrapPanel(napi_env env, napi_value thisVar);
    static const std::string CLASS_NAME;
    static constexpr size_t ARGC_MAX = 6;
    std::shared_ptr<InputMethodPanel> inputMethodPanel_ = nullptr;
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUTMETHOD_IMF_JSPANEL_H
