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
#include "js_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "string_ex.h"

namespace OHOS {
namespace MiscServices {
thread_local napi_ref JsTextInputClientEngine::TICRef_ = nullptr;
const std::string JsTextInputClientEngine::TIC_CLASS_NAME = "TextInputClient";

napi_value JsTextInputClientEngine::Init(napi_env env, napi_value info)
{
    IMSA_HILOGI("JsTextInputClientEngine init");
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
    };
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
        return JsUtils::GetValue(env, argv[0], ctxt->num);
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
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec);
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
        IMSA_HILOGE("JsTextInputClientEngine finalize");
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
    NAPI_ASSERT_BASE(env, status == napi_ok, "failed to get start value", status);

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
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec);
}

napi_value JsTextInputClientEngine::DeleteForward(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<DeleteForwardContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        return JsUtils::GetValue(env, argv[0], ctxt->length);
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, ctxt->isDeleteForward, result);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
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
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec);
}

napi_value JsTextInputClientEngine::DeleteBackward(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<DeleteBackwardContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        return JsUtils::GetValue(env, argv[0], ctxt->length);
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
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec);
}

napi_value JsTextInputClientEngine::InsertText(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<InsertTextContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        return JsUtils::GetValue(env, argv[0], ctxt->text);
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, ctxt->isInsertText, result);
        return status;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
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
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec);
}

napi_value JsTextInputClientEngine::GetForward(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<GetForwardContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        return JsUtils::GetValue(env, argv[0], ctxt->length);
    };
    auto output = [ctxt](napi_env env, napi_value *result) -> napi_status {
        napi_value data = GetResult(env, ctxt->text);
        *result = data;
        return napi_ok;
    };
    auto exec = [ctxt](AsyncCall::Context *ctx) {
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
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec);
}

napi_value JsTextInputClientEngine::GetBackward(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<GetBackwardContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        return JsUtils::GetValue(env, argv[0], ctxt->length);
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
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec);
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
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec);
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
        return GetSelectRange(env, argv[0], ctxt);
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
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec);
}

napi_value JsTextInputClientEngine::SelectByMovement(napi_env env, napi_callback_info info)
{
    IMSA_HILOGD("run in");
    auto ctxt = std::make_shared<SelectContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        PARAM_CHECK_RETURN(env, valueType == napi_object, "movement", TYPE_NUMBER, napi_generic_failure);
        return GetSelectMovement(env, argv[0], ctxt);
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
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec);
}

napi_value JsTextInputClientEngine::SendExtendAction(napi_env env, napi_callback_info info)
{
    auto ctxt = std::make_shared<SendExtendActionContext>();
    auto input = [ctxt](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        PARAM_CHECK_RETURN(env, argc > 0, "should 1 or 2 parameters!", TYPE_NONE, napi_generic_failure);
        return JsUtils::GetValue(env, argv[0], ctxt->action);
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
    AsyncCall asyncCall(env, info, ctxt, 2);
    return asyncCall.Call(env, exec);
}

napi_value JsTextInputClientEngine::GetTextIndexAtCursor(napi_env env, napi_callback_info info)
{
    IMSA_HILOGE("GetTextIndexAtCursor");
    auto ctxt = std::make_shared<GetTextIndexAtCursorContext>();
    auto input = [](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status { return napi_ok; };
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
    AsyncCall asyncCall(env, info, ctxt, 1);
    return asyncCall.Call(env, exec);
}
} // namespace MiscServices
} // namespace OHOS
