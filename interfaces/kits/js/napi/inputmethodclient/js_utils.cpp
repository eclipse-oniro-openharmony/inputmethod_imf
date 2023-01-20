/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#include "js_utils.h"

namespace OHOS {
namespace MiscServices {
const std::map<int32_t, int32_t> JsUtils::ERROR_CODE_MAP = {
    { ErrorCode::ERROR_STATUS_PERMISSION_DENIED, EXCEPTION_PERMISSION },
    { ErrorCode::ERROR_REMOTE_IME_DIED, EXCEPTION_IMENGINE },
    { ErrorCode::ERROR_REMOTE_CLIENT_DIED, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NOT_FOUND, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NULL_POINTER, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_NOT_IME_PACKAGE, EXCEPTION_SETTINGS },
    { ErrorCode::ERROR_IME_PACKAGE_DUPLICATED, EXCEPTION_SETTINGS },
    { ErrorCode::ERROR_SETTING_SAME_VALUE, EXCEPTION_SETTINGS },
    { ErrorCode::ERROR_NULL_POINTER, EXCEPTION_IMMS },
    { ErrorCode::ERROR_BAD_PARAMETERS, EXCEPTION_IMMS },
    { ErrorCode::ERROR_SERVICE_START_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_USER_NOT_STARTED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_USER_ALREADY_STARTED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_USER_NOT_UNLOCKED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_USER_NOT_LOCKED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_NOT_AVAILABLE, EXCEPTION_IMMS },
    { ErrorCode::ERROR_SECURITY_IME_NOT_AVAILABLE, EXCEPTION_IMMS },
    { ErrorCode::ERROR_TOKEN_CREATE_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_TOKEN_DESTROY_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_BIND_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_UNBIND_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_START_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_KBD_SHOW_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_KBD_HIDE_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_NOT_STARTED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_KBD_IS_OCCUPIED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_KBD_IS_NOT_SHOWING, EXCEPTION_IMMS },
    { ErrorCode::ERROR_EX_NULL_POINTER, EXCEPTION_IMMS },
    { ErrorCode::ERROR_PERSIST_CONFIG, EXCEPTION_CONFPERSIST },
    { ErrorCode::ERROR_PACKAGE_MANAGER, EXCEPTION_PACKAGEMANAGER },
    { ErrorCode::ERROR_EX_UNSUPPORTED_OPERATION, EXCEPTION_IMMS },
    { ErrorCode::ERROR_EX_SERVICE_SPECIFIC, EXCEPTION_IMMS },
    { ErrorCode::ERROR_EX_PARCELABLE, EXCEPTION_IMMS },
    { ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT, EXCEPTION_IMMS },
    { ErrorCode::ERROR_EX_ILLEGAL_STATE, EXCEPTION_IMMS },
    { ErrorCode::ERROR_EX_NETWORK_MAIN_THREAD, EXCEPTION_IMMS },
    { ErrorCode::ERROR_STATUS_UNKNOWN_ERROR, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_NO_MEMORY, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_INVALID_OPERATION, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_BAD_VALUE, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_BAD_TYPE, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_NAME_NOT_FOUND, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_ALREADY_EXISTS, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_DEAD_OBJECT, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_FAILED_TRANSACTION, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_BAD_INDEX, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_NOT_ENOUGH_DATA, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_WOULD_BLOCK, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_TIMED_OUT, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_FDS_NOT_ALLOWED, EXCEPTION_OTHERS },
    { ErrorCode::ERROR_STATUS_UNEXPECTED_NULL, EXCEPTION_OTHERS },
};

const std::map<int32_t, std::string> JsUtils::ERROR_CODE_CONVERT_MESSAGE_MAP = {
    { EXCEPTION_PERMISSION, "the permissions check fails." },
    { EXCEPTION_PARAMCHECK, "the parameters check fails." },
    { EXCEPTION_UNSUPPORTED, "call unsupported api." },
    { EXCEPTION_PACKAGEMANAGER, "package manager error." },
    { EXCEPTION_IMENGINE, "input method engine error." },
    { EXCEPTION_IMCLIENT, "input method client error." },
    { EXCEPTION_KEYEVENT, "key event processing error." },
    { EXCEPTION_CONFPERSIST, "configuration persisting error." },
    { EXCEPTION_CONTROLLER, "input method controller error." },
    { EXCEPTION_SETTINGS, "input method settings extension error." },
    { EXCEPTION_IMMS, "input method manager service error." },
    { EXCEPTION_OTHERS, "others error." },
};

const std::map<int32_t, std::string> JsUtils::PARAMETER_TYPE = {
    { TYPE_UNDEFINED, "napi_undefine." },
    { TYPE_NULL, "napi_null." },
    { TYPE_BOOLEAN, "napi_boolean." },
    { TYPE_NUMBER, "napi_number." },
    { TYPE_STRING, "napi_string." },
    { TYPE_SYMBOL, "napi_symbol." },
    { TYPE_OBJECT, "napi_object." },
    { TYPE_FUNCTION, "napi_function." },
    { TYPE_EXTERNAL, "napi_external." },
    { TYPE_BIGINT, "napi_bigint." },
};

void JsUtils::ThrowException(napi_env env, int32_t err, const std::string &msg, TypeCode type)
{
    std::string errMsg = ToMessage(err);
    napi_value error;
    napi_value code;
    napi_value message;
    if (type == TypeCode::TYPE_NONE) {
        errMsg = errMsg + msg;
        IMSA_HILOGE("THROW_PARAMETER_ERROR message: %{public}s", errMsg.c_str());
    } else {
        auto iter = PARAMETER_TYPE.find(type);
        if (iter != PARAMETER_TYPE.end()) {
            errMsg = errMsg + "The type of " + msg + " must be " + iter->second;
            IMSA_HILOGE("THROW_PARAMETER_TYPE_ERROR message: %{public}s", errMsg.c_str());
        }
    }
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &message));
    NAPI_CALL_RETURN_VOID(env, napi_create_error(env, nullptr, message, &error));
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, err, &code));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, error, "code", code));
    NAPI_CALL_RETURN_VOID(env, napi_throw(env, error));
}

