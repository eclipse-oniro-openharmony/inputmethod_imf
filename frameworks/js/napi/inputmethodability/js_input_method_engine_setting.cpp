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

#include "event_checker.h"
#include "input_method_ability.h"
#include "input_method_property.h"
#include "input_method_utils.h"
#include "js_callback_handler.h"
#include "js_keyboard_controller_engine.h"
#include "js_runtime_utils.h"
#include "js_text_input_client_engine.h"
#include "js_util.h"
#include "js_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_base_context.h"

namespace OHOS {
namespace MiscServices {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_MAX = 6;
const std::string JsInputMethodEngineSetting::IMES_CLASS_NAME = "InputMethodEngine";
thread_local napi_ref JsInputMethodEngineSetting::IMESRef_ = nullptr;

std::mutex JsInputMethodEngineSetting::engineMutex_;
std::shared_ptr<JsInputMethodEngineSetting> JsInputMethodEngineSetting::inputMethodEngine_{ nullptr };
std::mutex JsInputMethodEngineSetting::eventHandlerMutex_;
std::shared_ptr<AppExecFwk::EventHandler> JsInputMethodEngineSetting::handler_{ nullptr };

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
        DECLARE_NAPI_PROPERTY(
            "ENTER_KEY_TYPE_NEWLINE", GetJsConstProperty(env, static_cast<uint32_t>(EnterKeyType::NEW_LINE))),
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
        DECLARE_NAPI_PROPERTY(
            "PATTERN_PASSWORD_NUMBER", GetJsConstProperty(env, static_cast<uint32_t>(TextInputType::NUMBER_PASSWORD))),
        DECLARE_NAPI_PROPERTY("PATTERN_PASSWORD_SCREEN_LOCK",
            GetJsConstProperty(env, static_cast<uint32_t>(TextInputType::SCREEN_LOCK_PASSWORD))),
        DECLARE_NAPI_FUNCTION("getInputMethodEngine", GetInputMethodEngine),
        DECLARE_NAPI_FUNCTION("getInputMethodAbility", GetInputMethodAbility),
        DECLARE_NAPI_STATIC_PROPERTY("PanelType", GetJsPanelTypeProperty(env)),
        DECLARE_NAPI_STATIC_PROPERTY("PanelFlag", GetJsPanelFlagProperty(env)),
        DECLARE_NAPI_STATIC_PROPERTY("Direction", GetJsDirectionProperty(env)),
        DECLARE_NAPI_STATIC_PROPERTY("ExtendAction", GetJsExtendActionProperty(env)),
        DECLARE_NAPI_STATIC_PROPERTY("SecurityMode", GetJsSecurityModeProperty(env)),
        DECLARE_NAPI_STATIC_PROPERTY("ImmersiveMode", GetJsImmersiveModeProperty(env)),
    };
    NAPI_CALL(
        env, napi_define_properties(env, exports, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));
    return InitProperty(env, exports);
};

napi_value JsInputMethodEngineSetting::InitProperty(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", Subscribe),
        DECLARE_NAPI_FUNCTION("off", UnSubscribe),
        DECLARE_NAPI_FUNCTION("createPanel", CreatePanel),
        DECLARE_NAPI_FUNCTION("destroyPanel", DestroyPanel),
        DECLARE_NAPI_FUNCTION("getSecurityMode", GetSecurityMode),
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, IMES_CLASS_NAME.c_str(), IMES_CLASS_NAME.size(), JsConstructor, nullptr,
                       sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &IMESRef_));
    NAPI_CALL(env, napi_set_named_property(env, exports, IMES_CLASS_NAME.c_str(), cons));
    return exports;
}

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

