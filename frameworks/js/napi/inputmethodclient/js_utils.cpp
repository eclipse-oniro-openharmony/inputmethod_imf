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

#include "js_util.h"

namespace OHOS {
namespace MiscServices {
constexpr int32_t STR_MAX_LENGTH = 4096;
constexpr size_t STR_TAIL_LENGTH = 1;
constexpr size_t ARGC_MAX = 6;
constexpr size_t ARGC_ONE = 1;
const std::map<int32_t, int32_t> JsUtils::ERROR_CODE_MAP = {
    { ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED, EXCEPTION_CONTROLLER },
    { ErrorCode::ERROR_STATUS_PERMISSION_DENIED, EXCEPTION_PERMISSION },
    { ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION, EXCEPTION_SYSTEM_PERMISSION },
    { ErrorCode::ERROR_REMOTE_CLIENT_DIED, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NOT_FOUND, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NULL_POINTER, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NOT_FOCUSED, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NOT_EDITABLE, EXCEPTION_EDITABLE },
    { ErrorCode::ERROR_CLIENT_NOT_BOUND, EXCEPTION_DETACHED },
    { ErrorCode::ERROR_CLIENT_ADD_FAILED, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_NULL_POINTER, EXCEPTION_IMMS },
    { ErrorCode::ERROR_BAD_PARAMETERS, EXCEPTION_IMMS },
    { ErrorCode::ERROR_SERVICE_START_FAILED, EXCEPTION_IMMS },
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
    { ErrorCode::ERROR_IME_START_INPUT_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_NOT_IME, EXCEPTION_IME },
    { ErrorCode::ERROR_IME, EXCEPTION_IMENGINE },
    { ErrorCode::ERROR_PARAMETER_CHECK_FAILED, EXCEPTION_PARAMCHECK },
    { ErrorCode::ERROR_NOT_DEFAULT_IME, EXCEPTION_DEFAULTIME },
    { ErrorCode::ERROR_ENABLE_IME, EXCEPTION_IMMS },
    { ErrorCode::ERROR_NOT_CURRENT_IME, EXCEPTION_IMMS },
    { ErrorCode::ERROR_PANEL_NOT_FOUND, EXCEPTION_PANEL_NOT_FOUND },
    { ErrorCode::ERROR_WINDOW_MANAGER, EXCEPTION_WINDOW_MANAGER },
    { ErrorCode::ERROR_GET_TEXT_CONFIG, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_INVALID_PRIVATE_COMMAND_SIZE, EXCEPTION_PARAMCHECK },
    { ErrorCode::ERROR_TEXT_LISTENER_ERROR, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_TEXT_PREVIEW_NOT_SUPPORTED, EXCEPTION_TEXT_PREVIEW_NOT_SUPPORTED },
    { ErrorCode::ERROR_INVALID_RANGE, EXCEPTION_PARAMCHECK },
    { ErrorCode::ERROR_SECURITY_MODE_OFF, EXCEPTION_BASIC_MODE },
    { ErrorCode::ERROR_MSG_HANDLER_NOT_REGIST, EXCEPTION_REQUEST_NOT_ACCEPT },
    { ErrorCode::ERROR_MESSAGE_HANDLER, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_INVALID_ARRAY_BUFFER_SIZE, EXCEPTION_PARAMCHECK },
    { ErrorCode::ERROR_INVALID_PANEL_TYPE, EXCEPTION_INVALID_PANEL_TYPE_FLAG },
    { ErrorCode::ERROR_INVALID_PANEL_FLAG, EXCEPTION_INVALID_PANEL_TYPE_FLAG },
    { ErrorCode::ERROR_IMA_CHANNEL_NULLPTR, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_IPC_REMOTE_NULLPTR, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMA_NULLPTR, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_INPUT_TYPE_NOT_FOUND, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_DEFAULT_IME_NOT_FOUND, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_CLIENT_INPUT_READY_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_MALLOC_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_NULLPTR, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_GET_IME_INFO_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_IME_TO_START_NULLPTR, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_REBOOT_OLD_IME_NOT_STOP, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_IME_EVENT_CONVERT_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_IME_CONNECT_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_IME_DISCONNECT_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_IME_START_TIMEOUT, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_IME_START_MORE_THAN_EIGHT_SECOND, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMSA_FORCE_STOP_IME_TIMEOUT, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMC_NULLPTR, EXCEPTION_IMMS },
    { ErrorCode::ERROR_DEVICE_UNSUPPORTED, EXCEPTION_UNSUPPORTED },
    { ErrorCode::ERROR_IME_NOT_FOUND, EXCEPTION_IME_NOT_FOUND },
    { ErrorCode::ERROR_DISABLE_SYS_IME, EXCEPTION_DISABLE_SYS_IME },
};

const std::map<int32_t, std::string> JsUtils::ERROR_CODE_CONVERT_MESSAGE_MAP = {
    { EXCEPTION_PERMISSION, "the permissions check fails." },
    { EXCEPTION_SYSTEM_PERMISSION, "not system application." },
    { EXCEPTION_PARAMCHECK, "the parameters check fails." },
    { EXCEPTION_UNSUPPORTED, "capability not supported." },
    { EXCEPTION_PACKAGEMANAGER, "bundle manager error." },
    { EXCEPTION_IMENGINE, "input method engine error." },
    { EXCEPTION_IMCLIENT, "input method client error." },
    { EXCEPTION_IME, "not an input method." },
    { EXCEPTION_CONFPERSIST, "configuration persistence error." },
    { EXCEPTION_CONTROLLER, "input method controller error." },
    { EXCEPTION_SETTINGS, "input method setter error." },
    { EXCEPTION_IMMS, "input method manager service error." },
    { EXCEPTION_DETACHED, "input method client detached." },
    { EXCEPTION_DEFAULTIME, "not the preconfigured default input method." },
    { EXCEPTION_TEXT_PREVIEW_NOT_SUPPORTED, "text preview not supported." },
    { EXCEPTION_PANEL_NOT_FOUND, "the input method panel does not exist." },
    { EXCEPTION_WINDOW_MANAGER, "window manager service error." },
    { EXCEPTION_BASIC_MODE, "the input method is basic mode." },
    { EXCEPTION_REQUEST_NOT_ACCEPT, "the another side does not accept the request." },
    { EXCEPTION_EDITABLE, "the edit mode need enable." },
    { EXCEPTION_INVALID_PANEL_TYPE_FLAG, "invalid panel type or panel flag." },
	{ EXCEPTION_IME_NOT_FOUND, "the input method does not exist." },
    { EXCEPTION_DISABLE_SYS_IME, " the sys ime does not allow to disabled" },
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
    { TYPE_ARRAY_BUFFER, "ArrayBuffer." },
    { TYPE_ARRAY, "napi_array." },
};

void JsUtils::ThrowException(napi_env env, int32_t err, const std::string &msg, TypeCode type)
{
    std::string errMsg = ToMessage(err);
    napi_value error;
    napi_value code;
    napi_value message;
    if (type == TypeCode::TYPE_NONE) {
        errMsg = errMsg + " " + msg;
        IMSA_HILOGE("THROW_ERROR message: %{public}s!", errMsg.c_str());
    } else {
        auto iter = PARAMETER_TYPE.find(type);
        if (iter != PARAMETER_TYPE.end()) {
            errMsg = errMsg + "The type of " + msg + " must be " + iter->second;
            IMSA_HILOGE("THROW_ERROR message: %{public}s!", errMsg.c_str());
        }
    }
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &message));
    NAPI_CALL_RETURN_VOID(env, napi_create_error(env, nullptr, message, &error));
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, err, &code));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, error, "code", code));
    NAPI_CALL_RETURN_VOID(env, napi_throw(env, error));
}

