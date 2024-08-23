/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "global.h"
#include "native_inputmethod_types.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

InputMethod_PrivateCommand *OH_PrivateCommand_Create(char key[], size_t keyLength)
{
    return new InputMethod_PrivateCommand({ std::string(key, keyLength), false });
}
void OH_PrivateCommand_Destroy(InputMethod_PrivateCommand *command)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return;
    }
    delete command;
}
InputMethod_ErrorCode OH_PrivateCommand_SetKey(InputMethod_PrivateCommand *command, char key[], size_t keyLength)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (key == nullptr) {
        IMSA_HILOGE("key is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    command->key = std::string(key, keyLength);
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_PrivateCommand_SetBoolValue(InputMethod_PrivateCommand *command, bool value)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    command->value = value;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_PrivateCommand_SetIntValue(InputMethod_PrivateCommand *command, int32_t value)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    command->value = value;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_PrivateCommand_SetStrValue(
    InputMethod_PrivateCommand *command, char value[], size_t valueLength)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (value == nullptr) {
        IMSA_HILOGE("value is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    command->value = std::string(value, valueLength);
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_PrivateCommand_GetKey(InputMethod_PrivateCommand *command, const char **key, size_t *keyLength)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (key == nullptr) {
        IMSA_HILOGE("key is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (keyLength == nullptr) {
        IMSA_HILOGE("keyLength is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *key = command->key.c_str();
    *keyLength = command->key.length();
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_PrivateCommand_GetValueType(
    InputMethod_PrivateCommand *command, InputMethod_CommandValueType *type)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (type == nullptr) {
        IMSA_HILOGE("type is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (std::holds_alternative<bool>(command->value)) {
        *type = IME_COMMAND_VALUE_TYPE_BOOL;
    } else if (std::holds_alternative<int32_t>(command->value)) {
        *type = IME_COMMAND_VALUE_TYPE_INT32;
    } else if (std::holds_alternative<std::string>(command->value)) {
        *type = IME_COMMAND_VALUE_TYPE_STRING;
    } else {
        IMSA_HILOGE("value is not bool or int or string");
        *type = IME_COMMAND_VALUE_TYPE_NONE;
    }

    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_PrivateCommand_GetBoolValue(InputMethod_PrivateCommand *command, bool *value)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (value == nullptr) {
        IMSA_HILOGE("value is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (!std::holds_alternative<bool>(command->value)) {
        IMSA_HILOGE("value is not bool");
        return IME_ERR_QUERY_FAILED;
    }
    *value = std::get<bool>(command->value);
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_PrivateCommand_GetIntValue(InputMethod_PrivateCommand *command, int32_t *value)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (value == nullptr) {
        IMSA_HILOGE("value is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (!std::holds_alternative<int32_t>(command->value)) {
        IMSA_HILOGE("value is not int32_t");
        return IME_ERR_QUERY_FAILED;
    }

    *value = std::get<int32_t>(command->value);
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_PrivateCommand_GetStrValue(
    InputMethod_PrivateCommand *command, const char **value, size_t *valueLength)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (value == nullptr) {
        IMSA_HILOGE("value is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (valueLength == nullptr) {
        IMSA_HILOGE("valueLength is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (!std::holds_alternative<std::string>(command->value)) {
        IMSA_HILOGE("value is not string");
        return IME_ERR_QUERY_FAILED;
    }

    *value = std::get<std::string>(command->value).c_str();
    *valueLength = std::get<std::string>(command->value).length();
    return IME_ERR_OK;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */