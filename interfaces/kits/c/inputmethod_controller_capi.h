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
 * @brief InputMethod Controller API.
 *
 * @since 12
 */

/**
 * @file inputmethod_controller_capi.h
 *
 * @brief xxxx
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
typedef enum {
    KEYBOARD_STATUS_NONE = 0,
    KEYBOARD_STATUS_HIDE = 1,
    KEYBOARD_STATUS_SHOW = 2,
} InputMethod_KeyboardStatus;

typedef enum {
    ENTER_KEY_UNSPECIFIED = 0,
    ENTER_KEY_NONE = 1,
    ENTER_KEY_GO = 2,
    ENTER_KEY_SEARCH = 3,
    ENTER_KEY_SEND = 4,
    ENTER_KEY_NEXT = 5,
    ENTER_KEY_DONE = 6,
    ENTER_KEY_PREVIOUS = 7,
    ENTER_KEY_NEWLINE = 8,
} InputMethod_EnterKeyType;

typedef enum {
    DIRECTION_NONE = 0,
    DIRECTION_UP = 1,
    DIRECTION_DOWN = 2,
    DIRECTION_LEFT = 3,
    DIRECTION_RIGHT = 4,
} InputMethod_Direction;

typedef enum {
    EXTEND_ACTION_SELECT_ALL = 0,
    EXTEND_ACTION_CUT = 3,
    EXTEND_ACTION_COPY = 4,
    EXTEND_ACTION_PASTE = 5,
} InputMethod_ExtendAction;

typedef enum {
    TEXT_INPUT_TYPE_NONE = -1,
    TEXT_INPUT_TYPE_TEXT = 0,
    TEXT_INPUT_TYPE_MULTILINE = 1,
    TEXT_INPUT_TYPE_NUMBER = 2,
    TEXT_INPUT_TYPE_PHONE = 3,
    TEXT_INPUT_TYPE_DATETIME = 4,
    TEXT_INPUT_TYPE_EMAIL_ADDRESS = 5,
    TEXT_INPUT_TYPE_URL = 6,
    TEXT_INPUT_TYPE_VISIBLE_PASSWORD = 7,
    TEXT_INPUT_TYPE_NUMBER_PASSWORD = 8,
    TEXT_INPUT_TYPE_SCREEN_LOCK_PASSWORD = 9,
    TEXT_INPUT_TYPE_USER_NAME = 10,
    TEXT_INPUT_TYPE_NEW_PASSWORD = 11,
    TEXT_INPUT_TYPE_NUMBER_DECIMAL = 12,
} InputMethod_TextInputType;

typedef enum {
    COMMAND_VALUE_TYPE_NONE = 0,
    COMMAND_VALUE_TYPE_STRING = 1,
    COMMAND_VALUE_TYPE_BOOL = 2,
    COMMAND_VALUE_TYPE_INT32 = 3,
} InputMethod_CommandValueType;

typedef enum {
    INPUT_METHOD_ERR_OK = 0,
    INPUT_METHOD_ERR_PERMISSION = 201,
    INPUT_METHOD_ERR_SYSTEM_PERMISSION = 202,
    INPUT_METHOD_ERR_PARAMCHECK = 401,
    INPUT_METHOD_ERR_UNSUPPORTED = 801,
    INPUT_METHOD_ERR_PACKAGEMANAGER = 12800001,
    INPUT_METHOD_ERR_IMENGINE = 12800002,
    INPUT_METHOD_ERR_IMCLIENT = 12800003,
    INPUT_METHOD_ERR_IME = 12800004,
    INPUT_METHOD_ERR_CONFPERSIST = 12800005,
    INPUT_METHOD_ERR_CONTROLLER = 12800006,
    INPUT_METHOD_ERR_SETTINGS = 12800007,
    INPUT_METHOD_ERR_IMMS = 12800008,
    INPUT_METHOD_ERR_DETACHED = 12800009,
    INPUT_METHOD_ERR_NULL_POINTER = 12800010,
} InputMethod_ErrorCode;

typedef struct OH_InputMethod_CursorInfo OH_InputMethod_CursorInfo;
typedef struct OH_InputMethod_TextConfig OH_InputMethod_TextConfig;
typedef struct OH_InputMethod_TextEditorProxy OH_InputMethod_TextEditorProxy;
typedef struct OH_InputMethod_InputMethodProxy OH_InputMethod_InputMethodProxy;
typedef struct OH_InputMethod_AttachOptions OH_InputMethod_AttachOptions;
typedef struct OH_InputMethod_TextAvoidInfo OH_InputMethod_TextAvoidInfo;
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