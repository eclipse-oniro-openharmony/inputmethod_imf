/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "js_get_input_method_controller.h"

#include <set>

#include "event_checker.h"
#include "inputmethod_trace.h"
#include "input_method_controller.h"
#include "input_method_utils.h"
#include "js_callback_handler.h"
#include "js_get_input_method_textchange_listener.h"
#include "js_util.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "string_ex.h"
#include "string_utils.h"

namespace OHOS {
namespace MiscServices {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr int32_t MAX_WAIT_TIME_MESSAGE_HANDLER = 2000;
constexpr int32_t MAX_WAIT_TIME_ATTACH = 2000;
const std::set<std::string> EVENT_TYPE{
    "selectByRange",
    "selectByMovement",
};
const std::set<std::string> JsGetInputMethodController::TEXT_EVENT_TYPE{
    "insertText",
    "deleteLeft",
    "deleteRight",
    "sendKeyboardStatus",
    "sendFunctionKey",
    "moveCursor",
    "handleExtendAction",
    "getLeftTextOfCursor",
    "getRightTextOfCursor",
    "getTextIndexAtCursor",
};
thread_local napi_ref JsGetInputMethodController::IMCRef_ = nullptr;
const std::string JsGetInputMethodController::IMC_CLASS_NAME = "InputMethodController";
std::mutex JsGetInputMethodController::controllerMutex_;
std::shared_ptr<JsGetInputMethodController> JsGetInputMethodController::controller_{ nullptr };
std::mutex JsGetInputMethodController::eventHandlerMutex_;
std::shared_ptr<AppExecFwk::EventHandler> JsGetInputMethodController::handler_{ nullptr };
BlockQueue<MessageHandlerInfo> JsGetInputMethodController::messageHandlerQueue_{ MAX_WAIT_TIME_MESSAGE_HANDLER };
BlockQueue<AttachInfo> JsGetInputMethodController::attachQueue_{ MAX_WAIT_TIME_ATTACH };
napi_value JsGetInputMethodController::Init(napi_env env, napi_value info)
{
    napi_property_descriptor descriptor[] = {
        DECLARE_NAPI_FUNCTION("getInputMethodController", GetInputMethodController),
        DECLARE_NAPI_FUNCTION("getController", GetController),
        DECLARE_NAPI_STATIC_PROPERTY("KeyboardStatus", GetJsKeyboardStatusProperty(env)),
        DECLARE_NAPI_STATIC_PROPERTY("EnterKeyType", GetJsEnterKeyTypeProperty(env)),
        DECLARE_NAPI_STATIC_PROPERTY("TextInputType", GetJsTextInputTypeProperty(env)),
        DECLARE_NAPI_STATIC_PROPERTY("Direction", GetJsDirectionProperty(env)),
        DECLARE_NAPI_STATIC_PROPERTY("ExtendAction", GetJsExtendActionProperty(env)),
        DECLARE_NAPI_STATIC_PROPERTY("EnabledState", GetJsEnabledStateProperty(env)),
        DECLARE_NAPI_STATIC_PROPERTY("RequestKeyboardReason", GetJsRequestKeyboardReasonProperty(env)),
        DECLARE_NAPI_STATIC_PROPERTY("CapitalizeMode", GetJsCapitalizeModeProperty(env))
    };
    NAPI_CALL(env,
        napi_define_properties(env, info, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("attach", Attach),
        DECLARE_NAPI_FUNCTION("detach", Detach),
        DECLARE_NAPI_FUNCTION("showTextInput", ShowTextInput),
        DECLARE_NAPI_FUNCTION("hideTextInput", HideTextInput),
        DECLARE_NAPI_FUNCTION("setCallingWindow", SetCallingWindow),
        DECLARE_NAPI_FUNCTION("updateCursor", UpdateCursor),
        DECLARE_NAPI_FUNCTION("changeSelection", ChangeSelection),
        DECLARE_NAPI_FUNCTION("updateAttribute", UpdateAttribute),
        DECLARE_NAPI_FUNCTION("stopInput", StopInput),
        DECLARE_NAPI_FUNCTION("stopInputSession", StopInputSession),
        DECLARE_NAPI_FUNCTION("hideSoftKeyboard", HideSoftKeyboard),
        DECLARE_NAPI_FUNCTION("showSoftKeyboard", ShowSoftKeyboard),
        DECLARE_NAPI_FUNCTION("on", Subscribe),
        DECLARE_NAPI_FUNCTION("off", UnSubscribe),
        DECLARE_NAPI_FUNCTION("sendMessage", SendMessage),
        DECLARE_NAPI_FUNCTION("recvMessage", RecvMessage),
        DECLARE_NAPI_FUNCTION("discardTypingText", DiscardTypingText),
    };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, IMC_CLASS_NAME.c_str(), IMC_CLASS_NAME.size(), JsConstructor, nullptr,
                       sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &IMCRef_));
    NAPI_CALL(env, napi_set_named_property(env, info, IMC_CLASS_NAME.c_str(), cons));

    return info;
}

napi_value JsGetInputMethodController::GetJsKeyboardStatusProperty(napi_env env)
{
    napi_value keyboardStatus = nullptr;
    napi_value statusNone = nullptr;
    napi_value statusHide = nullptr;
    napi_value statusShow = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(KeyboardStatus::NONE), &statusNone));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(KeyboardStatus::HIDE), &statusHide));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(KeyboardStatus::SHOW), &statusShow));
    NAPI_CALL(env, napi_create_object(env, &keyboardStatus));
    NAPI_CALL(env, napi_set_named_property(env, keyboardStatus, "NONE", statusNone));
    NAPI_CALL(env, napi_set_named_property(env, keyboardStatus, "HIDE", statusHide));
    NAPI_CALL(env, napi_set_named_property(env, keyboardStatus, "SHOW", statusShow));
    return keyboardStatus;
}

napi_value JsGetInputMethodController::GetJsEnterKeyTypeProperty(napi_env env)
{
    napi_value enterKeyType = nullptr;
    napi_value typeUnspecified = nullptr;
    napi_value typeNone = nullptr;
    napi_value typeGo = nullptr;
    napi_value typeSearch = nullptr;
    napi_value typeSend = nullptr;
    napi_value typeNext = nullptr;
    napi_value typeDone = nullptr;
    napi_value typePrevious = nullptr;
    napi_value typeNewline = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::UNSPECIFIED), &typeUnspecified));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::NONE), &typeNone));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::GO), &typeGo));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::SEARCH), &typeSearch));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::SEND), &typeSend));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::NEXT), &typeNext));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::DONE), &typeDone));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::PREVIOUS), &typePrevious));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::NEW_LINE), &typeNewline));
    NAPI_CALL(env, napi_create_object(env, &enterKeyType));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "UNSPECIFIED", typeUnspecified));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "NONE", typeNone));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "GO", typeGo));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "SEARCH", typeSearch));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "SEND", typeSend));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "NEXT", typeNext));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "DONE", typeDone));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "PREVIOUS", typePrevious));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "NEWLINE", typeNewline));
    return enterKeyType;
}