napi_value JsUtils::ToError(napi_env env, int32_t code, const std::string &msg)
{
    IMSA_HILOGD("ToError start");
    napi_value errorObj;
    NAPI_CALL(env, napi_create_object(env, &errorObj));
    napi_value errorCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, Convert(code), &errorCode));
    napi_value errorMessage = nullptr;
    std::string errMsg = ToMessage(Convert(code)) + " " + msg;
    NAPI_CALL(env, napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &errorMessage));
    NAPI_CALL(env, napi_set_named_property(env, errorObj, "code", errorCode));
    NAPI_CALL(env, napi_set_named_property(env, errorObj, "message", errorMessage));
    IMSA_HILOGD("ToError end");
    return errorObj;
}

int32_t JsUtils::Convert(int32_t code)
{
    IMSA_HILOGD("Convert start.");
    auto iter = ERROR_CODE_MAP.find(code);
    if (iter != ERROR_CODE_MAP.end()) {
        IMSA_HILOGD("ErrorCode: %{public}d", iter->second);
        return iter->second;
    }
    IMSA_HILOGD("Convert end.");
    return ERROR_CODE_QUERY_FAILED;
}

const std::string JsUtils::ToMessage(int32_t code)
{
    IMSA_HILOGD("ToMessage start");
    auto iter = ERROR_CODE_CONVERT_MESSAGE_MAP.find(code);
    if (iter != ERROR_CODE_CONVERT_MESSAGE_MAP.end()) {
        IMSA_HILOGD("ErrorMessage: %{public}s", (iter->second).c_str());
        return iter->second;
    }
    return "error is out of definition.";
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
    CHECK_RETURN((self != nullptr && argc <= ARGC_MAX), "napi_get_cb_info failed!", nullptr);

    status = napi_unwrap(env, self, &native);
    CHECK_RETURN((status == napi_ok && native != nullptr), "napi_unwrap failed!", nullptr);
    return native;
}

