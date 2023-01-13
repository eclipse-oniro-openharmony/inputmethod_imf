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
#include "js_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "string_ex.h"

namespace OHOS {
namespace MiscServices {
int32_t MAX_TYPE_NUM = 128;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_MAX = 6;
thread_local napi_ref JsGetInputMethodSetting::IMSRef_ = nullptr;
const std::string JsGetInputMethodSetting::IMS_CLASS_NAME = "InputMethodSetting";

std::mutex JsGetInputMethodSetting::msMutex_;
std::shared_ptr<JsGetInputMethodSetting> JsGetInputMethodSetting::inputMethod_{ nullptr };
napi_value JsGetInputMethodSetting::Init(napi_env env, napi_value exports)
{
    napi_value maxTypeNumber = nullptr;
    napi_create_int32(env, MAX_TYPE_NUM, &maxTypeNumber);

    napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("getInputMethodSetting", GetInputMethodSetting),
        DECLARE_NAPI_FUNCTION("getSetting", GetSetting),

        DECLARE_NAPI_PROPERTY("EXCEPTION_PERMISSION",
            GetJsConstProperty(env, static_cast<uint32_t>(IMFErrorCode::EXCEPTION_PERMISSION))),
        DECLARE_NAPI_PROPERTY("EXCEPTION_PARAMCHECK",
            GetJsConstProperty(env, static_cast<uint32_t>(IMFErrorCode::EXCEPTION_PARAMCHECK))),
        DECLARE_NAPI_PROPERTY("EXCEPTION_UNSUPPORTED",
            GetJsConstProperty(env, static_cast<uint32_t>(IMFErrorCode::EXCEPTION_UNSUPPORTED))),
        DECLARE_NAPI_PROPERTY("EXCEPTION_PACKAGEMANAGER",
            GetJsConstProperty(env, static_cast<uint32_t>(IMFErrorCode::EXCEPTION_PACKAGEMANAGER))),
        DECLARE_NAPI_PROPERTY(
            "EXCEPTION_IMENGINE", GetJsConstProperty(env, static_cast<uint32_t>(IMFErrorCode::EXCEPTION_IMENGINE))),
        DECLARE_NAPI_PROPERTY(
            "EXCEPTION_IMCLIENT", GetJsConstProperty(env, static_cast<uint32_t>(IMFErrorCode::EXCEPTION_IMCLIENT))),
        DECLARE_NAPI_PROPERTY(
            "EXCEPTION_KEYEVENT", GetJsConstProperty(env, static_cast<uint32_t>(IMFErrorCode::EXCEPTION_KEYEVENT))),
        DECLARE_NAPI_PROPERTY("EXCEPTION_CONFPERSIST",
            GetJsConstProperty(env, static_cast<uint32_t>(IMFErrorCode::EXCEPTION_CONFPERSIST))),
        DECLARE_NAPI_PROPERTY("EXCEPTION_CONTROLLER",
            GetJsConstProperty(env, static_cast<uint32_t>(IMFErrorCode::EXCEPTION_CONTROLLER))),
        DECLARE_NAPI_PROPERTY(
            "EXCEPTION_SETTINGS", GetJsConstProperty(env, static_cast<uint32_t>(IMFErrorCode::EXCEPTION_SETTINGS))),
        DECLARE_NAPI_PROPERTY(
            "EXCEPTION_IMMS", GetJsConstProperty(env, static_cast<uint32_t>(IMFErrorCode::EXCEPTION_IMMS))),
        DECLARE_NAPI_PROPERTY(
            "EXCEPTION_OTHERS", GetJsConstProperty(env, static_cast<uint32_t>(IMFErrorCode::EXCEPTION_OTHERS))),
        DECLARE_NAPI_PROPERTY("MAX_TYPE_NUM", maxTypeNumber),
    };
    NAPI_CALL(
        env, napi_define_properties(env, exports, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("listInputMethod", ListInputMethod),
        DECLARE_NAPI_FUNCTION("listInputMethodSubtype", ListInputMethodSubtype),
        DECLARE_NAPI_FUNCTION("listCurrentInputMethodSubtype", ListCurrentInputMethodSubtype),
        DECLARE_NAPI_FUNCTION("getInputMethods", GetInputMethods),
        DECLARE_NAPI_FUNCTION("displayOptionalInputMethod", DisplayOptionalInputMethod),
        DECLARE_NAPI_FUNCTION("showOptionalInputMethods", ShowOptionalInputMethods),
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
    IMSA_HILOGI("run in JsConstructor");
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));

    auto delegate = GetInputMethodSettingInstance();
    if (delegate == nullptr) {
        IMSA_HILOGE("settingObject is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_wrap(
        env, thisVar, delegate.get(),
        [](napi_env env, void *data, void *hint) { IMSA_HILOGE("delete JsInputMethodSetting"); }, nullptr, nullptr);
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
                IMSA_HILOGE("input method nullptr");
                return nullptr;
            }
            inputMethod_ = engine;
            InputMethodController::GetInstance()->setImeListener(inputMethod_);
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
        IMSA_HILOGE("GetSetting::napi_get_reference_value not ok");
        if (needThrowException) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_SETTINGS, "", TYPE_OBJECT);
        }
        return nullptr;
    }
    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("GetSetting::napi_new_instance not ok");
        if (needThrowException) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_SETTINGS, "", TYPE_OBJECT);
        }
        return nullptr;
    }
    return instance;
}

napi_status JsGetInputMethodSetting::GetInputMethodProperty(
    napi_env env, napi_value argv, std::shared_ptr<ListInputContext> ctxt)
{
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_generic_failure;
    status = napi_typeof(env, argv, &valueType);
    if (valueType == napi_object) {
        napi_value result = nullptr;
        napi_get_named_property(env, argv, "packageName", &result);
        ctxt->property.name = JsInputMethod::GetStringProperty(env, result);

        result = nullptr;
        napi_get_named_property(env, argv, "methodId", &result);
        ctxt->property.id = JsInputMethod::GetStringProperty(env, result);

        if (ctxt->property.name.empty() || ctxt->property.id.empty()) {
            result = nullptr;
            napi_get_named_property(env, argv, "name", &result);
            ctxt->property.name = JsInputMethod::GetStringProperty(env, result);

            result = nullptr;
            napi_get_named_property(env, argv, "id", &result);
            ctxt->property.id = JsInputMethod::GetStringProperty(env, result);
        }
        if (ctxt->property.name.empty() || ctxt->property.id.empty()) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "Parameter error.", TYPE_NONE);
            return napi_invalid_arg;
        }

        result = nullptr;
        napi_get_named_property(env, argv, "label", &result);
        ctxt->property.label = JsInputMethod::GetStringProperty(env, result);

        result = nullptr;
        napi_get_named_property(env, argv, "icon", &result);
        ctxt->property.icon = JsInputMethod::GetStringProperty(env, result);

        result = nullptr;
        napi_get_named_property(env, argv, "iconId", &result);
        ctxt->property.iconId = JsInputMethod::GetNumberProperty(env, result);
        IMSA_HILOGD("methodId:%{public}s, packageName:%{public}s", ctxt->property.id.c_str(),
                ctxt->property.name.c_str());
    }
    return status;
}

napi_value JsGetInputMethodSetting::ListInputMethod(napi_env env, napi_callback_info info)
{
    IMSA_HILOGI("run in ListInputMethod");
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
            IMSA_HILOGI("exec ---- ListInputMethod success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            return;
        }
        ctxt->SetErrorCode(errCode);
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt));
    return asyncCall.Call(env, exec);
}

napi_value JsGetInputMethodSetting::GetInputMethods(napi_env env, napi_callback_info info)
{
    IMSA_HILOGI("run in GetInputMethods");
    auto ctxt = std::make_shared<ListInputContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (argc < 1) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "should has one parameter.", TYPE_NONE);
            return napi_invalid_arg;
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
        int32_t errCode =
            InputMethodController::GetInstance()->ListInputMethod(ctxt->inputMethodStatus == ENABLE, ctxt->properties);
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec ---- GetInputMethods success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            return;
        }
        ctxt->SetErrorCode(errCode);
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt));
    return asyncCall.Call(env, exec);
}

napi_value JsGetInputMethodSetting::DisplayOptionalInputMethod(napi_env env, napi_callback_info info)
{
    return DisplayInputMethod(env, info, false);
}