napi_value JsGetInputMethodController::GetJsTextInputTypeProperty(napi_env env)
{
    napi_value textInputType = nullptr;
    napi_value typeNone = nullptr;
    napi_value typeText = nullptr;
    napi_value typeMultiline = nullptr;
    napi_value typeNumber = nullptr;
    napi_value typePhone = nullptr;
    napi_value typeDatatime = nullptr;
    napi_value typeEmailAddress = nullptr;
    napi_value typeUrl = nullptr;
    napi_value typeVisiblePassword = nullptr;
    napi_value typeNumberPassword = nullptr;
    napi_value typeScreenLockPassword = nullptr;
    napi_value typeUserName = nullptr;
    napi_value typeNewPassword = nullptr;
    napi_value typeNumberDecimal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::NONE), &typeNone));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::TEXT), &typeText));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::MULTILINE), &typeMultiline));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::NUMBER), &typeNumber));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::PHONE), &typePhone));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::DATETIME), &typeDatatime));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::EMAIL_ADDRESS), &typeEmailAddress));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::URL), &typeUrl));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::VISIBLE_PASSWORD), &typeVisiblePassword));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::NUMBER_PASSWORD), &typeNumberPassword));
    NAPI_CALL(env, napi_create_int32(env,
        static_cast<int32_t>(TextInputType::SCREEN_LOCK_PASSWORD), &typeScreenLockPassword));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::USER_NAME), &typeUserName));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::NEW_PASSWORD), &typeNewPassword));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::NUMBER_DECIMAL), &typeNumberDecimal));
    NAPI_CALL(env, napi_create_object(env, &textInputType));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "NONE", typeNone));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "TEXT", typeText));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "MULTILINE", typeMultiline));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "NUMBER", typeNumber));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "PHONE", typePhone));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "DATETIME", typeDatatime));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "EMAIL_ADDRESS", typeEmailAddress));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "URL", typeUrl));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "VISIBLE_PASSWORD", typeVisiblePassword));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "NUMBER_PASSWORD", typeNumberPassword));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "SCREEN_LOCK_PASSWORD", typeScreenLockPassword));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "USER_NAME", typeUserName));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "NEW_PASSWORD", typeNewPassword));
    NAPI_CALL(env, napi_set_named_property(env, textInputType, "NUMBER_DECIMAL", typeNumberDecimal));
    bool ret = JsUtil::Object::WriteProperty(env, textInputType, "ONE_TIME_CODE",
        static_cast<int32_t>(TextInputType::ONE_TIME_CODE));
    return ret ? textInputType : JsUtil::Const::Null(env);
}

napi_value JsGetInputMethodController::GetJsDirectionProperty(napi_env env)
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

napi_value JsGetInputMethodController::GetJsExtendActionProperty(napi_env env)
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

napi_value JsGetInputMethodController::GetJsCapitalizeModeProperty(napi_env env)
{
    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);
    bool ret = JsUtil::Object::WriteProperty(env, jsObject, "NONE", static_cast<int32_t>(CapitalizeMode::NONE));
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "SENTENCES",
        static_cast<int32_t>(CapitalizeMode::SENTENCES));
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "WORDS", static_cast<int32_t>(CapitalizeMode::WORDS));
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "CHARACTERS",
        static_cast<int32_t>(CapitalizeMode::CHARACTERS));
    return ret ? jsObject : JsUtil::Const::Null(env);
}

napi_value JsGetInputMethodController::GetJsEnabledStateProperty(napi_env env)
{
    napi_value status = nullptr;
    napi_value disabled = nullptr;
    napi_value basicMode = nullptr;
    napi_value fullExperience = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnabledStatus::DISABLED), &disabled));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnabledStatus::BASIC_MODE), &basicMode));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnabledStatus::FULL_EXPERIENCE_MODE), &fullExperience));
    NAPI_CALL(env, napi_create_object(env, &status));
    NAPI_CALL(env, napi_set_named_property(env, status, "DISABLED", disabled));
    NAPI_CALL(env, napi_set_named_property(env, status, "BASIC_MODE", basicMode));
    NAPI_CALL(env, napi_set_named_property(env, status, "FULL_EXPERIENCE_MODE", fullExperience));
    return status;
}

napi_value JsGetInputMethodController::GetJsRequestKeyboardReasonProperty(napi_env env)
{
    napi_value requestKeyboardReason = nullptr;
    napi_value none = nullptr;
    napi_value mouse = nullptr;
    napi_value touch = nullptr;
    napi_value other = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(RequestKeyboardReason::NONE), &none));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(RequestKeyboardReason::MOUSE), &mouse));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(RequestKeyboardReason::TOUCH), &touch));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(RequestKeyboardReason::OTHER), &other));
    NAPI_CALL(env, napi_create_object(env, &requestKeyboardReason));
    NAPI_CALL(env, napi_set_named_property(env, requestKeyboardReason, "NONE", none));
    NAPI_CALL(env, napi_set_named_property(env, requestKeyboardReason, "MOUSE", mouse));
    NAPI_CALL(env, napi_set_named_property(env, requestKeyboardReason, "TOUCH", touch));
    NAPI_CALL(env, napi_set_named_property(env, requestKeyboardReason, "OTHER", other));
    return requestKeyboardReason;
}

napi_value JsGetInputMethodController::JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    {
        std::lock_guard<std::mutex> lock(eventHandlerMutex_);
        handler_ = AppExecFwk::EventHandler::Current();
    }
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));

    auto controllerObject = GetInstance();
    if (controllerObject == nullptr) {
        IMSA_HILOGE("controllerObject is nullptr!");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_status status = napi_wrap(
        env, thisVar, controllerObject.get(), [](napi_env env, void *data, void *hint) {}, nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("failed to wrap: %{public}d!", status);
        return nullptr;
    }

    if (controllerObject->loop_ == nullptr) {
        napi_get_uv_event_loop(env, &controllerObject->loop_);
    }

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
        IMSA_HILOGE("failed to get reference value!");
        if (needThrowException) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_CONTROLLER, "", TYPE_OBJECT);
        }
        return nullptr;
    }

    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("failed to create new instance!");
        if (needThrowException) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_CONTROLLER, "", TYPE_OBJECT);
        }
        return nullptr;
    }
    return instance;
}

std::shared_ptr<JsGetInputMethodController> JsGetInputMethodController::GetInstance()
{
    if (controller_ == nullptr) {
        std::lock_guard<std::mutex> lock(controllerMutex_);
        if (controller_ == nullptr) {
            auto controller = std::make_shared<JsGetInputMethodController>();
            controller_ = controller;
            auto instance = InputMethodController::GetInstance();
            if (instance != nullptr) {
                instance->SetControllerListener(controller_);
            } else {
                IMSA_HILOGE("instance is nullptr!");
            }
        }
    }
    return controller_;
}

