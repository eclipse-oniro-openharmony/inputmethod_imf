/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "js_text_input_client_engine.h"

#include "input_method_ability.h"
#include "inputmethod_trace.h"
#include "js_util.h"
#include "js_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "string_ex.h"

namespace OHOS {
namespace MiscServices {
using namespace std::chrono;
thread_local napi_ref JsTextInputClientEngine::TICRef_ = nullptr;
const std::string JsTextInputClientEngine::TIC_CLASS_NAME = "TextInputClient";
constexpr int32_t MAX_WAIT_TIME = 5000;
constexpr int32_t MAX_WAIT_TIME_PRIVATE_COMMAND = 2000;
BlockQueue<EditorEventInfo> JsTextInputClientEngine::editorQueue_{ MAX_WAIT_TIME };
BlockQueue<PrivateCommandInfo> JsTextInputClientEngine::privateCommandQueue_{ MAX_WAIT_TIME_PRIVATE_COMMAND };
napi_value JsTextInputClientEngine::Init(napi_env env, napi_value info)
{
    IMSA_HILOGD("JsTextInputClientEngine init");
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("sendKeyFunction", SendKeyFunction),
        DECLARE_NAPI_FUNCTION("deleteForward", DeleteForward),
        DECLARE_NAPI_FUNCTION("deleteBackward", DeleteBackward),
        DECLARE_NAPI_FUNCTION("insertText", InsertText),
        DECLARE_NAPI_FUNCTION("getForward", GetForward),
        DECLARE_NAPI_FUNCTION("getBackward", GetBackward),
        DECLARE_NAPI_FUNCTION("getEditorAttribute", GetEditorAttribute),
        DECLARE_NAPI_FUNCTION("getTextIndexAtCursor", GetTextIndexAtCursor),
        DECLARE_NAPI_FUNCTION("moveCursor", MoveCursor),
        DECLARE_NAPI_FUNCTION("selectByRange", SelectByRange),
        DECLARE_NAPI_FUNCTION("selectByMovement", SelectByMovement),
        DECLARE_NAPI_FUNCTION("sendExtendAction", SendExtendAction),
        DECLARE_NAPI_FUNCTION("insertTextSync", InsertTextSync),
        DECLARE_NAPI_FUNCTION("moveCursorSync", MoveCursorSync),
        DECLARE_NAPI_FUNCTION("getEditorAttributeSync", GetEditorAttributeSync),
        DECLARE_NAPI_FUNCTION("selectByRangeSync", SelectByRangeSync),
        DECLARE_NAPI_FUNCTION("selectByMovementSync", SelectByMovementSync),
        DECLARE_NAPI_FUNCTION("getTextIndexAtCursorSync", GetTextIndexAtCursorSync),
        DECLARE_NAPI_FUNCTION("deleteForwardSync", DeleteForwardSync),
        DECLARE_NAPI_FUNCTION("deleteBackwardSync", DeleteBackwardSync),
        DECLARE_NAPI_FUNCTION("getForwardSync", GetForwardSync),
        DECLARE_NAPI_FUNCTION("getBackwardSync", GetBackwardSync),
        DECLARE_NAPI_FUNCTION("sendPrivateCommand", SendPrivateCommand) };
    napi_value cons = nullptr;
    NAPI_CALL(env, napi_define_class(env, TIC_CLASS_NAME.c_str(), TIC_CLASS_NAME.size(), JsConstructor, nullptr,
                       sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NAPI_CALL(env, napi_create_reference(env, cons, 1, &TICRef_));
    NAPI_CALL(env, napi_set_named_property(env, info, TIC_CLASS_NAME.c_str(), cons));

    return info;
}

napi_value JsTextInputClientEngine::MoveCursor(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<MoveCursorContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, " should 1 or 2 parameters! ", TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->num);
        if (status == napi_ok) {
            ctxt->info = { std::chrono::system_clock::now(), EditorEvent::MOVE_CURSOR };
            editorQueue_.Push(ctxt->info);
        }
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        editorQueue_.Wait(ctxt->info);
        int32_t code = InputMethodAbility::GetInstance()->MoveCursor(ctxt->num);
        editorQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input));
    // 2 means JsAPI:moveCursor has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "moveCursor");
}

