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

#include "js_input_method_engine_setting.h"

#include <thread>

#include "input_method_ability.h"
#include "input_method_property.h"
#include "input_method_utils.h"
#include "js_keyboard_controller_engine.h"
#include "js_runtime_utils.h"
#include "js_text_input_client_engine.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace MiscServices {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_MAX = 6;
constexpr int32_t V9_FLAG = 1;
constexpr int32_t ORIGINAL_FLAG = 2;
const std::string JsInputMethodEngineSetting::IMES_CLASS_NAME = "InputMethodEngine";
thread_local napi_ref JsInputMethodEngineSetting::IMESRef_ = nullptr;

std::mutex JsInputMethodEngineSetting::engineMutex_;
std::shared_ptr<JsInputMethodEngineSetting> JsInputMethodEngineSetting::inputMethodEngine_{ nullptr };

napi_value JsInputMethodEngineSetting::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_PROPERTY(
            "ENTER_KEY_TYPE_UNSPECIFIED", GetJsConstProperty(env, static_cast<uint32_t>(EnterKeyType::UNSPECIFIED))),
        DECLARE_NAPI_PROPERTY("ENTER_KEY_TYPE_GO", GetJsConstProperty(env, static_cast<uint32_t>(EnterKeyType::GO))),
        DECLARE_NAPI_PROPERTY(
            "ENTER_KEY_TYPE_SEARCH", GetJsConstProperty(env, static_cast<uint32_t>(EnterKeyType::SEARCH))),
        DECLARE_NAPI_PROPERTY(
            "ENTER_KEY_TYPE_SEND", GetJsConstProperty(env, static_cast<uint32_t>(EnterKeyType::SEND))),
        DECLARE_NAPI_PROPERTY(
            "ENTER_KEY_TYPE_NEXT", GetJsConstProperty(env, static_cast<uint32_t>(EnterKeyType::NEXT))),
        DECLARE_NAPI_PROPERTY(
            "ENTER_KEY_TYPE_DONE", GetJsConstProperty(env, static_cast<uint32_t>(EnterKeyType::DONE))),
        DECLARE_NAPI_PROPERTY(
            "ENTER_KEY_TYPE_PREVIOUS", GetJsConstProperty(env, static_cast<uint32_t>(EnterKeyType::PREVIOUS))),

        DECLARE_NAPI_PROPERTY("PATTERN_NULL", GetIntJsConstProperty(env, static_cast<int32_t>(TextInputType::NONE))),
        DECLARE_NAPI_PROPERTY("PATTERN_TEXT", GetJsConstProperty(env, static_cast<uint32_t>(TextInputType::TEXT))),
        DECLARE_NAPI_PROPERTY("PATTERN_NUMBER", GetJsConstProperty(env, static_cast<uint32_t>(TextInputType::NUMBER))),
        DECLARE_NAPI_PROPERTY("PATTERN_PHONE", GetJsConstProperty(env, static_cast<uint32_t>(TextInputType::PHONE))),
        DECLARE_NAPI_PROPERTY(
            "PATTERN_DATETIME", GetJsConstProperty(env, static_cast<uint32_t>(TextInputType::DATETIME))),
        DECLARE_NAPI_PROPERTY(
            "PATTERN_EMAIL", GetJsConstProperty(env, static_cast<uint32_t>(TextInputType::EMAIL_ADDRESS))),
        DECLARE_NAPI_PROPERTY("PATTERN_URI", GetJsConstProperty(env, static_cast<uint32_t>(TextInputType::URL))),
        DECLARE_NAPI_PROPERTY(
            "PATTERN_PASSWORD", GetJsConstProperty(env, static_cast<uint32_t>(TextInputType::VISIBLE_PASSWORD))),

        DECLARE_NAPI_FUNCTION("MoveCursor", MoveCursor),
        DECLARE_NAPI_FUNCTION("getInputMethodEngine", GetInputMethodEngine),
        DECLARE_NAPI_FUNCTION("getInputMethodAbility", GetInputMethodAbility),
    };
    NAPI_CALL(
        env, napi_define_properties(env, exports, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", Subscribe),
        DECLARE_NAPI_FUNCTION("off", UnSubscribe),
        DECLARE_NAPI_FUNCTION("createPanel", CreatePanel),
        DECLARE_NAPI_FUNCTION("destroyPanel", DestroyPanel),
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, IMES_CLASS_NAME.c_str(), IMES_CLASS_NAME.size(), JsConstructor, nullptr,
                       sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &IMESRef_));
    NAPI_CALL(env, napi_set_named_property(env, exports, IMES_CLASS_NAME.c_str(), cons));
    return exports;
};

