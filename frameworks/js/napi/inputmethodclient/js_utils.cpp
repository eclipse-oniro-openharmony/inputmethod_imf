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

#include "js_utils.h"

namespace OHOS {
namespace MiscServices {
constexpr int32_t STR_MAX_LENGTH = 4096;
constexpr size_t STR_TAIL_LENGTH = 1;
constexpr size_t ARGC_MAX = 6;
const std::map<int32_t, int32_t> JsUtils::ERROR_CODE_MAP = {
    { ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED, EXCEPTION_CONTROLLER },
    { ErrorCode::ERROR_STATUS_PERMISSION_DENIED, EXCEPTION_PERMISSION },
    { ErrorCode::ERROR_REMOTE_CLIENT_DIED, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NOT_FOUND, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NULL_POINTER, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NOT_FOCUSED, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NOT_EDITABLE, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NULL_POINTER, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NOT_BOUND, EXCEPTION_DETACHED },
    { ErrorCode::ERROR_CLIENT_ADD_FAILED, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_NULL_POINTER, EXCEPTION_IMMS },
    { ErrorCode::ERROR_BAD_PARAMETERS, EXCEPTION_IMMS },
    { ErrorCode::ERROR_SERVICE_START_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_START_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_KBD_SHOW_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_KBD_HIDE_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_NOT_STARTED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_EX_NULL_POINTER, EXCEPTION_IMMS },
    { ErrorCode::ERROR_PERSIST_CONFIG, EXCEPTION_CONFPERSIST },
    { ErrorCode::ERROR_PACKAGE_MANAGER, EXCEPTION_PACKAGEMANAGER },
    { ErrorCode::ERROR_EX_UNSUPPORTED_OPERATION, EXCEPTION_IMMS },
    { ErrorCode::ERROR_EX_SERVICE_SPECIFIC, EXCEPTION_IMMS },
    { ErrorCode::ERROR_EX_PARCELABLE, EXCEPTION_IMMS },
    { ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT, EXCEPTION_IMMS },
    { ErrorCode::ERROR_EX_ILLEGAL_STATE, EXCEPTION_IMMS },
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
    { EXCEPTION_DETACHED, "input method not attached." },
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
        IMSA_HILOGE("THROW_ERROR message: %{public}s", errMsg.c_str());
    } else {
        auto iter = PARAMETER_TYPE.find(type);
        if (iter != PARAMETER_TYPE.end()) {
            errMsg = errMsg + "The type of " + msg + " must be " + iter->second;
            IMSA_HILOGE("THROW_ERROR message: %{public}s", errMsg.c_str());
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

bool JsUtils::TraverseCallback(
    const std::vector<std::shared_ptr<JSCallbackObject>> &vecCopy, size_t paramNum, ArgsProvider argsProvider)
{
    bool isResult = false;
    bool isOnKeyEvent = false;
    for (const auto &item : vecCopy) {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(item->env_, &scope);
        if (item->threadId_ != std::this_thread::get_id()) {
            napi_close_handle_scope(item->env_, scope);
            continue;
        }
        napi_value args[MAX_ARGMENT_COUNT];
        if (!argsProvider(args, MAX_ARGMENT_COUNT, item)) {
            napi_close_handle_scope(item->env_, scope);
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
                IMSA_HILOGE("notify data change failed callStatus:%{public}d", callStatus);
                result = nullptr;
            }
        }
        if (result != nullptr && !isOnKeyEvent) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(item->env_, result, &valueType);
            if (valueType != napi_boolean) {
                continue;
            }
            GetValue(item->env_, result, isResult);
            if (isResult) {
                isOnKeyEvent = true;
            }
        }
        napi_close_handle_scope(item->env_, scope);
    }
    return isOnKeyEvent;
}

bool JsUtils::Equals(napi_env env, napi_value value, napi_ref copy, std::thread::id threadId)
{
    if (copy == nullptr) {
        return value == nullptr;
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

void *JsUtils::GetNativeSelf(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_MAX;
    void *native = nullptr;
    napi_value self = nullptr;
    napi_value argv[ARGC_MAX] = { nullptr };
    napi_status status = napi_invalid_arg;
    napi_get_cb_info(env, info, &argc, argv, &self, nullptr);
    NAPI_ASSERT(env, (self != nullptr && argc <= ARGC_MAX), "napi_get_cb_info failed!");

    status = napi_unwrap(env, self, &native);
    NAPI_ASSERT(env, (status == napi_ok && native != nullptr), "napi_unwrap failed!");
    return native;
}

napi_status JsUtils::GetValue(napi_env env, napi_value in, int32_t &out)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    NAPI_ASSERT_BASE(env, (status == napi_ok) && (type == napi_number), "invalid type", napi_generic_failure);
    return napi_get_value_int32(env, in, &out);
}

/* napi_value <-> uint32_t */
napi_status JsUtils::GetValue(napi_env env, napi_value in, uint32_t &out)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    NAPI_ASSERT_BASE(env, (status == napi_ok) && (type == napi_number), "invalid type", napi_generic_failure);
    return napi_get_value_uint32(env, in, &out);
}

napi_status JsUtils::GetValue(napi_env env, napi_value in, bool &out)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    NAPI_ASSERT_BASE(env, (status == napi_ok) && (type == napi_boolean), "invalid type", napi_generic_failure);
    return napi_get_value_bool(env, in, &out);
}

napi_status JsUtils::GetValue(napi_env env, napi_value in, double &out)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    NAPI_ASSERT_BASE(env, (status == napi_ok) && (type == napi_number), "invalid double type", napi_generic_failure);
    return napi_get_value_double(env, in, &out);
}