napi_value JsTextInputClientEngine::MoveCursorSync(napi_env env, napi_callback_info info)
{
    EditorEventInfo eventInfo = { std::chrono::system_clock::now(), EditorEvent::MOVE_CURSOR};
    editorQueue_.Push(eventInfo);
    editorQueue_.Wait(eventInfo);
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t direction = 0;
    // 1 means least param num.
    if (argc < 1 || JsUtil::GetType(env, argv[0]) != napi_number || !JsUtil::GetValue(env, argv[0], direction)
        || direction < 0) {
        editorQueue_.Pop();
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "please check the params", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    IMSA_HILOGD("moveCursor , direction: %{public}d", direction);
    int32_t ret = InputMethodAbility::GetInstance()->MoveCursor(direction);
    editorQueue_.Pop();
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to move cursor", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, nullptr));

    JsTextInputClientEngine *clientObject = new (std::nothrow) JsTextInputClientEngine();
    if (clientObject == nullptr) {
        IMSA_HILOGE("clientObject is nullptr");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    auto finalize = [](napi_env env, void *data, void *hint) {
        IMSA_HILOGD("JsTextInputClientEngine finalize");
        auto *objInfo = reinterpret_cast<JsTextInputClientEngine *>(data);
        if (objInfo != nullptr) {
            delete objInfo;
        }
    };
    napi_status status = napi_wrap(env, thisVar, clientObject, finalize, nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("JsTextInputClientEngine napi_wrap failed: %{public}d", status);
        delete clientObject;
        return nullptr;
    }
    return thisVar;
}

napi_value JsTextInputClientEngine::GetTextInputClientInstance(napi_env env)
{
    napi_value instance = nullptr;
    napi_value cons = nullptr;
    if (napi_get_reference_value(env, TICRef_, &cons) != napi_ok) {
        IMSA_HILOGE("JsTextInputClientEngine::napi_get_reference_value not ok");
        return nullptr;
    }
    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("JsTextInputClientEngine::napi_new_instance not ok");
        return nullptr;
    }
    return instance;
}

napi_value JsTextInputClientEngine::GetResult(napi_env env, std::string &text)
{
    napi_value jsText = nullptr;
    napi_create_string_utf8(env, text.c_str(), NAPI_AUTO_LENGTH, &jsText);
    return jsText;
}

napi_value JsTextInputClientEngine::GetResultEditorAttribute(
    napi_env env, std::shared_ptr<GetEditorAttributeContext> getEditorAttribute)
{
    napi_value editorAttribute = nullptr;
    napi_create_object(env, &editorAttribute);

    napi_value jsValue = nullptr;
    napi_create_int32(env, getEditorAttribute->enterKeyType, &jsValue);
    napi_set_named_property(env, editorAttribute, "enterKeyType", jsValue);

    napi_value jsMethodId = nullptr;
    napi_create_int32(env, getEditorAttribute->inputPattern, &jsMethodId);
    napi_set_named_property(env, editorAttribute, "inputPattern", jsMethodId);

    return editorAttribute;
}

napi_status JsTextInputClientEngine::GetSelectRange(napi_env env, napi_value argv, std::shared_ptr<SelectContext> ctxt)
{
    napi_status status = napi_generic_failure;
    napi_value napiValue = nullptr;
    status = napi_get_named_property(env, argv, "start", &napiValue);
    PARAM_CHECK_RETURN(env, status == napi_ok, "missing start parameter.", TYPE_NONE, status);
    status = JsUtils::GetValue(env, napiValue, ctxt->start);
    CHECK_RETURN(status == napi_ok, "failed to get start value", status);

    status = napi_get_named_property(env, argv, "end", &napiValue);
    PARAM_CHECK_RETURN(env, status == napi_ok, "missing end parameter.", TYPE_NONE, status);
    status = JsUtils::GetValue(env, napiValue, ctxt->end);
    if (status != napi_ok) {
        IMSA_HILOGE("failed to get end value");
    }
    return status;
}

napi_status JsTextInputClientEngine::GetSelectMovement(
    napi_env env, napi_value argv, std::shared_ptr<SelectContext> ctxt)
{
    napi_status status = napi_generic_failure;
    napi_value napiValue = nullptr;
    status = napi_get_named_property(env, argv, "direction", &napiValue);
    PARAM_CHECK_RETURN(env, status == napi_ok, "missing direction parameter.", TYPE_NONE, status);
    status = JsUtils::GetValue(env, napiValue, ctxt->direction);
    if (status != napi_ok) {
        IMSA_HILOGE("failed to get direction value");
    }
    return status;
}

napi_value JsTextInputClientEngine::SendKeyFunction(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SendKeyFunctionContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        return JsUtils::GetValue(env, argv[0], ctxt->action);
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, ctxt->isSendKeyFunction, result);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t code = InputMethodAbility::GetInstance()->SendFunctionKey(ctxt->action);
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            ctxt->isSendKeyFunction = true;
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 2 means JsAPI:sendKeyFunction has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "sendKeyFunction");
}