napi_value JsInputMethodEngineSetting::GetJsConstProperty(napi_env env, uint32_t num)
{
    napi_value jsNumber = nullptr;
    napi_create_uint32(env, num, &jsNumber);
    return jsNumber;
}

napi_value JsInputMethodEngineSetting::GetIntJsConstProperty(napi_env env, int32_t num)
{
    napi_value jsNumber = nullptr;
    napi_create_int32(env, num, &jsNumber);
    return jsNumber;
}

std::shared_ptr<JsInputMethodEngineSetting> JsInputMethodEngineSetting::GetInputMethodEngineSetting()
{
    if (inputMethodEngine_ == nullptr) {
        std::lock_guard<std::mutex> lock(engineMutex_);
        if (inputMethodEngine_ == nullptr) {
            auto engine = std::make_shared<JsInputMethodEngineSetting>();
            if (engine == nullptr) {
                IMSA_HILOGE("input method engine nullptr");
                return nullptr;
            }
            inputMethodEngine_ = engine;
            InputMethodAbility::GetInstance()->SetImeListener(inputMethodEngine_);
        }
    }
    return inputMethodEngine_;
}

napi_value JsInputMethodEngineSetting::JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    IMSA_HILOGI("run in JsConstructor");
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));
    auto delegate = GetInputMethodEngineSetting();
    if (delegate == nullptr) {
        IMSA_HILOGE("get delegate nullptr");
        JsUtils::ThrowException(
            env, static_cast<int32_t>(IMFErrorCode::EXCEPTION_IMENGINE), "get delegate nullptr", TypeCode::TYPE_NONE);
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_status status = napi_wrap(
        env, thisVar, delegate.get(), [](napi_env env, void *nativeObject, void *hint) {}, nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("JsInputMethodEngineSetting napi_wrap failed: %{public}d", status);
        return nullptr;
    }
    if (delegate->loop_ == nullptr) {
        napi_get_uv_event_loop(env, &delegate->loop_);
    }
    return thisVar;
};

napi_value JsInputMethodEngineSetting::GetInputMethodAbility(napi_env env, napi_callback_info info)
{
    return GetIMEInstance(env, info, V9_FLAG);
}

napi_value JsInputMethodEngineSetting::GetInputMethodEngine(napi_env env, napi_callback_info info)
{
    return GetIMEInstance(env, info, ORIGINAL_FLAG);
}

napi_value JsInputMethodEngineSetting::GetIMEInstance(napi_env env, napi_callback_info info, int flag)
{
    napi_value instance = nullptr;
    napi_value cons = nullptr;

    if (flag == V9_FLAG) {
        size_t argc = ARGC_MAX;
        napi_value argv[ARGC_MAX] = { nullptr };

        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    }

    if (napi_get_reference_value(env, IMESRef_, &cons) != napi_ok) {
        return nullptr;
    }

    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        return nullptr;
    }

    return instance;
}

napi_value JsInputMethodEngineSetting::MoveCursor(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_MAX;
    napi_value argv[ARGC_MAX] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments, requires 1");

    int32_t number = 0;
    if (JsUtils::GetValue(env, argv[ARGC_ZERO], number) != napi_ok) {
        IMSA_HILOGE("Get number error");
    }

    InputMethodAbility::GetInstance()->MoveCursor(number);

    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

void JsInputMethodEngineSetting::RegisterListener(
    napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj)
{
    IMSA_HILOGI("RegisterListener %{public}s", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGE("methodName: %{public}s not registered!", type.c_str());
    }
    auto callbacks = jsCbMap_[type];
    bool ret = std::any_of(callbacks.begin(), callbacks.end(), [&callback](std::shared_ptr<JSCallbackObject> cb) {
        return JsUtils::Equals(cb->env_, callback, cb->callback_, cb->threadId_);
    });
    if (ret) {
        IMSA_HILOGE("JsInputMethodEngineListener::RegisterListener callback already registered!");
        return;
    }

    IMSA_HILOGI("Add %{public}s callbackObj into jsCbMap_", type.c_str());
    jsCbMap_[type].push_back(std::move(callbackObj));
}

void JsInputMethodEngineSetting::UnRegisterListener(napi_value callback, std::string type)
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
        if (JsUtils::Equals((*item)->env_, callback, (*item)->callback_, (*item)->threadId_)) {
            jsCbMap_[type].erase(item);
            break;
        }
    }

    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
}

napi_value JsInputMethodEngineSetting::Subscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    PARAM_CHECK_RETURN(env, argc >= ARGC_TWO, "Wrong number of arguments, requires 2", TYPE_NONE, nullptr);

    std::string type = "";
    napi_status status = JsUtils::GetValue(env, argv[ARGC_ZERO], type);
    NAPI_ASSERT_BASE(env, status == napi_ok, "get type failed!", nullptr);
    IMSA_HILOGE("event type is: %{public}s", type.c_str());

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[ARGC_ONE], &valueType);
    PARAM_CHECK_RETURN(env, valueType == napi_function, "'callback'", TYPE_FUNCTION, nullptr);

    auto engine = reinterpret_cast<JsInputMethodEngineSetting *>(JsUtils::GetNativeSelf(env, info));
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

napi_value JsInputMethodEngineSetting::CreatePanel(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<PanelContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc >= 2, "should has 2 or 3 parameters!", TYPE_NONE, napi_invalid_arg);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[ARGC_ZERO], &valueType);
        PARAM_CHECK_RETURN(env, valueType == napi_object, " ctx: ", TYPE_OBJECT, napi_invalid_arg);

        void *contextPtr = nullptr;
        NativeValue *value = reinterpret_cast<NativeValue *>(argv[ARGC_ZERO]);
        GetNativeContext(env, value, contextPtr);
        ctxt->contextPtr = contextPtr;

        napi_typeof(env, argv[ARGC_ONE], &valueType);
        PARAM_CHECK_RETURN(env, valueType == napi_object, " panelInfo: ", TYPE_OBJECT, napi_invalid_arg);
        napi_value napiValue = nullptr;
        napi_status status = napi_get_named_property(env, argv[ARGC_ONE], "type", &napiValue);
        PARAM_CHECK_RETURN(env, status == napi_ok, " missing info parameter.", TYPE_NONE, status);
        status = JsUtils::GetValue(env, napiValue, ctxt->panelType);
        NAPI_ASSERT_BASE(env, status == napi_ok, "Get panelType error!", status);

        status = napi_get_named_property(env, argv[ARGC_ONE], "flag", &napiValue);
        PARAM_CHECK_RETURN(env, status == napi_ok, " missing info parameter.", TYPE_NONE, status);
        status = JsUtils::GetValue(env, napiValue, ctxt->panelFlag);
        NAPI_ASSERT_BASE(env, status == napi_ok, "Get panelFlag error!", status);
        ctxt->ref = NewWithRef(env, 0, nullptr, reinterpret_cast<void **>(&ctxt->jsPanel), JsPanel::Constructor(env));
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        std::shared_ptr<InputMethodPanel> panel = nullptr;
        PanelInfo panelInfo = { .panelType = PanelType(ctxt->panelType), .panelFlag = PanelFlag(ctxt->panelFlag) };
        auto context = static_cast<std::weak_ptr<AbilityRuntime::Context> *>(ctxt->contextPtr);
        CHECK_RETURN_VOID(ctxt->jsPanel != nullptr, "napi_create_reference failed");
        auto ret = InputMethodAbility::GetInstance()->CreatePanel(context->lock(), panelInfo, panel);
        ctxt->SetErrorCode(ret);
        CHECK_RETURN_VOID(ret == ErrorCode::NO_ERROR, "JsInputMethodEngineSetting CreatePanel failed!");
        ctxt->jsPanel->SetNative(panel);
        ctxt->SetState(napi_ok);
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        NAPI_ASSERT_BASE(env, ctxt->ref != nullptr, "ctxt->ref == nullptr!", napi_generic_failure);
        auto status = napi_get_reference_value(env, ctxt->ref, result);
        NAPI_ASSERT_BASE(env, (status == napi_ok || result != nullptr), "Get ref error!", napi_generic_failure);
        napi_delete_reference(env, ctxt->ref);
        return napi_ok;
    };

    ctxt->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, ctxt, ARGC_TWO);
    return asyncCall.Call(env, exec);
}

