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

InputMethod_ErrorCode OH_InputMethodController_Attach(InputMethod_TextEditorProxy *textEditorProxy,
    InputMethod_AttachOptions *options, InputMethod_InputMethodProxy **inputMethodProxy);
InputMethod_ErrorCode OH_InputMethodController_Detach(InputMethod_InputMethodProxy *inputMethodProxy);

InputMethod_ErrorCode OH_InputMethodProxy_ShowKeyboard(InputMethod_InputMethodProxy *inputMethodProxy);
InputMethod_ErrorCode OH_InputMethodProxy_HideKeyboard(InputMethod_InputMethodProxy *inputMethodProxy);
InputMethod_ErrorCode OH_InputMethodProxy_NotifySelectionChange(
    InputMethod_InputMethodProxy *inputMethodProxy, char16_t text[], size_t length, int start, int end);
InputMethod_ErrorCode OH_InputMethodProxy_NotifyConfigurationChange(InputMethod_InputMethodProxy *inputMethodProxy,
    InputMethod_EnterKeyType enterKey, InputMethod_TextInputType textType);
InputMethod_ErrorCode OH_InputMethodProxy_NotifyCursorUpdate(
    InputMethod_InputMethodProxy *inputMethodProxy, InputMethod_CursorInfo *cursorInfo);
InputMethod_ErrorCode OH_InputMethodProxy_SendPrivateCommand(InputMethod_PrivateCommand *privateCommand[], size_t size);

InputMethod_CursorInfo *OH_CursorInfo_New(double left, double top, double width, double height);
void OH_CursorInfo_Delete(InputMethod_CursorInfo *cursorInfo);
InputMethod_ErrorCode OH_CursorInfo_SetRect(
    InputMethod_CursorInfo *cursorInfo, double left, double top, double width, double height);
InputMethod_ErrorCode OH_CursorInfo_GetRect(
    InputMethod_CursorInfo *cursorInfo, double *left, double *top, double *width, double *height);

InputMethod_TextConfig *OH_TextConfig_New();
void OH_TextConfig_Delete(InputMethod_TextConfig *config);
InputMethod_ErrorCode OH_TextConfig_SetInputType(InputMethod_TextConfig *config, InputMethod_TextInputType inputType);
InputMethod_ErrorCode OH_TextConfig_SetEnterKeyType(
    InputMethod_TextConfig *config, InputMethod_EnterKeyType enterKeyType);
InputMethod_ErrorCode OH_TextConfig_SetIsPreviewTextSupported(InputMethod_TextConfig *config, bool supported);
InputMethod_ErrorCode OH_TextConfig_SetSelection(InputMethod_TextConfig *config, int32_t start, int32_t end);
InputMethod_ErrorCode OH_TextConfig_SetWindowId(InputMethod_TextConfig *config, int32_t windowId);

InputMethod_ErrorCode OH_TextConfig_GetInputType(InputMethod_TextConfig *config, InputMethod_TextInputType *inputType);
InputMethod_ErrorCode OH_TextConfig_GetEnterKeyType(
    InputMethod_TextConfig *config, InputMethod_EnterKeyType *enterKeyType);
InputMethod_ErrorCode OH_TextConfig_IsPreviewTextSupported(InputMethod_TextConfig *config, bool *supported);
InputMethod_ErrorCode OH_TextConfig_GetCursorInfo(InputMethod_TextConfig *config, InputMethod_CursorInfo *cursorInfo);
InputMethod_ErrorCode OH_TextConfig_GetSelection(InputMethod_TextConfig *config, int32_t *start, int32_t *end);
InputMethod_ErrorCode OH_TextConfig_GetWindowId(InputMethod_TextConfig *config, int32_t *windowId);

typedef void (*OH_TextEditorProxy_GetTextConfigFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, InputMethod_TextConfig *config);
typedef void (*OH_TextEditorProxy_InsertTextFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, const char16_t *text, size_t length);
typedef void (*OH_TextEditorProxy_DeleteForwardFunc)(InputMethod_TextEditorProxy *textEditorProxy, int32_t length);
typedef void (*OH_TextEditorProxy_DeleteBackwardFunc)(InputMethod_TextEditorProxy *textEditorProxy, int32_t length);
typedef void (*OH_TextEditorProxy_SendKeyboardStatusFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, InputMethod_KeyboardStatus keyboardStatus);
typedef void (*OH_TextEditorProxy_SendEnterKeyFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, InputMethod_EnterKeyType enterKeyType);
typedef void (*OH_TextEditorProxy_MoveCursorFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, InputMethod_Direction direction);
typedef void (*OH_TextEditorProxy_HandleSetSelectionFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, int32_t start, int32_t end);
typedef void (*OH_TextEditorProxy_HandleExtendActionFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, InputMethod_ExtendAction action);
typedef void (*OH_TextEditorProxy_GetLeftTextOfCursorFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, int32_t number, char16_t text[], size_t *length);
typedef void (*OH_TextEditorProxy_GetRightTextOfCursorFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, int32_t number, char16_t text[], size_t *length);
typedef int32_t (*OH_TextEditorProxy_GetTextIndexAtCursorFunc)(InputMethod_TextEditorProxy *textEditorProxy);
typedef int32_t (*OH_TextEditorProxy_ReceivePrivateCommandFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, InputMethod_PrivateCommand *privateCommand[], size_t size);
typedef int32_t (*OH_TextEditorProxy_SetPreviewTextFunc)(
    InputMethod_TextEditorProxy *textEditorProxy, const char16_t text[], size_t length, int32_t start, int32_t end);