napi_value JsInputMethodEngineSetting::GetJsPanelTypeProperty(napi_env env)
{
    napi_value panelType = nullptr;
    napi_value typeSoftKeyboard = nullptr;
    napi_value typeStatusBar = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(PanelType::SOFT_KEYBOARD), &typeSoftKeyboard));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(PanelType::STATUS_BAR), &typeStatusBar));
    NAPI_CALL(env, napi_create_object(env, &panelType));
    NAPI_CALL(env, napi_set_named_property(env, panelType, "SOFT_KEYBOARD", typeSoftKeyboard));
    NAPI_CALL(env, napi_set_named_property(env, panelType, "STATUS_BAR", typeStatusBar));
    return panelType;
}

napi_value JsInputMethodEngineSetting::GetJsPanelFlagProperty(napi_env env)
{
    napi_value panelFlag = nullptr;
    napi_value flagFixed = nullptr;
    napi_value flagFloating = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(PanelFlag::FLG_FIXED), &flagFixed));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(PanelFlag::FLG_FLOATING), &flagFloating));
    NAPI_CALL(env, napi_create_object(env, &panelFlag));
    NAPI_CALL(env, napi_set_named_property(env, panelFlag, "FLG_FIXED", flagFixed));
    NAPI_CALL(env, napi_set_named_property(env, panelFlag, "FLG_FLOATING", flagFloating));
    return panelFlag;
}

napi_value JsInputMethodEngineSetting::GetJsDirectionProperty(napi_env env)
{
    napi_value direction = nullptr;
    napi_value cursorUp = nullptr;
    napi_value cursorDown = nullptr;
    napi_value cursorLeft = nullptr;
    napi_value cursorRight = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(Direction::UP), &cursorUp));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(Direction::DOWN), &cursorDown));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(Direction::LEFT), &cursorLeft));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(Direction::RIGHT), &cursorRight));
    NAPI_CALL(env, napi_create_object(env, &direction));
    NAPI_CALL(env, napi_set_named_property(env, direction, "CURSOR_UP", cursorUp));
    NAPI_CALL(env, napi_set_named_property(env, direction, "CURSOR_DOWN", cursorDown));
    NAPI_CALL(env, napi_set_named_property(env, direction, "CURSOR_LEFT", cursorLeft));
    NAPI_CALL(env, napi_set_named_property(env, direction, "CURSOR_RIGHT", cursorRight));
    return direction;
}

napi_value JsInputMethodEngineSetting::GetJsExtendActionProperty(napi_env env)
{
    napi_value action = nullptr;
    napi_value actionSelectAll = nullptr;
    napi_value actionCut = nullptr;
    napi_value actionCopy = nullptr;
    napi_value actionPaste = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(ExtendAction::SELECT_ALL), &actionSelectAll));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(ExtendAction::CUT), &actionCut));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(ExtendAction::COPY), &actionCopy));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(ExtendAction::PASTE), &actionPaste));
    NAPI_CALL(env, napi_create_object(env, &action));
    NAPI_CALL(env, napi_set_named_property(env, action, "SELECT_ALL", actionSelectAll));
    NAPI_CALL(env, napi_set_named_property(env, action, "CUT", actionCut));
    NAPI_CALL(env, napi_set_named_property(env, action, "COPY", actionCopy));
    NAPI_CALL(env, napi_set_named_property(env, action, "PASTE", actionPaste));
    return action;
}

napi_value JsInputMethodEngineSetting::GetJsSecurityModeProperty(napi_env env)
{
    napi_value securityMode = nullptr;
    napi_value basic = nullptr;
    napi_value full = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(SecurityMode::BASIC), &basic));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(SecurityMode::FULL), &full));
    NAPI_CALL(env, napi_create_object(env, &securityMode));
    NAPI_CALL(env, napi_set_named_property(env, securityMode, "BASIC", basic));
    NAPI_CALL(env, napi_set_named_property(env, securityMode, "FULL", full));
    return securityMode;
}

