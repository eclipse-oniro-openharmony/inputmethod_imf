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

#ifndef OHOS_INPUTMETHOD_CONTROLLER_CAPI_H
#define OHOS_INPUTMETHOD_CONTROLLER_CAPI_H
/**
 * @addtogroup InputMethod
 * @{
 *
 * @brief InputMethod provides functions to use input methods and develop input methods.
 *
 * @since 12
 */

/**
 * @file inputmethod_controller_capi.h
 *
 * @brief Provides the functions for using input method.
 *
 * @library libohinputmethod.so
 * @kit IMEKit
 * @syscap SystemCapability.MiscServices.InputMethodFramework
 * @since 12
 * @version 1.0
 */

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * @brief Keyboard status.
 *
 * @since 12
 */
typedef enum {
    /**
     * The keyboard status is none.
     */
    IME_KEYBOARD_STATUS_NONE = 0,
    /**
     * The keyboard status is hide.
     */
    IME_KEYBOARD_STATUS_HIDE = 1,
    /**
     * The keyboard status is show.
     */
    IME_KEYBOARD_STATUS_SHOW = 2,
} InputMethod_KeyboardStatus;

/**
 * @brief Enter key type.
 *
 * @since 12
 */
typedef enum {
    /**
     * The enter key type is UNSPECIFIED.
     */
    IME_ENTER_KEY_UNSPECIFIED = 0,
    /**
     * The enter key type is NONE.
     */
    IME_ENTER_KEY_NONE = 1,
    /**
     * The enter key type is GO.
     */
    IME_ENTER_KEY_GO = 2,
    /**
     * The enter key type is SEARCH.
     */
    IME_ENTER_KEY_SEARCH = 3,
    /**
     * The enter key type is SEND.
     */
    IME_ENTER_KEY_SEND = 4,
    /**
     * The enter key type is NEXT.
     */
    IME_ENTER_KEY_NEXT = 5,
    /**
     * The enter key type is DONE.
     */
    IME_ENTER_KEY_DONE = 6,
    /**
     * The enter key type is PREVIOUS.
     */
    IME_ENTER_KEY_PREVIOUS = 7,
    /**
     * The enter key type is NEWLINE.
     */
    IME_ENTER_KEY_NEWLINE = 8,
} InputMethod_EnterKeyType;

/**
 * @brief Direction.
 *
 * @since 12
 */
typedef enum {
    /**
     * The direction is NONE.
     */
    IME_DIRECTION_NONE = 0,
    /**
     * The direction is UP.
     */
    IME_DIRECTION_UP = 1,
    /**
     * The direction is DOWN.
     */
    IME_DIRECTION_DOWN = 2,
    /**
     * The direction is LEFT.
     */
    IME_DIRECTION_LEFT = 3,
    /**
     * The direction is RIGHT.
     */
    IME_DIRECTION_RIGHT = 4,
} InputMethod_Direction;

/**
 * @brief The extend action.
 *
 * @since 12
 */
typedef enum {
    /**
     * Select all text.
     */
    IME_EXTEND_ACTION_SELECT_ALL = 0,
    /**
     * Cut selected text.
     */
    IME_EXTEND_ACTION_CUT = 3,
    /**
     * Copy selected text.
     */
    IME_EXTEND_ACTION_COPY = 4,
    /**
     * Paste from paste board.
     */
    IME_EXTEND_ACTION_PASTE = 5,
} InputMethod_ExtendAction;

/**
 * @brief The text input type.
 *
 * @since 12
 */