void JsInputMethodEngineSetting::GetNativeContext(napi_env env, NativeValue *nativeContext, void *&contextPtr)
{
    if (nativeContext != nullptr) {
        auto objContext = AbilityRuntime::ConvertNativeValueTo<NativeObject>(nativeContext);
        if (objContext == nullptr) {
            IMSA_HILOGE("Get context object failed.");
            return;
        }
        contextPtr = objContext->GetNativePointer();
    }
}

napi_value JsInputMethodEngineSetting::DestroyPanel(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_MAX;
    napi_value argv[ARGC_MAX] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    PARAM_CHECK_RETURN(env, argc > 0, " should has 1 parameter! ", TYPE_NONE, nullptr);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    PARAM_CHECK_RETURN(env, valueType == napi_object, " target: ", TYPE_OBJECT, nullptr);
    bool isPanel = false;
    napi_status status = napi_instanceof(env, argv[0], JsPanel::Constructor(env), &isPanel);
    NAPI_ASSERT_BASE(env, status == napi_ok, "run napi_instanceof failed!", nullptr);
    JsPanel *panel = nullptr;
    status = napi_unwrap(env, argv[0], (void **)(&panel));
    NAPI_ASSERT_BASE(env, (status == napi_ok) && (panel != nullptr), "can not unwrap to JsPanel!", nullptr);

    AbilityRuntime::AsyncTask::CompleteCallback complete = [&panel](NativeEngine &engine,
        AbilityRuntime::AsyncTask &task, int32_t status) {
        auto ret = InputMethodAbility::GetInstance()->DestroyPanel(panel->GetNative());
        if(ret != ErrorCode::NO_ERROR) {
            task.Reject(engine, AbilityRuntime::CreateJsError(engine, 1, "Destroy panel failed."));
            return;
        }
        task.Resolve(engine, engine.CreateUndefined());
    };
    NativeCallbackInfo *callbackInfo = reinterpret_cast<NativeCallbackInfo *>(info);
    NativeValue* lastParam = (callbackInfo->argc < 2) ? nullptr :
                             (callbackInfo->argv[1]->TypeOf() == NATIVE_FUNCTION ? callbackInfo->argv[1] : nullptr);
    NativeValue *result = nullptr;
    NativeEngine *nativeEngine = reinterpret_cast<NativeEngine *>(env);
    AbilityRuntime::AsyncTask::Schedule("JsInputMethodEngineSetting::DestroyPanel", *nativeEngine,
        CreateAsyncTaskWithLastParam(*nativeEngine, lastParam, nullptr, std::move(complete), &result));
    return reinterpret_cast<napi_value>(result);
}