napi_status JsUtils::GetValue(napi_env env, napi_value in, int32_t &out)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    CHECK_RETURN((status == napi_ok) && (type == napi_number), "invalid type", napi_generic_failure);
    return napi_get_value_int32(env, in, &out);
}

/* napi_value <-> uint32_t */
napi_status JsUtils::GetValue(napi_env env, napi_value in, uint32_t &out)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    CHECK_RETURN((status == napi_ok) && (type == napi_number), "invalid type", napi_generic_failure);
    return napi_get_value_uint32(env, in, &out);
}

napi_status JsUtils::GetValue(napi_env env, napi_value in, bool &out)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    CHECK_RETURN((status == napi_ok) && (type == napi_boolean), "invalid type", napi_generic_failure);
    return napi_get_value_bool(env, in, &out);
}

napi_status JsUtils::GetValue(napi_env env, napi_value in, double &out)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    CHECK_RETURN((status == napi_ok) && (type == napi_number), "invalid double type", napi_generic_failure);
    return napi_get_value_double(env, in, &out);
}

/* napi_value <-> std::string */
napi_status JsUtils::GetValue(napi_env env, napi_value in, std::string &out)
{
    IMSA_HILOGD("JsUtils get string value in.");
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    CHECK_RETURN((status == napi_ok) && (type == napi_string), "invalid type", napi_generic_failure);

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

/* napi_value <-> std::unordered_map<string, string> */
napi_status JsUtils::GetValue(napi_env env, napi_value in, std::unordered_map<std::string, PrivateDataValue> &out)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    PARAM_CHECK_RETURN(env, type != napi_undefined, "param is undefined.", TYPE_NONE, napi_generic_failure);

    napi_value keys = nullptr;
    napi_get_property_names(env, in, &keys);
    uint32_t arrLen = 0;
    status = napi_get_array_length(env, keys, &arrLen);
    if (status != napi_ok) {
        IMSA_HILOGE("napi_get_array_length error");
        return status;
    }
    // 5 means max private command count.
    PARAM_CHECK_RETURN(env, arrLen <= 5 && arrLen > 0, "privateCommand must more than 0 and less than 5.", TYPE_NONE,
        napi_generic_failure);
    IMSA_HILOGD("length : %{public}u", arrLen);
    for (size_t iter = 0; iter < arrLen; ++iter) {
        napi_value key = nullptr;
        status = napi_get_element(env, keys, iter, &key);
        CHECK_RETURN(status == napi_ok, "napi_get_element error", status);

        napi_value value = nullptr;
        status = napi_get_property(env, in, key, &value);
        CHECK_RETURN(status == napi_ok, "napi_get_property error", status);

        std::string keyStr;
        status = GetValue(env, key, keyStr);
        CHECK_RETURN(status == napi_ok, "GetValue keyStr error", status);

        PrivateDataValue privateCommand;
        status = GetValue(env, value, privateCommand);
        CHECK_RETURN(status == napi_ok, "GetValue privateCommand error", status);
        out.emplace(keyStr, privateCommand);
    }
    return status;
}

