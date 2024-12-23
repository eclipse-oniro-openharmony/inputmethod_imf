/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "wm_common.h"

namespace OHOS {
namespace MiscServices {
#define ASYNC_POST(env, ctx) asyncCall.Post((env), (ctx), taskQueue_, __FUNCTION__)
using namespace std::chrono;
thread_local napi_ref JsTextInputClientEngine::TICRef_ = nullptr;
const std::string JsTextInputClientEngine::TIC_CLASS_NAME = "TextInputClient";
constexpr int32_t MAX_WAIT_TIME = 5000;
constexpr int32_t MAX_WAIT_TIME_PRIVATE_COMMAND = 2000;
constexpr int32_t MAX_WAIT_TIME_MESSAGE_HANDLER = 2000;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_ONE = 1;
std::shared_ptr<AsyncCall::TaskQueue> JsTextInputClientEngine::taskQueue_ = std::make_shared<AsyncCall::TaskQueue>();
BlockQueue<PrivateCommandInfo> JsTextInputClientEngine::privateCommandQueue_{ MAX_WAIT_TIME_PRIVATE_COMMAND };
BlockQueue<MessageHandlerInfo> JsTextInputClientEngine::messageHandlerQueue_{ MAX_WAIT_TIME_MESSAGE_HANDLER };
uint32_t JsTextInputClientEngine::traceId_{ 0 };
napi_value JsTextInputClientEngine::Init(napi_env env, napi_value info)
{
    IMSA_HILOGD("JsTextInputClientEngine init");
    napi_property_descriptor properties[] = { DECLARE_NAPI_FUNCTION("sendKeyFunction", SendKeyFunction),
        DECLARE_NAPI_FUNCTION("deleteForward", DeleteForward), DECLARE_NAPI_FUNCTION("deleteBackward", DeleteBackward),
        DECLARE_NAPI_FUNCTION("insertText", InsertText), DECLARE_NAPI_FUNCTION("getForward", GetForward),
        DECLARE_NAPI_FUNCTION("getBackward", GetBackward),
        DECLARE_NAPI_FUNCTION("getEditorAttribute", GetEditorAttribute),
        DECLARE_NAPI_FUNCTION("getTextIndexAtCursor", GetTextIndexAtCursor),
        DECLARE_NAPI_FUNCTION("moveCursor", MoveCursor), DECLARE_NAPI_FUNCTION("selectByRange", SelectByRange),
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
        DECLARE_NAPI_FUNCTION("sendPrivateCommand", SendPrivateCommand),
        DECLARE_NAPI_FUNCTION("getCallingWindowInfo", GetCallingWindowInfo),
        DECLARE_NAPI_FUNCTION("setPreviewText", SetPreviewText),
        DECLARE_NAPI_FUNCTION("setPreviewTextSync", SetPreviewTextSync),
        DECLARE_NAPI_FUNCTION("finishTextPreview", FinishTextPreview),
        DECLARE_NAPI_FUNCTION("finishTextPreviewSync", FinishTextPreviewSync),
        DECLARE_NAPI_FUNCTION("sendMessage", SendMessage),
        DECLARE_NAPI_FUNCTION("recvMessage", RecvMessage), };
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
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_number, "direction type must be number!",
            TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->num);
        // 1 means least param num.
        PARAM_CHECK_RETURN(env, ctxt->num >= 0, "direction should be not less than 0!", TYPE_NONE,
            napi_generic_failure);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t code = InputMethodAbility::GetInstance()->MoveCursor(ctxt->num);
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
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::MoveCursorSync(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t direction = 0;
    // 1 means least param num.
    PARAM_CHECK_RETURN(env, argc >= 1, "at least one parameter is required!", TYPE_NONE, HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_number, "direction must be number!", TYPE_NUMBER,
        HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetValue(env, argv[0], direction), "direction covert failed!", TYPE_NONE,
        HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, direction >= 0, "direction should be no less than 0!", TYPE_NONE,
        HandleParamCheckFailure(env));
    IMSA_HILOGD("moveCursor , direction: %{public}d", direction);
    int32_t ret = InputMethodAbility::GetInstance()->MoveCursor(direction);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to move cursor!", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::JsConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    JsTextInputClientEngine *clientObject = new (std::nothrow) JsTextInputClientEngine();
    if (clientObject == nullptr) {
        IMSA_HILOGE("clientObject is nullptr!");
        napi_value result = nullptr;
        napi_get_null(env, &result);
        return result;
    }
    auto finalize = [](napi_env env, void *data, void *hint) {
        IMSA_HILOGD("finalize.");
        auto *objInfo = reinterpret_cast<JsTextInputClientEngine *>(data);
        if (objInfo != nullptr) {
            delete objInfo;
        }
    };
    napi_status status = napi_wrap(env, thisVar, clientObject, finalize, nullptr, nullptr);
    if (status != napi_ok) {
        IMSA_HILOGE("failed to wrap: %{public}d!", status);
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
        IMSA_HILOGE("failed to get reference value!");
        return nullptr;
    }
    if (napi_new_instance(env, cons, 0, nullptr, &instance) != napi_ok) {
        IMSA_HILOGE("failed to new instance!");
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

napi_status JsTextInputClientEngine::GetSelectRange(napi_env env, napi_value argv, std::shared_ptr<SelectContext> ctxt)
{
    napi_status status = napi_generic_failure;
    napi_value napiValue = nullptr;
    status = napi_get_named_property(env, argv, "start", &napiValue);
    PARAM_CHECK_RETURN(env, status == napi_ok, "start of range cannot empty and must be number.", TYPE_NONE, status);
    status = JsUtils::GetValue(env, napiValue, ctxt->start);
    CHECK_RETURN(status == napi_ok, "failed to get start value!", status);

    status = napi_get_named_property(env, argv, "end", &napiValue);
    PARAM_CHECK_RETURN(env, status == napi_ok, "end of range cannot empty and must be number.", TYPE_NONE, status);
    status = JsUtils::GetValue(env, napiValue, ctxt->end);
    if (status != napi_ok) {
        IMSA_HILOGE("failed to get end value!");
    }
    return status;
}

napi_status JsTextInputClientEngine::GetSelectMovement(napi_env env, napi_value argv,
    std::shared_ptr<SelectContext> ctxt)
{
    napi_status status = napi_generic_failure;
    napi_value napiValue = nullptr;
    status = napi_get_named_property(env, argv, "direction", &napiValue);
    PARAM_CHECK_RETURN(env, status == napi_ok, "direction must be exist!", TYPE_NONE, status);
    status = JsUtils::GetValue(env, napiValue, ctxt->direction);
    if (status != napi_ok) {
        IMSA_HILOGE("failed to get direction value!");
    }
    PARAM_CHECK_RETURN(env, status == napi_ok, "direction type must be Direction!", TYPE_NONE, status);
    return status;
}

napi_value JsTextInputClientEngine::SendKeyFunction(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SendKeyFunctionContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        napi_status ret = JsUtils::GetValue(env, argv[0], ctxt->action);
        PARAM_CHECK_RETURN(env, ret == napi_ok, "action type must be number!", TYPE_NONE, napi_generic_failure);
        return napi_ok;
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
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::SendPrivateCommand(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SendPrivateCommandContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        napi_status status = JsUtils::GetValue(env, argv[0], ctxt->privateCommand);
        CHECK_RETURN(status == napi_ok,
            "commandData covert failed, type must be Record<string, CommandDataType>", status);
        PARAM_CHECK_RETURN(env, TextConfig::IsPrivateCommandValid(ctxt->privateCommand),
            "commandData size limit 32KB, count limit 5.", TYPE_NONE, napi_generic_failure);
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
    // 1 means JsAPI:SendPrivateCommand has 1 param at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::DeleteForwardSync(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JS_DeleteForwardSync", GenerateTraceId());
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t length = 0;
    // 1 means least param num.
    PARAM_CHECK_RETURN(env, argc >= 1, "at least one parameter is required!", TYPE_NONE, HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_number, "length must be number!", TYPE_NUMBER,
        HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetValue(env, argv[0], length), "length covert failed", TYPE_NONE,
        HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, length >= 0, "length should not less than 0!", TYPE_NONE, HandleParamCheckFailure(env));
    IMSA_HILOGD("delete forward, length: %{public}d.", length);
    int32_t ret = InputMethodAbility::GetInstance()->DeleteForward(length);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to delete forward", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::DeleteForward(napi_env env, napi_callback_info info)
{
    auto traceId = GenerateTraceId();
    InputMethodSyncTrace tracer("JS_DeleteForward_Start", traceId);
    auto ctxt = std::make_shared<DeleteForwardContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_number, "length type must be number!",
            TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->length);
        PARAM_CHECK_RETURN(env, ctxt->length >= 0, "length should no less than 0!", TYPE_NONE, napi_generic_failure);
        return status;
    };
    auto output = [ctxt, traceId](napi_env env, napi_value *result) -> napi_status {
        InputMethodSyncTrace tracer("JS_DeleteForward_Complete", traceId);
        napi_status status = napi_get_boolean(env, ctxt->isDeleteForward, result);
        return status;
    };
    auto exec = [ctxt, traceId](AsyncCall::Context *ctx) {
        InputMethodSyncTrace tracer("JS_DeleteForward_Exec", traceId);
        int32_t code = InputMethodAbility::GetInstance()->DeleteForward(ctxt->length);
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
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::DeleteBackwardSync(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t length = 0;
    // 1 means least param num.
    PARAM_CHECK_RETURN(env, argc >= 1, "at least one parameter is required!", TYPE_NONE, HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_number, "length must be number!", TYPE_NUMBER,
        HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetValue(env, argv[0], length), "length covert failed!", TYPE_NONE,
        HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, length >= 0, "length should no less than 0!", TYPE_NONE, HandleParamCheckFailure(env));
    IMSA_HILOGD("delete backward, length: %{public}d.", length);
    int32_t ret = InputMethodAbility::GetInstance()->DeleteBackward(length);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to delete backward", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::DeleteBackward(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<DeleteBackwardContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_number, "param length type must be number!",
            TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->length);
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, ctxt->isDeleteBackward, result);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t code = InputMethodAbility::GetInstance()->DeleteBackward(ctxt->length);
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
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::InsertText(napi_env env, napi_callback_info info)
{
    auto traceId = GenerateTraceId();
    InputMethodSyncTrace tracer("JS_InsertText_Start", traceId);
    auto ctxt = std::make_shared<InsertTextContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_string, "text type must be string",
            TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->text);
        return status;
    };
    auto output = [ctxt, traceId](napi_env env, napi_value *result) -> napi_status {
        InputMethodSyncTrace tracer("JS_InsertText_Complete", traceId);
        napi_status status = napi_get_boolean(env, ctxt->isInsertText, result);
        return status;
    };
    auto exec = [ctxt, traceId](AsyncCall::Context *ctx) {
        InputMethodSyncTrace tracer("JS_InsertText_Exec", traceId);
        int32_t code = InputMethodAbility::GetInstance()->InsertText(ctxt->text);
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
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::InsertTextSync(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JS_InsertTextSync", GenerateTraceId());
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    std::string text;
    // 1 means least param num.
    PARAM_CHECK_RETURN(env, argc >= 1, "at least one parameter is required!", TYPE_NONE, HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_string, "text must be string!", TYPE_STRING,
        HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetValue(env, argv[0], text), "text covert failed!", TYPE_NONE,
        HandleParamCheckFailure(env));
    int32_t ret = InputMethodAbility::GetInstance()->InsertText(text);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to insert text!", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::GetForwardSync(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JS_GetForwardSync", GenerateTraceId());
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t length = 0;
    // 1 means least param num.
    PARAM_CHECK_RETURN(env, argc >= 1, "at least one parameter is required!", TYPE_NONE, HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_number, "length must be string!", TYPE_NUMBER,
        HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetValue(env, argv[0], length), "length covert failed!", TYPE_NONE,
        HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, length >= 0, "length should no less than 0!", TYPE_NONE, HandleParamCheckFailure(env));
    IMSA_HILOGD("get forward, length: %{public}d.", length);
    std::u16string text;
    int32_t ret = InputMethodAbility::GetInstance()->GetTextBeforeCursor(length, text);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to get forward!", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    napi_value result = nullptr;
    auto status = JsUtils::GetValue(env, Str16ToStr8(text), result);
    CHECK_RETURN(status == napi_ok, "GetValue failed", JsUtil::Const::Null(env));
    return result;
}

napi_value JsTextInputClientEngine::GetForward(napi_env env, napi_callback_info info)
{
    auto traceId = GenerateTraceId();
    InputMethodSyncTrace tracer("JS_GetForward_Start", traceId);
    auto ctxt = std::make_shared<GetForwardContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_number, "length type must be number!",
            TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->length);
        return status;
    };
    auto output = [ctxt, traceId](napi_env env, napi_value *result) -> napi_status {
        InputMethodSyncTrace tracer("JS_GetForward_Complete", traceId);
        napi_value data = GetResult(env, ctxt->text);
        *result = data;
        return napi_ok;
    };
    auto exec = [ctxt, traceId](AsyncCall::Context *ctx) {
        InputMethodSyncTrace tracer("JS_GetForward_Exec", traceId);
        std::u16string temp;
        int32_t code = InputMethodAbility::GetInstance()->GetTextBeforeCursor(ctxt->length, temp);
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
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::GetBackwardSync(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t length = 0;
    // 1 means least param num.
    PARAM_CHECK_RETURN(env, argc >= 1, "at least one parameter is required!", TYPE_NONE, HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_number, "length must be number!", TYPE_NUMBER,
        HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetValue(env, argv[0], length), "length covert failed!", TYPE_NONE,
        HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, length >= 0, "length should not less than 0!", TYPE_NONE, HandleParamCheckFailure(env));
    IMSA_HILOGD("get backward, length: %{public}d.", length);
    std::u16string text;
    int32_t ret = InputMethodAbility::GetInstance()->GetTextAfterCursor(length, text);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to get backward!", TYPE_NONE);
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
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_number, "length type must be number!",
            TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->length);
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_value data = GetResult(env, ctxt->text);
        *result = data;
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        std::u16string temp;
        int32_t code = InputMethodAbility::GetInstance()->GetTextAfterCursor(ctxt->length, temp);
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
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::GetEditorAttributeSync(napi_env env, napi_callback_info info)
{
    TextTotalConfig config;
    int32_t ret = InputMethodAbility::GetInstance()->GetTextConfig(config);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to get text config: %{public}d!", ret);
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_IMCLIENT, "failed to get text config!", TYPE_NONE);
    }
    IMSA_HILOGD("inputPattern: %{public}d, enterKeyType: %{public}d, isTextPreviewSupported: %{public}d.",
        config.inputAttribute.inputPattern, config.inputAttribute.enterKeyType,
        config.inputAttribute.isTextPreviewSupported);
    return JsInputAttribute::Write(env, config.inputAttribute);
}

napi_value JsTextInputClientEngine::GetEditorAttribute(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<GetEditorAttributeContext>();
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        *result = JsInputAttribute::Write(env, ctxt->inputAttribute);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        TextTotalConfig config;
        int32_t ret = InputMethodAbility::GetInstance()->GetTextConfig(config);
        ctxt->inputAttribute = config.inputAttribute;
        if (ret == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            IMSA_HILOGD("inputPattern: %{public}d, enterKeyType: %{public}d, isTextPreviewSupported: %{public}d",
                config.inputAttribute.inputPattern, config.inputAttribute.enterKeyType,
                config.inputAttribute.isTextPreviewSupported);
        } else {
            IMSA_HILOGE("failed to get text config: %{public}d!", ret);
            ctxt->SetErrorCode(IMFErrorCode::EXCEPTION_IMCLIENT);
            ctxt->SetErrorMessage("failed to get text config!");
        }
    };
    ctxt->SetAction(nullptr, std::move(output));
    // 1 means JsAPI:getEditorAttribute has 1 param at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::SelectByRange(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in");
    auto ctxt = std::make_shared<SelectContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        PARAM_CHECK_RETURN(env, valueType == napi_object, "range type must be Range!", TYPE_NONE,
            napi_generic_failure);
        auto status = GetSelectRange(env, argv[0], ctxt);
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status { return napi_ok; };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t code = InputMethodAbility::GetInstance()->SelectByRange(ctxt->start, ctxt->end);
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
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::SelectByRangeSync(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("SelectByRangeSync");
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    PARAM_CHECK_RETURN(env, argc >= 1, "at least one parameter is required!", TYPE_NONE, HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_object, "range type must be Range!", TYPE_NONE,
        HandleParamCheckFailure(env));
    auto ctxt = std::make_shared<SelectContext>();
    auto status = GetSelectRange(env, argv[0], ctxt);
    if (status != napi_ok) {
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK,
            "failed to get start or end, should have start and end number!", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    IMSA_HILOGD("start: %{public}d, end: %{public}d.", ctxt->start, ctxt->end);
    int32_t ret = InputMethodAbility::GetInstance()->SelectByRange(ctxt->start, ctxt->end);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to select by range!", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::SelectByMovementSync(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in");
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    PARAM_CHECK_RETURN(env, argc >= 1, "at least one parameter is required!", TYPE_NONE, HandleParamCheckFailure(env));
    PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[0]) == napi_object, "movement type must be Movement!",
        TYPE_NONE, HandleParamCheckFailure(env));
    auto ctxt = std::make_shared<SelectContext>();
    auto status = GetSelectMovement(env, argv[0], ctxt);
    if (status != napi_ok) {
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, "direction covert failed!", TYPE_NONE);
        return JsUtil::Const::Null(env);
    }
    IMSA_HILOGD("direction: %{public}d.", ctxt->direction);
    int32_t ret = InputMethodAbility::GetInstance()->SelectByMovement(ctxt->direction);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to select by movement!", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::SelectByMovement(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in");
    auto ctxt = std::make_shared<SelectContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        PARAM_CHECK_RETURN(env, valueType == napi_object, "movement type must be Movement!", TYPE_NONE,
            napi_generic_failure);
        auto status = GetSelectMovement(env, argv[0], ctxt);
        return status;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status { return napi_ok; };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t code = InputMethodAbility::GetInstance()->SelectByMovement(ctxt->direction);
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
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::SendExtendAction(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SendExtendActionContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "at least one parameter is required!", TYPE_NONE, napi_generic_failure);
        auto status = JsUtils::GetValue(env, argv[0], ctxt->action);
        if (status != napi_ok) {
            ctxt->SetErrorMessage("action must be number and should in ExtendAction");
        }
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t code = InputMethodAbility::GetInstance()->SendExtendAction(ctxt->action);
        if (code == ErrorCode::NO_ERROR) {
            ctxt->SetState(napi_ok);
            return;
        }
        ctxt->SetErrorCode(code);
    };
    ctxt->SetAction(std::move(input));
    // 2 means JsAPI:sendExtendAction has 2 params at most.
    AsyncCall asyncCall(env, info, ctxt, 2);
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::GetTextIndexAtCursor(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("GetTextIndexAtCursor");
    auto ctxt = std::make_shared<GetTextIndexAtCursorContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        return napi_create_int32(env, ctxt->index, result);
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t code = InputMethodAbility::GetInstance()->GetTextIndexAtCursor(ctxt->index);
        if (code == ErrorCode::NO_ERROR) {
            ctxt->status = napi_ok;
            ctxt->SetState(ctxt->status);
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 1 means JsAPI:getTextIndexAtCursor has 1 param at most.
    AsyncCall asyncCall(env, info, ctxt, 1);
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::SetPreviewText(napi_env env, napi_callback_info info)
{
    auto traceId = GenerateTraceId();
    InputMethodSyncTrace tracer("JS_SetPreviewText_Start", traceId);
    IMSA_HILOGD("JsTextInputClientEngine in");
    auto ctxt = std::make_shared<SetPreviewTextContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (GetPreviewTextParam(env, argc, argv, ctxt->text, ctxt->range) != napi_ok) {
            return napi_generic_failure;
        }
        return napi_ok;
    };
    auto output = [ctxt, traceId](napi_env env, napi_value *result) -> napi_status {
        InputMethodSyncTrace tracer("JS_SetPreviewText_Complete", traceId);
        return napi_ok;
    };
    auto exec = [ctxt, traceId](AsyncCall::Context *ctx) {
        InputMethodSyncTrace tracer("JS_SetPreviewText_Exec", traceId);
        int32_t code = InputMethodAbility::GetInstance()->SetPreviewText(ctxt->text, ctxt->range);
        if (code == ErrorCode::NO_ERROR) {
            IMSA_HILOGD("exec setPreviewText success");
            ctxt->SetState(napi_ok);
        } else if (code == ErrorCode::ERROR_INVALID_RANGE) {
            ctxt->SetErrorCode(code);
            ctxt->SetErrorMessage("range should be included in preview text range, otherwise should be included in "
                                  "total text range!");
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 2 means JsAPI:setPreviewText needs 2 params at most
    AsyncCall asyncCall(env, info, ctxt, 2);
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::SetPreviewTextSync(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JS_SetPreviewTextSync", GenerateTraceId());
    IMSA_HILOGD("start.");
    // 2 means JsAPI:setPreviewText needs 2 params at most
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    std::string text;
    Range range;
    if (GetPreviewTextParam(env, argc, argv, text, range) != napi_ok) {
        return JsUtil::Const::Null(env);
    }
    int32_t ret = InputMethodAbility::GetInstance()->SetPreviewText(text, range);
    if (ret == ErrorCode::ERROR_INVALID_RANGE) {
        JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK,
            "range should be included in preview text range, otherwise should be included in total text range",
            TYPE_NONE);
    } else if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to set preview text!", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::FinishTextPreview(napi_env env, napi_callback_info info)
{
    auto traceId = GenerateTraceId();
    InputMethodSyncTrace tracer("JS_FinishTextPreview_Start", traceId);
    IMSA_HILOGD("start.");
    auto ctxt = std::make_shared<FinishTextPreviewContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto output = [ctxt, traceId](napi_env env, napi_value *result) -> napi_status {
        InputMethodSyncTrace tracer("JS_FinishTextPreview_Complete", traceId);
        return napi_ok;
    };
    auto exec = [ctxt, traceId](AsyncCall::Context *ctx) {
        InputMethodSyncTrace tracer("JS_FinishTextPreview_Exec", traceId);
        int32_t code = InputMethodAbility::GetInstance()->FinishTextPreview(false);
        if (code == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec finishTextPreview success.");
            ctxt->SetState(napi_ok);
        } else {
            ctxt->SetErrorCode(code);
        }
    };
    ctxt->SetAction(std::move(input), std::move(output));
    // 0 means JsAPI:finishTextPreview needs no param
    AsyncCall asyncCall(env, info, ctxt, 0);
    return ASYNC_POST(env, exec);
}

napi_value JsTextInputClientEngine::FinishTextPreviewSync(napi_env env, napi_callback_info info)
{
    InputMethodSyncTrace tracer("JS_FinishTextPreviewSync", GenerateTraceId());
    IMSA_HILOGD("JsTextInputClientEngine in");
    int32_t ret = InputMethodAbility::GetInstance()->FinishTextPreview(false);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to finish text preview!", TYPE_NONE);
    }
    return JsUtil::Const::Null(env);
}

napi_value JsTextInputClientEngine::GetTextIndexAtCursorSync(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("start.");
    int32_t index = 0;
    int32_t ret = InputMethodAbility::GetInstance()->GetTextIndexAtCursor(index);
    if (ret != ErrorCode::NO_ERROR) {
        JsUtils::ThrowException(env, JsUtils::Convert(ret), "failed to get text index at cursor!", TYPE_NONE);
    }
    return JsUtil::GetValue(env, index);
}

napi_value JsTextInputClientEngine::GetCallingWindowInfo(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("start.");
    auto ctxt = std::make_shared<GetCallingWindowInfoContext>();
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        *result = JsCallingWindowInfo::Write(env, ctxt->windowInfo);
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
        int32_t ret = InputMethodAbility::GetInstance()->GetCallingWindowInfo(ctxt->windowInfo);
        if (ret == ErrorCode::NO_ERROR) {
            IMSA_HILOGI("exec GetCallingWindowInfo success.");
            ctxt->SetState(napi_ok);
            return;
        }
        ctxt->SetErrorCode(ret);
    };
    ctxt->SetAction(nullptr, std::move(output));
    // 0 means JsAPI:getCallingWindowInfo needs no parameter.
    AsyncCall asyncCall(env, info, ctxt, 0);
    return ASYNC_POST(env, exec);
}

napi_status JsTextInputClientEngine::GetPreviewTextParam(napi_env env, size_t argc, napi_value *argv,
    std::string &text, Range &range)
{
    // 2 means JsAPI:setPreviewText needs 2 params at least.
    PARAM_CHECK_RETURN(env, argc >= 2, "at least two parameters is required!", TYPE_NONE, napi_generic_failure);
    PARAM_CHECK_RETURN(env, JsUtil::GetValue(env, argv[0], text), "text covert failed, must be string!",
        TYPE_NONE, napi_generic_failure);
    PARAM_CHECK_RETURN(env, JsUtil::GetType(env, argv[1]) == napi_object, "range type must be Range!", TYPE_NONE,
        napi_generic_failure);
    PARAM_CHECK_RETURN(env, JsRange::Read(env, argv[1], range),
        "range covert failed, the range should have numbers start and end", TYPE_NONE, napi_generic_failure);
    return napi_ok;
}

napi_value JsRect::Write(napi_env env, const Rosen::Rect &nativeObject)
{
    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);
    bool ret = JsUtil::Object::WriteProperty(env, jsObject, "left", nativeObject.posX_);
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "top", nativeObject.posY_);
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "width", nativeObject.width_);
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "height", nativeObject.height_);
    return ret ? jsObject : JsUtil::Const::Null(env);
}