typedef enum {
    /**
     * The text input type is NONE.
     */
    IME_TEXT_INPUT_TYPE_NONE = -1,
    /**
     * The text input type is TEXT.
     */
    IME_TEXT_INPUT_TYPE_TEXT = 0,
    /**
     * The text input type is MULTILINE.
     */
    IME_TEXT_INPUT_TYPE_MULTILINE = 1,
    /**
     * The text input type is NUMBER.
     */
    IME_TEXT_INPUT_TYPE_NUMBER = 2,
    /**
     * The text input type is PHONE.
     */
    IME_TEXT_INPUT_TYPE_PHONE = 3,
    /**
     * The text input type is DATETIME.
     */
    IME_TEXT_INPUT_TYPE_DATETIME = 4,
    /**
     * The text input type is EMAIL ADDRESS.
     */
    IME_TEXT_INPUT_TYPE_EMAIL_ADDRESS = 5,
    /**
     * The text input type is URL.
     */
    IME_TEXT_INPUT_TYPE_URL = 6,
    /**
     * The text input type is VISIBLE PASSWORD.
     */
    IME_TEXT_INPUT_TYPE_VISIBLE_PASSWORD = 7,
    /**
     * The text input type is NUMBER PASSWORD.
     */
    IME_TEXT_INPUT_TYPE_NUMBER_PASSWORD = 8,
    /**
     * The text input type is SCREEN LOCK PASSWORD.
     */
    IME_TEXT_INPUT_TYPE_SCREEN_LOCK_PASSWORD = 9,
    /**
     * The text input type is USER NAME.
     */
    IME_TEXT_INPUT_TYPE_USER_NAME = 10,
    /**
     * The text input type is NEW PASSWORD.
     */
    IME_TEXT_INPUT_TYPE_NEW_PASSWORD = 11,
    /**
     * The text input type is NUMBER DECIMAL.
     */
    IME_TEXT_INPUT_TYPE_NUMBER_DECIMAL = 12,
} InputMethod_TextInputType;

/**
 * @brief The value type of command data.
 *
 * @since 12
 */
typedef enum {
    /**
     * Value type is NONE.
     */
    IME_COMMAND_VALUE_TYPE_NONE = 0,
    /**
     * Value type is STRING.
     */
    IME_COMMAND_VALUE_TYPE_STRING = 1,
    /**
     * Value type is BOOL.
     */
    IME_COMMAND_VALUE_TYPE_BOOL = 2,
    /**
     * Value type is INT32.
     */
    IME_COMMAND_VALUE_TYPE_INT32 = 3,
} InputMethod_CommandValueType;

/**
 * @brief The value type of command data.
 *
 * @since 12
 */
typedef enum {
    /**
     * The error code in the correct case.
     */
    IME_ERR_OK = 0,

    /**
     * The error code when query failed.
     */
    IME_ERR_QUERY_FAILED = 1,
    /**
     * The error code when parameter check failed.
     */
    IME_ERR_PARAMCHECK = 401,
    /**
     * The error code when the package manager error.
     */
    IME_ERR_PACKAGEMANAGER = 12800001,
    /**
     * The error code when input method engine error.
     */
    IME_ERR_IMENGINE = 12800002,
    /**
     * The error code when input method client error.
     */
    IME_ERR_IMCLIENT = 12800003,
    /**
     * The error code when configuration persisting error.
     */
    IME_ERR_CONFIG_PERSIST = 12800005,
    /**
     * The error code when input method controller error.
     */
    IME_ERR_CONTROLLER = 12800006,
    /**
     * The error code when input method setting error.
     */
    IME_ERR_SETTINGS = 12800007,
    /**
     * The error code when input method manager service error.
     */
    IME_ERR_IMMS = 12800008,
    /**
     * The error code when input method client is detached.
     */
    IME_ERR_DETACHED = 12800009,
    /**
     * The error code when unexpected null pointer.
     */
    IME_ERR_NULL_POINTER = 12800010,
} InputMethod_ErrorCode;

/**
 * @brief Define the InputMethod_CursorInfo structure type.
 *
 * The coordinates and width and height information of the cursor.
 *
 * @since 12
 */
typedef struct InputMethod_CursorInfo InputMethod_CursorInfo;

/**
 * @brief Define the InputMethod_TextConfig structure type.
 *
 * The configuration of the text editor.
 *
 * @since 12
 */
typedef struct InputMethod_TextConfig InputMethod_TextConfig;

/**
 * @brief Define the InputMethod_TextEditorProxy structure type.
 *
 * Provides methods for getting requests and notifications from input method.\n
 * When input method sends request or notification to editor, the methods will be called.\n
 *
 * @since 12
 */
typedef struct InputMethod_TextEditorProxy InputMethod_TextEditorProxy;

/**
 * @brief Define the InputMethod_InputMethodProxy structure type.
 *
 * Provides methods for controlling input method.
 *
 * @since 12
 */
typedef struct InputMethod_InputMethodProxy InputMethod_InputMethodProxy;

/**
 * @brief Define the InputMethod_AttachOptions structure type.
 *
 * The options when attaching input method.
 *
 * @since 12
 */
typedef struct InputMethod_AttachOptions InputMethod_AttachOptions;

/**
 * @brief Define the InputMethod_TextAvoidInfo structure type.
 *
 * Information for text editor to avoid the keyboard.
 *
 * @since 12
 */
typedef struct InputMethod_TextAvoidInfo InputMethod_TextAvoidInfo;