napi_value JsInputMethodEngineSetting::GetJsImmersiveModeProperty(napi_env env)
{
    napi_value immersive = nullptr;
    NAPI_CALL(env, napi_create_object(env, &immersive));
    bool ret = JsUtil::Object::WriteProperty(
        env, immersive, "NONE_IMMERSIVE", static_cast<int32_t>(ImmersiveMode::NONE_IMMERSIVE));
    ret = ret &&
        JsUtil::Object::WriteProperty(env, immersive, "IMMERSIVE", static_cast<int32_t>(ImmersiveMode::IMMERSIVE));
    ret = ret &&
        JsUtil::Object::WriteProperty(
            env, immersive, "LIGHT_IMMERSIVE", static_cast<int32_t>(ImmersiveMode::LIGHT_IMMERSIVE));
    ret = ret &&
        JsUtil::Object::WriteProperty(
            env, immersive, "DARK_IMMERSIVE", static_cast<int32_t>(ImmersiveMode::DARK_IMMERSIVE));
    return ret ? immersive : JsUtil::Const::Null(env);
}

std::shared_ptr<JsInputMethodEngineSetting> JsInputMethodEngineSetting::GetInputMethodEngineSetting()
{
    if (inputMethodEngine_ == nullptr) {
        std::lock_guard<std::mutex> lock(engineMutex_);
        if (inputMethodEngine_ == nullptr) {
            auto engine = std::make_shared<JsInputMethodEngineSetting>();
            if (engine == nullptr) {
                IMSA_HILOGE("create engine failed.");
                return nullptr;
            }
            inputMethodEngine_ = engine;
        }
    }
    return inputMethodEngine_;
}

bool JsInputMethodEngineSetting::InitInputMethodSetting()
{
    if (!InputMethodAbility::GetInstance()->IsCurrentIme()) {
        return false;
    }
    auto engine = GetInputMethodEngineSetting();
    if (engine == nullptr) {
        return false;
    }
    InputMethodAbility::GetInstance()->SetImeListener(engine);
    {
        std::lock_guard<std::mutex> lock(eventHandlerMutex_);
        handler_ = AppExecFwk::EventHandler::Current();
    }
    return true;
}

napi_value JsInputMethodEngineSetting::JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));
    auto setting = GetInputMethodEngineSetting();
    if (setting == nullptr || !InitInputMethodSetting()) {
        IMSA_HILOGE("failed to get setting.");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_status status = napi_wrap(
        env, thisVar, setting.get(), [](napi_env env, void *nativeObject, void *hint) {}, nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("JsInputMethodEngineSetting napi_wrap failed: %{public}d", status);
        return nullptr;
    }
    if (setting->loop_ == nullptr) {
        napi_get_uv_event_loop(env, &setting->loop_);
    }
    return thisVar;
};

napi_value JsInputMethodEngineSetting::GetInputMethodAbility(napi_env env, napi_callback_info info)
{
    return GetIMEInstance(env, info);
}

napi_value JsInputMethodEngineSetting::GetInputMethodEngine(napi_env env, napi_callback_info info)
{
    return GetIMEInstance(env, info);
}

napi_value JsInputMethodEngineSetting::GetIMEInstance(napi_env env, napi_callback_info info)
{
    napi_value instance = nullptr;
    napi_value cons = nullptr;
    if (napi_get_reference_value(env, IMESRef_, &cons) != napi_ok) {
        IMSA_HILOGE("failed to get reference value.");
        return nullptr;
    }
    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("failed to new instance.");
        return nullptr;
    }
    return instance;
}

void JsInputMethodEngineSetting::RegisterListener(
    napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj)
{
    IMSA_HILOGD("register listener: %{public}s.", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGD("methodName: %{public}s not registered!", type.c_str());
    }
    auto callbacks = jsCbMap_[type];
    bool ret = std::any_of(callbacks.begin(), callbacks.end(), [&callback](std::shared_ptr<JSCallbackObject> cb) {
        return JsUtils::Equals(cb->env_, callback, cb->callback_, cb->threadId_);
    });
    if (ret) {
        IMSA_HILOGD("JsInputMethodEngineListener callback already registered!");
        return;
    }

    IMSA_HILOGI("add %{public}s callbackObj into jsCbMap_.", type.c_str());
    jsCbMap_[type].push_back(std::move(callbackObj));
}

