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
#ifndef OHOS_INPUTMETHOD_PRIVATE_COMMAND_CAPI_H
#define OHOS_INPUTMETHOD_PRIVATE_COMMAND_CAPI_H
#include <stddef.h>
#include <stdint.h>

#include "inputmethod_types_capi.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * @brief Define the InputMethod_PrivateCommand structure type.
 *
 * The private command between text editor and input method.
 *
 * @since 12
 */
typedef struct InputMethod_PrivateCommand InputMethod_PrivateCommand;

/**
 * @brief Create a new {@link InputMethod_PrivateCommand} instance.
 *
 * @param key The key of the private command.
 * @param keyLength The length of the key.
 * @return Returns a pointer to the newly created {@link InputMethod_PrivateCommand} instance.
 * @since 12
 */
InputMethod_PrivateCommand *OH_PrivateCommand_New(char key[], size_t keyLength);
/**
 * @brief Delete a {@link InputMethod_PrivateCommand} instance.
 *
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance which will be deleted.
 * @since 12
 */
void OH_PrivateCommand_Delete(InputMethod_PrivateCommand *command);
/**
 * @brief Set key value into {@link InputMethod_PrivateCommand}.
 *
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance which will be set value.
 * @param key Represents key value.
 * @param keyLength Represents key length.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_PrivateCommand_SetKey(InputMethod_PrivateCommand *command, char key[], size_t keyLength);
/**
 * @brief Set bool data value into {@link InputMethod_PrivateCommand}.
 *
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance which will be set value.
 * @param value Represents bool data value.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_PrivateCommand_SetBoolValue(InputMethod_PrivateCommand *command, bool value);
/**
 * @brief Set integer data value into {@link InputMethod_PrivateCommand}.
 *
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance which will be set value.
 * @param value Represents integer data value.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_PrivateCommand_SetIntValue(InputMethod_PrivateCommand *command, int32_t value);
/**
 * @brief Set string data value into {@link InputMethod_PrivateCommand}.
 *
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance which will be set value.
 * @param value Represents string data value.
 * @param valueLength Represents the length of string data value.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_PrivateCommand_SetStrValue(
    InputMethod_PrivateCommand *command, char value[], size_t valueLength);

/**
 * @brief Get key value from {@link InputMethod_PrivateCommand}.
 *
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance which will be get value from.
 * @param key Represents key value.
 * @param keyLength Represents key length.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_PrivateCommand_GetKey(
    InputMethod_PrivateCommand *command, const char **key, size_t *keyLength);
/**
 * @brief Get value type from {@link InputMethod_PrivateCommand}.
 *
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance which will be get value from.
 * @param type Represents a pointer to a {@link InputMethod_CommandValueType} instance. Indicates the data type of the
 * value.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_PrivateCommand_GetValueType(
    InputMethod_PrivateCommand *command, InputMethod_CommandValueType *type);
/**
 * @brief Get bool data value from {@link InputMethod_PrivateCommand}.
 *
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance which will be get value from.
 * @param value Represents bool data value.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_PrivateCommand_GetBoolValue(InputMethod_PrivateCommand *command, bool *value);
/**
 * @brief Get integer data value from {@link InputMethod_PrivateCommand}.
 *
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance which will be get value from.
 * @param value Represents integer data value.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_PrivateCommand_GetIntValue(InputMethod_PrivateCommand *command, int32_t *value);
/**
 * @brief Get string data value from {@link InputMethod_PrivateCommand}.
 *
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance which will be get value from.
 * @param value Represents string data value.
 * @param valueLength Represents the length of string data value.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_PrivateCommand_GetStrValue(
    InputMethod_PrivateCommand *command, const char **value, size_t *valueLength);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // OHOS_INPUTMETHOD_PRIVATE_COMMAND_CAPI_H