/**
 * @brief Define the InputMethod_PrivateCommand structure type.
 *
 * The private command between text editor and input method.
 *
 * @since 12
 */
typedef struct InputMethod_PrivateCommand InputMethod_PrivateCommand;

/**
 * @brief Attach application to the input method service.
 *
 * @param textEditorProxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance.
 *     The caller needs to manage the lifecycle of textEditorProxy.
 *     If the call succeeds, caller cannot release textEditorProxy until the next attach or detach call.
 * @param options Represents a pointer to an {@link InputMethod_AttachOptions} instance.
 *     The options when attaching input method.
 * @param inputMethodProxy Represents a pointer to an {@link OH_InputMethod_InputMethodProxy} instance.
 *     Lifecycle is mantianed until the next attach or detach call.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_PARAMCHECK} - The error code for common invalid args.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_InputMethodController_Attach(InputMethod_TextEditorProxy *textEditorProxy,
    InputMethod_AttachOptions *options, InputMethod_InputMethodProxy **inputMethodProxy);

/**
 * @brief Detach application from the input method service.
 *
 * @param inputMethodProxy Represents a pointer to an {@link OH_InputMethod_InputMethodProxy} instance.
 *     The inputMethodProxy is obtained from {@link OH_InputMethodController_Attach}.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_PARAMCHECK} - The error code for common invalid args.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_InputMethodController_Detach(InputMethod_InputMethodProxy *inputMethodProxy);

/**
 * @brief Show keyboard.
 *
 * @param inputMethodProxy Represents a pointer to an {@link InputMethod_InputMethodProxy} instance.
 *     The inputMethodProxy is obtained from {@link OH_InputMethodController_Attach}.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_DETACHED} - input method client is detached.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_InputMethodProxy_ShowKeyboard(InputMethod_InputMethodProxy *inputMethodProxy);

/**
 * @brief Hide keyboard.
 *
 * @param inputMethodProxy Represents a pointer to an {@link InputMethod_InputMethodProxy} instance.
 *     The inputMethodProxy is obtained from {@link OH_InputMethodController_Attach}.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_DETACHED} - input method client is detached.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_InputMethodProxy_HideKeyboard(InputMethod_InputMethodProxy *inputMethodProxy);

/**
 * @brief Notify selection change.
 * 
 * Notify selection change when text or cursor position or selected text changed.
 *
 * @param inputMethodProxy Represents a pointer to an {@link InputMethod_InputMethodProxy} instance.
 *     The inputMethodProxy is obtained from {@link OH_InputMethodController_Attach}.
 * @param text The whole input text.
 * @param length The length of text. Max 8K bytes.
 * @param start The start position of selected text.
 * @param end The end position of selected text.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_DETACHED} - input method client is detached.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_InputMethodProxy_NotifySelectionChange(
    InputMethod_InputMethodProxy *inputMethodProxy, char16_t text[], size_t length, int start, int end);

/**
 * @brief Notify text editor configuration change.
 *
 * @param inputMethodProxy Represents a pointer to an {@link InputMethod_InputMethodProxy} instance.
 *     The inputMethodProxy is obtained from {@link OH_InputMethodController_Attach}.
 * @param enterKey The enter key type.
 * @param textType The text input type.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_DETACHED} - input method client is detached.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_InputMethodProxy_NotifyConfigurationChange(InputMethod_InputMethodProxy *inputMethodProxy,
    InputMethod_EnterKeyType enterKey, InputMethod_TextInputType textType);

/**
 * @brief Notify cursor update.
 *
 * @param inputMethodProxy Represents a pointer to an {@link InputMethod_InputMethodProxy} instance.
 *     The inputMethodProxy is obtained from {@link OH_InputMethodController_Attach}.
 * @param cursorInfo Represents a pointer to an {@link InputMethod_CursorInfo} instance.
 *     The cursor information.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_DETACHED} - input method client is detached.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_InputMethodProxy_NotifyCursorUpdate(
    InputMethod_InputMethodProxy *inputMethodProxy, InputMethod_CursorInfo *cursorInfo);

/**
 * @brief Send private command.
 *
 * @param inputMethodProxy Represents a pointer to an {@link OH_InputMethod_InputMethodProxy} instance.
 *     The inputMethodProxy is obtained from {@link OH_InputMethodController_Attach}.
 * @param privateCommand The private commands, which is defined in {@link InputMethod_PrivateCommand}. Max size 32KB.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_PARAMCHECK} - parameter check failed.
 *     {@link IME_ERR_DETACHED} - input method client is detached.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_InputMethodProxy_SendPrivateCommand(InputMethod_PrivateCommand *privateCommand[], size_t size);

/**
 * @brief Create a new {@link InputMethod_CursorInfo} instance.
 * 
 * @param left The left point of the cursor and must be absolute coordinate of the physical screen.
 * @param top The top point of the cursor and must be absolute coordinate of the physical screen.
 * @param width The width of the cursor.
 * @param height The height of the cursor.
 * @return Returns a pointer to the newly created {@link InputMethod_CursorInfo} instance.
 * @since 12
 */