void JsInputMethodEngineSetting::UnRegisterListener(napi_value callback, std::string type)
{
    IMSA_HILOGI("unregister listener: %{public}s.", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGE("methodName: %{public}s already unregistered!", type.c_str());
        return;
    }

    if (callback == nullptr) {
        jsCbMap_.erase(type);
        IMSA_HILOGE("callback is nullptr.");
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
    size_t argc = ARGC_MAX;
    napi_value argv[ARGC_MAX] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    std::string type;
    // 2 means least param num.
    if (argc < 2 || !JsUtil::GetValue(env, argv[0], type) ||
        !EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_ABILITY, type) ||
        JsUtil::GetType(env, argv[1]) != napi_function) {
        IMSA_HILOGE("subscribe failed, type: %{public}s.", type.c_str());
        return nullptr;
    }
    if (type == "privateCommand" && !InputMethodAbility::GetInstance()->IsDefaultIme()) {
        JsUtils::ThrowException(
            env, JsUtils::Convert(ErrorCode::ERROR_NOT_DEFAULT_IME), "default ime check failed", TYPE_NONE);
    }
    IMSA_HILOGD("subscribe type:%{public}s.", type.c_str());
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

napi_status JsInputMethodEngineSetting::GetContext(
    napi_env env, napi_value in, std::shared_ptr<OHOS::AbilityRuntime::Context> &context)
{
    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, in, stageMode);
    if (status != napi_ok || (!stageMode)) {
        IMSA_HILOGE("it's not in stage mode.");
        return status;
    }
    context = OHOS::AbilityRuntime::GetStageModeContext(env, in);
    if (context == nullptr) {
        IMSA_HILOGE("context is nullptr.");
        return napi_generic_failure;
    }
    return napi_ok;
}

napi_value JsInputMethodEngineSetting::CreatePanel(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<PanelContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc >= 2, "at least two parameters is required.", TYPE_NONE, napi_invalid_arg);
        napi_valuetype valueType = napi_undefined;
        // 0 means parameter of ctx<BaseContext>
        napi_typeof(env, argv[0], &valueType);
        PARAM_CHECK_RETURN(env, valueType == napi_object, "ctx type must be BaseContext.", TYPE_NONE, napi_invalid_arg);
        napi_status status = GetContext(env, argv[0], ctxt->context);
        if (status != napi_ok) {
            return status;
        }
        // 1 means parameter of info<PanelInfo>
        napi_typeof(env, argv[1], &valueType);
        PARAM_CHECK_RETURN(env, valueType == napi_object, "param info type must be PanelInfo.", TYPE_NONE,
            napi_invalid_arg);
        status = JsUtils::GetValue(env, argv[1], ctxt->panelInfo);
        PARAM_CHECK_RETURN(env, status == napi_ok, "js param info covert failed", TYPE_NONE, napi_invalid_arg);
        return status;
    };

    auto exec = [ctxt](AsyncCall::Context *ctx) {
        auto ret = InputMethodAbility::GetInstance()->CreatePanel(ctxt->context, ctxt->panelInfo, ctxt->panel);
        ctxt->SetErrorCode(ret);
        CHECK_RETURN_VOID(ret == ErrorCode::NO_ERROR, "JsInputMethodEngineSetting CreatePanel failed!");
        ctxt->SetState(napi_ok);
    };

    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        JsPanel *jsPanel = nullptr;
        napi_value constructor = JsPanel::Init(env);
        CHECK_RETURN(constructor != nullptr, "failed to get panel constructor!", napi_generic_failure);

        napi_status status = napi_new_instance(env, constructor, 0, nullptr, result);
        CHECK_RETURN(status == napi_ok, "jsPanel new instance failed!", napi_generic_failure);

        status = napi_unwrap(env, *result, (void **)(&jsPanel));
        CHECK_RETURN((status == napi_ok) && (jsPanel != nullptr), "get jsPanel unwrap failed!", napi_generic_failure);
        jsPanel->SetNative(ctxt->panel);
        return napi_ok;
    };

    ctxt->SetAction(std::move(input), std::move(output));
    // 3 means JsAPI:createPanel has 3 params at most.
    AsyncCall asyncCall(env, info, ctxt, 3);
    return asyncCall.Call(env, exec, "createPanel");
}

