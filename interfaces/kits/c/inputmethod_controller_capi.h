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
    KEYBOARD_STATUS_NONE = 0,
    /**
     * The keyboard status is hide.
     */
    KEYBOARD_STATUS_HIDE = 1,
    /**
     * The keyboard status is show.
     */
    KEYBOARD_STATUS_SHOW = 2,
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
    ENTER_KEY_UNSPECIFIED = 0,
    /**
     * The enter key type is NONE.
     */
    ENTER_KEY_NONE = 1,
    /**
     * The enter key type is GO.
     */
    ENTER_KEY_GO = 2,
    /**
     * The enter key type is SEARCH.
     */
    ENTER_KEY_SEARCH = 3,
    /**
     * The enter key type is SEND.
     */
    ENTER_KEY_SEND = 4,
    /**
     * The enter key type is NEXT.
     */
    ENTER_KEY_NEXT = 5,
    /**
     * The enter key type is DONE.
     */
    ENTER_KEY_DONE = 6,
    /**
     * The enter key type is PREVIOUS.
     */
    ENTER_KEY_PREVIOUS = 7,
    /**
     * The enter key type is NEWLINE.
     */
    ENTER_KEY_NEWLINE = 8,
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
    DIRECTION_NONE = 0,
    /**
     * The direction is UP.
     */
    DIRECTION_UP = 1,
    /**
     * The direction is DOWN.
     */
    DIRECTION_DOWN = 2,
    /**
     * The direction is LEFT.
     */
    DIRECTION_LEFT = 3,
    /**
     * The direction is RIGHT.
     */
    DIRECTION_RIGHT = 4,
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
    EXTEND_ACTION_SELECT_ALL = 0,
    /**
     * Cut selected text.
     */
    EXTEND_ACTION_CUT = 3,
    /**
     * Copy selected text.
     */
    EXTEND_ACTION_COPY = 4,
    /**
     * Paste from paste board.
     */
    EXTEND_ACTION_PASTE = 5,
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
    TEXT_INPUT_TYPE_NONE = -1,
    /**
     * The text input type is TEXT.
     */
    TEXT_INPUT_TYPE_TEXT = 0,
    /**
     * The text input type is MULTILINE.
     */
    TEXT_INPUT_TYPE_MULTILINE = 1,
    /**
     * The text input type is NUMBER.
     */
    TEXT_INPUT_TYPE_NUMBER = 2,
    /**
     * The text input type is PHONE.
     */
    TEXT_INPUT_TYPE_PHONE = 3,
    /**
     * The text input type is DATETIME.
     */
    TEXT_INPUT_TYPE_DATETIME = 4,
    /**
     * The text input type is EMAIL ADDRESS.
     */
    TEXT_INPUT_TYPE_EMAIL_ADDRESS = 5,
    /**
     * The text input type is URL.
     */
    TEXT_INPUT_TYPE_URL = 6,
    /**
     * The text input type is VISIBLE PASSWORD.
     */
    TEXT_INPUT_TYPE_VISIBLE_PASSWORD = 7,
    /**
     * The text input type is NUMBER PASSWORD.
     */
    TEXT_INPUT_TYPE_NUMBER_PASSWORD = 8,
    /**
     * The text input type is SCREEN LOCK PASSWORD.
     */
    TEXT_INPUT_TYPE_SCREEN_LOCK_PASSWORD = 9,
    /**
     * The text input type is USER NAME.
     */
    TEXT_INPUT_TYPE_USER_NAME = 10,
    /**
     * The text input type is NEW PASSWORD.
     */
    TEXT_INPUT_TYPE_NEW_PASSWORD = 11,
    /**
     * The text input type is NUMBER DECIMAL.
     */
    TEXT_INPUT_TYPE_NUMBER_DECIMAL = 12,
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
    COMMAND_VALUE_TYPE_NONE = 0,
    /**
     * Value type is STRING.
     */
    COMMAND_VALUE_TYPE_STRING = 1,
    /**
     * Value type is BOOL.
     */
    COMMAND_VALUE_TYPE_BOOL = 2,
    /**
     * Value type is INT32.
     */
    COMMAND_VALUE_TYPE_INT32 = 3,
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
    INPUT_METHOD_ERR_OK = 0,
    /**
     * The error code when parameter check failed.
     */
    INPUT_METHOD_ERR_PARAMCHECK = 401,
    /**
     * The error code when the package manager error.
     */
    INPUT_METHOD_ERR_PACKAGEMANAGER = 12800001,
    /**
     * The error code when input method engine error.
     */
    INPUT_METHOD_ERR_IMENGINE = 12800002,
    /**
     * The error code when input method client error.
     */
    INPUT_METHOD_ERR_IMCLIENT = 12800003,
    /**
     * The error code when configuration persisting error.
     */
    INPUT_METHOD_ERR_CONFIG_PERSIST = 12800005,
    /**
     * The error code when input method controller error.
     */
    INPUT_METHOD_ERR_CONTROLLER = 12800006,
    /**
     * The error code when input method setting error.
     */
    INPUT_METHOD_ERR_SETTINGS = 12800007,
    /**
     * The error code when input method manager service error.
     */
    INPUT_METHOD_ERR_IMMS = 12800008,
    /**
     * The error code when input method client is detached.
     */
    INPUT_METHOD_ERR_DETACHED = 12800009,
    /**
     * The error code when unexpected null pointer.
     */
    INPUT_METHOD_ERR_NULL_POINTER = 12800010,
} InputMethod_ErrorCode;