InputMethod_CursorInfo *OH_CursorInfo_New(double left, double top, double width, double height);

/**
 * @brief Delete a {@link InputMethod_CursorInfo} instance.
 * 
 * @param cursorInfo Represents a pointer to an {@link InputMethod_CursorInfo} instance which will be deleted.
 * @since 12
 */
void OH_CursorInfo_Delete(InputMethod_CursorInfo *cursorInfo);

/**
 * @brief Set cursor info.
 * 
 * @param cursorInfo Represents a pointer to an {@link InputMethod_CursorInfo} instance.
 * @param left The left point of the cursor and must be absolute coordinate of the physical screen.
 * @param top The top point of the cursor and must be absolute coordinate of the physical screen.
 * @param width The width of the cursor.
 * @param height The height of the cursor.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_CursorInfo_SetRect(
    InputMethod_CursorInfo *cursorInfo, double left, double top, double width, double height);

/**
 * @brief Get cursor info.
 * 
 * @param cursorInfo Represents a pointer to an {@link InputMethod_CursorInfo} instance.
 * @param left The left point of the cursor and must be absolute coordinate of the physical screen.
 * @param top The top point of the cursor and must be absolute coordinate of the physical screen.
 * @param width The width of the cursor.
 * @param height The height of the cursor.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_CursorInfo_GetRect(
    InputMethod_CursorInfo *cursorInfo, double *left, double *top, double *width, double *height);

/**
 * @brief Create a new {@link InputMethod_TextConfig} instance.
 * 
 * @return Returns a pointer to the newly created {@link InputMethod_TextConfig} instance.
 * @since 12
 */
InputMethod_TextConfig *OH_TextConfig_New();
/**
 * @brief Delete a {@link InputMethod_TextConfig} instance.
 * 
 * @param config Represents a pointer to an {@link InputMethod_TextConfig} instance which will be deleted.
 * @since 12
 */
void OH_TextConfig_Delete(InputMethod_TextConfig *config);