napi_value JsInputMethodEngineSetting::DestroyPanel(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<PanelContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc >= 1, "at least one parameter is required!", TYPE_NONE, napi_invalid_arg);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        PARAM_CHECK_RETURN(env, valueType == napi_object, "param panel type must be InputMethodPanel.", TYPE_NONE,
            napi_invalid_arg);
        bool isPanel = false;
        napi_value constructor = JsPanel::Init(env);
        CHECK_RETURN(constructor != nullptr, "failed to get panel constructor.", napi_invalid_arg);
        napi_status status = napi_instanceof(env, argv[0], constructor, &isPanel);
        CHECK_RETURN((status == napi_ok) && isPanel, "param verification failed, it's not expected panel instance!",
            status);
        JsPanel *jsPanel = nullptr;
        status = napi_unwrap(env, argv[0], (void **)(&jsPanel));
        CHECK_RETURN((status == napi_ok) && (jsPanel != nullptr), "failed to unwrap JsPanel!", status);
        ctxt->panel = jsPanel->GetNative();
        CHECK_RETURN((ctxt->panel != nullptr), "panel is nullptr!", napi_invalid_arg);
        return status;
    };

    auto exec = [ctxt](AsyncCall::Context *ctx) { ctxt->SetState(napi_ok); };

    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        CHECK_RETURN((ctxt->panel != nullptr), "panel is nullptr!", napi_generic_failure);
        auto errCode = InputMethodAbility::GetInstance()->DestroyPanel(ctxt->panel);
        if (errCode != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("DestroyPanel failed, errCode: %{public}d!", errCode);
            return napi_generic_failure;
        }
        ctxt->panel = nullptr;
        return napi_ok;
    };

    ctxt->SetAction(std::move(input), std::move(output));
    // 2 means JsAPI:destroyPanel has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "destroyPanel");
}

napi_value JsInputMethodEngineSetting::GetSecurityMode(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("start get security mode.");
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t security;
    int32_t ret = InputMethodAbility::GetInstance()->GetSecurityMode(security);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to get security mode", TYPE_NONE);
    }
    napi_value result = nullptr;
    napi_create_int32(env, security, &result);
    return result;
}

napi_value JsInputMethodEngineSetting::UnSubscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    std::string type;
    // 1 means least param num.
    if (argc < 1 || !JsUtil::GetValue(env, argv[0], type) ||
        !EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_ABILITY, type)) {
        IMSA_HILOGE("unsubscribe failed, type: %{public}s!", type.c_str());
        return nullptr;
    }
    if (type == "privateCommand" && !InputMethodAbility::GetInstance()->IsDefaultIme()) {
        JsUtils::ThrowException(
            env, JsUtils::Convert(ErrorCode::ERROR_NOT_DEFAULT_IME), "default ime check failed", TYPE_NONE);
    }
    // if the second param is not napi_function/napi_null/napi_undefined, return
    auto paramType = JsUtil::GetType(env, argv[1]);
    if (paramType != napi_function && paramType != napi_null && paramType != napi_undefined) {
        return nullptr;
    }
    // if the second param is napi_function, delete it, else delete all
    argv[1] = paramType == napi_function ? argv[1] : nullptr;

    IMSA_HILOGD("unsubscribe type: %{public}s.", type.c_str());
    auto setting = reinterpret_cast<JsInputMethodEngineSetting *>(JsUtils::GetNativeSelf(env, info));
    if (setting == nullptr) {
        return nullptr;
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
    napi_create_uint32(env, property.labelId, &labelId);
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
    napi_create_uint32(env, property.iconId, &iconId);
    napi_set_named_property(env, subType, "iconId", iconId);

    napi_value extra = nullptr;
    napi_create_object(env, &extra);
    napi_set_named_property(env, subType, "extra", extra);

    return subType;
}