/**
 * @brief Define the OH_InputMethod_CursorInfo structure type.
 *
 * The coordinates and width and height information of the cursor.
 *
 * @since 12
 */
typedef struct OH_InputMethod_CursorInfo OH_InputMethod_CursorInfo;

/**
 * @brief Define the OH_InputMethod_TextConfig structure type.
 *
 * The configuration of the text editor.
 *
 * @since 12
 */
typedef struct OH_InputMethod_TextConfig OH_InputMethod_TextConfig;

/**
 * @brief Define the OH_InputMethod_TextEditorProxy structure type.
 *
 * Provides methods for getting requests and notifications from input method.\n
 * When input method sends request or notification to editor, the methods will be called.\n
 *
 * @since 12
 */
typedef struct OH_InputMethod_TextEditorProxy OH_InputMethod_TextEditorProxy;

/**
 * @brief Define the OH_InputMethod_InputMethodProxy structure type.
 *
 * Provides methods for controlling input method.
 *
 * @since 12
 */
typedef struct OH_InputMethod_InputMethodProxy OH_InputMethod_InputMethodProxy;

/**
 * @brief Define the OH_InputMethod_AttachOptions structure type.
 *
 * The options when attaching input method.
 *
 * @since 12
 */
typedef struct OH_InputMethod_AttachOptions OH_InputMethod_AttachOptions;

/**
 * @brief Define the OH_InputMethod_TextAvoidInfo structure type.
 *
 * Information for text editor to avoid the keyboard.
 *
 * @since 12
 */
typedef struct OH_InputMethod_TextAvoidInfo OH_InputMethod_TextAvoidInfo;

/**
 * @brief Define the OH_InputMethod_PrivateCommand structure type.
 *
 * The private command between text editor and input method.
 *
 * @since 12
 */
typedef struct OH_InputMethod_PrivateCommand OH_InputMethod_PrivateCommand;

int32_t OH_InputMethodController_Attach(OH_InputMethod_TextEditorProxy *textEditorProxy,
    OH_InputMethod_AttachOptions *options, OH_InputMethod_InputMethodProxy **inputMethodProxy);
int32_t OH_InputMethodController_Detach(OH_InputMethod_InputMethodProxy *inputMethodProxy);

int32_t OH_InputMethodProxy_ShowKeyboard(OH_InputMethod_InputMethodProxy *inputMethodProxy);
int32_t OH_InputMethodProxy_HideKeyboard(OH_InputMethod_InputMethodProxy *inputMethodProxy);
int32_t OH_InputMethodProxy_NotifySelectionChange(
    OH_InputMethod_InputMethodProxy *inputMethodProxy, char16_t text[], size_t length, int start, int end);