/**
 * @brief Set input type into TextConfig.
 * 
 * @param config Represents a pointer to an {@link InputMethod_TextConfig} instance which will be set.
 * @param inputType The text input type of text Editor, which is defined in {@link InputMethod_TextInputType}.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextConfig_SetInputType(InputMethod_TextConfig *config, InputMethod_TextInputType inputType);
/**
 * @brief Set enter key type into TextConfig.
 * 
 * @param config Represents a pointer to an {@link InputMethod_TextConfig} instance which will be set.
 * @param enterKeyType The enter key type of text Editor, which is defined in {@link InputMethod_EnterKeyType}.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextConfig_SetEnterKeyType(
    InputMethod_TextConfig *config, InputMethod_EnterKeyType enterKeyType);
/**
 * @brief Set is preview text supported into TextConfig.
 * 
 * @param config Represents a pointer to an {@link InputMethod_TextConfig} instance which will be set.
 * @param isSupported Indicates whether the preview text is supported.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextConfig_SetIsPreviewTextSupported(InputMethod_TextConfig *config, bool supported);
/**
 * @brief Set selection into TextConfig.
 * 
 * @param config Represents a pointer to an {@link InputMethod_TextConfig} instance which will be set.
 * @param start The start position of selection.
 * @param end The end position of selection.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextConfig_SetSelection(InputMethod_TextConfig *config, int32_t start, int32_t end);
/**
 * @brief Set window id into TextConfig.
 * 
 * @param config Represents a pointer to an {@link InputMethod_TextConfig} instance which will be set.
 * @param windowId The window ID of the application currently bound to the input method.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextConfig_SetWindowId(InputMethod_TextConfig *config, int32_t windowId);

/**
 * @brief Get input type from TextConfig
 * 
 * @param config Represents a pointer to an {@link InputMethod_TextConfig} instance whitch will be get from.
 * @param inputType Represents a pointer to an {@link InputMethod_TextInputType} instance.
 *     The text input type of text Editor
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextConfig_GetInputType(InputMethod_TextConfig *config, InputMethod_TextInputType *inputType);
/**
 * @brief Get enter key type from TextConfig
 * 
 * @param config Represents a pointer to an {@link InputMethod_TextConfig} instance whitch will be get from.
 * @param enterKeyType Represents a pointer to an {@link InputMethod_EnterKeyType} instance.
 *     Indicates the enter key type of text Editor
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextConfig_GetEnterKeyType(
    InputMethod_TextConfig *config, InputMethod_EnterKeyType *enterKeyType);
/**
 * @brief Get is preview text supported from TextConfig.
 * 
 * @param config Represents a pointer to an {@link InputMethod_TextConfig} instance whitch will be get from.
 * @param isSupported Indicates whether the preview text is supported.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextConfig_IsPreviewTextSupported(InputMethod_TextConfig *config, bool *supported);
/**
 * @brief Get cursor info from TextConfig.
 * 
 * @param config Represents a pointer to an {@link InputMethod_TextConfig} instance whitch will be get from.
 * @param cursorInfo Represents a pointer to an {@link InputMethod_CursorInfo} instance.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextConfig_GetCursorInfo(InputMethod_TextConfig *config, InputMethod_CursorInfo *cursorInfo);
/**
 * @brief Get selection from TextConfig.
 * 
 * @param config Represents a pointer to an {@link InputMethod_TextConfig} instance whitch will be get from.
 * @param inputType The text input type of text Editor, which is defined in {@link InputMethod_TextInputType}.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextConfig_GetSelection(InputMethod_TextConfig *config, int32_t *start, int32_t *end);
/**
 * @brief Get window id from TextConfig.
 * 
 * @param config Represents a pointer to an {@link InputMethod_TextConfig} instance whitch will be get from.
 * @param windowId The window ID of the application currently bound to the input method.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextConfig_GetWindowId(InputMethod_TextConfig *config, int32_t *windowId);

/**
 * @brief Defines the function called when input method getting text config.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetGetTextConfigFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance.
 * @param config Represents a pointer to an {@link InputMethod_TextConfig} instance.
 * @since 12
 */
typedef void (*OH_TextEditorProxy_GetTextConfigFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, InputMethod_TextConfig *config);
/**
 * @brief Defines the function called when input method inserting text.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetInsertTextFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to the {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @param text Represents a pointer to the text to be inserted.
 * @param length Represents the length of the text to be inserted.
 * @since 12
 */
typedef void (*OH_TextEditorProxy_InsertTextFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, const char16_t *text, size_t length);
/**
 * @brief Defines the function called when input method deleting text forward.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetDeleteForwardFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to the {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @param length Represents the length of the text to be deleted.
 * @since 12
 */
typedef void (*OH_TextEditorProxy_DeleteForwardFunc)(InputMethod_TextEditorProxy *textEditorProxy, int32_t length);
/**
 * @brief Defines the function called when input method deleting text backward.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetDeleteForwardFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to the {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @param length Represents the length of the text to be deleted.
 * @since 12
 */
typedef void (*OH_TextEditorProxy_DeleteBackwardFunc)(InputMethod_TextEditorProxy *textEditorProxy, int32_t length);
/**
 * @brief Called when input method notifying keyboard status.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetSendKeyboardStatusFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @param keyboardStatus Keyboard status, which is defined in {@link InputMethod_KeyboardStatus}.
 * @since 12
 */
typedef void (*OH_TextEditorProxy_SendKeyboardStatusFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, InputMethod_KeyboardStatus keyboardStatus);
/**
 * @brief Called when input method sending enter key.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetSendEnterKeyFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @param enterKeyType Enter key type, which is defined in {@link InputMethod_EnterKeyType}.
 * @since 12
 */
typedef void (*OH_TextEditorProxy_SendEnterKeyFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, InputMethod_EnterKeyType enterKeyType);
/**
 * @brief Called when input method requesting to move cursor.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetMoveCursorFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @param direction Represents the direction of the cursor movement, which is defined in {@link InputMethod_Direction}.
 * @since 12
 */
typedef void (*OH_TextEditorProxy_MoveCursorFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, InputMethod_Direction direction);
/**
 * @brief Called when input method requesting to set selection.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetHandleSetSelectionFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @param start Represents the start position of the selection.
 * @param end Represents the end position of the selection.
 * @since 12
 */