bool JsRect::Read(napi_env env, napi_value jsObject, Rosen::Rect &nativeObject)
{
    auto ret = JsUtil::Object::ReadProperty(env, jsObject, "left", nativeObject.posX_);
    ret = ret && JsUtil::Object::ReadProperty(env, jsObject, "top", nativeObject.posY_);
    ret = ret && JsUtil::Object::ReadProperty(env, jsObject, "width", nativeObject.width_);
    ret = ret && JsUtil::Object::ReadProperty(env, jsObject, "height", nativeObject.height_);
    return ret;
}

napi_value JsCallingWindowInfo::Write(napi_env env, const CallingWindowInfo &nativeObject)
{
    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);
    bool ret = JsUtil::Object::WriteProperty(env, jsObject, "rect", JsRect::Write(env, nativeObject.rect));
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "status", static_cast<uint32_t>(nativeObject.status));
    return ret ? jsObject : JsUtil::Const::Null(env);
}

bool JsCallingWindowInfo::Read(napi_env env, napi_value object, CallingWindowInfo &nativeObject)
{
    napi_value rectObject = nullptr;
    napi_get_named_property(env, object, "rect", &rectObject);
    auto ret = JsRect::Read(env, rectObject, nativeObject.rect);
    uint32_t status = 0;
    ret = ret && JsUtil::Object::ReadProperty(env, object, "status", status);
    nativeObject.status = static_cast<Rosen::WindowStatus>(status);
    return ret;
}