napi_ref JsInputMethodEngineSetting::NewWithRef(
    napi_env env, size_t argc, napi_value *argv, void **out, napi_value constructor)
{
    napi_value object = nullptr;
    napi_status status = napi_new_instance(env, constructor, argc, argv, &object);
    NAPI_ASSERT(env, (status == napi_ok) && (object != nullptr), "napi_new_instance failed!");

    status = napi_unwrap(env, object, out);
    NAPI_ASSERT(env, (status == napi_ok) && (out != nullptr), "napi_unwrap failed");

    napi_ref ref = nullptr;
    status = napi_create_reference(env, object, 1, &ref);
    NAPI_ASSERT(env, (status == napi_ok) && (ref != nullptr), "napi_create_reference failed");
    return ref;
}

napi_value JsInputMethodEngineSetting::UnSubscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    PARAM_CHECK_RETURN(env, argc >= 1, "Wrong number of arguments, requires 1 or 2", TYPE_NONE, nullptr);
    std::string type = "";
    JsUtils::GetValue(env, argv[ARGC_ZERO], type);
    IMSA_HILOGD("event type is: %{public}s", type.c_str());
    auto setting = reinterpret_cast<JsInputMethodEngineSetting *>(JsUtils::GetNativeSelf(env, info));
    if (setting == nullptr) {
        return nullptr;
    }

    if (argc == ARGC_TWO) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[ARGC_ONE], &valueType);
        PARAM_CHECK_RETURN(env, valueType == napi_function, " 'callback' ", TYPE_FUNCTION, nullptr);
    }
    setting->UnRegisterListener(argv[ARGC_ONE], type);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value JsInputMethodEngineSetting::GetResultOnSetSubtype(napi_env env, const SubProperty &property)
{
    napi_value subType = nullptr;
    napi_create_object(env, &subType);

    napi_value label = nullptr;
    napi_create_string_utf8(env, property.label.c_str(), property.name.size(), &label);
    napi_set_named_property(env, subType, "label", label);

    napi_value labelId = nullptr;
    napi_create_int32(env, property.labelId, &labelId);
    napi_set_named_property(env, subType, "labelId", labelId);

    napi_value name = nullptr;
    napi_create_string_utf8(env, property.name.c_str(), property.name.size(), &name);
    napi_set_named_property(env, subType, "name", name);

    napi_value id = nullptr;
    napi_create_string_utf8(env, property.id.c_str(), property.id.size(), &id);
    napi_set_named_property(env, subType, "id", id);

    napi_value mode = nullptr;
    napi_create_string_utf8(env, property.mode.c_str(), property.mode.size(), &mode);
    napi_set_named_property(env, subType, "mode", mode);

    napi_value locale = nullptr;
    napi_create_string_utf8(env, property.locale.c_str(), property.locale.size(), &locale);
    napi_set_named_property(env, subType, "locale", locale);

    napi_value language = nullptr;
    napi_create_string_utf8(env, property.language.c_str(), property.language.size(), &language);
    napi_set_named_property(env, subType, "language", language);

    napi_value icon = nullptr;
    napi_create_string_utf8(env, property.icon.c_str(), property.icon.size(), &icon);
    napi_set_named_property(env, subType, "icon", icon);

    napi_value iconId = nullptr;
    napi_create_int32(env, property.iconId, &iconId);
    napi_set_named_property(env, subType, "iconId", iconId);

    napi_value extra = nullptr;
    napi_create_object(env, &extra);
    napi_set_named_property(env, subType, "extra", extra);

    return subType;
}

void JsInputMethodEngineSetting::OnInputStart()
{
    IMSA_HILOGD("run in");
    std::string type = "inputStart";
    uv_work_t *work = GetUVwork(type);
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

            auto getInputStartProperty = [](napi_value *args, uint8_t argc,
                                             std::shared_ptr<JSCallbackObject> item) -> bool {
                if (argc < 2) {
                    return false;
                }
                napi_value textInput = JsTextInputClientEngine::GetTextInputClientInstance(item->env_);
                napi_value keyBoardController = JsKeyboardControllerEngine::GetKeyboardControllerInstance(item->env_);
                if (keyBoardController == nullptr || textInput == nullptr) {
                    IMSA_HILOGE("get KBCins or TICins failed:");
                    return false;
                }
                args[ARGC_ZERO] = keyBoardController;
                args[ARGC_ONE] = textInput;
                return true;
            };
            JsUtils::TraverseCallback(entry->vecCopy, ARGC_TWO, getInputStartProperty);
        });
}