typedef void (*OH_TextEditorProxy_HandleSetSelectionFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, int32_t start, int32_t end);
/**
 * @brief Called when input method sending extend action.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetHandleExtendActionFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @param action Represents the extend action, which is defined in {@link InputMethod_ExtendAction}.
 * @since 12
 */
typedef void (*OH_TextEditorProxy_HandleExtendActionFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, InputMethod_ExtendAction action);
/**
 * @brief Called when input method requesting to get left text of cursor.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetGetLeftTextOfCursorFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @param number Represents the number of characters to be get.
 * @param text Represents the left text of cursor, you need to assing this parameter.
 * @since 12
 */
typedef void (*OH_TextEditorProxy_GetLeftTextOfCursorFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, int32_t number, char16_t text[], size_t *length);
/**
 * @brief Called when input method requesting to get right text of cursor.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetGetRightTextOfCursorFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @param number Represents the number of characters to be get.
 * @param text Represents the right text of cursor, you need to assing this parameter.
 * @since 12
 */
typedef void (*OH_TextEditorProxy_GetRightTextOfCursorFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, int32_t number, char16_t text[], size_t *length);
/**
 * @brief Called when input method requesting to get text index at cursor.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetGetTextIndexAtCursorFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @return Returns the index of text at cursor.
 * @since 12
 */
typedef int32_t (*OH_TextEditorProxy_GetTextIndexAtCursorFunc)(InputMethod_TextEditorProxy *textEditorProxy);
/**
 * @brief Called when input method sending private command.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetReceivePrivateCommandFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @param privateCommand Private command from input method.
 * @param size Size of private command.
 * @return Returns the result of handling private command.
 * @since 12
 */
typedef int32_t (*OH_TextEditorProxy_ReceivePrivateCommandFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, InputMethod_PrivateCommand *privateCommand[], size_t size);
/**
 * @brief Called when input method setting preview text.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetReceivePrivateCommandFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @param text Represents text to be previewd.
 * @param length Length of preview text.
 * @param start Start position of preview text.
 * @param end End position of preview text.
 * @return Returns the result of setting preview text.
 * @since 12
 */
typedef int32_t (*OH_TextEditorProxy_SetPreviewTextFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, const char16_t text[], size_t length, int32_t start, int32_t end);
/**
 * @brief Called when input method finishing preview text.
 * 
 * You need to implement this function, set it to {@link InputMethod_TextEditorProxy} through {@link OH_TextEditorProxy_SetReceivePrivateCommandFunc},
 * and use {@link OH_InputMethodController_Attach} to complete the registration.\n
 * 
 * @param textEditorProxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set in.
 * @since 12
 */
typedef void (*OH_TextEditorProxy_FinishTextPreview)(InputMethod_TextEditorProxy *textEditorProxy);

/**
 * @brief Create a new {@link InputMethod_TextEditorProxy} instance.
 * 
 * @param showKeyboard Represents whether to show the keyboard.
 * @return Returns a pointer to the newly created {@link InputMethod_TextEditorProxy} instance.
 * @since 12
 */
InputMethod_TextEditorProxy *OH_TextEditorProxy_New();
/**
 * @brief Delete a {@link InputMethod_TextEditorProxy} instance.
 * 
 * @param options Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be deleted.
 * @since 12
 */
