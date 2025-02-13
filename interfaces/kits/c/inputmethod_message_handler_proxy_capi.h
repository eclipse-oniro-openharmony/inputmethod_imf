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
#ifndef OHOS_INPUTMETHOD_MESSAGE_HANDLER_PROXY_CAPI_H
#define OHOS_INPUTMETHOD_MESSAGE_HANDLER_PROXY_CAPI_H
/**
 * @addtogroup InputMethod
 * @{
 *
 * @brief InputMethod provides functions to use input methods and develop input methods.
 *
 * @since 12
 */

/**
 * @file inputmethod_text_editor_proxy_capi.h
 *
 * @brief Provides functions for getting requests and notifications from input method.
 *
 * @library libohinputmethod.so
 * @kit IMEKit
 * @syscap SystemCapability.MiscServices.InputMethodFramework
 * @since 16
 * @version 1.0
 */
#include <stddef.h>

#include "inputmethod_text_config_capi.h"
#include "inputmethod_types_capi.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * @brief Define the InputMethod_MessageHandlerProxy structure type.
 *
 * Provides methods for getting requests and notifications from input method.\n
 * When input method sends request or notification to editor, the methods will be called.\n
 *
 * @since 16
 */
typedef struct InputMethod_MessageHandlerProxy InputMethod_MessageHandlerProxy;

/**
 * @brief Called when input method sending private command.
 *
 * You need to implement this function, set it to {@link InputMethod_MessageHandlerProxy} through {@link
 * OH_TextEditorProxy_SetOnTerminatedFunc}, and use {@link OH_InputMethodProxy_RecvMessage} to complete the
 * registration.\n
 *
 * @param messageHandlerProxy Represents a pointer to an
 *                            {@link InputMethod_MessageHandlerProxy} instance which will be set in.
 * @return Returns the result of handling private command.
 * @since 16
 */
typedef int32_t (*OH_MessageHandlerProxy_OnTerminatedFunc)(InputMethod_MessageHandlerProxy *messageHandlerProxy);

/**
 * @brief Called when input method finishing preview text.
 *
 * You need to implement this function, set it to {@link InputMethod_MessageHandlerProxy} through {@link
 * OH_TextEditorProxy_SetOnMessageFunc}, and use {@link OH_InputMethodProxy_RecvMessage} to complete the
 * registration.\n
 *
 * @param messageHandlerProxy Represents a pointer to an
 *                            {@link InputMethod_MessageHandlerProxy} instance which will be set in.
 * @param msgId ArrayBuffer.msgId from input method.
 * @param msgIdLength Size of ArrayBuffer.msgId.
 * @param msgParam ArrayBuffer.msgParam from input method.
 * @param msgParamLength Size of ArrayBuffer.msgParam.
 * @since 16
 */
typedef int32_t (*OH_MessageHandlerProxy_OnMessageFunc)(InputMethod_MessageHandlerProxy *messageHandlerProxy,
    const char16_t msgId[], size_t msgIdLength, const uint8_t *msgParam, size_t msgParamLength);

/**
 * @brief Create a new {@link InputMethod_MessageHandlerProxy} instance.
 *
 * @return If the creation succeeds, a pointer to the newly created {@link InputMethod_MessageHandlerProxy}
 * instance is returned. If the creation fails, NULL is returned, possible cause is insufficient memory.
 * @since 12
 */
InputMethod_MessageHandlerProxy *OH_MessageHandlerProxy_Create(void);

/**
 * @brief Destroy a {@link InputMethod_MessageHandlerProxy} instance.
 *
 * @param proxy The {@link InputMethod_MessageHandlerProxy} instance to be destroyed.
 * @since 16
 */
void OH_MessageHandlerProxy_Destroy(InputMethod_MessageHandlerProxy *proxy);

/**
 * @brief Set function {@link OH_MessageHandlerProxy_OnTerminatedFunc} into {@link InputMethod_MessageHandlerProxy}.
 *
 * @param proxy Represents a pointer to an
 *              {@link InputMethod_MessageHandlerProxy} instance which will be set function in.
 * @param onTerminatedFunc Represents function {@link OH_MessageHandlerProxy_SetOnTerminatedFunc} which
 * will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 16
 */
InputMethod_ErrorCode OH_MessageHandlerProxy_SetOnTerminatedFunc(
    InputMethod_MessageHandlerProxy *proxy, OH_MessageHandlerProxy_OnTerminatedFunc onTerminatedFunc);

/**
 * @brief Get function {@link OH_MessageHandlerProxy_OnTerminatedFunc} from {@link InputMethod_MessageHandlerProxy}.
 *
 * @param proxy Represents a pointer to an
 *              {@link InputMethod_MessageHandlerProxy} instance which will be get function from.
 * @param onTerminatedFunc Represents function {@link OH_MessageHandlerProxy_GetOnTerminatedFunc} which
 * will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 16
 */
InputMethod_ErrorCode OH_MessageHandlerProxy_GetOnTerminatedFunc(
    InputMethod_MessageHandlerProxy *proxy, OH_MessageHandlerProxy_OnTerminatedFunc *onTerminatedFunc);

/**
 * @brief Set function {@link OH_MessageHandlerProxy_OnMessageFunc} into {@link InputMethod_MessageHandlerProxy}.
 *
 * @param proxy Represents a pointer to an
 *              {@link InputMethod_MessageHandlerProxy} instance which will be set function in.
 * @param onMessageFunc Represents function {@link OH_MessageHandlerProxy_SetOnMessageFunc} which
 * will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 16
 */
InputMethod_ErrorCode OH_MessageHandlerProxy_SetOnMessageFunc(
    InputMethod_MessageHandlerProxy *proxy, OH_MessageHandlerProxy_OnMessageFunc onMessageFunc);

/**
 * @brief Get function {@link OH_MessageHandlerProxy_OnMessageFunc} from {@link InputMethod_MessageHandlerProxy}.
 *
 * @param proxy Represents a pointer to an {@link InputMethod_MessageHandlerProxy} instance which will be get function
 * from.
 * @param onMessageFunc Represents function {@link OH_MessageHandlerProxy_GetOnMessageFunc} which
 * will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 16
 */
InputMethod_ErrorCode OH_MessageHandlerProxy_GetOnMessageFunc(
    InputMethod_MessageHandlerProxy *proxy, OH_MessageHandlerProxy_OnMessageFunc *onMessageFunc);
#ifdef __cplusplus
}
#endif /* __cplusplus */
/** @} */
#endif // OHOS_INPUTMETHOD_MESSAGE_HANDLER_PROXY_CAPI_H