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

#include "js_get_input_method_setting.h"

#include "event_checker.h"
#include "ime_event_monitor_manager_impl.h"
#include "input_client_info.h"
#include "input_method_controller.h"
#include "input_method_status.h"
#include "js_callback_handler.h"
#include "js_input_method.h"
#include "js_util.h"
#include "js_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "string_ex.h"

namespace OHOS {
namespace MiscServices {
int32_t MAX_TYPE_NUM = 128;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_MAX = 6;
thread_local napi_ref JsGetInputMethodSetting::IMSRef_ = nullptr;
const std::string JsGetInputMethodSetting::IMS_CLASS_NAME = "InputMethodSetting";
const std::unordered_map<std::string, uint32_t> EVENT_TYPE{ { "imeChange", EVENT_IME_CHANGE_MASK },
    { "imeShow", EVENT_IME_SHOW_MASK }, { "imeHide", EVENT_IME_HIDE_MASK } };
std::mutex JsGetInputMethodSetting::msMutex_;
std::shared_ptr<JsGetInputMethodSetting> JsGetInputMethodSetting::inputMethod_{ nullptr };
std::mutex JsGetInputMethodSetting::eventHandlerMutex_;
std::shared_ptr<AppExecFwk::EventHandler> JsGetInputMethodSetting::handler_{ nullptr };
napi_value JsGetInputMethodSetting::Init(napi_env env, napi_value exports)
{
    napi_value maxTypeNumber = nullptr;
    NAPI_CALL(env, napi_create_int32(env, MAX_TYPE_NUM, &maxTypeNumber));

    napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("getInputMethodSetting", GetInputMethodSetting),
        DECLARE_NAPI_FUNCTION("getSetting", GetSetting),
        DECLARE_NAPI_PROPERTY("MAX_TYPE_NUM", maxTypeNumber),
    };
    NAPI_CALL(env,
        napi_define_properties(env, exports, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("listInputMethod", ListInputMethod),
        DECLARE_NAPI_FUNCTION("listInputMethodSubtype", ListInputMethodSubtype),
        DECLARE_NAPI_FUNCTION("listCurrentInputMethodSubtype", ListCurrentInputMethodSubtype),
        DECLARE_NAPI_FUNCTION("getInputMethods", GetInputMethods),
        DECLARE_NAPI_FUNCTION("getInputMethodsSync", GetInputMethodsSync),
        DECLARE_NAPI_FUNCTION("getAllInputMethods", GetAllInputMethods),
        DECLARE_NAPI_FUNCTION("getAllInputMethodsSync", GetAllInputMethodsSync),
        DECLARE_NAPI_FUNCTION("displayOptionalInputMethod", DisplayOptionalInputMethod),
        DECLARE_NAPI_FUNCTION("showOptionalInputMethods", ShowOptionalInputMethods),
        DECLARE_NAPI_FUNCTION("isPanelShown", IsPanelShown),
        DECLARE_NAPI_FUNCTION("enableInputMethod", EnableInputMethod),
        DECLARE_NAPI_FUNCTION("getInputMethodState", GetInputMethodState),
        DECLARE_NAPI_FUNCTION("on", Subscribe),
        DECLARE_NAPI_FUNCTION("off", UnSubscribe),
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, IMS_CLASS_NAME.c_str(), IMS_CLASS_NAME.size(), JsConstructor, nullptr,
                       sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &IMSRef_));
    NAPI_CALL(env, napi_set_named_property(env, exports, IMS_CLASS_NAME.c_str(), cons));
    return exports;
}

napi_value JsGetInputMethodSetting::JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    {
        std::lock_guard<std::mutex> lock(eventHandlerMutex_);
        handler_ = AppExecFwk::EventHandler::Current();
    }
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));

    auto delegate = GetInputMethodSettingInstance();
    if (delegate == nullptr) {
        IMSA_HILOGE("delegate is nullptr!");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_status status = napi_wrap(
        env, thisVar, delegate.get(), [](napi_env env, void *data, void *hint) {}, nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("failed to wrap: %{public}d", status);
        return nullptr;
    }
    if (delegate->loop_ == nullptr) {
        napi_get_uv_event_loop(env, &delegate->loop_);
    }
    return thisVar;
}

napi_value JsGetInputMethodSetting::GetJsConstProperty(napi_env env, uint32_t num)
{
    napi_value jsNumber = nullptr;
    napi_create_int32(env, num, &jsNumber);
    return jsNumber;
}