int32_t OH_InputMethodProxy_NotifyConfigurationChange(OH_InputMethod_InputMethodProxy *inputMethodProxy,
    InputMethod_EnterKeyType enterKey, InputMethod_TextInputType textType);
int32_t OH_InputMethodProxy_NotifyCursorUpdate(
    OH_InputMethod_InputMethodProxy *inputMethodProxy, OH_InputMethod_CursorInfo *cursorInfo);
int32_t OH_InputMethodProxy_SendPrivateCommand(OH_InputMethod_PrivateCommand *privateCommand[], size_t size);

OH_InputMethod_CursorInfo *OH_CursorInfo_New(double left, double top, double width, double height);
void OH_CursorInfo_Delete(OH_InputMethod_CursorInfo *cursorInfo);
int32_t OH_CursorInfo_SetRect(
    OH_InputMethod_CursorInfo *cursorInfo, double left, double top, double width, double height);
int32_t OH_CursorInfo_GetRect(
    OH_InputMethod_CursorInfo *cursorInfo, double *left, double *top, double *width, double *height);

OH_InputMethod_TextConfig *OH_TextConfig_New();
void OH_TextConfig_Delete(OH_InputMethod_TextConfig *config);
int32_t OH_TextConfig_SetInputType(OH_InputMethod_TextConfig *config, InputMethod_TextInputType inputType);
int32_t OH_TextConfig_SetEnterKeyType(OH_InputMethod_TextConfig *config, InputMethod_EnterKeyType enterKeyType);
int32_t OH_TextConfig_SetIsPreviewTextSupported(OH_InputMethod_TextConfig *config, bool supported);
int32_t OH_TextConfig_SetSelection(OH_InputMethod_TextConfig *config, int32_t start, int32_t end);
int32_t OH_TextConfig_SetWindowId(OH_InputMethod_TextConfig *config, int32_t windowId);

int32_t OH_TextConfig_GetInputType(OH_InputMethod_TextConfig *config, InputMethod_TextInputType *inputType);
int32_t OH_TextConfig_GetEnterKeyType(OH_InputMethod_TextConfig *config, InputMethod_EnterKeyType *enterKeyType);
int32_t OH_TextConfig_IsPreviewTextSupported(OH_InputMethod_TextConfig *config, bool *supported);
int32_t OH_TextConfig_GetCursorInfo(OH_InputMethod_TextConfig *config, OH_InputMethod_CursorInfo *cursorInfo);
int32_t OH_TextConfig_GetSelection(OH_InputMethod_TextConfig *config, int32_t *start, int32_t *end);
int32_t OH_TextConfig_GetWindowId(OH_InputMethod_TextConfig *config, int32_t *windowId);

typedef void (*OH_TextEditorProxy_GetTextConfigFunc)(
    OH_InputMethod_TextEditorProxy *textEditorProxy, OH_InputMethod_TextConfig *config);
typedef void (*OH_TextEditorProxy_InsertTextFunc)(
    OH_InputMethod_TextEditorProxy *textEditorProxy, const char16_t *text, size_t length);
typedef void (*OH_TextEditorProxy_DeleteForwardFunc)(OH_InputMethod_TextEditorProxy *textEditorProxy, int32_t length);
typedef void (*OH_TextEditorProxy_DeleteBackwardFunc)(OH_InputMethod_TextEditorProxy *textEditorProxy, int32_t length);
typedef void (*OH_TextEditorProxy_SendKeyboardStatusFunc)(
    OH_InputMethod_TextEditorProxy *textEditorProxy, InputMethod_KeyboardStatus keyboardStatus);
typedef void (*OH_TextEditorProxy_SendEnterKeyTypeFunc)(
    OH_InputMethod_TextEditorProxy *textEditorProxy, InputMethod_EnterKeyType enterKeyType);
typedef void (*OH_TextEditorProxy_MoveCursor)(
    OH_InputMethod_TextEditorProxy *textEditorProxy, InputMethod_Direction direction);
typedef void (*OH_TextEditorProxy_HandleSetSelectionFunc)(
    OH_InputMethod_TextEditorProxy *textEditorProxy, int32_t start, int32_t end);
