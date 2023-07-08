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
#include "input_method_controller.h"
#include "input_method_utils.h"
#include "js_callback_handler.h"
#include "js_get_input_method_textchange_listener.h"
#include "js_util.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "string_ex.h"

namespace OHOS {
namespace MiscServices {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
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
    };
    NAPI_CALL(
        env, napi_define_properties(env, info, sizeof(descriptor) / sizeof(napi_property_descriptor), descriptor));

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
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::UNSPECIFIED), &typeUnspecified));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::NONE), &typeNone));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::GO), &typeGo));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::SEARCH), &typeSearch));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::SEND), &typeSend));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::NEXT), &typeNext));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::DONE), &typeDone));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(EnterKeyType::PREVIOUS), &typePrevious));
    NAPI_CALL(env, napi_create_object(env, &enterKeyType));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "UNSPECIFIED", typeUnspecified));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "NONE", typeNone));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "GO", typeGo));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "SEARCH", typeSearch));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "SEND", typeSend));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "NEXT", typeNext));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "DONE", typeDone));
    NAPI_CALL(env, napi_set_named_property(env, enterKeyType, "PREVIOUS", typePrevious));
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
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::NONE), &typeNone));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::TEXT), &typeText));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::MULTILINE), &typeMultiline));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::NUMBER), &typeNumber));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::PHONE), &typePhone));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::DATETIME), &typeDatatime));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::EMAIL_ADDRESS), &typeEmailAddress));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::URL), &typeUrl));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(TextInputType::VISIBLE_PASSWORD), &typeVisiblePassword));
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
    return textInputType;
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

napi_value JsGetInputMethodController::JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));

    auto controllerObject = GetInstance();
    if (controllerObject == nullptr) {
        IMSA_HILOGE("controllerObject is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    napi_status status = napi_wrap(
        env, thisVar, controllerObject.get(), [](napi_env env, void *data, void *hint) {}, nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("JsGetInputMethodController napi_wrap failed:%{public}d", status);
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
        IMSA_HILOGE("Failed to get constructor of input method controller.");
        if (needThrowException) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_CONTROLLER, "", TYPE_OBJECT);
        }
        return nullptr;
    }

    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("Failed to get instance of input method controller.");
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
            InputMethodController::GetInstance()->SetControllerListener(controller_);
        }
    }
    return controller_;
}

void JsGetInputMethodController::RegisterListener(
    napi_value callback, std::string type, std::shared_ptr<JSCallbackObject> callbackObj)
{
    IMSA_HILOGI("run in, type: %{public}s", type.c_str());
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        IMSA_HILOGE("methodName: %{public}s not registered!", type.c_str());
    }

    auto callbacks = jsCbMap_[type];
    bool ret = std::any_of(callbacks.begin(), callbacks.end(), [&callback](std::shared_ptr<JSCallbackObject> cb) {
        return JsUtils::Equals(cb->env_, callback, cb->callback_, cb->threadId_);
    });
    if (ret) {
        IMSA_HILOGE("JsGetInputMethodController callback already registered!");
        return;
    }

    IMSA_HILOGI("Add %{public}s callbackObj into jsCbMap_", type.c_str());
    jsCbMap_[type].push_back(std::move(callbackObj));
}

void JsGetInputMethodController::UnRegisterListener(napi_value callback, std::string type)
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
        if ((callback != nullptr)
            && (JsUtils::Equals((*item)->env_, callback, (*item)->callback_, (*item)->threadId_))) {
            jsCbMap_[type].erase(item);
            break;
        }
    }
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
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
    if (argc < 2 || !JsUtil::GetValue(env, argv[0], type)
        || !EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_CONTROLLER, type)
        || JsUtil::GetType(env, argv[1]) != napi_function) {
        IMSA_HILOGE("Subscribe failed, type:%{public}s", type.c_str());
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "please check the params", TYPE_NONE);
        return nullptr;
    }
    IMSA_HILOGD("Subscribe type:%{public}s.", type.c_str());
    if (TEXT_EVENT_TYPE.find(type) != TEXT_EVENT_TYPE.end()) {
        if (!InputMethodController::GetInstance()->WasAttached()) {
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_DETACHED, "need to be attached first", TYPE_NONE);
            return nullptr;
        }
    }

    auto engine = reinterpret_cast<JsGetInputMethodController *>(JsUtils::GetNativeSelf(env, info));
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