std::shared_ptr<JsGetInputMethodSetting> JsGetInputMethodSetting::GetInputMethodSettingInstance()
{
    if (inputMethod_ == nullptr) {
        std::lock_guard<std::mutex> lock(msMutex_);
        if (inputMethod_ == nullptr) {
            auto engine = std::make_shared<JsGetInputMethodSetting>();
            if (engine == nullptr) {
                IMSA_HILOGE("engine is nullptr!");
                return nullptr;
            }
            inputMethod_ = engine;
        }
    }
    return inputMethod_;
}

napi_value JsGetInputMethodSetting::GetSetting(napi_env env, napi_callback_info info)
{
    return GetIMSetting(env, info, true);
}

napi_value JsGetInputMethodSetting::GetInputMethodSetting(napi_env env, napi_callback_info info)
{
    return GetIMSetting(env, info, false);
}

napi_value JsGetInputMethodSetting::GetIMSetting(napi_env env, napi_callback_info info, bool needThrowException)
{
    napi_value instance = nullptr;
    napi_value cons = nullptr;
    if (napi_get_reference_value(env, IMSRef_, &cons) != napi_ok) {
        IMSA_HILOGE("failed to get reference value!");
        if (needThrowException) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_SETTINGS, "", TYPE_OBJECT);
        }
        return nullptr;
    }
    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("failed to new instance!");
        if (needThrowException) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_SETTINGS, "", TYPE_OBJECT);
        }
        return nullptr;
    }
    return instance;
}

napi_status JsGetInputMethodSetting::GetInputMethodProperty(napi_env env, napi_value argv,
    std::shared_ptr<ListInputContext> ctxt)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv, &valueType);
    PARAM_CHECK_RETURN(env, valueType == napi_object, "inputMethodProperty type must be InputMethodProperty",
        TYPE_NONE, napi_invalid_arg);
    napi_value result = nullptr;
    napi_get_named_property(env, argv, "name", &result);
    JsUtils::GetValue(env, result, ctxt->property.name);

    result = nullptr;
    napi_get_named_property(env, argv, "id", &result);
    JsUtils::GetValue(env, result, ctxt->property.id);

    if (ctxt->property.name.empty() || ctxt->property.id.empty()) {
        result = nullptr;
        napi_get_named_property(env, argv, "packageName", &result);
        JsUtils::GetValue(env, result, ctxt->property.name);

        result = nullptr;
        napi_get_named_property(env, argv, "methodId", &result);
        JsUtils::GetValue(env, result, ctxt->property.id);
    }
    PARAM_CHECK_RETURN(env, (!ctxt->property.name.empty() && !ctxt->property.id.empty()),
        "name and id must be string and cannot empty", TYPE_NONE, napi_invalid_arg);
    IMSA_HILOGD("methodId: %{public}s, packageName: %{public}s.", ctxt->property.id.c_str(),
        ctxt->property.name.c_str());
    return napi_ok;
}

napi_value JsGetInputMethodSetting::ListInputMethod(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("start ListInputMethod");
    auto ctxt = std::make_shared<ListInputContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        ctxt->inputMethodStatus = InputMethodStatus::ALL;
        return napi_ok;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        *result = JsInputMethod::GetJSInputMethodProperties(env, ctxt->properties);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->ListInputMethod(ctxt->properties);
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec ListInputMethod success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            return;
        }
        ctxt->SetErrorCode(errCode);
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI:listInputMethod has 1 param at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "listInputMethod");
}

napi_value JsGetInputMethodSetting::GetInputMethods(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in GetInputMethods");
    auto ctxt = std::make_shared<ListInputContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_invalid_arg);
        bool enable = false;
        // 0 means first param index
        napi_status status = JsUtils::GetValue(env, argv[0], enable);
        PARAM_CHECK_RETURN(env, status == napi_ok, "enable type must be boolean!", TYPE_NONE, napi_invalid_arg);
        ctxt->inputMethodStatus = enable ? InputMethodStatus::ENABLE : InputMethodStatus::DISABLE;
        return napi_ok;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        *result = JsInputMethod::GetJSInputMethodProperties(env, ctxt->properties);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode =
            InputMethodController::GetInstance()->ListInputMethod(ctxt->inputMethodStatus == ENABLE, ctxt->properties);
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec GetInputMethods success.");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            return;
        }
        ctxt->SetErrorCode(errCode);
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 2 means JsAPI:getInputMethods has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "getInputMethods");
}

