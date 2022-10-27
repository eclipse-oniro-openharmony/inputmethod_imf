/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "input_method_controller.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "string_ex.h"
#include "js_get_input_method_controller.h"

namespace OHOS {
namespace MiscServices {
thread_local napi_ref JsGetInputMethodController::IMCRef_ = nullptr;
const std::string JsGetInputMethodController::IMC_CLASS_NAME = "InputMethodController";
napi_value JsGetInputMethodController::Init(napi_env env, napi_value info)
{
    napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("getInputMethodController", GetInputMethodController),
        DECLARE_NAPI_FUNCTION("getController", GetController),
    };
    NAPI_CALL(
        env, napi_define_properties(env, info, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("stopInput", StopInput),
        DECLARE_NAPI_FUNCTION("stopInputSession", StopInputSession),
        DECLARE_NAPI_FUNCTION("hideSoftKeyboard", HideSoftKeyboard),
        DECLARE_NAPI_FUNCTION("showSoftKeyboard", ShowSoftKeyboard),
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, IMC_CLASS_NAME.c_str(), IMC_CLASS_NAME.size(),
        JsConstructor, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &IMCRef_));
    NAPI_CALL(env, napi_set_named_property(env, info, IMC_CLASS_NAME.c_str(), cons));

    return info;
}

napi_value JsGetInputMethodController::JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));

    JsGetInputMethodController *controllerObject = new (std::nothrow) JsGetInputMethodController();
    if (controllerObject == nullptr) {
        IMSA_HILOGE("controllerObject is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_wrap(env, thisVar, controllerObject, [](napi_env env, void *data, void *hint) {
        auto* objInfo = reinterpret_cast<JsGetInputMethodController*>(data);
        if (objInfo != nullptr) {
            IMSA_HILOGE("objInfo is nullptr");
            delete objInfo;
        }
    }, nullptr, nullptr);

    return thisVar;
}

napi_value JsGetInputMethodController::GetInputMethodController(napi_env env, napi_callback_info cbInfo)
{
    return GetIMController(env, cbInfo, false);
}

napi_value JsGetInputMethodController::GetController(napi_env env, napi_callback_info cbInfo)
{
    return GetIMController(env, cbInfo, true);
}

napi_value JsGetInputMethodController::GetIMController(napi_env env, napi_callback_info cbInfo, bool needThrowException)
{
    napi_value instance = nullptr;
    napi_value cons = nullptr;
    if (napi_get_reference_value(env, IMCRef_, &cons) != napi_ok) {
        IMSA_HILOGE("GetInputMethodSetting::napi_get_reference_value not ok");
        if (needThrowException) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_CONTROLLER, "", TYPE_OBJECT);
        }
        return nullptr;
    }

    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("GetInputMethodSetting::napi_new_instance not ok");
        if (needThrowException) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_CONTROLLER, "", TYPE_OBJECT);
        }
        return nullptr;
    }
    return instance;
}

napi_value JsGetInputMethodController::HandleSoftKeyboard(
    napi_env env, napi_callback_info info, std::function<int32_t()> callback, bool isOutput, bool needThrowException)
{
    auto ctxt = std::make_shared<HandleContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, ctxt->isHandle, result);
        IMSA_HILOGE("output napi_get_boolean != nullptr[%{public}d]", result != nullptr);
        return status;
    };
    auto exec = [ctxt, callback, needThrowException](AsyncCall::Context *ctx) {
        int errCode = callback();
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec success");
            ctxt->status = napi_ok;
            ctxt->isHandle = true;
            ctxt->SetState(ctxt->status);
            return;
        }
        IMSA_HILOGI("exec %{public}d", errCode);
        if (needThrowException) {
            ctxt->SetErrorCode(errCode);
        }
    };
    if (isOutput) {
        ctxt->SetAction(std::move(input), std::move(output));
    } else {
        ctxt->SetAction(std::move(input));
    }
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt), 0);
    return asyncCall.Call(env, exec);
}

napi_value JsGetInputMethodController::ShowSoftKeyboard(napi_env env, napi_callback_info info)
{
    return HandleSoftKeyboard(
        env, info, [] { return InputMethodController::GetInstance()->ShowSoftKeyboard(); }, false, true);
}

napi_value JsGetInputMethodController::HideSoftKeyboard(napi_env env, napi_callback_info info)
{
    return HandleSoftKeyboard(
        env, info, [] { return InputMethodController::GetInstance()->HideSoftKeyboard(); }, false, true);
}

napi_value JsGetInputMethodController::StopInputSession(napi_env env, napi_callback_info info)
{
    return HandleSoftKeyboard(
        env, info, [] { return InputMethodController::GetInstance()->StopInputSession(); }, true, true);
}

napi_value JsGetInputMethodController::StopInput(napi_env env, napi_callback_info info)
{
    return HandleSoftKeyboard(
        env, info, [] { return InputMethodController::GetInstance()->HideSoftKeyboard(); }, true, false);
}
} // namespace MiscServices
} // namespace OHOS