void JsInputMethodEngineSetting::OnInputStart()
{
    IMSA_HILOGD("start JsInputMethodEngineSetting.");
    std::string type = "inputStart";
    auto entry = GetEntry(type);
    if (entry == nullptr) {
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    auto task = [entry]() {
        auto paramGetter = [](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc < 2) {
                return false;
            }
            napi_value textInput = JsTextInputClientEngine::GetTextInputClientInstance(env);
            napi_value keyBoardController = JsKeyboardControllerEngine::GetKeyboardControllerInstance(env);
            if (keyBoardController == nullptr || textInput == nullptr) {
                IMSA_HILOGE("get KBCins or TICins failed!");
                return false;
            }
            // 0 means the first param of callback.
            args[0] = keyBoardController;
            // 1 means the second param of callback.
            args[1] = textInput;
            return true;
        };
        // 2 means callback has 2 params.
        JsCallbackHandler::Traverse(entry->vecCopy, { 2, paramGetter });
    };
    handler_->PostTask(task, type);
}

void JsInputMethodEngineSetting::OnKeyboardStatus(bool isShow)
{
    std::string type = isShow ? "keyboardShow" : "keyboardHide";
    auto entry = GetEntry(type);
    if (entry == nullptr) {
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }

    auto task = [entry]() { JsCallbackHandler::Traverse(entry->vecCopy); };
    handler_->PostTask(task, type);
}

int32_t JsInputMethodEngineSetting::OnInputStop()
{
    std::string type = "inputStop";
    auto entry = GetEntry(type);
    if (entry == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto task = [entry]() { JsCallbackHandler::Traverse(entry->vecCopy); };
    return handler_->PostTask(task, type) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_IME;
}

void JsInputMethodEngineSetting::OnSetCallingWindow(uint32_t windowId)
{
    std::string type = "setCallingWindow";
    auto entry = GetEntry(type, [&windowId](UvEntry &entry) { entry.windowid = windowId; });
    if (entry == nullptr) {
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGD("windowId: %{public}d.", windowId);
    auto task = [entry]() {
        auto paramGetter = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc == 0) {
                return false;
            }
            // 0 means the first param of callback.
            napi_create_uint32(env, entry->windowid, &args[0]);
            return true;
        };
        // 1 means callback has one param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, paramGetter });
    };
    handler_->PostTask(task, type);
}

void JsInputMethodEngineSetting::OnSetSubtype(const SubProperty &property)
{
    std::string type = "setSubtype";
    auto entry = GetEntry(type, [&property](UvEntry &entry) { entry.subProperty = property; });
    if (entry == nullptr) {
        IMSA_HILOGD("failed to get uv entry.");
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGI("subtypeId: %{public}s.", property.id.c_str());
    auto task = [entry]() {
        auto getSubtypeProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc == 0) {
                return false;
            }
            napi_value jsObject = GetResultOnSetSubtype(env, entry->subProperty);
            if (jsObject == nullptr) {
                IMSA_HILOGE("jsObject is nullptr!");
                return false;
            }
            // 0 means the first param of callback.
            args[0] = { jsObject };
            return true;
        };
        // 1 means callback has one param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, getSubtypeProperty });
    };
    eventHandler->PostTask(task, type);
}