napi_value JsGetInputMethodSetting::GetInputMethodsSync(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in");
    size_t argc = ARGC_MAX;
    napi_value argv[ARGC_MAX] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    bool enable = false;
    // 0 means first param index
    PARAM_CHECK_RETURN(env, argc >= 1, "at least one parameter is required!", TYPE_NONE, JsUtil::Const::Null(env));
    PARAM_CHECK_RETURN(env, JsUtils::GetValue(env, argv[0], enable) == napi_ok, "enable type must be boolean!",
        TYPE_NONE, JsUtil::Const::Null(env));

    std::vector<Property> properties;
    int32_t ret = InputMethodController::GetInstance()->ListInputMethod(enable, properties);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to get input methods!", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    return JsInputMethod::GetJSInputMethodProperties(env, properties);
}

napi_value JsGetInputMethodSetting::GetAllInputMethods(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("start GetAllInputMethods.");
    auto ctxt = std::make_shared<ListInputContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        *result = JsInputMethod::GetJSInputMethodProperties(env, ctxt->properties);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->ListInputMethod(ctxt->properties);
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec GetInputMethods success.");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            return;
        }
        ctxt->SetErrorCode(errCode);
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI:getAllInputMethods has 1 param at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "getInputMethods");
}

napi_value JsGetInputMethodSetting::GetAllInputMethodsSync(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in");
    std::vector<Property> properties;
    int32_t ret = InputMethodController::GetInstance()->ListInputMethod(properties);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to get input methods", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    return JsInputMethod::GetJSInputMethodProperties(env, properties);
}

napi_value JsGetInputMethodSetting::DisplayOptionalInputMethod(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("start JsGetInputMethodSetting.");
    auto ctxt = std::make_shared<DisplayOptionalInputMethodContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status { return napi_ok; };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->DisplayOptionalInputMethod();
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec DisplayOptionalInputMethod success.");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI:displayOptionalInputMethod has 1 param at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "displayOptionalInputMethod");
}

napi_value JsGetInputMethodSetting::ShowOptionalInputMethods(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("start JsGetInputMethodSetting.");
    auto ctxt = std::make_shared<DisplayOptionalInputMethodContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, ctxt->isDisplayed, result);
        IMSA_HILOGI("output get boolean != nullptr[%{public}d].", result != nullptr);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->DisplayOptionalInputMethod();
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec DisplayOptionalInputMethod success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            ctxt->isDisplayed = true;
            return;
        } else {
            ctxt->SetErrorCode(errCode);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI:showOptionalInputMethods has 1 param at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "showOptionalInputMethods");
}

napi_value JsGetInputMethodSetting::ListInputMethodSubtype(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in ListInputMethodSubtype");
    auto ctxt = std::make_shared<ListInputContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_invalid_arg);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        PARAM_CHECK_RETURN(env, valueType == napi_object, "inputMethodProperty type must be InputMethodProperty!",
            TYPE_NONE, napi_invalid_arg);
        napi_status status = JsGetInputMethodSetting::GetInputMethodProperty(env, argv[0], ctxt);
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        *result = JsInputMethod::GetJSInputMethodSubProperties(env, ctxt->subProperties);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode =
            InputMethodController::GetInstance()->ListInputMethodSubtype(ctxt->property, ctxt->subProperties);
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec ListInputMethodSubtype success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            return;
        }
        ctxt->SetErrorCode(errCode);
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 2 means JsAPI:listInputMethodSubtype has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "listInputMethodSubtype");
}

napi_value JsGetInputMethodSetting::ListCurrentInputMethodSubtype(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in ListCurrentInputMethodSubtype");
    auto ctxt = std::make_shared<ListInputContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        *result = JsInputMethod::GetJSInputMethodSubProperties(env, ctxt->subProperties);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->ListCurrentInputMethodSubtype(ctxt->subProperties);
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec ListCurrentInputMethodSubtype success.");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            return;
        }
        ctxt->SetErrorCode(errCode);
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI:listCurrentInputMethodSubtype has 1 param at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "listCurrentInputMethodSubtype");
}