void JsGetInputMethodController::RegisterListener(napi_value callback, std::string type,
    std::shared_ptr<JSCallbackObject> callbackObj)
{
    IMSA_HILOGD("start, type: %{public}s", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGD("methodName: %{public}s is not registered!", type.c_str());
    }

    auto callbacks = jsCbMap_[type];
    bool ret = std::any_of(callbacks.begin(), callbacks.end(), [&callback](std::shared_ptr<JSCallbackObject> cb) {
        return JsUtils::Equals(cb->env_, callback, cb->callback_, cb->threadId_);
    });
    if (ret) {
        IMSA_HILOGD("callback already registered.");
        return;
    }

    IMSA_HILOGI("add %{public}s callbackObj into jsCbMap_.", type.c_str());
    jsCbMap_[type].push_back(std::move(callbackObj));
}

void JsGetInputMethodController::UnRegisterListener(napi_value callback, std::string type)
{
    IMSA_HILOGI("unregister listener: %{public}s.", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGE("methodName: %{public}s already unRegistered!", type.c_str());
        return;
    }
    if (callback == nullptr) {
        jsCbMap_.erase(type);
        UpdateTextPreviewState(type);
        IMSA_HILOGE("callback is nullptr!");
        return;
    }

    for (auto item = jsCbMap_[type].begin(); item != jsCbMap_[type].end(); item++) {
        if ((JsUtils::Equals((*item)->env_, callback, (*item)->callback_, (*item)->threadId_))) {
            jsCbMap_[type].erase(item);
            break;
        }
    }
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
        UpdateTextPreviewState(type);
    }
}

napi_value JsGetInputMethodController::Subscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    std::string type;
    // 2 means least param num.
    PARAM_CHECK_RETURN(env, argc >= 2, "at least two parameters is required!", TYPE_NONE, nullptr);
    PARAM_CHECK_RETURN(env, JsUtil::GetValue(env, argv[0], type), "type must be string", TYPE_NONE, nullptr);
    PARAM_CHECK_RETURN(env, EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_CONTROLLER, type),
        "type verification failed, review the instructions and fill in the fixed values!", TYPE_NONE, nullptr);
    PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[1]) == napi_function, "callback", TYPE_FUNCTION, nullptr);
    IMSA_HILOGD("subscribe type: %{public}s.", type.c_str());
    if (TEXT_EVENT_TYPE.find(type) != TEXT_EVENT_TYPE.end()) {
        auto instance = InputMethodController::GetInstance();
        if (instance == nullptr || !instance->WasAttached()) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_DETACHED, "need to be attached first", TYPE_NONE);
            return nullptr;
        }
    }

    auto engine = reinterpret_cast<JsGetInputMethodController *>(JsUtils::GetNativeSelf(env, info));
    if (engine == nullptr) {
        return nullptr;
    }
    std::shared_ptr<JSCallbackObject> callback =
        std::make_shared<JSCallbackObject>(env, argv[ARGC_ONE], std::this_thread::get_id(),
            AppExecFwk::EventHandler::Current());
    engine->RegisterListener(argv[ARGC_ONE], type, callback);

    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value JsGetInputMethodController::UnSubscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    std::string type;
    // 1 means least param num.
    if (argc < 1 || !JsUtil::GetValue(env, argv[0], type) ||
        !EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_CONTROLLER, type)) {
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
    auto engine = reinterpret_cast<JsGetInputMethodController *>(JsUtils::GetNativeSelf(env, info));
    if (engine == nullptr) {
        return nullptr;
    }
    engine->UnRegisterListener(argv[1], type);

    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value JsGetInputMethodController::CreateSelectRange(napi_env env, int32_t start, int32_t end)
{
    napi_value range = nullptr;
    napi_create_object(env, &range);

    napi_value value = nullptr;
    napi_create_int32(env, start, &value);
    napi_set_named_property(env, range, "start", value);

    napi_create_int32(env, end, &value);
    napi_set_named_property(env, range, "end", value);

    return range;
}

napi_value JsGetInputMethodController::CreateSelectMovement(napi_env env, int32_t direction)
{
    napi_value movement = nullptr;
    napi_create_object(env, &movement);

    napi_value value = nullptr;
    napi_create_int32(env, direction, &value);
    napi_set_named_property(env, movement, "direction", value);

    return movement;
}

napi_value JsGetInputMethodController::HandleSoftKeyboard(napi_env env, napi_callback_info info,
    std::function<int32_t()> callback, bool isOutput, bool needThrowException)
{
    auto ctxt = std::make_shared<HandleContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto output = [ctxt, isOutput](napi_env env, napi_value *result) -> napi_status {
        if (!isOutput) {
            return napi_ok;
        }
        napi_status status = napi_get_boolean(env, ctxt->isHandle, result);
        IMSA_HILOGE("output get boolean != nullptr[%{public}d]", result != nullptr);
        return status;
    };
    auto exec = [ctxt, callback, needThrowException](AsyncCall::Context *ctx) {
        int errCode = callback();
        if (errCode == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec success.");
            ctxt->status = napi_ok;
            ctxt->isHandle = true;
            ctxt->SetState(ctxt->status);
            return;
        }
        if (needThrowException) {
            ctxt->SetErrorCode(errCode);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI has 1 param at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "handleSoftKeyboard");
}

bool JsGetInputMethodController::GetValue(napi_env env, napi_value in, Range &out)
{
    auto ret = JsUtil::Object::ReadProperty(env, in, "start", out.start);
    return ret && JsUtil::Object::ReadProperty(env, in, "end", out.end);
}

/**
 * let textConfig: TextConfig = {
 *   inputAttribute: InputAttribute = {
 *     textInputType: TextInputType = TextInputType.TEXT,
 *     enterKeyType: EnterKeyType = EnterKeyType.NONE
 *   },
 *   cursorInfo?: CursorInfo = {
 *     left: number,
 *     top: number,
 *     width: number,
 *     height: number,
 *   },
 *   selection?: Range = {
 *     start: number,
 *     end: number
 *   },
 *   windowId?: number
 *   newEditBox?: boolean
 * }
 */
bool JsGetInputMethodController::GetValue(napi_env env, napi_value in, TextConfig &out)
{
    napi_value attributeResult = nullptr;
    napi_status status = JsUtils::GetValue(env, in, "inputAttribute", attributeResult);
    CHECK_RETURN(status == napi_ok, "inputAttribute must be InputAttribute!", false);
    bool ret = JsGetInputMethodController::GetValue(env, attributeResult, out.inputAttribute);
    CHECK_RETURN(ret, "inputAttribute of TextConfig must be valid!", ret);
    napi_value cursorInfoResult = nullptr;
    status = JsUtils::GetValue(env, in, "cursorInfo", cursorInfoResult);
    bool result = false;
    if (status == napi_ok) {
        result = JsGetInputMethodController::GetValue(env, cursorInfoResult, out.cursorInfo);
        if (!result) {
            IMSA_HILOGE("get cursorInfo failed.");
        }
    }

    napi_value rangeResult = nullptr;
    status = JsUtils::GetValue(env, in, "selection", rangeResult);
    if (status == napi_ok) {
        result = JsGetInputMethodController::GetValue(env, rangeResult, out.range);
        if (!result) {
            IMSA_HILOGE("get selectionRange failed.");
        }
    }

    result = JsUtil::Object::ReadProperty(env, in, "windowId", out.windowId);
    if (!result) {
        IMSA_HILOGE("get windowId failed.");
    }
    result = JsUtil::Object::ReadProperty(env, in, "newEditBox", out.newEditBox);
    if (!result) {
        IMSA_HILOGE("get newEditBox failed.");
    }
    int32_t capitalizeMode = 0;
    CapitalizeMode tempCapitalizeMode = CapitalizeMode::NONE;
    result = JsUtil::Object::ReadProperty(env, in, "capitalizeMode", capitalizeMode);
    if (!result) {
        IMSA_HILOGE("not found capitalizeMode.");
    }
    if (capitalizeMode < static_cast<int32_t>(CapitalizeMode::NONE) ||
        capitalizeMode > static_cast<int32_t>(CapitalizeMode::CHARACTERS)) {
        capitalizeMode = 0; // 0 Default value
        IMSA_HILOGE("capitalizeMode value invalid.");
    }
    tempCapitalizeMode = static_cast<CapitalizeMode>(capitalizeMode);
    out.inputAttribute.capitalizeMode = tempCapitalizeMode;
    return ret;
}

napi_value JsGetInputMethodController::Attach(napi_env env, napi_callback_info info)
{
    IMSA_HILOGI("run in.");
    InputMethodSyncTrace tracer("JsGetInputMethodController_Attach");
    auto ctxt = std::make_shared<AttachContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 1, "at least two parameters is required!", TYPE_NONE, napi_generic_failure);
        PARAM_CHECK_RETURN(env, JsUtil::GetValue(env, argv[0], ctxt->showKeyboard),
            "showKeyboard covert failed, type must be boolean!", TYPE_NONE, napi_generic_failure);
        PARAM_CHECK_RETURN(env, JsGetInputMethodController::GetValue(env, argv[1], ctxt->textConfig),
            "textConfig covert failed, type must be TextConfig!", TYPE_NONE, napi_generic_failure);
        if (JsGetInputMethodController::IsTextPreviewSupported()) {
            ctxt->textConfig.inputAttribute.isTextPreviewSupported = true;
        }
        // requestKeyboardReason not must
        if (argc > 2) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[2], &valueType);
            if (valueType != napi_function) {
                JsUtil::GetValue(env, argv[2], ctxt->requestKeyboardReason);
            }
        }
        ctxt->info = { std::chrono::system_clock::now(), ctxt->attribute};
        attachQueue_.Push(ctxt->info);
        return napi_ok;
    };
    auto exec = [ctxt, env](AsyncCall::Context *ctx) {
        attachQueue_.Wait(ctxt->info);
        ctxt->textListener = JsGetInputMethodTextChangedListener::GetTextListener(ctxt->textConfig.newEditBox);
        OHOS::MiscServices::AttachOptions attachOptions;
        attachOptions.isShowKeyboard = ctxt->showKeyboard;
        attachOptions.requestKeyboardReason =
              static_cast<OHOS::MiscServices::RequestKeyboardReason>(ctxt->requestKeyboardReason);
        int32_t status = ErrorCode::ERROR_CLIENT_NULL_POINTER;
        auto instance = InputMethodController::GetInstance();
        if (instance != nullptr) {
            status = instance->Attach(ctxt->textListener, attachOptions, ctxt->textConfig, ClientType::JS);
        }
        attachQueue_.Pop();
        ctxt->SetErrorCode(status);
        CHECK_RETURN_VOID(status == ErrorCode::NO_ERROR, "attach return error!");
        ctxt->SetState(napi_ok);
    };
    ctxt->SetAction(std::move(input));
    // 3 means JsAPI:attach has 3 params at most.
    AsyncCall asyncCall(env, info, ctxt, 3);
    return asyncCall.Call(env, exec, "attach");
}

