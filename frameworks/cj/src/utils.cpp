/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "utils.h"

namespace OHOS::MiscServices {
const std::map<int32_t, IMFErrorCode> Utils::ERROR_CODE_MAP = {
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
    { ErrorCode::ERROR_OPERATE_SYSTEM_IME, EXCEPTION_OPERATE_DEFAULTIME },
    { ErrorCode::ERROR_SWITCH_IME, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IMA_DATA_CHANNEL_ABNORMAL, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_IMA_INVALID_IMMERSIVE_EFFECT, EXCEPTION_INVALID_IMMERSIVE_EFFECT },
    { ErrorCode::ERROR_IMA_PRECONDITION_REQUIRED, EXCEPTION_PRECONDITION_REQUIRED },
};

char* Utils::MallocCString(const std::string &origin)
{
    if (origin.empty()) {
        return nullptr;
    }
    auto len = origin.length() + 1;
    char *res = static_cast<char *>(malloc(sizeof(char) * len));
    if (res == nullptr) {
        return nullptr;
    }
    return std::char_traits<char>::copy(res, origin.c_str(), len);
}

void Utils::InputMethodProperty2C(CInputMethodProperty *props, const Property &property)
{
    if (props == nullptr) {
        IMSA_HILOGE("props is nullptr.");
        return;
    }
    props->name = Utils::MallocCString(property.name);
    props->id = Utils::MallocCString(property.id);
    props->label = Utils::MallocCString(property.label);
    props->labelId = property.labelId;
    props->icon = Utils::MallocCString(property.icon);
    props->iconId = property.iconId;
}

Property Utils::C2InputMethodProperty(CInputMethodProperty props)
{
    Property property;
    property.name = std::string(props.name);
    property.id = std::string(props.id);
    property.label = std::string(props.label);
    property.labelId = props.labelId;
    property.icon = std::string(props.icon);
    property.iconId = props.iconId;
    return property;
}

void Utils::InputMethodSubProperty2C(CInputMethodSubtype *props, const SubProperty &property)
{
    if (props == nullptr) {
        IMSA_HILOGE("props is nullptr.");
        return;
    }
    props->name = Utils::MallocCString(property.name);
    props->id = Utils::MallocCString(property.id);
    props->label = Utils::MallocCString(property.label);
    props->labelId = property.labelId;
    props->icon = Utils::MallocCString(property.icon);
    props->iconId = property.iconId;
    props->mode = Utils::MallocCString(property.mode);
    props->locale = Utils::MallocCString(property.locale);
    props->language = Utils::MallocCString(property.language);
}

int32_t Utils::ConvertErrorCode(int32_t code)
{
    IMSA_HILOGD("Convert start.");
    auto iter = ERROR_CODE_MAP.find(code);
    if (iter != ERROR_CODE_MAP.end()) {
        IMSA_HILOGD("ErrorCode: %{public}d", iter->second);
        return iter->second;
    }
    IMSA_HILOGD("Convert fail.");
    return code;
}
}