napi_value JsGetInputMethodSetting::DisplayInputMethod(napi_env env, napi_callback_info info, bool needThrowException)
{
    IMSA_HILOGI("run in DisplayInputMethod");
    auto ctxt = std::make_shared<DisplayOptionalInputMethodContext>();
    auto input = [ctxt](
                     napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status { return napi_ok; };
    auto output = [ctxt, needThrowException](napi_env env, napi_value *result) -> napi_status {
        if (needThrowException) {
            napi_status status = napi_get_boolean(env, ctxt->isDisplayed, result);
            IMSA_HILOGE("output napi_get_boolean != nullptr[%{public}d]", result != nullptr);
            return status;
        }
        return napi_ok;
    };
    auto exec = [ctxt, needThrowException](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->ShowOptionalInputMethod();
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGE("exec ---- DisplayOptionalInputMethod success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            ctxt->isDisplayed = true;
            return;
        }
        if (errCode == ErrorCode::ERROR_ABILITY_ACTIVATING) {
            IMSA_HILOGE("exec ---- DisplayOptionalInputMethod failed: ability already activited");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            ctxt->isDisplayed = true;
        }
        if (needThrowException) {
            ctxt->SetErrorCode(errCode);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt), 0);
    return asyncCall.Call(env, exec);
}

napi_value JsGetInputMethodSetting::ShowOptionalInputMethods(napi_env env, napi_callback_info info)
{
    return DisplayInputMethod(env, info, true);
}

napi_value JsGetInputMethodSetting::ListInputMethodSubtype(napi_env env, napi_callback_info info)
{
    IMSA_HILOGI("run in ListInputMethodSubtype");
    auto ctxt = std::make_shared<ListInputContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (argc < 1) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "should has one parameter.", TYPE_NONE);
            return napi_invalid_arg;
        }
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType != napi_object) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, " inputMethodProperty: ", TYPE_OBJECT);
            return napi_object_expected;
        }
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
            IMSA_HILOGI("exec ---- ListInputMethodSubtype success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            return;
        }
        ctxt->SetErrorCode(errCode);
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt));
    return asyncCall.Call(env, exec);
}

napi_value JsGetInputMethodSetting::ListCurrentInputMethodSubtype(napi_env env, napi_callback_info info)
{
    IMSA_HILOGI("run in ListCurrentInputMethodSubtype");
    auto ctxt = std::make_shared<ListInputContext>();
    auto input = [ctxt](
                     napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status { return napi_ok; };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        *result = JsInputMethod::GetJSInputMethodSubProperties(env, ctxt->subProperties);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->ListCurrentInputMethodSubtype(ctxt->subProperties);
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec ---- ListCurrentInputMethodSubtype success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            return;
        }
        ctxt->SetErrorCode(errCode);
    };
    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(ctxt));
    return asyncCall.Call(env, exec);
}

JsGetInputMethodSetting *JsGetInputMethodSetting::GetNative(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_MAX;
    void *native = nullptr;
    napi_value self = nullptr;
    napi_value argv[ARGC_MAX] = { nullptr };
    napi_status status = napi_invalid_arg;
    status = napi_get_cb_info(env, info, &argc, argv, &self, nullptr);
    if (self == nullptr && argc >= ARGC_MAX) {
        IMSA_HILOGE("napi_get_cb_info failed");
        return nullptr;
    }

    status = napi_unwrap(env, self, &native);
    NAPI_ASSERT(env, (status == napi_ok && native != nullptr), "napi_unwrap failed!");
    return reinterpret_cast<JsGetInputMethodSetting *>(native);
}

bool JsGetInputMethodSetting::Equals(napi_env env, napi_value value, napi_ref copy, std::thread::id threadId)
{
    if (copy == nullptr) {
        return (value == nullptr);
    }

    if (threadId != std::this_thread::get_id()) {
        IMSA_HILOGD("napi_value can not be compared");
        return false;
    }

    napi_value copyValue = nullptr;
    napi_get_reference_value(env, copy, &copyValue);

    bool isEquals = false;
    napi_strict_equals(env, value, copyValue, &isEquals);
    IMSA_HILOGD("value compare result: %{public}d", isEquals);
    return isEquals;
}

void JsGetInputMethodSetting::RegisterListener(
    napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj)
{
    IMSA_HILOGI("RegisterListener %{public}s", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGE("methodName: %{public}s not registered!", type.c_str());
    }

    auto callbacks = jsCbMap_[type];
    bool ret = std::any_of(callbacks.begin(), callbacks.end(), [&callback](std::shared_ptr<JSCallbackObject> cb) {
        return Equals(cb->env_, callback, cb->callback_, cb->threadId_);
    });
    if (ret) {
        IMSA_HILOGE("JsGetInputMethodSetting::RegisterListener callback already registered!");
        return;
    }

    IMSA_HILOGI("Add %{public}s callbackObj into jsCbMap_", type.c_str());
    jsCbMap_[type].push_back(std::move(callbackObj));
}