napi_value JsGetInputMethodSetting::IsPanelShown(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("start JsGetInputMethodSetting");
    // 1 means required param num
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    // 1 means least param num
    PARAM_CHECK_RETURN(env, argc >= 1, "at least one parameter is required!", TYPE_NONE, JsUtil::Const::Null(env));
    // 0 means parameter of info<PanelInfo>
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    PARAM_CHECK_RETURN(env, valueType == napi_object, "panelInfo type must be PanelInfo!", TYPE_NONE,
        JsUtil::Const::Null(env));

    PanelInfo panelInfo;
    napi_status status = JsUtils::GetValue(env, argv[0], panelInfo);
    PARAM_CHECK_RETURN(env, status == napi_ok, "panelInfo covert failed!", TYPE_NONE, JsUtil::Const::Null(env));

    bool isShown = false;
    int32_t errorCode = InputMethodController::GetInstance()->IsPanelShown(panelInfo, isShown);
    if (errorCode != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(errorCode), "failed to query is panel shown!", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    return JsUtil::GetValue(env, isShown);
}

napi_value JsGetInputMethodSetting::EnableInputMethod(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in");
    auto ctxt = std::make_shared<EnableInputContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 2, "at least three parameters is required!", TYPE_NONE, napi_invalid_arg);
        PARAM_CHECK_RETURN(env,
            JsUtil::GetType(env, argv[0]) == napi_string && JsUtil::GetValue(env, argv[0], ctxt->bundleName),
            "bundleName type must be string!", TYPE_NONE, napi_invalid_arg);
        PARAM_CHECK_RETURN(env, !ctxt->bundleName.empty(), "bundleName can not be empty!", TYPE_NONE, napi_invalid_arg);
        PARAM_CHECK_RETURN(env,
            JsUtil::GetType(env, argv[1]) == napi_string && JsUtil::GetValue(env, argv[1], ctxt->extName),
            "extensionName type must be string!", TYPE_NONE, napi_invalid_arg);
        PARAM_CHECK_RETURN(env, !ctxt->extName.empty(), "extensionName can not be empty!", TYPE_NONE, napi_invalid_arg);
        int32_t status = 0;
        PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[2]) == napi_number && JsUtil::GetValue(env, argv[2], status),
            "enabledState type must be EnabledState!", TYPE_NONE, napi_invalid_arg);
        ctxt->enabledStatus = static_cast<EnabledStatus>(status);
        return napi_ok;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status { return napi_ok; };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode =
            InputMethodController::GetInstance()->EnableIme(ctxt->bundleName, ctxt->extName, ctxt->enabledStatus);
        if (errCode == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            return;
        }
        ctxt->SetErrorCode(errCode);
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 3 means JsAPI:enableInputMethod has 3 params at most.
    AsyncCall asyncCall(env, info, ctxt, 3);
    return asyncCall.Call(env, exec, "EnableInputMethod");
}

napi_value JsGetInputMethodSetting::GetInputMethodState(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<GetInputMethodStateContext>();
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        int32_t enableStatus = static_cast<int32_t>(ctxt->enableStatus);
        *result = JsUtil::GetValue(env, enableStatus);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->GetInputMethodState(ctxt->enableStatus);
        if (errCode == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            return;
        }
        ctxt->SetErrorCode(errCode);
    };
    ctxt->SetAction(nullptr, std::move(output));
    // 0 means JsAPI:GetInputMethodState has no param.
    AsyncCall asyncCall(env, info, ctxt, 0);
    return asyncCall.Call(env, exec, "GetInputMethodState");
}

int32_t JsGetInputMethodSetting::RegisterListener(napi_value callback, std::string type,
    std::shared_ptr<JSCallbackObject> callbackObj)
{
    IMSA_HILOGD("register listener: %{public}s", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!jsCbMap_.empty() && jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGI("start type: %{public}s listening.", type.c_str());
    }

    auto callbacks = jsCbMap_[type];
    bool ret = std::any_of(callbacks.begin(), callbacks.end(), [&callback](std::shared_ptr<JSCallbackObject> cb) {
        return JsUtils::Equals(cb->env_, callback, cb->callback_, cb->threadId_);
    });
    if (ret) {
        IMSA_HILOGD("callback already registered!");
        return ErrorCode::NO_ERROR;
    }

    IMSA_HILOGI("add %{public}s callbackObj into jsCbMap_.", type.c_str());
    jsCbMap_[type].push_back(std::move(callbackObj));
    return ErrorCode::NO_ERROR;
}