typedef void (*OH_TextEditorProxy_HandleExtendActionFunc)(
    OH_InputMethod_TextEditorProxy *textEditorProxy, InputMethod_ExtendAction action);
typedef void (*OH_TextEditorProxy_GetLeftTextOfCursorFunc)(
    OH_InputMethod_TextEditorProxy *textEditorProxy, int32_t number, char16_t text[], size_t *length);
typedef void (*OH_TextEditorProxy_GetRightTextOfCursorFunc)(
    OH_InputMethod_TextEditorProxy *textEditorProxy, int32_t number, char16_t text[], size_t *length);
typedef int32_t (*OH_TextEditorProxy_GetTextIndexAtCursorFunc)(OH_InputMethod_TextEditorProxy *textEditorProxy);
typedef int32_t (*OH_TextEditorProxy_ReceivePrivateCommand)(
    OH_InputMethod_TextEditorProxy *textEditorProxy, OH_InputMethod_PrivateCommand *privateCommand[], size_t size);
typedef int32_t (*OH_TextEditorProxy_SetPreviewTextFunc)(
    OH_InputMethod_TextEditorProxy *textEditorProxy, const char16_t text[], size_t length, int32_t start, int32_t end);
typedef void (*OH_TextEditorProxy_FinishTextPreview)(OH_InputMethod_TextEditorProxy *textEditorProxy);

OH_InputMethod_TextEditorProxy *OH_TextEditorProxy_New();
void OH_TextEditorProxy_Delete(OH_InputMethod_TextEditorProxy *proxy);
int32_t OH_TextEditorProxy_SetGetTextConfigFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextConfigFunc getTextConfigFunc);
int32_t OH_TextEditorProxy_SetInsertTextFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_InsertTextFunc insertTextFunc);
int32_t OH_TextEditorProxy_SetDeleteForwardFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteForwardFunc deleteForwardFunc);
int32_t OH_TextEditorProxy_SetDeleteBackwardFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteBackwardFunc deleteBackwardFunc);
int32_t OH_TextEditorProxy_SetSendKeyboardStatusFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendKeyboardStatusFunc sendKeyboardStatusFunc);
int32_t OH_TextEditorProxy_SetSendEnterKeyFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendEnterKeyTypeFunc sendEnterKeyFunc);
int32_t OH_TextEditorProxy_SetMoveCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_MoveCursor moveCursorFunc);
int32_t OH_TextEditorProxy_SetHandleSetSelectionFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleSetSelectionFunc handleSetSelectionFunc);
int32_t OH_TextEditorProxy_SetHandleExtendActionFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleExtendActionFunc handleExtendActionFunc);
int32_t OH_TextEditorProxy_SetGetLeftTextOfCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetLeftTextOfCursorFunc getLeftTextOfCursorFunc);
int32_t OH_TextEditorProxy_SetGetRightTextOfCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetRightTextOfCursorFunc getRightTextOfCursorFunc);
int32_t OH_TextEditorProxy_SetGetTextIndexAtCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextIndexAtCursorFunc getTextIndexAtCursorFunc);
int32_t OH_TextEditorProxy_SetReceivePrivateCommandFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_ReceivePrivateCommand receivePrivateCommandFunc);
int32_t OH_TextEditorProxy_SetSetPreviewTextFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SetPreviewTextFunc setPreviewTextFunc);
int32_t OH_TextEditorProxy_SetFinishTextPreviewFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_FinishTextPreview finishTextPreviewFunc);