napi_value JsUtils::ToError(napi_env env, int32_t code)
{
    IMSA_HILOGE("ToError start");
    napi_value errorObj;
    NAPI_CALL(env, napi_create_object(env, &errorObj));
    napi_value errorCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, Convert(code), &errorCode));
    napi_value errorMessage = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, ToMessage(Convert(code)).c_str(), NAPI_AUTO_LENGTH, &errorMessage));
    NAPI_CALL(env, napi_set_named_property(env, errorObj, "code", errorCode));
    NAPI_CALL(env, napi_set_named_property(env, errorObj, "message", errorMessage));
    IMSA_HILOGE("ToError end");
    return errorObj;
}

int32_t JsUtils::Convert(int32_t code)
{
    IMSA_HILOGI("Convert start");
    auto iter = ERROR_CODE_MAP.find(code);
    if (iter != ERROR_CODE_MAP.end()) {
        IMSA_HILOGE("ErrorCode: %{public}d", iter->second);
        return iter->second;
    }
    IMSA_HILOGI("Convert end");
    return ERROR_CODE_QUERY_FAILED;
}

const std::string JsUtils::ToMessage(int32_t code)
{
    IMSA_HILOGI("ToMessage start");
    auto iter = ERROR_CODE_CONVERT_MESSAGE_MAP.find(code);
    if (iter != ERROR_CODE_CONVERT_MESSAGE_MAP.end()) {
        IMSA_HILOGI("ErrorMessage: %{public}s", (iter->second).c_str());
        return iter->second;
    }
    return "error is out of definition.";
}

bool JsUtils::TraverseCallback(std::vector<std::shared_ptr<JSCallbackObject>> &vecCopy, size_t paramNum,
                                ArgsProvider argsProvider)
{
    bool isResult = false;
    bool isOnKeyEvent = false;
    for (const auto &item : vecCopy) {
        if (item->threadId_ != std::this_thread::get_id()) {
            continue;
        }

        napi_value args[MAX_ARGMENT_COUNT];
        if (!argsProvider(args, MAX_ARGMENT_COUNT, item)) {
            continue;
        }

        napi_value callback = nullptr;
        napi_value global = nullptr;
        napi_value result = nullptr;
        napi_get_reference_value(item->env_, item->callback_, &callback);
        if (callback != nullptr) {
            IMSA_HILOGD("callback is not nullptr");
            napi_get_global(item->env_, &global);
            napi_status callStatus = napi_call_function(item->env_, global, callback, paramNum, args, &result);
            if (callStatus != napi_ok) {
                IMSA_HILOGE(
                    "notify data change failed callStatus:%{public}d", callStatus);
                result = nullptr;
            }
        }

        if (result != nullptr && !isOnKeyEvent) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(item->env_, result, &valueType);
            if (valueType != napi_boolean) {
                continue;
            }
            napi_get_value_bool(item->env_, result, &isResult);
            if (isResult) {
                isOnKeyEvent = true;
            }
        }
    }
    return isOnKeyEvent;
}
} // namespace MiscServices
} // namespace OHOS