napi_status JsUtils::GetValue(napi_env env, napi_value in, PrivateDataValue &out)
{
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, in, &valueType);
    CHECK_RETURN(status == napi_ok, "napi_typeof error", napi_generic_failure);
    if (valueType == napi_string) {
        std::string privateDataStr;
        status = GetValue(env, in, privateDataStr);
        CHECK_RETURN(status == napi_ok, "GetValue napi_string error", napi_generic_failure);
        out.emplace<std::string>(privateDataStr);
    } else if (valueType == napi_boolean) {
        bool privateDataBool = false;
        status = GetValue(env, in, privateDataBool);
        CHECK_RETURN(status == napi_ok, "GetValue napi_boolean error", napi_generic_failure);
        out.emplace<bool>(privateDataBool);
    } else if (valueType == napi_number) {
        int32_t privateDataInt = 0;
        status = GetValue(env, in, privateDataInt);
        CHECK_RETURN(status == napi_ok, "GetValue napi_number error", napi_generic_failure);
        out.emplace<int32_t>(privateDataInt);
    } else {
        PARAM_CHECK_RETURN(env, false, "value type must be string | boolean | number", TYPE_NONE, napi_generic_failure);
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
    CHECK_RETURN((status == napi_ok), "no property type ", status);
    int32_t panelType = 0;
    status = GetValue(env, propType, panelType);
    CHECK_RETURN((status == napi_ok), "no value of type ", status);

    // PanelFlag is optional, defaults to FLG_FIXED when empty.
    int32_t panelFlag = static_cast<int32_t>(PanelFlag::FLG_FIXED);
    napi_value panelFlagObj = nullptr;
    status = napi_get_named_property(env, in, "flag", &panelFlagObj);
    if (status == napi_ok) {
        JsUtils::GetValue(env, panelFlagObj, panelFlag);
    }

    out.panelType = PanelType(panelType);
    out.panelFlag = PanelFlag(panelFlag);
    return napi_ok;
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
        ++index;
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

napi_status JsUtils::GetValue(napi_env env, const std::string &in, napi_value &out)
{
    return napi_create_string_utf8(env, in.c_str(), in.size(), &out);
}

napi_value JsUtils::GetJsPrivateCommand(napi_env env, const std::unordered_map<std::string, PrivateDataValue> &in)
{
    napi_value jsPrivateCommand = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsPrivateCommand));
    for (const auto &iter : in) {
        size_t idx = iter.second.index();
        napi_value value = nullptr;
        if (idx == static_cast<size_t>(PrivateDataValueType::VALUE_TYPE_STRING)) {
            auto stringValue = std::get_if<std::string>(&iter.second);
            if (stringValue != nullptr) {
                NAPI_CALL(env, napi_create_string_utf8(env, (*stringValue).c_str(), (*stringValue).size(), &value));
            }
        } else if (idx == static_cast<size_t>(PrivateDataValueType::VALUE_TYPE_BOOL)) {
            auto boolValue = std::get_if<bool>(&iter.second);
            if (boolValue != nullptr) {
                NAPI_CALL(env, napi_get_boolean(env, *boolValue, &value));
            }
        } else if (idx == static_cast<size_t>(PrivateDataValueType::VALUE_TYPE_NUMBER)) {
            auto numberValue = std::get_if<int32_t>(&iter.second);
            if (numberValue != nullptr) {
                NAPI_CALL(env, napi_create_int32(env, *numberValue, &value));
            }
        }
        NAPI_CALL(env, napi_set_named_property(env, jsPrivateCommand, iter.first.c_str(), value));
    }
    return jsPrivateCommand;
}