/* napi_value <-> std::string */
napi_status JsUtils::GetValue(napi_env env, napi_value in, std::string &out)
{
    IMSA_HILOGD("JsUtils get string value in.");
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    NAPI_ASSERT_BASE(env, (status == napi_ok) && (type == napi_string), "invalid type", napi_generic_failure);

    size_t maxLen = STR_MAX_LENGTH;
    status = napi_get_value_string_utf8(env, in, NULL, 0, &maxLen);
    if (maxLen <= 0) {
        return status;
    }
    IMSA_HILOGD("napi_value -> std::string get length %{public}zu", maxLen);
    char *buf = new (std::nothrow) char[maxLen + STR_TAIL_LENGTH];
    if (buf != nullptr) {
        size_t len = 0;
        status = napi_get_value_string_utf8(env, in, buf, maxLen + STR_TAIL_LENGTH, &len);
        if (status == napi_ok) {
            buf[len] = 0;
            out = std::string(buf);
        }
        delete[] buf;
    } else {
        status = napi_generic_failure;
    }
    return status;
}

napi_status JsUtils::GetValue(napi_env env, napi_value in, const std::string &type, napi_value &out)
{
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, in, &valueType);
    if ((status == napi_ok) && (valueType == napi_object)) {
        status = napi_get_named_property(env, in, type.c_str(), &out);
        return status;
    }
    return napi_generic_failure;
}

/* napi_value <-> PanelInfo */
napi_status JsUtils::GetValue(napi_env env, napi_value in, PanelInfo &out)
{
    IMSA_HILOGD("napi_value -> PanelInfo ");
    napi_value propType = nullptr;
    napi_status status = napi_get_named_property(env, in, "type", &propType);
    NAPI_ASSERT_BASE(env, (status == napi_ok), "no property type ", status);
    int32_t panelType = 0;
    status = GetValue(env, propType, panelType);
    NAPI_ASSERT_BASE(env, (status == napi_ok), "no value of type ", status);

    // flag is optional. flag isn't need when panelType is status_bar.
    int32_t panelFlag = 0;
    if (panelType != PanelType::STATUS_BAR) {
        napi_value propFlag = nullptr;
        status = napi_get_named_property(env, in, "flag", &propFlag);
        NAPI_ASSERT_BASE(env, (status == napi_ok), "no property flag ", status);
        status = JsUtils::GetValue(env, propFlag, panelFlag);
        NAPI_ASSERT_BASE(env, (status == napi_ok), "no value of flag ", status);
    }

    out.panelType = PanelType(panelType);
    out.panelFlag = PanelFlag(panelFlag);
    return status;
}

napi_value JsUtils::GetValue(napi_env env, const std::vector<InputWindowInfo> &in)
{
    napi_value array = nullptr;
    uint32_t index = 0;
    napi_create_array(env, &array);
    if (array == nullptr) {
        IMSA_HILOGE("create array failed");
        return array;
    }
    for (const auto &info : in) {
        napi_value jsInfo = GetValue(env, info);
        napi_set_element(env, array, index, jsInfo);
        index++;
    }
    return array;
}

napi_value JsUtils::GetValue(napi_env env, const InputWindowInfo &in)
{
    napi_value info = nullptr;
    napi_create_object(env, &info);

    napi_value name = nullptr;
    napi_create_string_utf8(env, in.name.c_str(), in.name.size(), &name);
    napi_set_named_property(env, info, "name", name);

    napi_value left = nullptr;
    napi_create_int32(env, in.left, &left);
    napi_set_named_property(env, info, "left", left);

    napi_value top = nullptr;
    napi_create_int32(env, in.top, &top);
    napi_set_named_property(env, info, "top", top);

    napi_value width = nullptr;
    napi_create_uint32(env, in.width, &width);
    napi_set_named_property(env, info, "width", width);

    napi_value height = nullptr;
    napi_create_uint32(env, in.height, &height);
    napi_set_named_property(env, info, "height", height);

    return info;
}
} // namespace MiscServices
} // namespace OHOS
