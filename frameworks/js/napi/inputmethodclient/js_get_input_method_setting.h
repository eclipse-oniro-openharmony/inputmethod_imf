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

#include <uv.h>

#include "async_call.h"
#include "event_handler.h"
#include "global.h"
#include "input_method_controller.h"
#include "input_method_status.h"
#include "js_callback_object.h"

namespace OHOS {
namespace MiscServices {
struct ListInputContext : public AsyncCall::Context {
    InputMethodStatus inputMethodStatus = InputMethodStatus::ALL;
    std::vector<Property> properties;
    std::vector<SubProperty> subProperties;
    Property property;
    napi_status status = napi_generic_failure;
    ListInputContext() : Context(nullptr, nullptr){};
    ListInputContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr!", napi_invalid_arg);
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

struct DisplayOptionalInputMethodContext : public AsyncCall::Context {
    napi_status status = napi_generic_failure;
    bool isDisplayed = false;
    DisplayOptionalInputMethodContext() : Context(nullptr, nullptr){};
    DisplayOptionalInputMethodContext(InputAction input, OutputAction output)
        : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr!", napi_invalid_arg);
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

struct GetInputMethodControllerContext : public AsyncCall::Context {
    bool isStopInput = false;
    napi_status status = napi_generic_failure;
    GetInputMethodControllerContext() : Context(nullptr, nullptr){};
    GetInputMethodControllerContext(InputAction input, OutputAction output)
        : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr!", napi_invalid_arg);
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

struct EnableInputContext : public AsyncCall::Context {
    std::string bundleName;
    std::string extName;
    EnabledStatus enabledStatus{ EnabledStatus::DISABLED };
    napi_status status = napi_generic_failure;
    EnableInputContext() : Context(nullptr, nullptr){};
    EnableInputContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        CHECK_RETURN(self != nullptr, "self is nullptr!", napi_invalid_arg);
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

struct GetInputMethodStateContext : public AsyncCall::Context {
    napi_status status = napi_generic_failure;
    EnabledStatus enableStatus = EnabledStatus::DISABLED;
    GetInputMethodStateContext() : Context(nullptr, nullptr){};
    GetInputMethodStateContext(InputAction input, OutputAction output)
        : Context(std::move(input), std::move(output)){};
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            output_ = nullptr;
            return status;
        }
        return Context::operator()(env, result);
    }
};

class JsGetInputMethodSetting : public ImeEventListener {
public:
    JsGetInputMethodSetting() = default;
    ~JsGetInputMethodSetting() = default;
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value GetSetting(napi_env env, napi_callback_info info);
    static napi_value GetInputMethodSetting(napi_env env, napi_callback_info info);
    static napi_value ListInputMethod(napi_env env, napi_callback_info info);
    static napi_value ListInputMethodSubtype(napi_env env, napi_callback_info info);
    static napi_value ListCurrentInputMethodSubtype(napi_env env, napi_callback_info info);
    static napi_value GetInputMethods(napi_env env, napi_callback_info info);
    static napi_value GetInputMethodsSync(napi_env env, napi_callback_info info);
    static napi_value GetAllInputMethods(napi_env env, napi_callback_info info);
    static napi_value GetAllInputMethodsSync(napi_env env, napi_callback_info info);
    static napi_value DisplayOptionalInputMethod(napi_env env, napi_callback_info info);
    static napi_value ShowOptionalInputMethods(napi_env env, napi_callback_info info);
    static napi_value IsPanelShown(napi_env env, napi_callback_info info);
    static napi_value EnableInputMethod(napi_env env, napi_callback_info info);
    static napi_value GetInputMethodState(napi_env env, napi_callback_info info);
    static napi_value Subscribe(napi_env env, napi_callback_info info);
    static napi_value UnSubscribe(napi_env env, napi_callback_info info);
    static std::shared_ptr<JsGetInputMethodSetting> GetInputMethodSettingInstance();
    void OnImeChange(const Property &property, const SubProperty &subProperty) override;
    void OnImeShow(const ImeWindowInfo &info) override;
    void OnImeHide(const ImeWindowInfo &info) override;

private:
    static napi_status GetInputMethodProperty(napi_env env, napi_value argv, std::shared_ptr<ListInputContext> ctxt);
    static napi_value JsConstructor(napi_env env, napi_callback_info cbinfo);
    static napi_value GetJsConstProperty(napi_env env, uint32_t num);
    static napi_value GetIMSetting(napi_env env, napi_callback_info info, bool needThrowException);
    static std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler();
    int32_t RegisterListener(napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj);
    void UnRegisterListener(napi_value callback, std::string type, bool &isUpdateFlag);
    void OnPanelStatusChange(const std::string &type, const InputWindowInfo &info);
    struct UvEntry {
        std::vector<std::shared_ptr<JSCallbackObject>> vecCopy;
        std::string type;
        Property property;
        SubProperty subProperty;
        std::vector<InputWindowInfo> windowInfo;
        UvEntry(const std::vector<std::shared_ptr<JSCallbackObject>> &cbVec, const std::string &type)
            : vecCopy(cbVec), type(type)
        {
        }
    };
    using EntrySetter = std::function<void(UvEntry &)>;
    std::shared_ptr<UvEntry> GetEntry(const std::string &type, EntrySetter entrySetter = nullptr);
    static const std::string IMS_CLASS_NAME;
    static thread_local napi_ref IMSRef_;
    static std::mutex eventHandlerMutex_;
    static std::shared_ptr<AppExecFwk::EventHandler> handler_;
    uv_loop_s *loop_ = nullptr;
    std::recursive_mutex mutex_;
    std::map<std::string, std::vector<std::shared_ptr<JSCallbackObject>>> jsCbMap_;
    static std::mutex msMutex_;
    static std::shared_ptr<JsGetInputMethodSetting> inputMethod_;

    PanelFlag softKbShowingFlag_{ FLG_CANDIDATE_COLUMN };
    PanelFlag GetSoftKbShowingFlag();
    void SetSoftKbShowingFlag(PanelFlag flag);
};
} // namespace MiscServices
} // namespace OHOS
#endif // INTERFACE_KITS_JS_GETINPUT_METHOD_SETTING_H