int32_t OH_TextEditorProxy_GetGetTextConfigFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextConfigFunc *getTextConfigFunc);
int32_t OH_TextEditorProxy_GetInsertTextFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_InsertTextFunc *insertTextFunc);
int32_t OH_TextEditorProxy_GetDeleteForwardFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteForwardFunc *deleteForwardFunc);
int32_t OH_TextEditorProxy_GetDeleteBackwardFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteBackwardFunc *deleteBackwardFunc);
int32_t OH_TextEditorProxy_GetSendKeyboardStatusFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendKeyboardStatusFunc *sendKeyboardStatusFunc);
int32_t OH_TextEditorProxy_GetSendEnterKeyFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendEnterKeyTypeFunc *sendEnterKeyFunc);
int32_t OH_TextEditorProxy_GetMoveCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_MoveCursor *moveCursorFunc);
int32_t OH_TextEditorProxy_GetHandleSetSelectionFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleSetSelectionFunc *handleSetSelectionFunc);
int32_t OH_TextEditorProxy_GetHandleExtendActionFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleExtendActionFunc *handleExtendActionFunc);
int32_t OH_TextEditorProxy_GetGetLeftTextOfCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetLeftTextOfCursorFunc *getLeftTextOfCursorFunc);
int32_t OH_TextEditorProxy_GetGetRightTextOfCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetRightTextOfCursorFunc *getRightTextOfCursorFunc);
int32_t OH_TextEditorProxy_GetGetTextIndexAtCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextIndexAtCursorFunc *getTextIndexAtCursorFunc);
int32_t OH_TextEditorProxy_GetReceivePrivateCommandFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_ReceivePrivateCommand *receivePrivateCommandFunc);
int32_t OH_TextEditorProxy_GetSetPreviewTextFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SetPreviewTextFunc *setPreviewTextFunc);
int32_t OH_TextEditorProxy_GetFinishTextPreviewFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_FinishTextPreview *finishTextPreviewFunc);

OH_InputMethod_AttachOptions *OH_AttachOptions_New(bool showKeyboard);
void OH_AttachOptions_Delete(OH_InputMethod_AttachOptions *options);
int32_t OH_AttachOptions_IsShowKeyboard(OH_InputMethod_AttachOptions *options, bool *showKeyboard);

OH_InputMethod_TextAvoidInfo *OH_InputMethod_TextAvoidInfo_New(double positionY, double height);
void OH_InputMethod_TextAvoidInfo_Delete(OH_InputMethod_TextAvoidInfo *info);
int32_t OH_InputMethod_TextAvoidInfo_SetPositionY(OH_InputMethod_TextAvoidInfo *info, double positionY);
int32_t OH_InputMethod_TextAvoidInfo_SetHeight(OH_InputMethod_TextAvoidInfo *info, double height);
int32_t OH_InputMethod_TextAvoidInfo_GetPositionY(OH_InputMethod_TextAvoidInfo *info, double *positionY);
int32_t OH_InputMethod_TextAvoidInfo_GetHeight(OH_InputMethod_TextAvoidInfo *info, double *height);

OH_InputMethod_PrivateCommand *OH_InputMethod_PrivateCommand_New(char key[], size_t keyLength);
void OH_InputMethod_PrivateCommand_Delete(OH_InputMethod_PrivateCommand *command);
int32_t OH_InputMethod_PrivateCommand_SetKey(OH_InputMethod_PrivateCommand *command, char key[], size_t keyLength);
int32_t OH_InputMethod_PrivateCommand_SetBoolValue(OH_InputMethod_PrivateCommand *command, bool value);
int32_t OH_InputMethod_PrivateCommand_SetIntValue(OH_InputMethod_PrivateCommand *command, int32_t value);
int32_t OH_InputMethod_PrivateCommand_SetStrValue(
    OH_InputMethod_PrivateCommand *command, char value[], size_t valueLength);

int32_t OH_InputMethod_PrivateCommand_GetKey(OH_InputMethod_PrivateCommand *command, char **key, size_t keyLength);
int32_t OH_InputMethod_PrivateCommand_GetValueType(
    OH_InputMethod_PrivateCommand *command, InputMethod_CommandValueType *type);
int32_t OH_InputMethod_PrivateCommand_GetBoolValue(OH_InputMethod_PrivateCommand *command, bool *value);
int32_t OH_InputMethod_PrivateCommand_GetIntValue(OH_InputMethod_PrivateCommand *command, int32_t *value);
int32_t OH_InputMethod_PrivateCommand_GetStrValue(
    OH_InputMethod_PrivateCommand *command, char **value, size_t valueLength);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // OHOS_INPUTMETHOD_CONTROLLER_CAPI_H