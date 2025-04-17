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
#ifndef NATIVE_INPUTMETHOD_TYPES_H
#define NATIVE_INPUTMETHOD_TYPES_H
#include <string>
#include <variant>

#include "inputmethod_controller_capi.h"
constexpr size_t MAX_PLACEHOLDER_INPUT_SIZE = 513; // char16_t max size,includes string ending symbol (0x0000).
constexpr size_t MAX_ABILITY_NAME_INPUT_SIZE = 65; // char16_t max size,includes string ending symbol (0x0000).
constexpr char16_t UTF16_ENDING_SYMBOL = u'\0';
constexpr size_t ENDING_SYMBOL_SIZE = 1;

struct InputMethod_PrivateCommand {
    std::string key;
    std::variant<std::string, bool, int32_t> value;
};

struct InputMethod_CursorInfo {
    double left = -1.0;
    double top = -1.0;
    double width = -1.0;
    double height = -1.0;
};

struct InputMethod_TextAvoidInfo {
    double positionY;
    double height;
};

struct InputMethod_TextConfig {
    InputMethod_TextInputType inputType;
    InputMethod_EnterKeyType enterKeyType;
    bool previewTextSupported;
    InputMethod_CursorInfo cursorInfo;
    InputMethod_TextAvoidInfo avoidInfo;
    int32_t selectionStart;
    int32_t selectionEnd;
    int32_t windowId;
    char16_t placeholder[MAX_PLACEHOLDER_INPUT_SIZE];
    size_t placeholderLength = 1;
    char16_t abilityName[MAX_ABILITY_NAME_INPUT_SIZE];
    size_t abilityNameLength = 1;
};

struct InputMethod_MessageHandlerProxy {
    OH_MessageHandlerProxy_OnTerminatedFunc onTerminatedFunc;
    OH_MessageHandlerProxy_OnMessageFunc onMessageFunc;
};

struct InputMethod_TextEditorProxy {
    OH_TextEditorProxy_GetTextConfigFunc getTextConfigFunc;
    OH_TextEditorProxy_InsertTextFunc insertTextFunc;
    OH_TextEditorProxy_DeleteForwardFunc deleteForwardFunc;
    OH_TextEditorProxy_DeleteBackwardFunc deleteBackwardFunc;
    OH_TextEditorProxy_SendKeyboardStatusFunc sendKeyboardStatusFunc;
    OH_TextEditorProxy_SendEnterKeyFunc sendEnterKeyFunc;
    OH_TextEditorProxy_MoveCursorFunc moveCursorFunc;
    OH_TextEditorProxy_HandleSetSelectionFunc handleSetSelectionFunc;
    OH_TextEditorProxy_HandleExtendActionFunc handleExtendActionFunc;
    OH_TextEditorProxy_GetLeftTextOfCursorFunc getLeftTextOfCursorFunc;
    OH_TextEditorProxy_GetRightTextOfCursorFunc getRightTextOfCursorFunc;
    OH_TextEditorProxy_GetTextIndexAtCursorFunc getTextIndexAtCursorFunc;
    OH_TextEditorProxy_ReceivePrivateCommandFunc receivePrivateCommandFunc;
    OH_TextEditorProxy_SetPreviewTextFunc setPreviewTextFunc;
    OH_TextEditorProxy_FinishTextPreviewFunc finishTextPreviewFunc;
};

struct InputMethod_AttachOptions {
    bool showKeyboard;
    InputMethod_RequestKeyboardReason requestKeyboardReason =
        InputMethod_RequestKeyboardReason::IME_REQUEST_REASON_NONE;
};
constexpr int32_t MAX_TEXT_LENGTH = 8 * 1024;
#endif // NATIVE_INPUTMETHOD_TYPES_H
