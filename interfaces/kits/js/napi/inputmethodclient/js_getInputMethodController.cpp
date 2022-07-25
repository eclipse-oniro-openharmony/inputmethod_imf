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
#include "event_handler.h"
#include "event_runner.h"
#include "string_ex.h"
#include "js_getInputMethodController.h"

namespace OHOS {
namespace MiscServices {
void JsGetInputMethodController::CBOrPromiseStopInput(napi_env env, const StopInputInfo *stopInput, napi_value err, napi_value data)
{
    IMSA_HILOGI("run in CBOrPromiseStopInput");
    napi_value args[RESULT_COUNT] = {err, data};
    if (stopInput->deferred) {
        if (stopInput->status == napi_ok) {
            IMSA_HILOGE("CBOrPromiseStopInput::promise");
            napi_resolve_deferred(env, stopInput->deferred, args[RESULT_DATA]);
        } else {
            napi_reject_deferred(env, stopInput->deferred, args[RESULT_ERROR]);
        }
    } else {
        napi_value callback = nullptr;
        napi_get_reference_value(env, stopInput->callbackRef, &callback);
        if (stopInput->callbackRef == nullptr) {
            IMSA_HILOGE("CBOrPromiseStopInput::callback is null");
        }
        napi_value returnVal = nullptr;
        if (callback == nullptr) {
            IMSA_HILOGE("CBOrPromiseStopInput::callback is null");
        }
        napi_call_function(env, nullptr, callback, RESULT_COUNT, &args[0], &returnVal);
        if (stopInput->callbackRef != nullptr) {
            napi_delete_reference(env, stopInput->callbackRef);
        }
    }
}

napi_value JsGetInputMethodController::Init(napi_env env, napi_value info) 
{
    napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("getInputMethodController", GetInputMethodController),
    };
    NAPI_CALL(
        env, napi_define_properties(env, info, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("stopInput", StopInput),
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, IMC_CLASS_NAME.c_str(), IMC_CLASS_NAME.size(),
        JsConstructor, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &IMSRef_));
    NAPI_CALL(env, napi_set_named_property(env, info, IMC_CLASS_NAME.c_str(), cons));

    return info;
}

napi_value JsGetInputMethodController::JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    IMSA_HILOGI("run in JsConstructor");
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));

    JsGetInputMethodController *IMSobject = new (std::nothrow) JsGetInputMethodController();
    if (IMSobject == nullptr) {
        IMSA_HILOGE("IMSobject is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_wrap(env, thisVar, IMSobject, [](napi_env env, void *data, void *hint) {
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
    IMSA_HILOGI("run in GetInputMethodController");
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
    IMSA_HILOGE("New the js instance complete");
    return instance;
}

napi_value JsGetInputMethodController::GetErrorCodeValue(napi_env env, ErrCode errCode)
{
    IMSA_HILOGI("run in GetErrorCodeValue");
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &jsValue));
    NAPI_CALL(env, napi_create_object(env, &jsObject));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "code", jsValue));
    return jsObject;
}

napi_value JsGetInputMethodController::StopInput(napi_env env, napi_callback_info Info)
{
    IMSA_HILOGI("run in ListInputMethod");
    struct StopInputContext : public ContextBase {
        bool sStopInput = false;
    }
    StopInputContext *stopInput = new (std::nothrow) StopInputContext();
    if (stopInput == nullptr) {
        IMSA_HILOGE("ListInputMethod::stopInput is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }

    stopInput->env = env;
    stopInput->callbackRef = nullptr;
    stopInput->ParseContext(env, Info);

    napi_value promise = nullptr;
    if (stopInput->callbackRef == nullptr) {
        napi_create_promise(env, &stopInput->deferred, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "StopInput", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(env,
        nullptr,
        resource,
        [](napi_env env, void *data) {
            IMSA_HILOGI("ListInputMethod::napi_create_async_work in");
            StopInputInfo *stopInput = reinterpret_cast<StopInputInfo *>(data);
            stopInput->errCode = InputMethodController::GetInstance()->HideCurrentInput();
            if (stopInput->errCode == 0) {
                IMSA_HILOGE("JsGetInputMethodController::StopInput successful!");
                stopInput->sStopInput = true;
            }
            stopInput->status = (stopInput->errCode == 0) ? napi_ok : napi_generic_failure;
        },
        [](napi_env env, napi_status status, void *data) {
            IMSA_HILOGI("ListInputMethod::napi_create_async_work out");
            StopInputInfo *stopInput = reinterpret_cast<StopInputInfo *>(data);
            if (stopInput == nullptr) {
                IMSA_HILOGE("StopInput::stopInput is nullptr");
                return;
            }
            stopInput->errCode = 0;
            napi_value getResult[RESULT_COUNT] = {0};
            getResult[PARAMZERO] =  GetErrorCodeValue(env, stopInput->errCode);;
            napi_get_boolean(env, stopInput->sStopInput, &getResult[PARAMONE]);
            CBOrPromiseStopInput(env, stopInput, getResult[PARAMZERO], getResult[PARAMONE]);
            napi_delete_async_work(env, stopInput->work);
            delete stopInput;
            stopInput = nullptr;
        },
        reinterpret_cast<void *>(stopInput),
        &stopInput->work);
    napi_queue_async_work(env, stopInput->work);
    return promise;
}
}
}