napi_value JsGetInputMethodController::Detach(napi_env env, napi_callback_info info)
{
    return HandleSoftKeyboard(
        env, info, [] () -> int32_t {
            auto instance = InputMethodController::GetInstance();
            if (instance == nullptr) {
                IMSA_HILOGE("GetInstance return nullptr!");
                return ErrorCode::ERROR_CLIENT_NULL_POINTER;
            }
            return instance->Close();
        }, false, true);
}

napi_value JsGetInputMethodController::ShowTextInput(napi_env env, napi_callback_info info)
{
    IMSA_HILOGI("run in.");
    AttachOptions attachOptions;
    JsGetInputMethodController::GetAttachOptionsValue(env, info, attachOptions);
    InputMethodSyncTrace tracer("JsGetInputMethodController_ShowTextInput");
    return HandleSoftKeyboard(
        env, info,
        [attachOptions] () -> int32_t {
            auto instance = InputMethodController::GetInstance();
            if (instance == nullptr) {
                IMSA_HILOGE("GetInstance return nullptr!");
                return ErrorCode::ERROR_CLIENT_NULL_POINTER;
            }
            return instance->ShowTextInput(attachOptions, ClientType::JS);
        },
        false, true);
}

napi_value JsGetInputMethodController::GetAttachOptionsValue(
    napi_env env, napi_callback_info cbinfo, AttachOptions &attachOptions)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, 0, &result));
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data));
    int32_t requestKeyboardReason = 0;
    if (argc > 0) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType != napi_function) {
            JsUtil::GetValue(env, argv[0], requestKeyboardReason);
        }
    }
    IMSA_HILOGI("run in. requestKeyboardReason=%{public}d", requestKeyboardReason);
    attachOptions.requestKeyboardReason = static_cast<OHOS::MiscServices::RequestKeyboardReason>(requestKeyboardReason);

    return result;
}

napi_value JsGetInputMethodController::HideTextInput(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JsGetInputMethodController_HideTextInput");
    return HandleSoftKeyboard(
        env, info,
        [] () -> int32_t {
            auto instance = InputMethodController::GetInstance();
            if (instance == nullptr) {
                IMSA_HILOGE("GetInstance return nullptr!");
                return ErrorCode::ERROR_CLIENT_NULL_POINTER;
            }
            return instance->HideTextInput();
        },
        false, true);
}

napi_value JsGetInputMethodController::DiscardTypingText(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JsGetInputMethodController_DiscardTypingText");
    return HandleSoftKeyboard(
        env, info
        , [] {
            auto instance = InputMethodController::GetInstance();
            if (instance == nullptr) {
                IMSA_HILOGE("GetInstance return nullptr!");
                return static_cast<int32_t>(ErrorCode::ERROR_CLIENT_NULL_POINTER);
            }
            return instance->DiscardTypingText();
        },
        false, true);
}