napi_value JsGetInputMethodSetting::Subscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc == ARGC_TWO, "Wrong number of arguments, requires 2");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[ARGC_ZERO], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "type is not a string");
    std::string type = JsInputMethod::GetStringProperty(env, argv[ARGC_ZERO]);
    IMSA_HILOGE("event type is: %{public}s", type.c_str());

    valuetype = napi_undefined;
    napi_typeof(env, argv[ARGC_ONE], &valuetype);
    NAPI_ASSERT(env, valuetype == napi_function, "callback is not a function");

    auto engine = GetNative(env, info);
    if (engine == nullptr) {
        return nullptr;
    }
    std::shared_ptr<JSCallbackObject> callback =
        std::make_shared<JSCallbackObject>(env, argv[ARGC_ONE], std::this_thread::get_id());
    engine->RegisterListener(argv[ARGC_ONE], type, callback);

    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

void JsGetInputMethodSetting::UnRegisterListener(napi_value callback, std::string type)
{
    IMSA_HILOGI("UnRegisterListener %{public}s", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGE("methodName: %{public}s already unRegistered!", type.c_str());
        return;
    }

    if (callback == nullptr) {
        jsCbMap_.erase(type);
        IMSA_HILOGE("callback is nullptr");
        return;
    }

    for (auto item = jsCbMap_[type].begin(); item != jsCbMap_[type].end(); item++) {
        if (Equals((*item)->env_, callback, (*item)->callback_, (*item)->threadId_)) {
            jsCbMap_[type].erase(item);
            break;
        }
    }

    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
}

napi_value JsGetInputMethodSetting::UnSubscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc == ARGC_ONE || argc == ARGC_TWO, "Wrong number of arguments, requires 1 or 2");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[ARGC_ZERO], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "type is not a string");
    std::string type = JsInputMethod::GetStringProperty(env, argv[ARGC_ZERO]);
    IMSA_HILOGE("event type is: %{public}s", type.c_str());

    auto engine = GetNative(env, info);
    if (engine == nullptr) {
        return nullptr;
    }

    if (argc == ARGC_TWO) {
        valuetype = napi_undefined;
        napi_typeof(env, argv[ARGC_ONE], &valuetype);
        NAPI_ASSERT(env, valuetype == napi_function, "callback is not a function");
    }
    engine->UnRegisterListener(argv[ARGC_ONE], type);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

uv_work_t *JsGetInputMethodSetting::GetImeChangeUVwork(
    std::string type, const Property &property, const SubProperty &subProperty)
{
    IMSA_HILOGI("run in GetImeChangeUVwork");
    UvEntry *entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        if (jsCbMap_[type].empty()) {
            IMSA_HILOGE("%{public}s cb-vector is empty", type.c_str());
            return nullptr;
        }
        entry = new (std::nothrow) UvEntry(jsCbMap_[type], type);
        if (entry == nullptr) {
            IMSA_HILOGE("entry ptr is nullptr!");
            return nullptr;
        }
        entry->property = property;
        entry->subProperty = subProperty;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        IMSA_HILOGE("entry ptr is nullptr!");
        return nullptr;
    }
    work->data = entry;
    return work;
}

void JsGetInputMethodSetting::OnImeChange(const Property &property, const SubProperty &subProperty)
{
    IMSA_HILOGI("run in OnImeChange");
    std::string type = "imeChange";
    uv_work_t *work = GetImeChangeUVwork(type, property, subProperty);
    if (work == nullptr) {
        return;
    }
    uv_queue_work(
        loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<UvEntry> entry(static_cast<UvEntry *>(work->data), [work](UvEntry *data) {
                delete data;
                delete work;
            });
            if (entry == nullptr) {
                IMSA_HILOGE("OnInputStart:: entryptr is null");
                return;
            }
            for (size_t i = 0; i < entry->vecCopy.size(); ) {
                JsUtils::CompareThread(i, entry->vecCopy);
                napi_value subProperty = JsInputMethod::GetJsInputMethodSubProperty(entry->vecCopy[i]->env_, entry->subProperty);
                napi_value property = JsInputMethod::GetJsInputMethodProperty(entry->vecCopy[i]->env_, entry->property);
                if (subProperty == nullptr || property == nullptr) {
                    IMSA_HILOGE("get KBCins or TICins failed:");
                    break;
                }
                napi_value args[] = { property, subProperty };
                JsUtils::CallJsFunction(args, ARGC_TWO, entry->vecCopy[i]);
            }
        });
}
} // namespace MiscServices
} // namespace OHOS