typedef void (*OH_TextEditorProxy_FinishTextPreview)(InputMethod_TextEditorProxy *textEditorProxy);

InputMethod_TextEditorProxy *OH_TextEditorProxy_New();
void OH_TextEditorProxy_Delete(InputMethod_TextEditorProxy *proxy);
InputMethod_ErrorCode OH_TextEditorProxy_SetGetTextConfigFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextConfigFunc getTextConfigFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetInsertTextFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_InsertTextFunc insertTextFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetDeleteForwardFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteForwardFunc deleteForwardFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetDeleteBackwardFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteBackwardFunc deleteBackwardFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetSendKeyboardStatusFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendKeyboardStatusFunc sendKeyboardStatusFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetSendEnterKeyFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendEnterKeyFunc sendEnterKeyFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetMoveCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_MoveCursorFunc moveCursorFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetHandleSetSelectionFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleSetSelectionFunc handleSetSelectionFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetHandleExtendActionFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleExtendActionFunc handleExtendActionFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetGetLeftTextOfCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetLeftTextOfCursorFunc getLeftTextOfCursorFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetGetRightTextOfCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetRightTextOfCursorFunc getRightTextOfCursorFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetGetTextIndexAtCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextIndexAtCursorFunc getTextIndexAtCursorFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetReceivePrivateCommandFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_ReceivePrivateCommandFunc receivePrivateCommandFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetSetPreviewTextFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SetPreviewTextFunc setPreviewTextFunc);
InputMethod_ErrorCode OH_TextEditorProxy_SetFinishTextPreviewFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_FinishTextPreview finishTextPreviewFunc);

InputMethod_ErrorCode OH_TextEditorProxy_GetGetTextConfigFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextConfigFunc *getTextConfigFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetInsertTextFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_InsertTextFunc *insertTextFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetDeleteForwardFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteForwardFunc *deleteForwardFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetDeleteBackwardFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteBackwardFunc *deleteBackwardFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetSendKeyboardStatusFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendKeyboardStatusFunc *sendKeyboardStatusFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetSendEnterKeyFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendEnterKeyFunc *sendEnterKeyFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetMoveCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_MoveCursorFunc *moveCursorFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetHandleSetSelectionFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleSetSelectionFunc *handleSetSelectionFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetHandleExtendActionFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleExtendActionFunc *handleExtendActionFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetGetLeftTextOfCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetLeftTextOfCursorFunc *getLeftTextOfCursorFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetGetRightTextOfCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetRightTextOfCursorFunc *getRightTextOfCursorFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetGetTextIndexAtCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextIndexAtCursorFunc *getTextIndexAtCursorFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetReceivePrivateCommandFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_ReceivePrivateCommandFunc *receivePrivateCommandFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetSetPreviewTextFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SetPreviewTextFunc *setPreviewTextFunc);
InputMethod_ErrorCode OH_TextEditorProxy_GetFinishTextPreviewFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_FinishTextPreview *finishTextPreviewFunc);

InputMethod_AttachOptions *OH_AttachOptions_New(bool showKeyboard);
void OH_AttachOptions_Delete(InputMethod_AttachOptions *options);
InputMethod_ErrorCode OH_AttachOptions_IsShowKeyboard(InputMethod_AttachOptions *options, bool *showKeyboard);

InputMethod_TextAvoidInfo *OH_TextAvoidInfo_New(double positionY, double height);
void OH_TextAvoidInfo_Delete(InputMethod_TextAvoidInfo *info);
InputMethod_ErrorCode OH_TextAvoidInfo_SetPositionY(InputMethod_TextAvoidInfo *info, double positionY);
InputMethod_ErrorCode OH_TextAvoidInfo_SetHeight(InputMethod_TextAvoidInfo *info, double height);
InputMethod_ErrorCode OH_TextAvoidInfo_GetPositionY(InputMethod_TextAvoidInfo *info, double *positionY);
InputMethod_ErrorCode OH_TextAvoidInfo_GetHeight(InputMethod_TextAvoidInfo *info, double *height);

InputMethod_PrivateCommand *OH_PrivateCommand_New(char key[], size_t keyLength);
void OH_PrivateCommand_Delete(InputMethod_PrivateCommand *command);
InputMethod_ErrorCode OH_PrivateCommand_SetKey(InputMethod_PrivateCommand *command, char key[], size_t keyLength);
InputMethod_ErrorCode OH_PrivateCommand_SetBoolValue(InputMethod_PrivateCommand *command, bool value);
InputMethod_ErrorCode OH_PrivateCommand_SetIntValue(InputMethod_PrivateCommand *command, int32_t value);
InputMethod_ErrorCode OH_PrivateCommand_SetStrValue(
    InputMethod_PrivateCommand *command, char value[], size_t valueLength);

InputMethod_ErrorCode OH_PrivateCommand_GetKey(InputMethod_PrivateCommand *command, char **key, size_t keyLength);
InputMethod_ErrorCode OH_PrivateCommand_GetValueType(
    InputMethod_PrivateCommand *command, InputMethod_CommandValueType *type);
InputMethod_ErrorCode OH_PrivateCommand_GetBoolValue(InputMethod_PrivateCommand *command, bool *value);
InputMethod_ErrorCode OH_PrivateCommand_GetIntValue(InputMethod_PrivateCommand *command, int32_t *value);
InputMethod_ErrorCode OH_PrivateCommand_GetStrValue(
    InputMethod_PrivateCommand *command, char **value, size_t valueLength);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // OHOS_INPUTMETHOD_CONTROLLER_CAPI_H