napi_value JsTextInputClientEngine::HandleParamCheckFailure(napi_env env)
{
    return JsUtil::Const::Null(env);
}

napi_value JsRange::Write(napi_env env, const Range &nativeObject)
{
    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);
    bool ret = JsUtil::Object::WriteProperty(env, jsObject, "start", nativeObject.start);
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "end", nativeObject.end);
    return ret ? jsObject : JsUtil::Const::Null(env);
}

bool JsRange::Read(napi_env env, napi_value jsObject, Range &nativeObject)
{
    auto ret = JsUtil::Object::ReadProperty(env, jsObject, "start", nativeObject.start);
    ret = ret && JsUtil::Object::ReadProperty(env, jsObject, "end", nativeObject.end);
    return ret;
}

napi_value JsInputAttribute::Write(napi_env env, const InputAttribute &nativeObject)
{
    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);
    auto ret = JsUtil::Object::WriteProperty(env, jsObject, "inputPattern", nativeObject.inputPattern);
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "enterKeyType", nativeObject.enterKeyType);
    ret = ret &&
          JsUtil::Object::WriteProperty(env, jsObject, "isTextPreviewSupported", nativeObject.isTextPreviewSupported);
    // not care write bundleName fail
    JsUtil::Object::WriteProperty(env, jsObject, "bundleName", nativeObject.bundleName);
    return ret ? jsObject : JsUtil::Const::Null(env);
}

