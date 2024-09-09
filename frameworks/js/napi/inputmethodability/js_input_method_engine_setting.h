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
#include <unordered_map>

#include "async_call.h"
#include "event_handler.h"
#include "foundation/ability/ability_runtime/interfaces/kits/native/appkit/ability_runtime/context/context.h"
#include "global.h"
#include "input_method_engine_listener.h"
#include "input_method_panel.h"
#include "input_method_property.h"
#include "js_callback_object.h"
#include "js_panel.h"
#include "napi/native_api.h"

namespace OHOS {
namespace MiscServices {
class JsInputMethodEngineSetting : public InputMethodEngineListener {
public:
    JsInputMethodEngineSetting() = default;
    ~JsInputMethodEngineSetting() override = default;
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value InitProperty(napi_env env, napi_value exports);
    static napi_value GetInputMethodEngine(napi_env env, napi_callback_info info);
    static napi_value GetInputMethodAbility(napi_env env, napi_callback_info info);
    static napi_value Subscribe(napi_env env, napi_callback_info info);
    static napi_value UnSubscribe(napi_env env, napi_callback_info info);
    static napi_value CreatePanel(napi_env env, napi_callback_info info);
    static napi_value DestroyPanel(napi_env env, napi_callback_info info);
    static napi_value GetSecurityMode(napi_env env, napi_callback_info info);
    void OnInputStart() override;
    void OnKeyboardStatus(bool isShow) override;
    int32_t OnInputStop() override;
    void OnSetCallingWindow(uint32_t windowId) override;
    void OnSetSubtype(const SubProperty &property) override;
    void OnSecurityChange(int32_t security) override;
    void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;
    bool PostTaskToEventHandler(std::function<void()> task, const std::string &taskName) override;

private:
    struct PanelContext : public AsyncCall::Context {
        PanelInfo panelInfo = PanelInfo();
        std::shared_ptr<InputMethodPanel> panel = nullptr;
        std::shared_ptr<OHOS::AbilityRuntime::Context> context = nullptr;
        PanelContext() : Context(nullptr, nullptr){};
        PanelContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

        napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
        {
            CHECK_RETURN(self != nullptr, "self is nullptr", napi_invalid_arg);
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

    static napi_value JsConstructor(napi_env env, napi_callback_info cbinfo);
    static std::shared_ptr<JsInputMethodEngineSetting> GetInputMethodEngineSetting();
    static bool InitInputMethodSetting();
    static napi_value GetJsConstProperty(napi_env env, uint32_t num);
    static napi_value GetJsPanelTypeProperty(napi_env env);
    static napi_value GetJsPanelFlagProperty(napi_env env);
    static napi_value GetJsDirectionProperty(napi_env env);
    static napi_value GetJsExtendActionProperty(napi_env env);
    static napi_value GetJsSecurityModeProperty(napi_env env);
    static napi_value GetIntJsConstProperty(napi_env env, int32_t num);
    static napi_value GetIMEInstance(napi_env env, napi_callback_info info);
    static napi_status GetContext(napi_env env, napi_value in,
            std::shared_ptr<OHOS::AbilityRuntime::Context> &context);
    void RegisterListener(napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj);
    void UnRegisterListener(napi_value callback, std::string type);
    static napi_value GetResultOnSetSubtype(napi_env env, const SubProperty &property);
    static const std::string IMES_CLASS_NAME;
    static thread_local napi_ref IMESRef_;
    struct UvEntry {
        std::vector<std::shared_ptr<JSCallbackObject>> vecCopy;
        std::string type;
        uint32_t windowid = 0;
        int32_t security = 0;
        SubProperty subProperty;
        std::unordered_map<std::string, PrivateDataValue> privateCommand;
        UvEntry(const std::vector<std::shared_ptr<JSCallbackObject>> &cbVec, const std::string &type)
            : vecCopy(cbVec), type(type)
        {
        }
    };
    using EntrySetter = std::function<void(UvEntry &)>;
    static std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler();
    std::shared_ptr<UvEntry> GetEntry(const std::string &type, EntrySetter entrySetter = nullptr);
    uv_work_t *GetUVwork(const std::string &type, EntrySetter entrySetter = nullptr);
    void FreeWorkIfFail(int ret, uv_work_t *work);
    uv_loop_s *loop_ = nullptr;
    std::recursive_mutex mutex_;
    std::map<std::string, std::vector<std::shared_ptr<JSCallbackObject>>> jsCbMap_;
    static std::mutex engineMutex_;
    static std::shared_ptr<JsInputMethodEngineSetting> inputMethodEngine_;
    static std::mutex eventHandlerMutex_;
    static std::shared_ptr<AppExecFwk::EventHandler> handler_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INTERFACE_KITS_JS_INPUT_METHOD_ENGINE_SETTING_H