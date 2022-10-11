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

#include "input_method_controller.h"
#include "input_method_status.h"
#include "js_input_method.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "string_ex.h"

namespace OHOS {
namespace MiscServices {
int32_t MAX_TYPE_NUM = 128;
thread_local napi_ref JsGetInputMethodSetting::IMSRef_ = nullptr;
const std::string JsGetInputMethodSetting::IMS_CLASS_NAME = "InputMethodSetting";
napi_value JsGetInputMethodSetting::Init(napi_env env, napi_value exports)
{
        napi_value maxTypeNumber = nullptr;
        napi_create_int32(env, MAX_TYPE_NUM, &maxTypeNumber);
        
        napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("getInputMethodSetting", GetSetting),
        DECLARE_NAPI_FUNCTION("getSetting", GetSetting),
        DECLARE_NAPI_PROPERTY("MAX_TYPE_NUM", maxTypeNumber),
    };
    NAPI_CALL(
        env, napi_define_properties(env, exports, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("listInputMethod", ListInputMethod),
        DECLARE_NAPI_FUNCTION("getInputMethods", GetInputMethods),
        DECLARE_NAPI_FUNCTION("displayOptionalInputMethod", DisplayOptionalInputMethod),
        DECLARE_NAPI_FUNCTION("showOptionalInputMethods", ShowOptionalInputMethods),
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, IMS_CLASS_NAME.c_str(), IMS_CLASS_NAME.size(),
        JsConstructor, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &IMSRef_));
    NAPI_CALL(env, napi_set_named_property(env, exports, IMS_CLASS_NAME.c_str(), cons));
    return exports;
}

napi_value JsGetInputMethodSetting::JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));

    JsGetInputMethodSetting *settingObject = new (std::nothrow) JsGetInputMethodSetting();
    if (settingObject == nullptr) {
        IMSA_HILOGE("settingObject is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_wrap(env, thisVar, settingObject, [](napi_env env, void *data, void *hint) {
        auto* objInfo = reinterpret_cast<JsGetInputMethodSetting*>(data);
        if (objInfo != nullptr) {
            IMSA_HILOGE("objInfo is nullptr");
            delete objInfo;
        }
    }, nullptr, nullptr);

    return thisVar;
}

napi_value JsGetInputMethodSetting::GetSetting(napi_env env, napi_callback_info info)
{
    napi_value instance = nullptr;
    napi_value cons = nullptr;
    if (napi_get_reference_value(env, IMSRef_, &cons) != napi_ok) {
        IMSA_HILOGE("GetInputMethodSetting::napi_get_reference_value not ok");
        return nullptr;
    }
    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("GetInputMethodSetting::napi_new_instance not ok");
        return nullptr;
    }
    return instance;
}

napi_value JsGetInputMethodSetting::ListInputMethod(napi_env env, napi_callback_info info)
{
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
        ctxt->properties = InputMethodController::GetInstance()->ListInputMethod();
        ctxt->status = napi_ok;
        ctxt->SetState(ctxt->status);
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt));
    return asyncCall.Call(env, exec);
}

napi_value JsGetInputMethodSetting::GetInputMethods(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<ListInputContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        // parameter:1. null; 2.enable
        if (argc == 0) {
            ctxt->inputMethodStatus = InputMethodStatus::ALL;
            return napi_ok;
        }
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType == napi_boolean) {
            bool enable = false;
            napi_get_value_bool(env, argv[0], &enable);
            ctxt->inputMethodStatus = enable ? InputMethodStatus::ENABLE : InputMethodStatus::DISABLE;
            return napi_ok;
        }
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, " parameter's type is wrong.", TYPE_BOOLEAN);
        return napi_generic_failure;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        *result = JsInputMethod::GetJSInputMethodProperties(env, ctxt->properties);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        if (ctxt->inputMethodStatus == ALL) {
            ctxt->properties = InputMethodController::GetInstance()->ListInputMethod();
        } else {
            ctxt->properties = InputMethodController::GetInstance()->ListInputMethod(ctxt->inputMethodStatus == ENABLE);
        }
        ctxt->status = napi_ok;
        ctxt->SetState(ctxt->status);
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt));
    return asyncCall.Call(env, exec);
}

napi_value JsGetInputMethodSetting::DisplayOptionalInputMethod(napi_env env, napi_callback_info info)
{
    return DisplayInputMethod(env, info, false);
}

napi_value JsGetInputMethodSetting::DisplayInputMethod(napi_env env, napi_callback_info info, bool flag)
{
    auto ctxt = std::make_shared<DisplayOptionalInputMethodContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto exec = [ctxt, flag](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->ShowOptionalInputMethod();
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGE("exec ---- DisplayOptionalInputMethod success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            return;
        }
        if (flag) {
            ctxt->SetErrorCode(errCode);
        }
    };
    ctxt->SetAction(std::move(input));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt), 0);
    return asyncCall.Call(env, exec);
}

napi_value JsGetInputMethodSetting::ShowOptionalInputMethods(napi_env env, napi_callback_info info)
{
    return DisplayInputMethod(env, info, true);
}
}
}