napi_value JsTextInputClientEngine::SendPrivateCommand(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SendPrivateCommandContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 parameter!", TYPE_NONE, napi_generic_failure);
        napi_status status = JsUtils::GetValue(env, argv[0], ctxt->privateCommand);
        CHECK_RETURN(status == napi_ok, "GetValue privateCommand error", status);
        if (!TextConfig::IsPrivateCommandValid(ctxt->privateCommand)) {
            PARAM_CHECK_RETURN(
                env, false, "privateCommand size limit 32KB, count limit 5.", TYPE_NONE, napi_generic_failure);
        }
        ctxt->info = { std::chrono::system_clock::now(), ctxt->privateCommand };
        privateCommandQueue_.Push(ctxt->info);
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status { return napi_ok; };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        privateCommandQueue_.Wait(ctxt->info);
        int32_t code = InputMethodAbility::GetInstance()->SendPrivateCommand(ctxt->privateCommand);
        privateCommandQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI:SendPrivateCommand has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "SendPrivateCommand");
}

napi_value JsTextInputClientEngine::DeleteForwardSync(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JS_DeleteForwardSync");
    EditorEventInfo eventInfo = { std::chrono::system_clock::now(), EditorEvent::DELETE_FORWARD };
    editorQueue_.Push(eventInfo);
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    editorQueue_.Wait(eventInfo);
    PrintEditorQueueInfoIfTimeout(start, eventInfo);
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t length = 0;
    // 1 means least param num.
    if (argc < 1 || JsUtil::GetType(env, argv[0]) != napi_number || !JsUtil::GetValue(env, argv[0], length)
        || length < 0) {
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "please check the params", TYPE_NONE);
        editorQueue_.Pop();
        return JsUtil::Const::Null(env);
    }
    IMSA_HILOGD("Delete forward, length: %{public}d", length);
    int32_t ret = InputMethodAbility::GetInstance()->DeleteForward(length);
    editorQueue_.Pop();
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to delete forward", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::DeleteForward(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JS_DeleteForward");
    auto ctxt = std::make_shared<DeleteForwardContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->length);
        if (status == napi_ok) {
            ctxt->info = { std::chrono::system_clock::now(), EditorEvent::DELETE_FORWARD };
            editorQueue_.Push(ctxt->info);
        }
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, ctxt->isDeleteForward, result);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        InputMethodSyncTrace tracer("JS_DeleteForward_Exec");
        int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        editorQueue_.Wait(ctxt->info);
        PrintEditorQueueInfoIfTimeout(start, ctxt->info);
        int32_t code = InputMethodAbility::GetInstance()->DeleteForward(ctxt->length);
        editorQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            ctxt->isDeleteForward = true;
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 2 means JsAPI:deleteForward has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "deleteForward");
}

napi_value JsTextInputClientEngine::DeleteBackwardSync(napi_env env, napi_callback_info info)
{
    EditorEventInfo eventInfo = { std::chrono::system_clock::now(), EditorEvent::DELETE_BACKWARD };
    editorQueue_.Push(eventInfo);
    editorQueue_.Wait(eventInfo);
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t length = 0;
    // 1 means least param num.
    if (argc < 1 || JsUtil::GetType(env, argv[0]) != napi_number || !JsUtil::GetValue(env, argv[0], length)
        || length < 0) {
        editorQueue_.Pop();
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "please check the params", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    IMSA_HILOGD("Delete backward, length: %{public}d", length);
    int32_t ret = InputMethodAbility::GetInstance()->DeleteBackward(length);
    editorQueue_.Pop();
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to delete backward", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::DeleteBackward(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<DeleteBackwardContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->length);
        if (status == napi_ok) {
            ctxt->info = { std::chrono::system_clock::now(), EditorEvent::DELETE_BACKWARD };
            editorQueue_.Push(ctxt->info);
        }
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, ctxt->isDeleteBackward, result);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        editorQueue_.Wait(ctxt->info);
        int32_t code = InputMethodAbility::GetInstance()->DeleteBackward(ctxt->length);
        editorQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            ctxt->isDeleteBackward = true;
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 2 means JsAPI:deleteBackward has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "deleteBackward");
}

