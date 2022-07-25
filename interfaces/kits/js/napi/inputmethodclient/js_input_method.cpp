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

#include "input_method_controller.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "event_handler.h"
#include "event_runner.h"
#include "string_ex.h"
#include "js_input_method.h"

namespace OHOS {
namespace MiscServices {
napi_value JsInputMethod::Init(napi_env env, napi_value exports) {
    napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("switchInputMethod", SwitchInputMethod),
    };
    NAPI_CALL(
        env, napi_define_properties(env, exports, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

    napi_property_descriptor properties[] = {
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, IMS_CLASS_NAME.c_str(), IMS_CLASS_NAME.size(),
        JsConstructor, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &IMSRef_));
    NAPI_CALL(env, napi_set_named_property(env, exports, IMS_CLASS_NAME.c_str(), cons));
    return exports;
};

napi_value JsInputMethod::JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    IMSA_HILOGE("run in JsConstructor");
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));

    JsInputMethod *IMSobject = new (std::nothrow) JsInputMethod();
    if (IMSobject == nullptr) {
        IMSA_HILOGE("IMSobject == nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_wrap(env, thisVar, IMSobject, [](napi_env env, void *data, void *hint) {
        auto* objInfo = reinterpret_cast<JsInputMethod*>(data);
        if (objInfo != nullptr) {
            IMSA_HILOGE("objInfo != nullptr");
            delete objInfo;
        }
    }, nullptr, nullptr);

    return thisVar;
}

void JsInputMethod::CallbackOrPromiseSwitchInput(
    napi_env env, const SwitchInput *switchInput, napi_value err, napi_value data)
{    
    IMSA_HILOGE("run in CallbackOrPromiseSwitchInput");
    napi_value args[RESULT_COUNT] = {err, data};
    if (switchInput->deferred) {
        if (switchInput->status == napi_ok) {
            IMSA_HILOGE("CallbackOrPromiseSwitchInput::promise");
            napi_resolve_deferred(env, switchInput->deferred, args[RESULT_DATA]);
        } else {
            napi_reject_deferred(env, switchInput->deferred, args[RESULT_ERROR]);
        }
    } else {
        IMSA_HILOGE("CallbackOrPromiseSwitchInput::callback");
        napi_value callback = nullptr;
        napi_get_reference_value(env, switchInput->callbackRef, &callback);
        if (switchInput->callbackRef == nullptr) {
            IMSA_HILOGE("CallbackOrPromiseSwitchInput::callback2222222222xxxxxxxxx");
        }
        IMSA_HILOGE("CallbackOrPromiseSwitchInput::callback2222222222");
        napi_value returnVal = nullptr;
        if (callback == nullptr) {
            IMSA_HILOGE("CallbackOrPromiseSwitchInput::callback333333xxxxxxxxxxxxxx");
        }
        napi_call_function(env, nullptr, callback, RESULT_COUNT, &args[0], &returnVal);
        IMSA_HILOGE("CallbackOrPromiseSwitchInput::callback3333333333");
        if (switchInput->callbackRef != nullptr) {
            napi_delete_reference(env, switchInput->callbackRef);
        }
    }
}

napi_value JsInputMethod::GetErrorCodeValue(napi_env env, int errCode)
{
    // ACCOUNT_LOGD("enter");
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &jsValue));
    NAPI_CALL(env, napi_create_object(env, &jsObject));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "code", jsValue));
    return jsObject;
}

napi_value JsInputMethod::SwitchInputMethod(napi_env env, napi_callback_info info)
{
    struct SwitchInputMethodContext : public ContextBase{
        bool sSwitchInput = false;
        std::string packageName;
        std::string methodId;
    }
    
    SwitchInputMethodContext *switchInpput = new  (std::nothrow) SwitchInputMethodContext();
    if (switchInpput == nullptr) {
        napi_value promise = nullptr;
        napi_get_null(env, &promise);
        return promise;
    }

    auto input = [env, ctxt](size_t argc, napi_value* argv) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType == napi_object) {
            napi_value result = nullptr;
            napi_get_named_property(env, argv[0], "packageName", &result);
            ctxt->packageName = GetStringProperty(env, result);
            IMSA_HILOGE("packageName:%{public}s", packageName.c_str());
            result = nullptr;
            napi_get_named_property(env, argv[0], "methodId", &result);
            ctxt->methodId = GetStringProperty(env, result);
            IMSA_HILOGE("methodId:%{public}s", methodId.c_str());
        } 
    };

    switchInpput->env = env;
    switchInpput->callbackRef = nullptr;
    switchInpput->ParseContext(env, info, input);

    napi_value promise = nullptr;
    if (switchInpput->callbackRef == nullptr) {
        napi_create_promise(env, &switchInpput->deferred, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "SwitchInputMethod", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(env,
        nullptr,
        resource,
        [](napi_env env, void *data) {
            IMSA_HILOGE("SwitchInputMethod::napi_create_async_work in");
            SwitchInput *switchInpput = reinterpret_cast<SwitchInput *>(data);
            InputMethodProperty *property = new (std::nothrow) InputMethodProperty();
            if (property == nullptr){
                IMSA_HILOGE("SwitchInputMethod:: property == nullptr");
                return;
            }
            property->mPackageName = Str8ToStr16(switchInpput->packageName);
            property->mImeId = Str8ToStr16(switchInpput->methodId);
            switchInpput->errCode = InputMethodController::GetInstance()->SwitchInputMethod(property);
             IMSA_HILOGI("ListInputMethod::************************************");
            if (switchInpput->errCode == 0) {
                IMSA_HILOGE("JsInputMethod::switchInpput successful!");
                switchInpput->sSwitchInput = true;
            }
            switchInpput->status = (switchInpput->errCode == 0) ? napi_ok : napi_generic_failure;
            //delete property;
            //property = nullptr;
        },
        [](napi_env env, napi_status status, void *data) {
            IMSA_HILOGE("JsInputMethod::SwitchInputMethod   napi_status status");
            SwitchInput *switchInpput = reinterpret_cast<SwitchInput *>(data);
            switchInpput->errCode = 0;
            napi_value getResult[RESULT_ALL] = {0};
            IMSA_HILOGE("JsInputMethod::SwitchInputMethod   GetErrorCodeValue start ");
            getResult[PARAMZERO] = GetErrorCodeValue(env, switchInpput->errCode);
            IMSA_HILOGE("JsInputMethod::SwitchInputMethod   GetErrorCodeValue end ");
            napi_get_boolean(env, switchInpput->sSwitchInput, &getResult[PARAMONE]);
            CallbackOrPromiseSwitchInput(env, switchInpput, getResult[PARAMZERO], getResult[PARAMONE]);
            napi_delete_async_work(env, switchInpput->work);
            delete switchInpput;
            switchInpput = nullptr;
        },
        reinterpret_cast<void *>(switchInpput),
        &switchInpput->work);

    napi_queue_async_work(env, switchInpput->work);
    return promise;
}
}
}