napi_value JsGetInputMethodController::UnSubscribe(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    std::string type;
    // 1 means least param num.
    if (argc < 1 || !JsUtil::GetValue(env, argv[0], type)
        || !EventChecker::IsValidEventType(EventSubscribeModule::INPUT_METHOD_CONTROLLER, type)) {
        IMSA_HILOGE("UnSubscribe failed, type:%{public}s", type.c_str());
        return nullptr;
    }
    // If the type of optional parameter is wrong, make it nullptr
    if (JsUtil::GetType(env, argv[1]) != napi_function) {
        argv[1] = nullptr;
    }
    IMSA_HILOGD("UnSubscribe type:%{public}s.", type.c_str());
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

napi_value JsGetInputMethodController::HandleSoftKeyboard(
    napi_env env, napi_callback_info info, std::function<int32_t()> callback, bool isOutput, bool needThrowException)
{
    auto ctxt = std::make_shared<HandleContext>();
    auto input = [ctxt](
                     napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status { return napi_ok; };
    auto output = [ctxt, isOutput](napi_env env, napi_value *result) -> napi_status {
        if (!isOutput) {
            return napi_ok;
        }
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
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "handleSoftKeyboard");
}

napi_status JsGetInputMethodController::ParseAttachInput(
    napi_env env, size_t argc, napi_value *argv, const std::shared_ptr<AttachContext> &ctxt)
{
    // 0 means the first parameter: showkeyboard
    napi_status status = JsUtils::GetValue(env, argv[0], ctxt->showKeyboard);
    if (status != napi_ok) {
        return status;
    }

    // 1 means the second parameter: textConfig
    napi_value attributeResult = nullptr;
    status = JsUtils::GetValue(env, argv[1], "inputAttribute", attributeResult);
    if (status != napi_ok) {
        return status;
    }
    napi_value textResult = nullptr;
    status = JsUtils::GetValue(env, attributeResult, "textInputType", textResult);
    if (status != napi_ok) {
        return status;
    }
    status = JsUtils::GetValue(env, textResult, ctxt->attribute.inputPattern);
    if (status != napi_ok) {
        return status;
    }
    napi_value enterResult = nullptr;
    status = JsUtils::GetValue(env, attributeResult, "enterKeyType", enterResult);
    if (status != napi_ok) {
        return status;
    }
    return JsUtils::GetValue(env, enterResult, ctxt->attribute.enterKeyType);
}

napi_value JsGetInputMethodController::Attach(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<AttachContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 1, "should 2 or 3 parameters!", TYPE_NONE, napi_generic_failure);
        napi_status status = ParseAttachInput(env, argc, argv, ctxt);
        PARAM_CHECK_RETURN(env, status == napi_ok, "paramters of attach is error. ", TYPE_NONE, status);
        return status;
    };
    auto exec = [ctxt, env](AsyncCall::Context *ctx) {
        ctxt->textListener = JsGetInputMethodTextChangedListener::GetInstance();
        auto status =
            InputMethodController::GetInstance()->Attach(ctxt->textListener, ctxt->showKeyboard, ctxt->attribute);
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
        env, info, [] { return InputMethodController::GetInstance()->Close(); }, false, true);
}

napi_value JsGetInputMethodController::ShowTextInput(napi_env env, napi_callback_info info)
{
    return HandleSoftKeyboard(
        env, info, [] { return InputMethodController::GetInstance()->ShowTextInput(); }, false, true);
}

napi_value JsGetInputMethodController::HideTextInput(napi_env env, napi_callback_info info)
{
    return HandleSoftKeyboard(
        env, info, [] { return InputMethodController::GetInstance()->HideTextInput(); }, false, true);
}