napi_value JsTextInputClientEngine::InsertText(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JS_InsertText");
    auto ctxt = std::make_shared<InsertTextContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->text);
        if (status == napi_ok) {
            ctxt->info = { std::chrono::system_clock::now(), EditorEvent::INSERT_TEXT };
            editorQueue_.Push(ctxt->info);
        }
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, ctxt->isInsertText, result);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        InputMethodSyncTrace tracer("JS_InsertText_Exec");
        int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        editorQueue_.Wait(ctxt->info);
        PrintEditorQueueInfoIfTimeout(start, ctxt->info);
        int32_t code = InputMethodAbility::GetInstance()->InsertText(ctxt->text);
        editorQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            ctxt->isInsertText = true;
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 2 means JsAPI:insertText has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "insertText");
}

napi_value JsTextInputClientEngine::InsertTextSync(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JS_InsertTextSync");
    EditorEventInfo eventInfo = { std::chrono::system_clock::now(), EditorEvent::INSERT_TEXT};
    editorQueue_.Push(eventInfo);
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    editorQueue_.Wait(eventInfo);
    PrintEditorQueueInfoIfTimeout(start, eventInfo);
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    std::string text;
    // 1 means least param num.
    if (argc < 1 || JsUtil::GetType(env, argv[0]) != napi_string || !JsUtil::GetValue(env, argv[0], text)) {
        editorQueue_.Pop();
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "please check the params", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    IMSA_HILOGD("insert text , text: %{public}s", text.c_str());
    int32_t ret = InputMethodAbility::GetInstance()->InsertText(text);
    editorQueue_.Pop();
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to insert text", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::GetForwardSync(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JS_GetForwardSync");
    EditorEventInfo eventInfo = { std::chrono::system_clock::now(), EditorEvent::GET_FORWARD };
    editorQueue_.Push(eventInfo);
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    editorQueue_.Wait(eventInfo);
    PrintEditorQueueInfoIfTimeout(start, eventInfo);
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t length = 0;
    // 1 means least param num.
    if (argc < 1 || JsUtil::GetType(env, argv[0]) != napi_number || !JsUtil::GetValue(env, argv[0], length)
        || length < 0) {
        editorQueue_.Pop();
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "please check the params", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    IMSA_HILOGD("Get forward, length: %{public}d", length);
    std::u16string text;
    int32_t ret = InputMethodAbility::GetInstance()->GetTextBeforeCursor(length, text);
    editorQueue_.Pop();
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to get forward", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    napi_value result = nullptr;
    auto status = JsUtils::GetValue(env, Str16ToStr8(text), result);
    CHECK_RETURN(status == napi_ok, "GetValue failed", JsUtil::Const::Null(env));
    return result;
}

napi_value JsTextInputClientEngine::GetForward(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JS_GetForward");
    auto ctxt = std::make_shared<GetForwardContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->length);
        if (status == napi_ok) {
            ctxt->info = { std::chrono::system_clock::now(), EditorEvent::GET_FORWARD };
            editorQueue_.Push(ctxt->info);
        }
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_value data = GetResult(env, ctxt->text);
        *result = data;
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        InputMethodSyncTrace tracer("JS_GetForward_Exec");
        int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        editorQueue_.Wait(ctxt->info);
        PrintEditorQueueInfoIfTimeout(start, ctxt->info);
        std::u16string temp;
        int32_t code = InputMethodAbility::GetInstance()->GetTextBeforeCursor(ctxt->length, temp);
        editorQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            ctxt->text = Str16ToStr8(temp);
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 2 means JsAPI:getForward has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "getForward");
}