napi_value JsGetInputMethodSetting::Subscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    std::string type;
    // 2 means least param num.
    if (argc < 2 || !JsUtil::GetValue(env, argv[0], type) ||
        !EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_SETTING, type) ||
        JsUtil::GetType(env, argv[1]) != napi_function) {
        IMSA_HILOGE("subscribe failed, type:%{public}s", type.c_str());
        return nullptr;
    }
    IMSA_HILOGD("subscribe type: %{public}s.", type.c_str());
    auto engine = reinterpret_cast<JsGetInputMethodSetting *>(JsUtils::GetNativeSelf(env, info));
    if (engine == nullptr) {
        return nullptr;
    }
    auto iter = EVENT_TYPE.find(type);
    if (iter == EVENT_TYPE.end()) {
        return nullptr;
    }
    std::shared_ptr<JSCallbackObject> callback =
        std::make_shared<JSCallbackObject>(env, argv[ARGC_ONE], std::this_thread::get_id(),
            AppExecFwk::EventHandler::Current());
    auto ret = ImeEventMonitorManagerImpl::GetInstance().RegisterImeEventListener(iter->second, inputMethod_);
    if (ret == ErrorCode::NO_ERROR) {
        engine->RegisterListener(argv[ARGC_ONE], type, callback);
    } else {
        auto errCode = JsUtils::Convert(ret);
        if (errCode == EXCEPTION_SYSTEM_PERMISSION) {
            IMSA_HILOGE("failed to UpdateListenEventFlag , ret: %{public}d, type: %{public}s!", ret, type.c_str());
            JsUtils::ThrowException(env, errCode, "", TYPE_NONE);
        }
    }
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

void JsGetInputMethodSetting::UnRegisterListener(napi_value callback, std::string type, bool &isUpdateFlag)
{
    IMSA_HILOGI("unregister listener: %{public}s!", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGE("methodName: %{public}s already unRegistered!", type.c_str());
        return;
    }

    if (callback == nullptr) {
        jsCbMap_.erase(type);
        IMSA_HILOGI("stop all type: %{public}s listening.", type.c_str());
        isUpdateFlag = true;
        return;
    }

    for (auto item = jsCbMap_[type].begin(); item != jsCbMap_[type].end(); item++) {
        if (JsUtils::Equals((*item)->env_, callback, (*item)->callback_, (*item)->threadId_)) {
            jsCbMap_[type].erase(item);
            break;
        }
    }

    if (jsCbMap_[type].empty()) {
        IMSA_HILOGI("stop last type: %{public}s listening.", type.c_str());
        jsCbMap_.erase(type);
        isUpdateFlag = true;
    }
}

napi_value JsGetInputMethodSetting::UnSubscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    std::string type;
    // 1 means least param num.
    if (argc < 1 || !JsUtil::GetValue(env, argv[0], type) ||
        !EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_SETTING, type)) {
        IMSA_HILOGE("unsubscribe failed, type: %{public}s!", type.c_str());
        return nullptr;
    }

    // if the second param is not napi_function/napi_null/napi_undefined, return
    auto paramType = JsUtil::GetType(env, argv[1]);
    if (paramType != napi_function && paramType != napi_null && paramType != napi_undefined) {
        return nullptr;
    }
    // if the second param is napi_function, delete it, else delete all
    argv[1] = paramType == napi_function ? argv[1] : nullptr;

    IMSA_HILOGD("unsubscribe type: %{public}s.", type.c_str());

    auto engine = reinterpret_cast<JsGetInputMethodSetting *>(JsUtils::GetNativeSelf(env, info));
    if (engine == nullptr) {
        return nullptr;
    }
    bool isUpdateFlag = false;
    engine->UnRegisterListener(argv[ARGC_ONE], type, isUpdateFlag);
    auto iter = EVENT_TYPE.find(type);
    if (iter == EVENT_TYPE.end()) {
        return nullptr;
    }
    if (isUpdateFlag) {
        auto ret = ImeEventMonitorManagerImpl::GetInstance().UnRegisterImeEventListener(iter->second, inputMethod_);
        IMSA_HILOGI("UpdateListenEventFlag, ret: %{public}d, type: %{public}s.", ret, type.c_str());
    }
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