napi_value JsGetInputMethodController::SetCallingWindow(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SetCallingWindowContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        // 0 means the first parameter: windowId
        napi_status status = JsUtils::GetValue(env, argv[0], ctxt->windID);
        PARAM_CHECK_RETURN(env, status == napi_ok, "paramters of setCallingWindow is error. ", TYPE_NONE, status);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        auto errcode = InputMethodController::GetInstance()->SetCallingWindow(ctxt->windID);
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
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        // 0 means the first parameter: cursorInfo
        bool ret = JsGetInputMethodController::GetValue(env, argv[0], ctxt->cursorInfo);
        PARAM_CHECK_RETURN(env, ret, "paramters of updateCursor is error. ", TYPE_NONE, napi_generic_failure);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        auto errcode = InputMethodController::GetInstance()->OnCursorUpdate(ctxt->cursorInfo);
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
        PARAM_CHECK_RETURN(env, argc > 2, "should 3 or 4 parameters!", TYPE_NONE, napi_generic_failure);
        bool ret = JsUtil::GetValue(env, argv[0], ctxt->text);
        ret = ret && JsUtil::GetValue(env, argv[1], ctxt->start);
        ret = ret && JsUtil::GetValue(env, argv[2], ctxt->end);
        PARAM_CHECK_RETURN(env, ret, "paramters of changeSelection is error. ", TYPE_NONE, napi_generic_failure);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        auto errcode = InputMethodController::GetInstance()->OnSelectionChange(ctxt->text, ctxt->start, ctxt->end);
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
    return ret && JsUtil::Object::ReadProperty(env, in, "enterKeyType", out.enterKeyType);
}

napi_value JsGetInputMethodController::UpdateAttribute(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<UpdateAttributeContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        bool ret = JsGetInputMethodController::GetValue(env, argv[0], ctxt->attribute);
        PARAM_CHECK_RETURN(env, ret, "paramters of updateAttribute is error. ", TYPE_NONE, napi_generic_failure);
        ctxt->configuration.SetTextInputType(static_cast<TextInputType>(ctxt->attribute.inputPattern));
        ctxt->configuration.SetEnterKeyType(static_cast<EnterKeyType>(ctxt->attribute.enterKeyType));
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        auto errcode = InputMethodController::GetInstance()->OnConfigurationChange(ctxt->configuration);
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
        env, info, [] { return InputMethodController::GetInstance()->HideCurrentInput(); }, true, false);
}

void JsGetInputMethodController::OnSelectByRange(int32_t start, int32_t end)
{
    IMSA_HILOGD("run in, start: %{public}d, end: %{public}d", start, end);
    std::string type = "selectByRange";
    uv_work_t *work = GetUVwork("selectByRange", [start, end](UvEntry &entry) {
        entry.start = start;
        entry.end = end;
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
                IMSA_HILOGE("OnSelectByRange entryptr is null");
                return;
            }
            auto getProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
                if (argc < ARGC_ONE) {
                    return false;
                }
                napi_value range = CreateSelectRange(env, entry->start, entry->end);
                if (range == nullptr) {
                    IMSA_HILOGE("set select range failed");
                    return false;
                }
                // 0 means the first param of callback.
                args[0] = range;
                return true;
            };
            // 1 means the callback has one param.
            JsCallbackHandler::Traverse(entry->vecCopy, { 1, getProperty });
        });
}

void JsGetInputMethodController::OnSelectByMovement(int32_t direction)
{
    IMSA_HILOGD("run in, direction: %{public}d", direction);
    std::string type = "selectByMovement";
    uv_work_t *work = GetUVwork(type, [direction](UvEntry &entry) { entry.direction = direction; });
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
                IMSA_HILOGE("OnSelectByMovement entryptr is null");
                return;
            }
            auto getProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
                if (argc < 1) {
                    return false;
                }
                napi_value movement = CreateSelectMovement(env, entry->direction);
                if (movement == nullptr) {
                    IMSA_HILOGE("set select movement failed");
                    return false;
                }
                // 0 means the first param of callback.
                args[0] = movement;
                return true;
            };
            // 1 means the callback has one param.
            JsCallbackHandler::Traverse(entry->vecCopy, { 1, getProperty });
        });
}