napi_value JsTextInputClientEngine::GetBackwardSync(napi_env env, napi_callback_info info)
{
    EditorEventInfo eventInfo = { std::chrono::system_clock::now(), EditorEvent::GET_BACKWARD };
    editorQueue_.Push(eventInfo);
    editorQueue_.Wait(eventInfo);
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t length = 0;
    // 1 means least param num.
    if (argc < 1 || JsUtil::GetType(env, argv[0]) != napi_number || !JsUtil::GetValue(env, argv[0], length)
        || length < 0) {
        editorQueue_.Pop();
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "please check the params", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    IMSA_HILOGD("Get backward, length: %{public}d", length);
    std::u16string text;
    int32_t ret = InputMethodAbility::GetInstance()->GetTextAfterCursor(length, text);
    editorQueue_.Pop();
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to get backward", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    napi_value result = nullptr;
    auto status = JsUtils::GetValue(env, Str16ToStr8(text), result);
    CHECK_RETURN(status == napi_ok, "GetValue failed", JsUtil::Const::Null(env));
    return result;
}

napi_value JsTextInputClientEngine::GetBackward(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<GetBackwardContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->length);
        if (status == napi_ok) {
            ctxt->info = { std::chrono::system_clock::now(), EditorEvent::GET_BACKWARD };
            editorQueue_.Push(ctxt->info);
        }
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_value data = GetResult(env, ctxt->text);
        *result = data;
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        editorQueue_.Wait(ctxt->info);
        std::u16string temp;
        int32_t code = InputMethodAbility::GetInstance()->GetTextAfterCursor(ctxt->length, temp);
        editorQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
            ctxt->text = Str16ToStr8(temp);
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 2 means JsAPI:getBackward has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "getBackward");
}

napi_value JsTextInputClientEngine::GetEditorAttributeSync(napi_env env, napi_callback_info info)
{
    int32_t enterKeyType = 0;
    int32_t ret =  InputMethodAbility::GetInstance()->GetEnterKeyType(enterKeyType);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to getEnterKeyType", TYPE_NONE);
    }
    IMSA_HILOGD("enterKeyType: %{public}d", enterKeyType);

    int32_t inputPattern = 0;
    ret =  InputMethodAbility::GetInstance()->GetInputPattern(inputPattern);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to getInputPattern", TYPE_NONE);
    }
    IMSA_HILOGD("patternCode: %{public}d", inputPattern);

    const InputAttribute attribute =  { .inputPattern = inputPattern, .enterKeyType = enterKeyType };
    return JsUtils::GetValue(env, attribute);
}

napi_value JsTextInputClientEngine::GetEditorAttribute(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<GetEditorAttributeContext>();
    auto input = [ctxt](
                     napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status { return napi_ok; };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_value data = GetResultEditorAttribute(env, ctxt);
        *result = data;
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t typeCode = InputMethodAbility::GetInstance()->GetEnterKeyType(ctxt->enterKeyType);
        int32_t patternCode = InputMethodAbility::GetInstance()->GetInputPattern(ctxt->inputPattern);
        if (typeCode == ErrorCode::NO_ERROR && patternCode == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
        } else {
            typeCode == ErrorCode::NO_ERROR ? ctxt->SetErrorCode(patternCode) : ctxt->SetErrorCode(typeCode);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI:getEditorAttribute has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "getEditorAttribute");
}

napi_value JsTextInputClientEngine::SelectByRange(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in");
    auto ctxt = std::make_shared<SelectContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        PARAM_CHECK_RETURN(env, valueType == napi_object, "range", TYPE_OBJECT, napi_generic_failure);
        auto status = GetSelectRange(env, argv[0], ctxt);
        if (status == napi_ok) {
            ctxt->info = { std::chrono::system_clock::now(), EditorEvent::SELECT_BY_RANGE };
            editorQueue_.Push(ctxt->info);
        }
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status { return napi_ok; };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        editorQueue_.Wait(ctxt->info);
        int32_t code = InputMethodAbility::GetInstance()->SelectByRange(ctxt->start, ctxt->end);
        editorQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 2 means JsAPI:selectByRange has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "selectByRange");
}