napi_value JsUtils::GetValue(napi_env env, const std::vector<uint8_t> &in)
{
    void *data = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t length = in.size();
    NAPI_CALL(env, napi_create_arraybuffer(env, length, &data, &arrayBuffer));
    // 0 means the size of data.
    CHECK_RETURN(length != 0, "Data size is 0.", arrayBuffer);
    if (memcpy_s(data, length, reinterpret_cast<const void *>(in.data()), length) != 0) {
        return nullptr;
    }
    return arrayBuffer;
}

napi_status JsUtils::GetValue(napi_env env, napi_value in, std::vector<uint8_t> &out)
{
    size_t length = 0;
    void *data = nullptr;
    auto status = napi_get_arraybuffer_info(env, in, &data, &length);
    if (status != napi_ok) {
        IMSA_HILOGE("Get ArrayBuffer info failed!");
        return status;
    }
    if (data == nullptr && length == 0) {
        IMSA_HILOGE("Empty ArrayBuffer.");
        out.clear();
        return napi_ok;
    }
    if (data == nullptr) {
        IMSA_HILOGE("ArrayBuffer data is nullptr!");
        return napi_generic_failure;
    }
    IMSA_HILOGD("ArrayBuffer data size: %{public}zu.", length);
    out.assign(reinterpret_cast<const uint8_t *>(data), reinterpret_cast<const uint8_t *>(data) + length);
    return napi_ok;
}

napi_status JsUtils::GetMessageHandlerCallbackParam(napi_value *argv,
    const std::shared_ptr<JSMsgHandlerCallbackObject> &jsMessageHandler, const ArrayBuffer &arrayBuffer, size_t size)
{
    if (argv == nullptr) {
        IMSA_HILOGE("argv is nullptr!.");
        return napi_generic_failure;
    }
    if (size < ARGC_ONE) {
        IMSA_HILOGE("argv size is less than 1!.");
        return napi_generic_failure;
    }
    if (jsMessageHandler == nullptr) {
        IMSA_HILOGE("jsMessageHandler is nullptr!.");
        return napi_generic_failure;
    }
    napi_value jsMsgId = nullptr;
    auto status = napi_create_string_utf8(
        jsMessageHandler->env_, arrayBuffer.msgId.c_str(), NAPI_AUTO_LENGTH, &jsMsgId);
    if (status != napi_ok) {
        IMSA_HILOGE("napi_create_string_utf8 failed!.");
        return napi_generic_failure;
    }
    // 0 means the first param index of callback.
    argv[0] = { jsMsgId };
    if (arrayBuffer.jsArgc > ARGC_ONE) {
        napi_value jsMsgParam = JsUtils::GetValue(jsMessageHandler->env_, arrayBuffer.msgParam);
        if (jsMsgParam == nullptr) {
            IMSA_HILOGE("Get js messageParam object failed!.");
            return napi_generic_failure;
        }
        // 0 means the second param index of callback.
        argv[1] = { jsMsgParam };
    }
    return napi_ok;
}

napi_status JsUtils::GetValue(napi_env env, napi_value in, Rosen::Rect &out)
{
    bool ret = JsUtil::Object::ReadProperty(env, in, "left", out.posX_);
    ret = ret && JsUtil::Object::ReadProperty(env, in, "top", out.posY_);
    ret = ret && JsUtil::Object::ReadProperty(env, in, "width", out.width_);
    ret = ret && JsUtil::Object::ReadProperty(env, in, "height", out.height_);
    return ret ? napi_ok : napi_generic_failure;
}

napi_value JsUtils::GetValue(napi_env env, const Rosen::Rect &in)
{
    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);
    bool ret = JsUtil::Object::WriteProperty(env, jsObject, "left", in.posX_);
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "top", in.posY_);
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "width", in.width_);
    ret = ret && JsUtil::Object::WriteProperty(env, jsObject, "height", in.height_);
    return ret ? jsObject : JsUtil::Const::Null(env);
}
} // namespace MiscServices
} // namespace OHOS