void OH_TextEditorProxy_Delete(InputMethod_TextEditorProxy *proxy);
/**
 * @brief Set function {@link OH_TextEditorProxy_GetTextConfigFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetTextConfigFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetGetTextConfigFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextConfigFunc getTextConfigFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetInsertTextFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetInsertTextFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetInsertTextFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_InsertTextFunc insertTextFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetDeleteForwardFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetDeleteForwardFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetDeleteForwardFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteForwardFunc deleteForwardFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetDeleteBackwardFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetDeleteBackwardFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetDeleteBackwardFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteBackwardFunc deleteBackwardFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetSendKeyboardStatusFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetSendKeyboardStatusFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetSendKeyboardStatusFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendKeyboardStatusFunc sendKeyboardStatusFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetSendEnterKeyFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetSendEnterKeyFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetSendEnterKeyFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendEnterKeyFunc sendEnterKeyFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetMoveCursorFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetMoveCursorFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetMoveCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_MoveCursorFunc moveCursorFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetHandleSetSelectionFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetHandleSetSelectionFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetHandleSetSelectionFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleSetSelectionFunc handleSetSelectionFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetHandleExtendActionFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetHandleExtendActionFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetHandleExtendActionFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleExtendActionFunc handleExtendActionFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetGetLeftTextOfCursorFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetGetLeftTextOfCursorFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetGetLeftTextOfCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetLeftTextOfCursorFunc getLeftTextOfCursorFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetGetRightTextOfCursorFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetGetRightTextOfCursorFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetGetRightTextOfCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetRightTextOfCursorFunc getRightTextOfCursorFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetGetTextIndexAtCursorFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetGetTextIndexAtCursorFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetGetTextIndexAtCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextIndexAtCursorFunc getTextIndexAtCursorFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetReceivePrivateCommandFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetReceivePrivateCommandFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetReceivePrivateCommandFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_ReceivePrivateCommandFunc receivePrivateCommandFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetSetPreviewTextFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetSetPreviewTextFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetSetPreviewTextFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SetPreviewTextFunc setPreviewTextFunc);
/**
 * @brief Set function {@link OH_TextEditorProxy_SetFinishTextPreviewFunc} into {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be set function in.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_SetFinishTextPreviewFunc} whitch will be set.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_SetFinishTextPreviewFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_FinishTextPreview finishTextPreviewFunc);

/**
 * @brief Get function {@link OH_TextEditorProxy_GetGetTextConfigFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetGetTextConfigFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetGetTextConfigFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextConfigFunc *getTextConfigFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetInsertTextFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetInsertTextFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetInsertTextFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_InsertTextFunc *insertTextFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetDeleteForwardFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetDeleteForwardFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetDeleteForwardFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteForwardFunc *deleteForwardFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetDeleteBackwardFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetDeleteBackwardFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetDeleteBackwardFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteBackwardFunc *deleteBackwardFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetSendKeyboardStatusFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetSendKeyboardStatusFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetSendKeyboardStatusFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendKeyboardStatusFunc *sendKeyboardStatusFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetSendEnterKeyFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetSendEnterKeyFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetSendEnterKeyFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendEnterKeyFunc *sendEnterKeyFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetMoveCursorFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetMoveCursorFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetMoveCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_MoveCursorFunc *moveCursorFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetHandleSetSelectionFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetHandleSetSelectionFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetHandleSetSelectionFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleSetSelectionFunc *handleSetSelectionFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetHandleExtendActionFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetHandleExtendActionFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetHandleExtendActionFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleExtendActionFunc *handleExtendActionFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetGetLeftTextOfCursorFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetGetLeftTextOfCursorFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetGetLeftTextOfCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetLeftTextOfCursorFunc *getLeftTextOfCursorFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetGetRightTextOfCursorFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetGetRightTextOfCursorFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetGetRightTextOfCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetRightTextOfCursorFunc *getRightTextOfCursorFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetGetTextIndexAtCursorFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetGetTextIndexAtCursorFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetGetTextIndexAtCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextIndexAtCursorFunc *getTextIndexAtCursorFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetReceivePrivateCommandFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetReceivePrivateCommandFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetReceivePrivateCommandFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_ReceivePrivateCommandFunc *receivePrivateCommandFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetSetPreviewTextFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetSetPreviewTextFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetSetPreviewTextFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SetPreviewTextFunc *setPreviewTextFunc);
/**
 * @brief Get function {@link OH_TextEditorProxy_GetFinishTextPreviewFunc} from {@link InputMethod_TextEditorProxy}.
 * 
 * @param proxy Represents a pointer to an {@link InputMethod_TextEditorProxy} instance whitch will be get function from.
 * @param getTextConfigFunc Represents function {@link OH_TextEditorProxy_GetFinishTextPreviewFunc} whitch will be get.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextEditorProxy_GetFinishTextPreviewFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_FinishTextPreview *finishTextPreviewFunc);

/**
 * @brief Create a new {@link InputMethod_AttachOptions} instance.
 * 
 * @param showKeyboard Represents whether to show the keyboard.
 * @return Returns a pointer to the newly created {@link InputMethod_AttachOptions} instance.
 * @since 12
 */
InputMethod_AttachOptions *OH_AttachOptions_New(bool showKeyboard);
/**
 * @brief Delete a {@link InputMethod_AttachOptions} instance.
 * 
 * @param options Represents a pointer to an {@link InputMethod_AttachOptions} instance whitch will be deleted.
 * @since 12
 */
