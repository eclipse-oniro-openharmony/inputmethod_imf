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

#include "callback_Processor.h"
#include "event_checker.h"
#include "input_client_info.h"
#include "input_method_controller.h"
#include "input_method_status.h"
#include "js_input_method.h"
#include "js_util.h"
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
thread_local napi_ref JsGetInputMethodSetting::IMSRef_ = nullptr;
const std::string JsGetInputMethodSetting::IMS_CLASS_NAME = "InputMethodSetting";
const std::map<InputWindowStatus, std::string> PANEL_STATUS{ { InputWindowStatus::SHOW, "imeShow" },
    { InputWindowStatus::HIDE, "imeHide" } };
std::mutex JsGetInputMethodSetting::msMutex_;
std::shared_ptr<JsGetInputMethodSetting> JsGetInputMethodSetting::inputMethod_{ nullptr };
napi_value JsGetInputMethodSetting::Init(napi_env env, napi_value exports)
{
    napi_value maxTypeNumber = nullptr;
    napi_create_int32(env, MAX_TYPE_NUM, &maxTypeNumber);

    napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("getInputMethodSetting", GetInputMethodSetting),
        DECLARE_NAPI_FUNCTION("getSetting", GetSetting),
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
    napi_status status = napi_wrap(
        env, thisVar, delegate.get(), [](napi_env env, void *data, void *hint) {}, nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("JsGetInputMethodSetting napi_wrap failed: %{public}d", status);
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
                IMSA_HILOGE("input method nullptr");
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
    napi_typeof(env, argv, &valueType);
    PARAM_CHECK_RETURN(env, valueType == napi_object, "Parameter error.", TYPE_OBJECT, napi_invalid_arg);
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
    PARAM_CHECK_RETURN(env, (!ctxt->property.name.empty() && !ctxt->property.id.empty()), "Parameter error.",
        TYPE_NONE, napi_invalid_arg);
    IMSA_HILOGD("methodId:%{public}s, packageName:%{public}s", ctxt->property.id.c_str(), ctxt->property.name.c_str());
    return napi_ok;
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
    // 1 means JsAPI:listInputMethod has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "listInputMethod");
}

napi_value JsGetInputMethodSetting::GetInputMethods(napi_env env, napi_callback_info info)
{
    IMSA_HILOGI("run in GetInputMethods");
    auto ctxt = std::make_shared<ListInputContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should has one parameter.", TYPE_NONE, napi_invalid_arg);
        bool enable = false;
        napi_status status = JsUtils::GetValue(env, argv[ARGC_ZERO], enable);
        PARAM_CHECK_RETURN(env, status == napi_ok, "enable.", TYPE_NUMBER, napi_invalid_arg);
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
            IMSA_HILOGI("exec ---- GetInputMethods success");
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

napi_value JsGetInputMethodSetting::DisplayOptionalInputMethod(napi_env env, napi_callback_info info)
{
    IMSA_HILOGI("JsGetInputMethodSetting run in");
    auto ctxt = std::make_shared<DisplayOptionalInputMethodContext>();
    auto input = [ctxt](
                     napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status { return napi_ok; };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status { return napi_ok; };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->DisplayOptionalInputMethod();
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec ---- DisplayOptionalInputMethod success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI:displayOptionalInputMethod has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "displayOptionalInputMethod");
}

napi_value JsGetInputMethodSetting::ShowOptionalInputMethods(napi_env env, napi_callback_info info)
{
    IMSA_HILOGI("JsGetInputMethodSetting run in");
    auto ctxt = std::make_shared<DisplayOptionalInputMethodContext>();
    auto input = [ctxt](
                     napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status { return napi_ok; };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, ctxt->isDisplayed, result);
        IMSA_HILOGI("output napi_get_boolean != nullptr[%{public}d]", result != nullptr);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errCode = InputMethodController::GetInstance()->DisplayOptionalInputMethod();
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGE("exec ---- DisplayOptionalInputMethod success");
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            ctxt->isDisplayed = true;
            return;
        } else {
            ctxt->SetErrorCode(errCode);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI:showOptionalInputMethods has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "showOptionalInputMethods");
}

napi_value JsGetInputMethodSetting::ListInputMethodSubtype(napi_env env, napi_callback_info info)
{
    IMSA_HILOGI("run in ListInputMethodSubtype");
    auto ctxt = std::make_shared<ListInputContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should has one parameter.", TYPE_NONE, napi_invalid_arg);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        PARAM_CHECK_RETURN(env, valueType == napi_object, "inputMethodProperty", TYPE_OBJECT, napi_invalid_arg);
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
    // 2 means JsAPI:listInputMethodSubtype has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "listInputMethodSubtype");
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
    // 1 means JsAPI:listCurrentInputMethodSubtype has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "listCurrentInputMethodSubtype");
}

int32_t JsGetInputMethodSetting::RegisterListener(
    napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj)
{
    IMSA_HILOGD("RegisterListener %{public}s", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty()) {
        InputMethodController::GetInstance()->SetSettingListener(inputMethod_);
        auto ret = InputMethodController::GetInstance()->UpdateListenEventFlag(type, true);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("UpdateListenEventFlag failed, ret: %{public}d, type: %{public}s", ret, type.c_str());
            return ret;
        }
    }
    if (!jsCbMap_.empty() && jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGI("start type: %{public}s listening.", type.c_str());
        auto ret = InputMethodController::GetInstance()->UpdateListenEventFlag(type, true);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("UpdateListenEventFlag failed, ret: %{public}d, type: %{public}s", ret, type.c_str());
            return ret;
        }
    }

    auto callbacks = jsCbMap_[type];
    bool ret = std::any_of(callbacks.begin(), callbacks.end(), [&callback](std::shared_ptr<JSCallbackObject> cb) {
        return JsUtils::Equals(cb->env_, callback, cb->callback_, cb->threadId_);
    });
    if (ret) {
        IMSA_HILOGE("JsGetInputMethodSetting::RegisterListener callback already registered!");
        return ErrorCode::NO_ERROR;
    }

    IMSA_HILOGI("Add %{public}s callbackObj into jsCbMap_", type.c_str());
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
    if (argc < 2 || !JsUtil::GetValue(env, argv[0], type)
        || !EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_SETTING, type)
        || JsUtil::GetType(env, argv[1]) != napi_function) {
        IMSA_HILOGE("Subscribe failed, type:%{public}s", type.c_str());
        return nullptr;
    }
    IMSA_HILOGD("Subscribe type:%{public}s.", type.c_str());
    auto engine = reinterpret_cast<JsGetInputMethodSetting *>(JsUtils::GetNativeSelf(env, info));
    if (engine == nullptr) {
        return nullptr;
    }
    std::shared_ptr<JSCallbackObject> callback =
        std::make_shared<JSCallbackObject>(env, argv[ARGC_ONE], std::this_thread::get_id());
    auto ret = engine->RegisterListener(argv[ARGC_ONE], type, callback);
    auto errCode = JsUtils::Convert(ret);
    if (errCode == EXCEPTION_PERMISSION) {
        JsUtils::ThrowException(env, errCode, "", TYPE_NONE);
    }
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
        IMSA_HILOGI("stop all type: %{public}s listening.", type.c_str());
        InputMethodController::GetInstance()->UpdateListenEventFlag(type, false);
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
        InputMethodController::GetInstance()->UpdateListenEventFlag(type, false);
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
    if (argc < 1 || !JsUtil::GetValue(env, argv[0], type)
        || !EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_SETTING, type)) {
        IMSA_HILOGE("UnSubscribe failed, type:%{public}s", type.c_str());
        return nullptr;
    }
    // If the type of optional parameter is wrong, make it nullptr
    if (JsUtil::GetType(env, argv[1]) != napi_function) {
        argv[1] = nullptr;
    }
    IMSA_HILOGD("UnSubscribe type:%{public}s.", type.c_str());

    auto engine = reinterpret_cast<JsGetInputMethodSetting *>(JsUtils::GetNativeSelf(env, info));
    if (engine == nullptr) {
        return nullptr;
    }
    engine->UnRegisterListener(argv[ARGC_ONE], type);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

