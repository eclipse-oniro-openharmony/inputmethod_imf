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
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "string_ex.h"
#include "input_method_controller.h"

namespace OHOS {
namespace MiscServices {

napi_value JsGetInputMethodSetting::Init(napi_env env, napi_value exports) {
        napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("getInputMethodSetting", GetInputMethodSetting),
    };
    NAPI_CALL(
        env, napi_define_properties(env, exports, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("listInputMethod", ListInputMethod),
        DECLARE_NAPI_FUNCTION("displayOptionalInputMethod", DisplayOptionalInputMethod),
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
    IMSA_HILOGE("run in JsConstructor");
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));

    JsGetInputMethodSetting *IMSobject = new (std::nothrow) JsGetInputMethodSetting();
    if (IMSobject == nullptr) {
        IMSA_HILOGE("IMSobject is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_wrap(env, thisVar, IMSobject, [](napi_env env, void *data, void *hint) {
        auto* objInfo = reinterpret_cast<JsGetInputMethodSetting*>(data);
        if (objInfo != nullptr) {
            IMSA_HILOGE("objInfo is nullptr");
            delete objInfo;
        }
        //LOG()
    }, nullptr, nullptr);

    return thisVar;
}

napi_value JsGetInputMethodSetting::GetInputMethodSetting(napi_env env, napi_callback_info info)
{
    IMSA_HILOGE("run in GetInputMethodSetting");
    napi_value instance = nullptr;
    napi_value cons = nullptr;
    if (napi_get_reference_value(env, IMSRef_, &cons) != napi_ok) {
        IMSA_HILOGE("GetInputMethodSetting::napi_get_reference_value not ok");
        return nullptr;
    }
    // LOG("Get a reference to the global variable appAccountRef_ complete");
    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("GetInputMethodSetting::napi_new_instance not ok");
        return nullptr;
    }
    // LOGI("New the js instance complete");
    IMSA_HILOGE("New the js instance complete");
    return instance;
}

void JsGetInputMethodSetting::GetResult(napi_env env, std::vector<InputMethodProperty*> &properties, napi_value &result)
{
    IMSA_HILOGE("run in GetResult");
    uint32_t index = 0;

    for (const auto &item : properties) {

        if (item == nullptr) {
            IMSA_HILOGE("GetResult::item is null");
            continue;
        }
        napi_value InputMethodSetting = nullptr;
        napi_create_object(env, &InputMethodSetting);

        std::string packageName = Str16ToStr8(item->mPackageName);
        napi_value jsPackageName = nullptr;
        napi_create_string_utf8(env, packageName.c_str(), NAPI_AUTO_LENGTH, &jsPackageName);
        napi_set_named_property(env, InputMethodSetting, "packageName", jsPackageName);

        std::string methodId = Str16ToStr8(item->mAbilityName);
        napi_value jsMethodId = nullptr;
        napi_create_string_utf8(env, packageName.c_str(), NAPI_AUTO_LENGTH, &jsMethodId);
        napi_set_named_property(env, InputMethodSetting, "methodId", jsMethodId);

        napi_set_element(env, result, index, InputMethodSetting);
        index++;
    }
    IMSA_HILOGE("GetResult::index is %{public}d", index);
}

void JsGetInputMethodSetting::ProcessCallbackOrPromiseCBArray(napi_env env,ContextBase *asyncContext)
{
    IMSA_HILOGE("run in ProcessCallbackOrPromiseCBArray");
    napi_value jsCode = asyncContext->GetErrorCodeValue(env, asyncContext->errCode);
    napi_value args[RESULT_ALL] = { jsCode, asyncContext->outData };

    if (asyncContext->deferred) {
        IMSA_HILOGE("ProcessCallbackOrPromiseCBArray::promise");
        if (asyncContext->errCode == ErrorCode::NO_ERROR) { 
            napi_resolve_deferred(env, asyncContext->deferred, args[RESULT_DATA]);
        } else {
            napi_reject_deferred(env, asyncContext->deferred, args[RESULT_ERROR]);
        }
    } else {
        IMSA_HILOGE("ProcessCallbackOrPromiseCBArray::callback");
        napi_value callback = nullptr;
        napi_get_reference_value(env, asyncContext->callbackRef, &callback);
        if (asyncContext->callbackRef == nullptr) {
            IMSA_HILOGE("ProcessCallbackOrPromiseCBArray::callback2222222222xxxxxxxxx");
        }
        IMSA_HILOGE("ProcessCallbackOrPromiseCBArray::callback2222222222");
        napi_value returnVal = nullptr;
        if (callback == nullptr) {
            IMSA_HILOGE("ProcessCallbackOrPromiseCBArray::callback333333xxxxxxxxxxxxxx");
        }
        napi_call_function(env, nullptr, callback, RESULT_ALL, args, &returnVal);//RESULT_CODE
        IMSA_HILOGE("ProcessCallbackOrPromiseCBArray::callback3333333333");
        if (asyncContext->callbackRef != nullptr) {
            napi_delete_reference(env, asyncContext->callbackRef);
        }
    }
}