void JsGetInputMethodController::InsertText(const std::u16string &text)
{
    std::string insertText = Str16ToStr8(text);
    std::string type = "insertText";
    uv_work_t *work = GetUVwork(type, [&insertText](UvEntry &entry) { entry.text = insertText; });
    if (work == nullptr) {
        IMSA_HILOGE("failed to get uv entry.");
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
                IMSA_HILOGE("insertText entryptr is null.");
                return;
            }

            auto getInsertTextProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
                if (argc == ARGC_ZERO) {
                    IMSA_HILOGE("insertText:getInsertTextProperty the number of argc is invalid.");
                    return false;
                }
                // 0 means the first param of callback.
                napi_create_string_utf8(env, entry->text.c_str(), NAPI_AUTO_LENGTH, &args[0]);
                return true;
            };
            // 1 means the callback has one param.
            JsCallbackHandler::Traverse(entry->vecCopy, { 1, getInsertTextProperty });
        });
}

void JsGetInputMethodController::DeleteRight(int32_t length)
{
    std::string type = "deleteRight";
    uv_work_t *work = GetUVwork(type, [&length](UvEntry &entry) { entry.length = length; });
    if (work == nullptr) {
        IMSA_HILOGE("failed to get uv entry.");
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
                IMSA_HILOGE("deleteRight entryptr is null.");
                return;
            }

            auto getDeleteForwardProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
                if (argc == ARGC_ZERO) {
                    IMSA_HILOGE("deleteRight:getDeleteForwardProperty the number of argc is invalid.");
                    return false;
                }
                // 0 means the first param of callback.
                napi_create_int32(env, entry->length, &args[0]);
                return true;
            };
            // 1 means the callback has one param.
            JsCallbackHandler::Traverse(entry->vecCopy, { 1, getDeleteForwardProperty });
        });
}

void JsGetInputMethodController::DeleteLeft(int32_t length)
{
    std::string type = "deleteLeft";
    uv_work_t *work = GetUVwork(type, [&length](UvEntry &entry) { entry.length = length; });
    if (work == nullptr) {
        IMSA_HILOGE("failed to get uv entry.");
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
                IMSA_HILOGE("deleteLeft entryptr is null.");
                return;
            }

            auto getDeleteBackwardProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
                if (argc == ARGC_ZERO) {
                    IMSA_HILOGE("deleteLeft::getDeleteBackwardProperty the number of argc is invalid.");
                    return false;
                }
                // 0 means the first param of callback.
                napi_create_int32(env, entry->length, &args[0]);
                return true;
            };
            // 1 means the callback has one param.
            JsCallbackHandler::Traverse(entry->vecCopy, { 1, getDeleteBackwardProperty });
        });
}

void JsGetInputMethodController::SendKeyboardStatus(const KeyboardStatus &status)
{
    std::string type = "sendKeyboardStatus";
    uv_work_t *work =
        GetUVwork(type, [&status](UvEntry &entry) { entry.keyboardStatus = static_cast<int32_t>(status); });
    if (work == nullptr) {
        IMSA_HILOGE("failed to get uv entry.");
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
                IMSA_HILOGE("sendKeyboardStatus entryptr is null.");
                return;
            }

            auto getSendKeyboardStatusProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
                if (argc == ARGC_ZERO) {
                    IMSA_HILOGE("sendKeyboardStatus:getSendKeyboardStatusProperty the number of argc is invalid.");
                    return false;
                }
                // 0 means the first param of callback.
                napi_create_int32(env, entry->keyboardStatus, &args[0]);
                return true;
            };
            // 1 means the callback has one param.
            JsCallbackHandler::Traverse(entry->vecCopy, { 1, getSendKeyboardStatusProperty });
        });
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
    uv_work_t *work = GetUVwork(type,
        [&functionKey](UvEntry &entry) { entry.enterKeyType = static_cast<int32_t>(functionKey.GetEnterKeyType()); });
    if (work == nullptr) {
        IMSA_HILOGE("failed to get uv entry.");
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
                IMSA_HILOGE("sendFunctionKey entryptr is null.");
                return;
            }

            auto getSendFunctionKeyProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
                if (argc == ARGC_ZERO) {
                    IMSA_HILOGE("sendFunctionKey:getSendFunctionKeyProperty the number of argc is invalid.");
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
        });
}