void OH_AttachOptions_Delete(InputMethod_AttachOptions *options);
/**
 * @brief Get showKeyboard value from {@link InputMethod_AttachOptions}.
 * 
 * @param options Represents a pointer to an {@link InputMethod_AttachOptions} instance whitch will be get value from.
 * @param showKeyboard  Represents showKeyboard value.
 *     true - need to show keyboard.
 *     false - no need to show keyboard.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_AttachOptions_IsShowKeyboard(InputMethod_AttachOptions *options, bool *showKeyboard);

/**
 * @brief Create a new {@link InputMethod_TextAvoidInfo} instance.
 * 
 * @param showKeyboard Represents whether to show the keyboard.
 * @return Returns a pointer to the newly created {@link InputMethod_TextAvoidInfo} instance.
 * @since 12
 */
InputMethod_TextAvoidInfo *OH_TextAvoidInfo_New(double positionY, double height);
/**
 * @brief Delete a {@link InputMethod_TextAvoidInfo} instance.
 * 
 * @param options Represents a pointer to an {@link InputMethod_TextAvoidInfo} instance whitch will be deleted.
 * @since 12
 */
void OH_TextAvoidInfo_Delete(InputMethod_TextAvoidInfo *info);
/**
 * @brief Set positionY value into {@link InputMethod_TextAvoidInfo}.
 * 
 * @param info Represents a pointer to an {@link InputMethod_TextAvoidInfo} instance whitch will be set value.
 * @param positionY Represents positionY value.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextAvoidInfo_SetPositionY(InputMethod_TextAvoidInfo *info, double positionY);
/**
 * @brief Set height value into {@link InputMethod_TextAvoidInfo}.
 * 
 * @param info Represents a pointer to an {@link InputMethod_TextAvoidInfo} instance whitch will be set value.
 * @param positionY Represents height value.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextAvoidInfo_SetHeight(InputMethod_TextAvoidInfo *info, double height);
/**
 * @brief Get positionY value from {@link InputMethod_TextAvoidInfo}.
 * 
 * @param info Represents a pointer to an {@link InputMethod_TextAvoidInfo} instance whitch will be get value from.
 * @param positionY Represents positionY value.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextAvoidInfo_GetPositionY(InputMethod_TextAvoidInfo *info, double *positionY);
/**
 * @brief Get height value into {@link InputMethod_TextAvoidInfo}.
 * 
 * @param info Represents a pointer to an {@link InputMethod_TextAvoidInfo} instance whitch will be get value from.
 * @param positionY Represents height value.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_TextAvoidInfo_GetHeight(InputMethod_TextAvoidInfo *info, double *height);

/**
 * @brief Create a new {@link InputMethod_PrivateCommand} instance.
 * 
 * @param showKeyboard Represents whether to show the keyboard.
 * @return Returns a pointer to the newly created {@link InputMethod_PrivateCommand} instance.
 * @since 12
 */
InputMethod_PrivateCommand *OH_PrivateCommand_New(char key[], size_t keyLength);
/**
 * @brief Delete a {@link InputMethod_PrivateCommand} instance.
 * 
 * @param options Represents a pointer to an {@link InputMethod_PrivateCommand} instance whitch will be deleted.
 * @since 12
 */
void OH_PrivateCommand_Delete(InputMethod_PrivateCommand *command);
/**
 * @brief Set key value into {@link InputMethod_PrivateCommand}.
 * 
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance whitch will be set value.
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
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance whitch will be set value.
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
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance whitch will be set value.
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
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance whitch will be set value.
 * @param value Represents string data value.
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
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance whitch will be get value from.
 * @param key Represents key value.
 * @param keyLength Represents key length.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_PrivateCommand_GetKey(InputMethod_PrivateCommand *command, char **key, size_t keyLength);
/**
 * @brief Get value type from {@link InputMethod_PrivateCommand}.
 * 
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance whitch will be get value from.
 * @param type Represents a pointer to a {@link InputMethod_CommandValueType} instance. Indicates the data type of the value.
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
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance whitch will be get value from.
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
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance whitch will be get value from.
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
 * @param command Represents a pointer to an {@link InputMethod_PrivateCommand} instance whitch will be get value from.
 * @param value Represents string data value.
 * @return Returns a specific error code.
 *     {@link IME_ERR_OK} - success.
 *     {@link IME_ERR_NULL_POINTER} - unexpected null pointer.
 * Specific error codes can be referenced {@link InputMethod_ErrorCode}.
 * @since 12
 */
InputMethod_ErrorCode OH_PrivateCommand_GetStrValue(
    InputMethod_PrivateCommand *command, char **value, size_t valueLength);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // OHOS_INPUTMETHOD_CONTROLLER_CAPI_H