napi_value JsTextInputClientEngine::SelectByRangeSync(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("SelectByRangeSync");
    EditorEventInfo eventInfo = { std::chrono::system_clock::now(), EditorEvent::SELECT_BY_RANGE};
    editorQueue_.Push(eventInfo);
    editorQueue_.Wait(eventInfo);
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < 1 || JsUtil::GetType(env, argv[0]) != napi_object) {
        editorQueue_.Pop();
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "please check the params", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    auto ctxt = std::make_shared<SelectContext>();
    auto status = GetSelectRange(env, argv[0], ctxt);
    if (status != napi_ok) {
        editorQueue_.Pop();
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "failed to get start or end.", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    IMSA_HILOGD("start: %{public}d, end: %{public}d", ctxt->start, ctxt->end);
    int32_t ret = InputMethodAbility::GetInstance()->SelectByRange(ctxt->start, ctxt->end);
    editorQueue_.Pop();
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to select by range.", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::SelectByMovementSync(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in");
    EditorEventInfo eventInfo = { std::chrono::system_clock::now(), EditorEvent::SELECT_BY_MOVEMENT};
    editorQueue_.Push(eventInfo);
    editorQueue_.Wait(eventInfo);
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < 1 || JsUtil::GetType(env, argv[0]) != napi_object) {
        editorQueue_.Pop();
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "please check the params", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    auto ctxt = std::make_shared<SelectContext>();
    auto status = GetSelectMovement(env, argv[0], ctxt);
    if (status != napi_ok) {
        editorQueue_.Pop();
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "failed to get direction.", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    IMSA_HILOGD("direction: %{public}d", ctxt->direction);
    int32_t ret = InputMethodAbility::GetInstance()->SelectByMovement(ctxt->direction);
    editorQueue_.Pop();
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to select by movement.", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::SelectByMovement(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in");
    auto ctxt = std::make_shared<SelectContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        PARAM_CHECK_RETURN(env, valueType == napi_object, "movement", TYPE_OBJECT, napi_generic_failure);
        auto status = GetSelectMovement(env, argv[0], ctxt);
        if (status == napi_ok) {
            ctxt->info = { std::chrono::system_clock::now(), EditorEvent::SELECT_BY_MOVEMENT };
            editorQueue_.Push(ctxt->info);
        }
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status { return napi_ok; };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        editorQueue_.Wait(ctxt->info);
        int32_t code = InputMethodAbility::GetInstance()->SelectByMovement(ctxt->direction);
        editorQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 2 means JsAPI:selectByMovement has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "selectByMovement");
}

napi_value JsTextInputClientEngine::SendExtendAction(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SendExtendActionContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->action);
        if (status == napi_ok) {
            ctxt->info = { std::chrono::system_clock::now(), EditorEvent::SEND_EXTEND_ACTION };
            editorQueue_.Push(ctxt->info);
        }
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        editorQueue_.Wait(ctxt->info);
        int32_t code = InputMethodAbility::GetInstance()->SendExtendAction(ctxt->action);
        editorQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            return;
        }
        ctxt->SetErrorCode(code);
    };
    ctxt->SetAction(std::move(input));
    // 2 means JsAPI:sendExtendAction has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec, "sendExtendAction");
}

napi_value JsTextInputClientEngine::GetTextIndexAtCursor(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("GetTextIndexAtCursor");
    auto ctxt = std::make_shared<GetTextIndexAtCursorContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        ctxt->info = { std::chrono::system_clock::now(), EditorEvent::GET_TEXT_INDEX_AT_CURSOR };
        editorQueue_.Push(ctxt->info);
        return napi_ok;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        return napi_create_int32(env, ctxt->index, result);
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        editorQueue_.Wait(ctxt->info);
        int32_t code = InputMethodAbility::GetInstance()->GetTextIndexAtCursor(ctxt->index);
        editorQueue_.Pop();
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI:getTextIndexAtCursor has 1 params at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec, "getTextIndexAtCursor");
}

napi_value JsTextInputClientEngine::GetTextIndexAtCursorSync(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in");
    EditorEventInfo eventInfo = { std::chrono::system_clock::now(), EditorEvent::GET_TEXT_INDEX_AT_CURSOR};
    editorQueue_.Push(eventInfo);
    editorQueue_.Wait(eventInfo);
    int32_t index = 0;
    int32_t ret = InputMethodAbility::GetInstance()->GetTextIndexAtCursor(index);
    editorQueue_.Pop();
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to get text index at cursor.", TYPE_NONE);
    }
    return JsUtil::GetValue(env, index);
}

void JsTextInputClientEngine::PrintEditorQueueInfoIfTimeout(int64_t start, const EditorEventInfo &currentInfo)
{
    int64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    if (end - start >= MAX_WAIT_TIME) {
        EditorEventInfo frontInfo;
        auto ret = editorQueue_.GetFront(frontInfo);
        int64_t frontTime = duration_cast<microseconds>(frontInfo.timestamp.time_since_epoch()).count();
        int64_t currentTime = duration_cast<microseconds>(currentInfo.timestamp.time_since_epoch()).count();
        IMSA_HILOGW("ret:%{public}d,front[%{public}" PRId64 ",%{public}d],current[%{public}" PRId64 ",%{public}d]", ret,
            frontTime, static_cast<int32_t>(frontInfo.event), currentTime, static_cast<int32_t>(currentInfo.event));
    }
}
} // namespace MiscServices
} // namespace OHOS