void JsGetInputMethodSetting::ProcessCallbackOrPromise(napi_env env, ContextBase *asyncContext)
{
    IMSA_HILOGE("run in ProcessCallbackOrPromise");
    napi_value jsCode = asyncContext->GetErrorCodeValue(env, asyncContext->errCode);
    napi_value args[RESULT_ALL] = { jsCode, asyncContext->outData };
    if (asyncContext->deferred) {
        IMSA_HILOGE("ProcessCallbackOrPromise::promise");
        if (asyncContext->errCode == ErrorCode::NO_ERROR) {
            napi_resolve_deferred(env, asyncContext->deferred, args[RESULT_DATA]);
        } else {
            napi_reject_deferred(env, asyncContext->deferred, args[RESULT_ERROR]);
        }
    } else {
        IMSA_HILOGE("ProcessCallbackOrPromise::callback");
        napi_value callback = nullptr;
        napi_get_reference_value(env, asyncContext->callbackRef, &callback);
        napi_value returnVal = nullptr;
        napi_call_function(env, nullptr, callback, RESULT_ALL, args, &returnVal);
        if (asyncContext->callbackRef != nullptr) {
            napi_delete_reference(env, asyncContext->callbackRef);
        }
    }
}

napi_value JsGetInputMethodSetting::ListInputMethod(napi_env env, napi_callback_info info)
{    
    IMSA_HILOGE("run in ListInputMethod");
    // auto ctxt = std::make_shared<ContextBase>();
    struct ListInputContext : public ContextBase {
        std::vector<InputMethodProperty*> properties;
    };

    ListInputContext *ctxt = new (std::nothrow) ListInputContext();
    if (ctxt == nullptr) {
        IMSA_HILOGE("ListInputMethod::ctxt is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    ctxt->env = env;
    ctxt->callbackRef = nullptr;
    ctxt->ParseContext(env, info);

    napi_value promise = nullptr;//封装进函数
    if (ctxt->callbackRef == nullptr) {
        napi_create_promise(env, &ctxt->deferred, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "ListInputMethod", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(env,
        nullptr,
        resource,
        [ctxt](napi_env env, void *data) {
            // ListInputContext *ctxt = reinterpret_cast<ListInputContext*>(data);
            IMSA_HILOGE("ListInputMethod::napi_create_async_work in");
            ListInputContext *ctxt = reinterpret_cast<ListInputContext*>(data);
            if (ctxt == nullptr) {
                IMSA_HILOGE("ListInputMethod::ctxt is nullptr");
                return;
            }
            ctxt->properties = InputMethodController::GetInstance()->ListInputMethod();
            if (!ctxt->properties.empty()) {
                // ctxt->jsstatus = napi_ok;
                ctxt->errCode = ErrorCode::NO_ERROR;
                IMSA_HILOGE("JsInputMethodSetting::ListInputMethod get properties successful!");
            } else {
                ctxt->errCode = ErrorCode::ERROR_STATUS_BAD_VALUE;
                IMSA_HILOGE("JsInputMethodSetting::ListInputMethod properties is empty");
            }
        },
        [](napi_env env, napi_status status, void *data) {
            IMSA_HILOGE("ListInputMethod::napi_create_async_work out");
            ListInputContext *ctxt = reinterpret_cast<ListInputContext*>(data);
            if (ctxt == nullptr) {
                IMSA_HILOGE("ListInputMethod::ctxt is nullptr");
                return;
            }
            napi_create_array(env, &ctxt->outData);
            GetResult(env, ctxt->properties, ctxt->outData);
            ProcessCallbackOrPromiseCBArray(env, ctxt);
            napi_delete_async_work(env, ctxt->work);
            delete ctxt;
            ctxt = nullptr;
        },
        reinterpret_cast<void *>(ctxt),
        &ctxt->work);
    napi_queue_async_work(env, ctxt->work);
    return promise;
}

napi_value JsGetInputMethodSetting::DisplayOptionalInputMethod(napi_env env, napi_callback_info info)
{    
    IMSA_HILOGE("run in DisplayOptionalInputMethod");
    ContextBase *ctxt = new (std::nothrow) ContextBase();
    if (ctxt == nullptr) {
        IMSA_HILOGE("DisplayOptionalInputMethod::ctxt is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }

    ctxt->env = env;
    ctxt->callbackRef = nullptr;
    ctxt->ParseContext(env, info);

    napi_value promise = nullptr;
    if (ctxt->callbackRef == nullptr) {
        napi_create_promise(env, &ctxt->deferred, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "DisplayOptionalInputMethod", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(env,
        nullptr,
        resource,
        [](napi_env env, void *data) {
            IMSA_HILOGE("DisplayOptionalInputMethod::napi_create_async_work in");
            ContextBase *ctxt = reinterpret_cast<ContextBase*>(data);
            if (ctxt == nullptr) {
                IMSA_HILOGE("DisplayOptionalInputMethod::ctxt is nullptr");
                return;
            }
            ctxt->errCode = InputMethodController::GetInstance()->DisplayOptionalInputMethod();
        },
        [](napi_env env, napi_status status, void *data) {
            ContextBase *ctxt = reinterpret_cast<ContextBase*>(data);
            if (ctxt == nullptr) {
                IMSA_HILOGE("DisplayOptionalInputMethod::ctxt is nullptr");
                return;
            }
            napi_get_undefined(env, &ctxt->outData);//PARAMONE = 1
            ProcessCallbackOrPromise(env, ctxt);
            napi_delete_async_work(env, ctxt->work);
            delete ctxt;
            ctxt = nullptr;
        },
        reinterpret_cast<void *>(ctxt),
        &ctxt->work);
    napi_queue_async_work(env, ctxt->work);
    return promise;
}
}
}