void JsInputMethodEngineSetting::OnKeyboardStatus(bool isShow)
{
    std::string type = isShow ? "keyboardShow" : "keyboardHide";
    IMSA_HILOGD("run in, %{public}s", type.c_str());
    uv_work_t *work = GetUVwork(type);
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

            auto getKeyboardStatusProperty = [](napi_value *args, uint8_t argc,
                                                 std::shared_ptr<JSCallbackObject> item) -> bool {
                if (argc == 0) {
                    return false;
                }
                args[ARGC_ZERO] = nullptr;
                return true;
            };
            JsUtils::TraverseCallback(entry->vecCopy, ARGC_ZERO, getKeyboardStatusProperty);
        });
}

void JsInputMethodEngineSetting::OnInputStop(const std::string &imeId)
{
    IMSA_HILOGD("run in");
    std::string type = "inputStop";
    uv_work_t *work = GetUVwork(type, [&imeId](UvEntry &entry) { entry.imeid = imeId; });
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
                IMSA_HILOGE("OnInputStop:: entryptr is null");
                return;
            }

            auto getInputStopProperty = [entry](napi_value *args, uint8_t argc,
                                            std::shared_ptr<JSCallbackObject> item) -> bool {
                if (argc == 0) {
                    return false;
                }
                napi_create_string_utf8(item->env_, entry->imeid.c_str(), NAPI_AUTO_LENGTH, &args[ARGC_ZERO]);
                return true;
            };
            JsUtils::TraverseCallback(entry->vecCopy, ARGC_ONE, getInputStopProperty);
        });
}

void JsInputMethodEngineSetting::OnSetCallingWindow(uint32_t windowId)
{
    IMSA_HILOGD("run in");
    std::string type = "setCallingWindow";
    uv_work_t *work = GetUVwork(type, [windowId](UvEntry &entry) { entry.windowid = windowId; });
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
                IMSA_HILOGE("setCallingWindow:: entryptr is null");
                return;
            }

            auto getCallingWindowProperty = [entry](napi_value *args, uint8_t argc,
                                                std::shared_ptr<JSCallbackObject> item) -> bool {
                if (argc == 0) {
                    return false;
                }
                napi_create_int32(item->env_, entry->windowid, &args[ARGC_ZERO]);
                return true;
            };
            JsUtils::TraverseCallback(entry->vecCopy, ARGC_ONE, getCallingWindowProperty);
        });
}

void JsInputMethodEngineSetting::OnSetSubtype(const SubProperty &property)
{
    IMSA_HILOGD("run in");
    std::string type = "setSubtype";
    uv_work_t *work = GetUVwork(type, [&property](UvEntry &entry) { entry.subProperty = property; });
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
                IMSA_HILOGE("OnSetSubtype:: entryptr is null");
                return;
            }

            auto getSubtypeProperty = [entry](napi_value *args, uint8_t argc,
                                          std::shared_ptr<JSCallbackObject> item) -> bool {
                if (argc == 0) {
                    return false;
                }
                napi_value jsObject = GetResultOnSetSubtype(item->env_, entry->subProperty);
                if (jsObject == nullptr) {
                    IMSA_HILOGE("get GetResultOnSetSubtype failed: jsObject is nullptr");
                    return false;
                }
                args[ARGC_ZERO] = { jsObject };
                return true;
            };
            JsUtils::TraverseCallback(entry->vecCopy, ARGC_ONE, getSubtypeProperty);
        });
}

uv_work_t *JsInputMethodEngineSetting::GetUVwork(const std::string &type, EntrySetter entrySetter)
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