void JsGetInputMethodSetting::OnImeChange(const Property &property, const SubProperty &subProperty)
{
    IMSA_HILOGD("run in");
    std::string type = "imeChange";
    uv_work_t *work = GetUVwork(type, [&property, &subProperty](UvEntry &entry) {
        entry.property = property;
        entry.subProperty = subProperty;
    });
    if (work == nullptr) {
        IMSA_HILOGD("failed to get uv entry");
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
            auto getImeChangeProperty = [entry](napi_value *args, uint8_t argc,
                                            std::shared_ptr<JSCallbackObject> item) -> bool {
                if (argc < 2) {
                    return false;
                }
                napi_value subProperty = JsInputMethod::GetJsInputMethodSubProperty(item->env_, entry->subProperty);
                napi_value property = JsInputMethod::GetJsInputMethodProperty(item->env_, entry->property);
                if (subProperty == nullptr || property == nullptr) {
                    IMSA_HILOGE("get KBCins or TICins failed:");
                    return false;
                }
                args[ARGC_ZERO] = property;
                args[ARGC_ONE] = subProperty;
                return true;
            };
            CallbackProcessor::TraverseCallback({ entry->vecCopy, ARGC_TWO, getImeChangeProperty });
        });
}

void JsGetInputMethodSetting::OnPanelStatusChange(
    const InputWindowStatus &status, const std::vector<InputWindowInfo> &windowInfo)
{
    IMSA_HILOGI("status: %{public}u", static_cast<uint32_t>(status));
    auto it = PANEL_STATUS.find(status);
    if (it == PANEL_STATUS.end()) {
        return;
    }
    auto type = it->second;
    uv_work_t *work = GetUVwork(type, [&windowInfo](UvEntry &entry) { entry.windowInfo = windowInfo; });
    if (work == nullptr) {
        IMSA_HILOGD("failed to get uv entry");
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
                IMSA_HILOGE("OnInputStart:: entry is nullptr");
                return;
            }
            auto getWindowInfo = [entry](
                                     napi_value *args, uint8_t argc, std::shared_ptr<JSCallbackObject> item) -> bool {
                if (argc < 1) {
                    return false;
                }
                auto windowInfo = JsUtils::GetValue(item->env_, entry->windowInfo);
                if (windowInfo == nullptr) {
                    IMSA_HILOGE("converse windowInfo failed");
                    return false;
                }
                args[ARGC_ZERO] = windowInfo;
                return true;
            };
            CallbackProcessor::TraverseCallback({ entry->vecCopy, ARGC_ONE, getWindowInfo });
        });
}

uv_work_t *JsGetInputMethodSetting::GetUVwork(const std::string &type, EntrySetter entrySetter)
{
    IMSA_HILOGD("run in, type: %{public}s", type.c_str());
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
        if (entrySetter != nullptr) {
            entrySetter(*entry);
        }
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        IMSA_HILOGE("entry ptr is nullptr!");
        return nullptr;
    }
    work->data = entry;
    return work;
}
} // namespace MiscServices
} // namespace OHOS