bool JsInputAttribute::Read(napi_env env, napi_value jsObject, InputAttribute &nativeObject)
{
    auto ret = JsUtil::Object::ReadProperty(env, jsObject, "inputPattern", nativeObject.inputPattern);
    ret = ret && JsUtil::Object::ReadProperty(env, jsObject, "enterKeyType", nativeObject.enterKeyType);
    ret = ret &&
          JsUtil::Object::ReadProperty(env, jsObject, "isTextPreviewSupported", nativeObject.isTextPreviewSupported);
    // not care read bundleName fail
    JsUtil::Object::ReadProperty(env, jsObject, "bundleName", nativeObject.bundleName);
    return ret;
}

napi_value JsTextInputClientEngine::SendMessage(napi_env env, napi_callback_info info)
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
        int32_t code = InputMethodAbility::GetInstance()->SendMessage(ctxt->arrayBuffer);
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
    return asyncCall.Call(env, exec, "imaSendMessage");
}

napi_value JsTextInputClientEngine::RecvMessage(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    if (argc < 0) {
        IMSA_HILOGE("RecvMessage failed! argc abnormal.");
        return nullptr;
    }
    void *native = nullptr;
    auto status = napi_unwrap(env, thisVar, &native);
    CHECK_RETURN((status == napi_ok && native != nullptr), "napi_unwrap failed!", nullptr);
    auto inputClient = reinterpret_cast<JsTextInputClientEngine *>(native);
    if (inputClient == nullptr) {
        IMSA_HILOGI("Unwrap js object self is nullptr.");
        return nullptr;
    }
    if (argc == 0) {
        IMSA_HILOGI("RecvMessage off.");
        InputMethodAbility::GetInstance()->RegisterMsgHandler();
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
        "Get onMessage property failed!", nullptr);
    CHECK_RETURN(JsUtil::GetType(env, onTerminated) == napi_function, "onTerminated is not napi_function!", nullptr);

    std::shared_ptr<MsgHandlerCallbackInterface> callback =
        std::make_shared<JsTextInputClientEngine::JsMessageHandler>(env, onTerminated, onMessage);
    InputMethodAbility::GetInstance()->RegisterMsgHandler(callback);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

int32_t JsTextInputClientEngine::JsMessageHandler::OnTerminated()
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
	eventHandler->PostTask(task, "IMA_MsgHandler_OnTerminated", 0, AppExecFwk::EventQueue::Priority::VIP);
    return ErrorCode::NO_ERROR;
}

int32_t JsTextInputClientEngine::JsMessageHandler::OnMessage(const ArrayBuffer &arrayBuffer)
{
    std::lock_guard<decltype(callbackObjectMutex_)> lock(callbackObjectMutex_);
    if (jsMessageHandler_ == nullptr) {
        IMSA_HILOGE("MessageHandler was not regist!.");
        return ErrorCode::ERROR_MSG_HANDLER_NOT_REGIST;
    }
    auto eventHandler = jsMessageHandler_->GetEventHandler();
    if (eventHandler == nullptr) {
        IMSA_HILOGI("EventHandler is nullptr!.");
        return ErrorCode::ERROR_NULL_POINTER;
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
            if (JsUtils::GetMessageHandlerCallbackParam(argv, jsCallbackObject, arrayBuffer) != napi_ok) {
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
} // namespace MiscServices
} // namespace OHOS