napi_value JsGetInputMethodController::SetCallingWindow(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SetCallingWindowContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        // 0 means the first parameter: windowId
        napi_status status = JsUtils::GetValue(env, argv[0], ctxt->windID);
        PARAM_CHECK_RETURN(env, status == napi_ok, "windowId type must be number", TYPE_NONE, status);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errcode = ErrorCode::ERROR_CLIENT_NULL_POINTER;
        auto instance = InputMethodController::GetInstance();
        if (instance != nullptr) {
            errcode = instance->SetCallingWindow(ctxt->windID);
        }
        ctxt->SetErrorCode(errcode);
        CHECK_RETURN_VOID(errcode == ErrorCode::NO_ERROR, "setCallingWindow return error!");
        ctxt->SetState(napi_ok);
    };
    ctxt->SetAction(std::move(input));
    // 2 means JsAPI:setCallingWindow has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "setCallingWindow");
}

bool JsGetInputMethodController::GetValue(napi_env env, napi_value in, CursorInfo &out)
{
    auto ret = JsUtil::Object::ReadProperty(env, in, "left", out.left);
    ret = ret && JsUtil::Object::ReadProperty(env, in, "top", out.top);
    ret = ret && JsUtil::Object::ReadProperty(env, in, "width", out.width);
    return ret && JsUtil::Object::ReadProperty(env, in, "height", out.height);
}

napi_value JsGetInputMethodController::UpdateCursor(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<UpdateCursorContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        // 0 means the first parameter: cursorInfo
        PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_object,
            "cursorInfo type must be CursorInfo", TYPE_NONE, napi_generic_failure);
        bool ret = JsGetInputMethodController::GetValue(env, argv[0], ctxt->cursorInfo);
        PARAM_CHECK_RETURN(env, ret, "cursorInfo covert failed, must contain four numbers!", TYPE_NONE,
            napi_generic_failure);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errcode = ErrorCode::ERROR_CLIENT_NULL_POINTER;
        auto instance = InputMethodController::GetInstance();
        if (instance != nullptr) {
            errcode = instance->OnCursorUpdate(ctxt->cursorInfo);
        }
        ctxt->SetErrorCode(errcode);
        CHECK_RETURN_VOID(errcode == ErrorCode::NO_ERROR, "updateCursor return error!");
        ctxt->SetState(napi_ok);
    };
    ctxt->SetAction(std::move(input));
    // 2 means JsAPI:updateCursor has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "updateCursor");
}

napi_value JsGetInputMethodController::ChangeSelection(napi_env env, napi_callback_info info)
{
    std::shared_ptr<ChangeSelectionContext> ctxt = std::make_shared<ChangeSelectionContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 2, "at least three parameters is required!", TYPE_NONE, napi_generic_failure);
        PARAM_CHECK_RETURN(env, JsUtil::GetValue(env, argv[0], ctxt->text), "text type must be string!",
            TYPE_NONE, napi_generic_failure);
        PARAM_CHECK_RETURN(env, JsUtil::GetValue(env, argv[1], ctxt->start), "start type must be number!",
            TYPE_NONE, napi_generic_failure);
        PARAM_CHECK_RETURN(env, JsUtil::GetValue(env, argv[2], ctxt->end), "end type must be number!", TYPE_NONE,
            napi_generic_failure);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errcode = ErrorCode::ERROR_CLIENT_NULL_POINTER;
        auto instance = InputMethodController::GetInstance();
        if (instance != nullptr) {
            errcode = instance->OnSelectionChange(ctxt->text, ctxt->start, ctxt->end);
        }
        ctxt->SetErrorCode(errcode);
        CHECK_RETURN_VOID(errcode == ErrorCode::NO_ERROR, "changeSelection return error!");
        ctxt->SetState(napi_ok);
    };
    ctxt->SetAction(std::move(input));
    // 4 means JsAPI:changeSelection has 4 params at most.
    AsyncCall asyncCall(env, info, ctxt, 4);
    return asyncCall.Call(env, exec, "changeSelection");
}

bool JsGetInputMethodController::GetValue(napi_env env, napi_value in, InputAttribute &out)
{
    auto ret = JsUtil::Object::ReadProperty(env, in, "textInputType", out.inputPattern);
    ret = ret && JsUtil::Object::ReadProperty(env, in, "enterKeyType", out.enterKeyType);
    // compatibility with older versions may not exist
    JsUtil::Object::ReadPropertyU16String(env, in, "placeholder", out.placeholder);
    IMSA_HILOGD("placeholder:%{public}s", StringUtils::ToHex(out.placeholder).c_str());
    // compatibility with older versions may not exist
    JsUtil::Object::ReadPropertyU16String(env, in, "abilityName", out.abilityName);
    IMSA_HILOGD("abilityName:%{public}s", StringUtils::ToHex(out.abilityName).c_str());
    return ret;
}

napi_value JsGetInputMethodController::UpdateAttribute(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<UpdateAttributeContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        bool ret = JsGetInputMethodController::GetValue(env, argv[0], ctxt->attribute);
        PARAM_CHECK_RETURN(env, ret, "attribute type must be InputAttribute!", TYPE_NONE, napi_generic_failure);
        ctxt->configuration.SetTextInputType(static_cast<TextInputType>(ctxt->attribute.inputPattern));
        ctxt->configuration.SetEnterKeyType(static_cast<EnterKeyType>(ctxt->attribute.enterKeyType));
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t errcode = ErrorCode::ERROR_CLIENT_NULL_POINTER;
        auto instance = InputMethodController::GetInstance();
        if (instance != nullptr) {
            errcode = instance->OnConfigurationChange(ctxt->configuration);
        }
        ctxt->SetErrorCode(errcode);
        CHECK_RETURN_VOID(errcode == ErrorCode::NO_ERROR, "updateAttribute return error!");
        ctxt->SetState(napi_ok);
    };
    ctxt->SetAction(std::move(input));
    // 2 means JsAPI:updateAttribute has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "updateAttribute");
}

napi_value JsGetInputMethodController::ShowSoftKeyboard(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JsGetInputMethodController_ShowSoftKeyboard");
    return HandleSoftKeyboard(
        env, info,
        [] () -> int32_t {
            auto instance = InputMethodController::GetInstance();
            if (instance == nullptr) {
                IMSA_HILOGE("GetInstance return nullptr!");
                return ErrorCode::ERROR_CLIENT_NULL_POINTER;
            }
            return instance->ShowSoftKeyboard(ClientType::JS);
        },
        false, true);
}

napi_value JsGetInputMethodController::HideSoftKeyboard(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JsGetInputMethodController_HideSoftKeyboard");
    return HandleSoftKeyboard(
        env, info,
        [] () -> int32_t {
            auto instance = InputMethodController::GetInstance();
            if (instance == nullptr) {
                IMSA_HILOGE("GetInstance return nullptr!");
                return ErrorCode::ERROR_CLIENT_NULL_POINTER;
            }
            return instance->HideSoftKeyboard();
        },
        false, true);
}

napi_value JsGetInputMethodController::StopInputSession(napi_env env, napi_callback_info info)
{
    return HandleSoftKeyboard(
        env, info,
        [] () -> int32_t {
            auto instance = InputMethodController::GetInstance();
            if (instance == nullptr) {
                IMSA_HILOGE("GetInstance return nullptr!");
                return ErrorCode::ERROR_CLIENT_NULL_POINTER;
            }
            return instance->StopInputSession();
        },
        true, true);
}