void JsGetInputMethodController::MoveCursor(const Direction direction)
{
    std::string type = "moveCursor";
    uv_work_t *work =
        GetUVwork(type, [&direction](UvEntry &entry) { entry.direction = static_cast<int32_t>(direction); });
    if (work == nullptr) {
        IMSA_HILOGE("failed to get uv entry.");
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
                IMSA_HILOGE("moveCursor entryptr is null.");
                return;
            }

            auto getMoveCursorProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
                if (argc == ARGC_ZERO) {
                    IMSA_HILOGE("moveCursor:getMoveCursorProperty the number of argc is invalid.");
                    return false;
                }
                // 0 means the first param of callback.
                napi_create_int32(env, static_cast<int32_t>(entry->direction), &args[0]);
                return true;
            };
            // 1 means the callback has one param.
            JsCallbackHandler::Traverse(entry->vecCopy, { 1, getMoveCursorProperty });
        });
}

void JsGetInputMethodController::HandleExtendAction(int32_t action)
{
    std::string type = "handleExtendAction";
    uv_work_t *work = GetUVwork(type, [&action](UvEntry &entry) { entry.action = action; });
    if (work == nullptr) {
        IMSA_HILOGE("failed to get uv entry.");
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
                IMSA_HILOGE("handleExtendAction entryptr is null.");
                return;
            }
            auto getHandleExtendActionProperty = [entry](napi_env env, napi_value *args, uint8_t argc) -> bool {
                if (argc == ARGC_ZERO) {
                    IMSA_HILOGE("handleExtendAction:getHandleExtendActionProperty the number of argc is invalid.");
                    return false;
                }
                // 0 means the first param of callback.
                napi_create_int32(env, entry->action, &args[0]);
                return true;
            };
            // 1 means the callback has one param.
            JsCallbackHandler::Traverse(entry->vecCopy, { 1, getHandleExtendActionProperty });
        });
}

std::u16string JsGetInputMethodController::GetText(const std::string &type, int32_t number)
{
    auto textResultHandler = std::make_shared<BlockData<std::string>>(MAX_TIMEOUT, "");
    uv_work_t *work = GetUVwork(type, [&number, textResultHandler](UvEntry &entry) {
        entry.number = number;
        entry.textResultHandler = textResultHandler;
    });
    if (work == nullptr) {
        IMSA_HILOGE("failed to get uv entry.");
        return u"";
    }
    uv_queue_work(
        loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<UvEntry> entry(static_cast<UvEntry *>(work->data), [work](UvEntry *data) {
                delete data;
                delete work;
            });
            if (entry == nullptr) {
                IMSA_HILOGE("handleExtendAction entryptr is null.");
                return;
            }
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
        });
    return Str8ToStr16(textResultHandler->GetValue());
}

int32_t JsGetInputMethodController::GetTextIndexAtCursor()
{
    std::string type = "getTextIndexAtCursor";
    auto indexResultHandler = std::make_shared<BlockData<int32_t>>(MAX_TIMEOUT, -1);
    uv_work_t *work =
        GetUVwork(type, [indexResultHandler](UvEntry &entry) { entry.indexResultHandler = indexResultHandler; });
    if (work == nullptr) {
        IMSA_HILOGE("failed to get uv entry.");
        return -1;
    }
    uv_queue_work(
        loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<UvEntry> entry(static_cast<UvEntry *>(work->data), [work](UvEntry *data) {
                delete data;
                delete work;
            });
            if (entry == nullptr) {
                IMSA_HILOGE("handleExtendAction entryptr is null.");
                return;
            }
            int32_t index = -1;
            // 0 means callback has no params.
            JsCallbackHandler::Traverse(entry->vecCopy, { 0, nullptr }, index);
            entry->indexResultHandler->SetValue(index);
        });
    return indexResultHandler->GetValue();
}

uv_work_t *JsGetInputMethodController::GetUVwork(const std::string &type, EntrySetter entrySetter)
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