void JsGetInputMethodSetting::OnImeChange(const Property &property, const SubProperty &subProperty)
{
    std::string type = "imeChange";
    auto entry = GetEntry(type, [&property, &subProperty](UvEntry &entry) {
        entry.property = property;
        entry.subProperty = subProperty;
    });
    if (entry == nullptr) {
        IMSA_HILOGD("failed to get uv entry.");
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGI("start");
    auto task = [entry]() {
        auto getImeChangeProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc < 2) {
                return false;
            }
            napi_value subProperty = JsInputMethod::GetJsInputMethodSubProperty(env, entry->subProperty);
            napi_value property = JsInputMethod::GetJsInputMethodProperty(env, entry->property);
            if (subProperty == nullptr || property == nullptr) {
                IMSA_HILOGE("get KBCins or TICins failed!");
                return false;
            }
            // 0 means the first param of callback.
            args[0] = property;
            // 1 means the second param of callback.
            args[1] = subProperty;
            return true;
        };
        // 2 means callback has two params.
        JsCallbackHandler::Traverse(entry->vecCopy, { 2, getImeChangeProperty });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

PanelFlag JsGetInputMethodSetting::GetSoftKbShowingFlag()
{
    return softKbShowingFlag_;
}
void JsGetInputMethodSetting::SetSoftKbShowingFlag(PanelFlag flag)
{
    softKbShowingFlag_ = flag;
}

void JsGetInputMethodSetting::OnImeShow(const ImeWindowInfo &info)
{
    if (info.panelInfo.panelType != PanelType::SOFT_KEYBOARD ||
        (info.panelInfo.panelFlag != FLG_FLOATING && info.panelInfo.panelFlag != FLG_FIXED)) {
        return;
    }
    auto showingFlag = GetSoftKbShowingFlag();
    // FLG_FIXED->FLG_FLOATING in show
    if (info.panelInfo.panelFlag == FLG_FLOATING && showingFlag == FLG_FIXED) {
        InputWindowInfo windowInfo{ info.windowInfo.name, 0, 0, 0, 0 };
        OnPanelStatusChange("imeHide", windowInfo);
    }
    // FLG_FLOATING->FLG_FIXED in show/show FLG_FIXED/ rotating(resize) in FLG_FIXED show
    if ((info.panelInfo.panelFlag == FLG_FIXED && showingFlag == FLG_FLOATING) ||
        (info.panelInfo.panelFlag == FLG_FIXED && showingFlag == FLG_CANDIDATE_COLUMN) ||
        (info.panelInfo.panelFlag == FLG_FIXED && showingFlag == FLG_FIXED)) {
        OnPanelStatusChange("imeShow", info.windowInfo);
    }
    SetSoftKbShowingFlag(info.panelInfo.panelFlag);
}

void JsGetInputMethodSetting::OnImeHide(const ImeWindowInfo &info)
{
    SetSoftKbShowingFlag(FLG_CANDIDATE_COLUMN);
    if (info.panelInfo.panelType != PanelType::SOFT_KEYBOARD || info.panelInfo.panelFlag != PanelFlag::FLG_FIXED) {
        return;
    }
    OnPanelStatusChange("imeHide", info.windowInfo);
}

void JsGetInputMethodSetting::OnPanelStatusChange(const std::string &type, const InputWindowInfo &info)
{
    IMSA_HILOGI("type: %{public}s, rect[%{public}d, %{public}d, %{public}u, %{public}u].", type.c_str(), info.left,
        info.top, info.width, info.height);
    auto entry = GetEntry(type, [&info](UvEntry &entry) { entry.windowInfo = { info }; });
    if (entry == nullptr) {
        IMSA_HILOGD("failed to get uv entry.");
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    auto task = [entry]() {
        auto getWindowInfo = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc < 1) {
                return false;
            }
            auto windowInfo = JsUtils::GetValue(env, entry->windowInfo);
            if (windowInfo == nullptr) {
                IMSA_HILOGE("failed to converse windowInfo!");
                return false;
            }
            // 0 means the first param of callback.
            args[0] = windowInfo;
            return true;
        };
        // 1 means callback has one param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, getWindowInfo });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

std::shared_ptr<AppExecFwk::EventHandler> JsGetInputMethodSetting::GetEventHandler()
{
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    return handler_;
}

std::shared_ptr<JsGetInputMethodSetting::UvEntry> JsGetInputMethodSetting::GetEntry(const std::string &type,
    EntrySetter entrySetter)
{
    IMSA_HILOGD("start, type: %{public}s.", type.c_str());
    std::shared_ptr<UvEntry> entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (jsCbMap_[type].empty()) {
            IMSA_HILOGD("%{public}s cb-vector is empty.", type.c_str());
            return nullptr;
        }
        entry = std::make_shared<UvEntry>(jsCbMap_[type], type);
    }
    if (entrySetter != nullptr) {
        entrySetter(*entry);
    }
    return entry;
}
} // namespace MiscServices
} // namespace OHOS