napi_value JsGetInputMethodController::StopInput(napi_env env, napi_callback_info info)
{
    return HandleSoftKeyboard(
        env, info,
        []() -> int32_t {
            auto instance = InputMethodController::GetInstance();
            if (instance == nullptr) {
                IMSA_HILOGE("GetInstance return nullptr!");
                return ErrorCode::ERROR_CLIENT_NULL_POINTER;
            }
            return instance->HideCurrentInput();
        },
        true, false);
}

void JsGetInputMethodController::OnSelectByRange(int32_t start, int32_t end)
{
    std::string type = "selectByRange";
    auto entry = GetEntry("selectByRange", [start, end](UvEntry &entry) {
        entry.start = start;
        entry.end = end;
    });
    if (entry == nullptr) {
        IMSA_HILOGD("entry is nullptr.");
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    auto task = [entry]() {
        auto getProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc < ARGC_ONE) {
                return false;
            }
            napi_value range = CreateSelectRange(env, entry->start, entry->end);
            if (range == nullptr) {
                IMSA_HILOGE("set select range failed!");
                return false;
            }
            // 0 means the first param of callback.
            args[0] = range;
            return true;
        };
        // 1 means the callback has one param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, getProperty });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

void JsGetInputMethodController::OnSelectByMovement(int32_t direction)
{
    std::string type = "selectByMovement";
    auto entry = GetEntry(type, [direction](UvEntry &entry) { entry.direction = direction; });
    if (entry == nullptr) {
        IMSA_HILOGE("failed to get uv entry!");
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGI("direction: %{public}d.", direction);
    auto task = [entry]() {
        auto getProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc < 1) {
                return false;
            }
            napi_value movement = CreateSelectMovement(env, entry->direction);
            if (movement == nullptr) {
                IMSA_HILOGE("set select movement failed!");
                return false;
            }
            // 0 means the first param of callback.
            args[0] = movement;
            return true;
        };
        // 1 means the callback has one param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, getProperty });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

void JsGetInputMethodController::InsertText(const std::u16string &text)
{
    std::string insertText = Str16ToStr8(text);
    std::string type = "insertText";
    auto entry = GetEntry(type, [&insertText](UvEntry &entry) { entry.text = insertText; });
    if (entry == nullptr) {
        IMSA_HILOGD("failed to get uv entry.");
        auto instance = InputMethodController::GetInstance();
        if (instance == nullptr) {
            IMSA_HILOGE("GetInstance return nullptr!");
            return;
        }
        instance->ReportBaseTextOperation(
            static_cast<int32_t>(IInputDataChannelIpcCode::COMMAND_INSERT_TEXT), ErrorCode::ERROR_JS_CB_NOT_REGISTER);
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGI("start.");
    auto task = [entry]() {
        auto getInsertTextProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc == ARGC_ZERO) {
                IMSA_HILOGE("getInsertTextProperty the number of argc is invalid.");
                return false;
            }
            // 0 means the first param of callback.
            napi_create_string_utf8(env, entry->text.c_str(), NAPI_AUTO_LENGTH, &args[0]);
            return true;
        };
        // 1 means the callback has one param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, getInsertTextProperty });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

void JsGetInputMethodController::DeleteRight(int32_t length)
{
    std::string type = "deleteRight";
    auto entry = GetEntry(type, [&length](UvEntry &entry) { entry.length = length; });
    if (entry == nullptr) {
        IMSA_HILOGD("failed to get uv entry.");
        auto instance = InputMethodController::GetInstance();
        if (instance == nullptr) {
            IMSA_HILOGE("GetInstance return nullptr!");
            return;
        }
        instance->ReportBaseTextOperation(static_cast<int32_t>(IInputDataChannelIpcCode::COMMAND_DELETE_FORWARD),
            ErrorCode::ERROR_JS_CB_NOT_REGISTER);
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGI("length: %{public}d", length);

    auto task = [entry]() {
        auto getDeleteForwardProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc == ARGC_ZERO) {
                IMSA_HILOGE("getDeleteForwardProperty the number of argc is invalid.");
                return false;
            }
            // 0 means the first param of callback.
            napi_create_int32(env, entry->length, &args[0]);
            return true;
        };
        // 1 means the callback has one param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, getDeleteForwardProperty });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

void JsGetInputMethodController::DeleteLeft(int32_t length)
{
    std::string type = "deleteLeft";
    auto entry = GetEntry(type, [&length](UvEntry &entry) { entry.length = length; });
    if (entry == nullptr) {
        IMSA_HILOGD("failed to get uv entry.");
        auto instance = InputMethodController::GetInstance();
        if (instance == nullptr) {
            IMSA_HILOGE("GetInstance return nullptr!");
            return;
        }
        instance->ReportBaseTextOperation(static_cast<int32_t>(IInputDataChannelIpcCode::COMMAND_DELETE_BACKWARD),
            ErrorCode::ERROR_JS_CB_NOT_REGISTER);
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGI("length: %{public}d", length);
    auto task = [entry]() {
        auto getDeleteBackwardProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc == ARGC_ZERO) {
                IMSA_HILOGE("getDeleteBackwardProperty the number of argc is invalid.");
                return false;
            }
            // 0 means the first param of callback.
            napi_create_int32(env, entry->length, &args[0]);
            return true;
        };
        // 1 means the callback has one param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, getDeleteBackwardProperty });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

void JsGetInputMethodController::SendKeyboardStatus(const KeyboardStatus &status)
{
    std::string type = "sendKeyboardStatus";
    auto entry = GetEntry(type, [&status](UvEntry &entry) { entry.keyboardStatus = static_cast<int32_t>(status); });
    if (entry == nullptr) {
        IMSA_HILOGD("failed to get uv entry.");
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGI("status: %{public}d", static_cast<int32_t>(status));
    auto task = [entry]() {
        auto getSendKeyboardStatusProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc == ARGC_ZERO) {
                IMSA_HILOGE("getSendKeyboardStatusProperty the number of argc is invalid.");
                return false;
            }
            // 0 means the first param of callback.
            napi_create_int32(env, entry->keyboardStatus, &args[0]);
            return true;
        };
        // 1 means the callback has one param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, getSendKeyboardStatusProperty });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

napi_value JsGetInputMethodController::CreateSendFunctionKey(napi_env env, int32_t functionKey)
{
    napi_value functionkey = nullptr;
    napi_create_object(env, &functionkey);

    napi_value value = nullptr;
    napi_create_int32(env, functionKey, &value);
    napi_set_named_property(env, functionkey, "enterKeyType", value);

    return functionkey;
}

void JsGetInputMethodController::SendFunctionKey(const FunctionKey &functionKey)
{
    std::string type = "sendFunctionKey";
    auto entry = GetEntry(type,
        [&functionKey](UvEntry &entry) { entry.enterKeyType = static_cast<int32_t>(functionKey.GetEnterKeyType()); });
    if (entry == nullptr) {
        IMSA_HILOGD("failed to get uv entry.");
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGI("functionKey: %{public}d", static_cast<int32_t>(functionKey.GetEnterKeyType()));
    auto task = [entry]() {
        auto getSendFunctionKeyProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc == ARGC_ZERO) {
                IMSA_HILOGE("getSendFunctionKeyProperty the number of argc is invalid.");
                return false;
            }
            napi_value functionKey = CreateSendFunctionKey(env, entry->enterKeyType);
            if (functionKey == nullptr) {
                IMSA_HILOGE("set select movement failed");
                return false;
            }
            // 0 means the first param of callback.
            args[0] = functionKey;
            return true;
        };
        // 1 means the callback has one param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, getSendFunctionKeyProperty });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

void JsGetInputMethodController::MoveCursor(const Direction direction)
{
    std::string type = "moveCursor";
    auto entry = GetEntry(type, [&direction](UvEntry &entry) { entry.direction = static_cast<int32_t>(direction); });
    if (entry == nullptr) {
        IMSA_HILOGD("failed to get uv entry.");
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGI("direction: %{public}d", static_cast<int32_t>(direction));
    auto task = [entry]() {
        auto getMoveCursorProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc == ARGC_ZERO) {
                IMSA_HILOGE("getMoveCursorProperty the number of argc is invalid.");
                return false;
            }
            // 0 means the first param of callback.
            napi_create_int32(env, static_cast<int32_t>(entry->direction), &args[0]);
            return true;
        };
        // 1 means the callback has one param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, getMoveCursorProperty });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

void JsGetInputMethodController::HandleExtendAction(int32_t action)
{
    std::string type = "handleExtendAction";
    auto entry = GetEntry(type, [&action](UvEntry &entry) { entry.action = action; });
    if (entry == nullptr) {
        IMSA_HILOGD("failed to get uv entry.");
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGI("action: %{public}d", action);
    auto task = [entry]() {
        auto getHandleExtendActionProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc == ARGC_ZERO) {
                IMSA_HILOGE("getHandleExtendActionProperty the number of argc is invalid.");
                return false;
            }
            // 0 means the first param of callback.
            napi_create_int32(env, entry->action, &args[0]);
            return true;
        };
        // 1 means the callback has one param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, getHandleExtendActionProperty });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

std::u16string JsGetInputMethodController::GetText(const std::string &type, int32_t number)
{
    auto textResultHandler = std::make_shared<BlockData<std::string>>(MAX_TIMEOUT, "");
    auto entry = GetEntry(type, [&number, textResultHandler](UvEntry &entry) {
        entry.number = number;
        entry.textResultHandler = textResultHandler;
    });
    if (entry == nullptr) {
        IMSA_HILOGE("failed to get uv entry.");
        return u"";
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return u"";
    }
    IMSA_HILOGI("type: %{public}s, number: %{public}d.", type.c_str(), number);
    auto task = [entry]() {
        auto fillArguments = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            if (argc < 1) {
                IMSA_HILOGE("argc is err.");
                return false;
            }
            // 0 means the first param of callback.
            napi_create_int32(env, entry->number, &args[0]);
            return true;
        };
        std::string text;
        // 1 means callback has one param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 1, fillArguments }, text);
        entry->textResultHandler->SetValue(text);
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
    return Str8ToStr16(textResultHandler->GetValue());
}

int32_t JsGetInputMethodController::GetTextIndexAtCursor()
{
    std::string type = "getTextIndexAtCursor";
    auto indexResultHandler = std::make_shared<BlockData<int32_t>>(MAX_TIMEOUT, -1);
    auto entry =
        GetEntry(type, [indexResultHandler](UvEntry &entry) { entry.indexResultHandler = indexResultHandler; });
    if (entry == nullptr) {
        IMSA_HILOGE("failed to get uv entry!");
        return -1;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return -1;
    }
    IMSA_HILOGI("run in");
    auto task = [entry]() {
        int32_t index = -1;
        // 0 means callback has no params.
        JsCallbackHandler::Traverse(entry->vecCopy, { 0, nullptr }, index);
        entry->indexResultHandler->SetValue(index);
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
    return indexResultHandler->GetValue();
}

int32_t JsGetInputMethodController::SetPreviewText(const std::u16string &text, const Range &range)
{
    std::string previewText = Str16ToStr8(text);
    std::string type = "setPreviewText";
    auto entry = GetEntry(type, [&previewText, &range](UvEntry &entry) {
        entry.text = previewText;
        entry.start = range.start;
        entry.end = range.end;
    });
    if (entry == nullptr) {
        IMSA_HILOGD("failed to get uv entry!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    IMSA_HILOGI("previewText start.");
    auto task = [entry]() {
        auto getPreviewTextProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
            // 2 means the callback has two params.
            if (argc < 2) {
                return false;
            }
            // 0 means the first param of callback.
            args[0] = JsUtil::GetValue(env, entry->text);
            // 1 means the second param of callback.
            args[1] = CreateSelectRange(env, entry->start, entry->end);
            return true;
        };

        // 2 means the callback has two param.
        JsCallbackHandler::Traverse(entry->vecCopy, { 2, getPreviewTextProperty });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
    return ErrorCode::NO_ERROR;
}

void JsGetInputMethodController::FinishTextPreview()
{
    std::string type = "finishTextPreview";
    auto entry = GetEntry(type, nullptr);
    if (entry == nullptr) {
        IMSA_HILOGE("failed to get uv entry!");
        return;
    }
    auto eventHandler = GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGE("eventHandler is nullptr!");
        return;
    }
    IMSA_HILOGI("run in");
    auto task = [entry]() {
        // 0 means callback has no params.
        JsCallbackHandler::Traverse(entry->vecCopy, { 0, nullptr });
    };
    eventHandler->PostTask(task, type, 0, AppExecFwk::EventQueue::Priority::VIP);
}

std::shared_ptr<AppExecFwk::EventHandler> JsGetInputMethodController::GetEventHandler()
{
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    return handler_;
}

std::shared_ptr<JsGetInputMethodController::UvEntry> JsGetInputMethodController::GetEntry(const std::string &type,
    EntrySetter entrySetter)
{
    IMSA_HILOGD("type: %{public}s", type.c_str());
    std::shared_ptr<UvEntry> entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (jsCbMap_.find(type) == jsCbMap_.end() || jsCbMap_[type].empty()) {
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

napi_value JsGetInputMethodController::SendMessage(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SendMessageContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_string, "msgId",
            TYPE_STRING, napi_generic_failure);
        CHECK_RETURN(JsUtils::GetValue(env, argv[0], ctxt->arrayBuffer.msgId) == napi_ok,
            "msgId covert failed!", napi_generic_failure);
        ctxt->arrayBuffer.jsArgc = argc;
        // 1 means first param msgId.
        if (argc > 1) {
            bool isArryBuffer = false;
            //  1 means second param msgParam index.
            CHECK_RETURN(napi_is_arraybuffer(env, argv[1], &isArryBuffer) == napi_ok,
                "napi_is_arraybuffer failed!", napi_generic_failure);
            PARAM_CHECK_RETURN(env, isArryBuffer, "msgParam", TYPE_ARRAY_BUFFER, napi_generic_failure);
            CHECK_RETURN(JsUtils::GetValue(env, argv[1], ctxt->arrayBuffer.msgParam) == napi_ok,
                "msgParam covert failed!", napi_generic_failure);
        }
        PARAM_CHECK_RETURN(env, ArrayBuffer::IsSizeValid(ctxt->arrayBuffer),
            "msgId limit 256B and msgParam limit 128KB.", TYPE_NONE, napi_generic_failure);
        ctxt->info = { std::chrono::system_clock::now(), ctxt->arrayBuffer };
        messageHandlerQueue_.Push(ctxt->info);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        messageHandlerQueue_.Wait(ctxt->info);
        int32_t code = ErrorCode::ERROR_CLIENT_NULL_POINTER;
        auto instance = InputMethodController::GetInstance();
        if (instance != nullptr) {
            code = instance->SendMessage(ctxt->arrayBuffer);
        }
        messageHandlerQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), nullptr);
    // 2 means JsAPI:sendMessage has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "imcSendMessage");
}

napi_value JsGetInputMethodController::RecvMessage(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    std::string type;
    if (argc < 0) {
        IMSA_HILOGE("RecvMessage failed! argc abnormal.");
        return nullptr;
    }

    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("RecvMessage failed! GetInstance() is nullptr.");
        return nullptr;
    }
    if (argc == 0) {
        IMSA_HILOGI("RecvMessage off.");
        instance->RegisterMsgHandler();
        return nullptr;
    }
    IMSA_HILOGI("RecvMessage on.");
    PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_object, "msgHnadler (MessageHandler)",
        TYPE_OBJECT, nullptr);

    napi_value onMessage = nullptr;
    CHECK_RETURN(napi_get_named_property(env, argv[0], "onMessage", &onMessage) == napi_ok,
        "Get onMessage property failed!", nullptr);
    CHECK_RETURN(JsUtil::GetType(env, onMessage) == napi_function, "onMessage is not napi_function!", nullptr);
    napi_value onTerminated = nullptr;
    CHECK_RETURN(napi_get_named_property(env, argv[0], "onTerminated", &onTerminated) == napi_ok,
        "Get onTerminated property failed!", nullptr);
    CHECK_RETURN(JsUtil::GetType(env, onTerminated) == napi_function, "onTerminated is not napi_function!", nullptr);

    std::shared_ptr<MsgHandlerCallbackInterface> callback =
        std::make_shared<JsGetInputMethodController::JsMessageHandler>(env, onTerminated, onMessage);
    instance->RegisterMsgHandler(callback);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

int32_t JsGetInputMethodController::JsMessageHandler::OnTerminated()
{
    std::lock_guard<decltype(callbackObjectMutex_)> lock(callbackObjectMutex_);
    if (jsMessageHandler_ == nullptr) {
        IMSA_HILOGI("jsCallbackObject is nullptr, can not call OnTerminated!.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto eventHandler = jsMessageHandler_->GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGI("EventHandler is nullptr!.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    // Ensure jsMessageHandler_ destructor run in current thread.
    auto task = [jsCallback = std::move(jsMessageHandler_)]() {
        napi_value callback = nullptr;
        napi_value global = nullptr;
        if (jsCallback == nullptr) {
            IMSA_HILOGI("jsCallback is nullptr!.");
            return;
        }
        napi_get_reference_value(jsCallback->env_, jsCallback->onTerminatedCallback_, &callback);
        if (callback != nullptr) {
            napi_get_global(jsCallback->env_, &global);
            napi_value output = nullptr;
            // 0 means the callback has no param.
            auto status = napi_call_function(jsCallback->env_, global, callback, 0, nullptr, &output);
            if (status != napi_ok) {
                IMSA_HILOGI("Call js function failed!.");
                output = nullptr;
            }
        }
    };
    eventHandler->PostTask(task, "IMC_MsgHandler_OnTerminated", 0, AppExecFwk::EventQueue::Priority::VIP);
    return ErrorCode::NO_ERROR;
}

int32_t JsGetInputMethodController::JsMessageHandler::OnMessage(const ArrayBuffer &arrayBuffer)
{
    std::lock_guard<decltype(callbackObjectMutex_)> lock(callbackObjectMutex_);
    if (jsMessageHandler_ == nullptr) {
        IMSA_HILOGI("jsCallbackObject is nullptr, can not call OnTerminated!.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto eventHandler = jsMessageHandler_->GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGI("EventHandler is nullptr!.");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto task = [jsCallbackObject = jsMessageHandler_, arrayBuffer]() {
        napi_value callback = nullptr;
        napi_value global = nullptr;
        if (jsCallbackObject == nullptr) {
            IMSA_HILOGI("jsCallbackObject is nullptr!.");
            return;
        }
        napi_get_reference_value(jsCallbackObject->env_, jsCallbackObject->onMessageCallback_, &callback);
        if (callback != nullptr) {
            napi_get_global(jsCallbackObject->env_, &global);
            napi_value output = nullptr;
            napi_value argv[ARGC_TWO] = { nullptr };
            // 2 means just use the first two parameters
            if (JsUtils::GetMessageHandlerCallbackParam(argv, jsCallbackObject, arrayBuffer, 2) != napi_ok) {
                IMSA_HILOGE("Get message handler callback param failed!.");
                return;
            }
            // The maximum valid parameters count of callback is 2.
            auto callbackArgc = arrayBuffer.jsArgc > ARGC_ONE ? ARGC_TWO : ARGC_ONE;
            auto status = napi_call_function(jsCallbackObject->env_, global, callback, callbackArgc, argv, &output);
            if (status != napi_ok) {
                IMSA_HILOGI("Call js function failed!.");
                output = nullptr;
            }
        }
    };
    eventHandler->PostTask(task, "IMC_MsgHandler_OnMessage", 0, AppExecFwk::EventQueue::Priority::VIP);
    return ErrorCode::NO_ERROR;
}

bool JsGetInputMethodController::IsRegister(const std::string &type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGD("methodName: %{public}s is not registered!", type.c_str());
        return false;
    }
    if (jsCbMap_[type].empty()) {
        IMSA_HILOGD("methodName: %{public}s cb-vector is empty!", type.c_str());
        return false;
    }
    return true;
}

bool JsGetInputMethodController::IsTextPreviewSupported()
{
    auto engine = JsGetInputMethodController::GetInstance();
    if (engine == nullptr) {
        return false;
    }
    return engine->IsRegister("setPreviewText") && engine->IsRegister("finishTextPreview");
}

void JsGetInputMethodController::UpdateTextPreviewState(const std::string &type)
{
    if (type == "setPreviewText" || type == "finishTextPreview") {
        auto instance = InputMethodController::GetInstance();
        if (instance == nullptr) {
            IMSA_HILOGE("GetInstance() is nullptr!");
            return;
        }
        instance->UpdateTextPreviewState(false);
    }
}
} // namespace MiscServices
} // namespace OHOS