void JsInputMethodEngineSetting::OnSecurityChange(int32_t security)
{
    std::string type = "securityModeChange";
    uv_work_t *work = GetUVwork(type, [&security](UvEntry &entry) { entry.security = security; });
    if (work == nullptr) {
        IMSA_HILOGD("failed to get uv entry.");
        return;
    }
    IMSA_HILOGI("run in: %{public}s", type.c_str());
    auto ret = uv_queue_work_with_qos(
        loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<UvEntry> entry(static_cast<UvEntry *>(work->data), [work](UvEntry *data) {
                delete data;
                delete work;
            });
            if (entry == nullptr) {
                IMSA_HILOGE("entry is nullptr!");
                return;
            }
            auto getSecurityProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
                if (argc == 0) {
                    return false;
                }
                // 0 means the first param of callback.
                napi_create_int32(env, entry->security, &args[0]);
                return true;
            };
            // 1 means callback has one param.
            JsCallbackHandler::Traverse(entry->vecCopy, { 1, getSecurityProperty });
        },
        uv_qos_user_initiated);
    FreeWorkIfFail(ret, work);
}

void JsInputMethodEngineSetting::ReceivePrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    IMSA_HILOGD("start.");
    std::string type = "privateCommand";
    auto entry = GetEntry(type, [&privateCommand](UvEntry &entry) { entry.privateCommand = privateCommand; });
    if (entry == nullptr) {
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    auto task = [entry]() {
        auto paramGetter = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc < 1) {
                return false;
            }
            napi_value jsObject = JsUtils::GetJsPrivateCommand(env, entry->privateCommand);
            if (jsObject == nullptr) {
                IMSA_HILOGE("jsObject is nullptr!");
                return false;
            }
            // 0 means the first param of callback.
            args[0] = { jsObject };
            return true;
        };
        // 1 means callback has 1 params.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, paramGetter });
    };
    eventHandler->PostTask(task, type);
}

uv_work_t *JsInputMethodEngineSetting::GetUVwork(const std::string &type, EntrySetter entrySetter)
{
    IMSA_HILOGD("run in, type: %{public}s.", type.c_str());
    UvEntry *entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        if (jsCbMap_[type].empty()) {
            IMSA_HILOGD("%{public}s cb-vector is empty.", type.c_str());
            return nullptr;
        }
        entry = new (std::nothrow) UvEntry(jsCbMap_[type], type);
        if (entry == nullptr) {
            IMSA_HILOGE("entry is nullptr!");
            return nullptr;
        }
        if (entrySetter != nullptr) {
            entrySetter(*entry);
        }
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
            IMSA_HILOGE("work is nullptr!");
        delete entry;
        return nullptr;
    }
    work->data = entry;
    return work;
}

std::shared_ptr<AppExecFwk::EventHandler> JsInputMethodEngineSetting::GetEventHandler()
{
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    return handler_;
}

std::shared_ptr<JsInputMethodEngineSetting::UvEntry> JsInputMethodEngineSetting::GetEntry(
    const std::string &type, EntrySetter entrySetter)
{
    IMSA_HILOGD("type: %{public}s.", type.c_str());
    std::shared_ptr<UvEntry> entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (jsCbMap_[type].empty()) {
            IMSA_HILOGD("%{public}s cb-vector is empty", type.c_str());
            return nullptr;
        }
        entry = std::make_shared<UvEntry>(jsCbMap_[type], type);
    }
    if (entrySetter != nullptr) {
        entrySetter(*entry);
    }
    return entry;
}

void JsInputMethodEngineSetting::FreeWorkIfFail(int ret, uv_work_t *work)
{
    if (ret == 0 || work == nullptr) {
        return;
    }

    UvEntry *data = static_cast<UvEntry *>(work->data);
    delete data;
    delete work;
    IMSA_HILOGE("uv_queue_work failed retCode: %{public}d!", ret);
}

bool JsInputMethodEngineSetting::PostTaskToEventHandler(std::function<void()> task, const std::string &taskName)
{
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return false;
    }
    if (eventHandler == AppExecFwk::EventHandler::Current()) {
        IMSA_HILOGE("in current thread!");
        return false;
    }
    handler_->PostTask(task, taskName);
    return true;
}
} // namespace MiscServices
} // namespace OHOS