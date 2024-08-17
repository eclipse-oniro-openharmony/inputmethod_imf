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
#include "native_text_editor.h"

using namespace OHOS::MiscServices;
OH_InputMethod_TextEditorProxy *OH_TextEditorProxy_New()
{
    return new OH_InputMethod_TextEditorProxy();
}
void OH_TextEditorProxy_Delete(OH_InputMethod_TextEditorProxy *proxy)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return;
    }
    delete proxy;
}

int32_t OH_TextEditorProxy_SetGetTextConfigFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextConfigFunc getTextConfigFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (getTextConfigFunc == nullptr) {
        IMSA_HILOGE("getTextConfigFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->getTextConfigFunc = getTextConfigFunc;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextEditorProxy_SetInsertTextFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_InsertTextFunc insertTextFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (insertTextFunc == nullptr) {
        IMSA_HILOGE("insertTextFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    proxy->insertTextFunc = insertTextFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_SetDeleteForwardFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteForwardFunc deleteForwardFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (deleteForwardFunc == nullptr) {
        IMSA_HILOGE("deleteForwardFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->deleteForwardFunc = deleteForwardFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_SetDeleteBackwardFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteBackwardFunc deleteBackwardFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (deleteBackwardFunc == nullptr) {
        IMSA_HILOGE("deleteBackwardFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->deleteBackwardFunc = deleteBackwardFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_SetSendKeyboardStatusFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendKeyboardStatusFunc sendKeyboardStatusFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (sendKeyboardStatusFunc == nullptr) {
        IMSA_HILOGE("sendKeyboardStatusFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->sendKeyboardStatusFunc = sendKeyboardStatusFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_SetSendEnterKeyFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendEnterKeyTypeFunc sendEnterKeyFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (sendEnterKeyFunc == nullptr) {
        IMSA_HILOGE("sendEnterKeyFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->sendEnterKeyFunc = sendEnterKeyFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_SetMoveCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_MoveCursor moveCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (moveCursorFunc == nullptr) {
        IMSA_HILOGE("moveCursorFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->moveCursorFunc = moveCursorFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_SetHandleSetSelectionFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleSetSelectionFunc handleSetSelectionFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (handleSetSelectionFunc == nullptr) {
        IMSA_HILOGE("handleSetSelectionFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->handleSetSelectionFunc = handleSetSelectionFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_SetHandleExtendActionFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleExtendActionFunc handleExtendActionFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (handleExtendActionFunc == nullptr) {
        IMSA_HILOGE("handleExtendActionFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->handleExtendActionFunc = handleExtendActionFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_SetGetLeftTextOfCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetLeftTextOfCursorFunc getLeftTextOfCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (getLeftTextOfCursorFunc == nullptr) {
        IMSA_HILOGE("getLeftTextOfCursorFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->getLeftTextOfCursorFunc = getLeftTextOfCursorFunc;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextEditorProxy_SetGetRightTextOfCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetRightTextOfCursorFunc getRightTextOfCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (getRightTextOfCursorFunc == nullptr) {
        IMSA_HILOGE("getRightTextOfCursorFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->getRightTextOfCursorFunc = getRightTextOfCursorFunc;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextEditorProxy_SetGetTextIndexAtCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextIndexAtCursorFunc getTextIndexAtCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (getTextIndexAtCursorFunc == nullptr) {
        IMSA_HILOGE("getTextIndexAtCursorFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->getTextIndexAtCursorFunc = getTextIndexAtCursorFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_SetReceivePrivateCommandFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_ReceivePrivateCommand receivePrivateCommandFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (receivePrivateCommandFunc == nullptr) {
        IMSA_HILOGE("receivePrivateCommandFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->receivePrivateCommandFunc = receivePrivateCommandFunc;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextEditorProxy_SetSetPreviewTextFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SetPreviewTextFunc setPreviewTextFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (setPreviewTextFunc == nullptr) {
        IMSA_HILOGE("setPreviewTextFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->setPreviewTextFunc = setPreviewTextFunc;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextEditorProxy_SetFinishTextPreviewFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_FinishTextPreview finishTextPreviewFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (finishTextPreviewFunc == nullptr) {
        IMSA_HILOGE("finishTextPreviewFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    proxy->finishTextPreviewFunc = finishTextPreviewFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetGetTextConfigFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextConfigFunc *getTextConfigFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (getTextConfigFunc == nullptr) {
        IMSA_HILOGE("getTextConfigFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *getTextConfigFunc = proxy->getTextConfigFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetInsertTextFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_InsertTextFunc *insertTextFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (insertTextFunc == nullptr) {
        IMSA_HILOGE("insertTextFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    *insertTextFunc = proxy->insertTextFunc;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextEditorProxy_GetDeleteForwardFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteForwardFunc *deleteForwardFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (deleteForwardFunc == nullptr) {
        IMSA_HILOGE("deleteForwardFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *deleteForwardFunc = proxy->deleteForwardFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetDeleteBackwardFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteBackwardFunc *deleteBackwardFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (deleteBackwardFunc == nullptr) {
        IMSA_HILOGE("deleteBackwardFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *deleteBackwardFunc = proxy->deleteBackwardFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetSendKeyboardStatusFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendKeyboardStatusFunc *sendKeyboardStatusFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (sendKeyboardStatusFunc == nullptr) {
        IMSA_HILOGE("sendKeyboardStatusFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *sendKeyboardStatusFunc = proxy->sendKeyboardStatusFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetSendEnterKeyFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendEnterKeyTypeFunc *sendEnterKeyFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (sendEnterKeyFunc == nullptr) {
        IMSA_HILOGE("sendEnterKeyFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *sendEnterKeyFunc = proxy->sendEnterKeyFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetMoveCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_MoveCursor *moveCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (moveCursorFunc == nullptr) {
        IMSA_HILOGE("moveCursorFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *moveCursorFunc = proxy->moveCursorFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetHandleSetSelectionFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleSetSelectionFunc *handleSetSelectionFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (handleSetSelectionFunc == nullptr) {
        IMSA_HILOGE("handleSetSelectionFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *handleSetSelectionFunc = proxy->handleSetSelectionFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetHandleExtendActionFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleExtendActionFunc *handleExtendActionFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (handleExtendActionFunc == nullptr) {
        IMSA_HILOGE("handleExtendActionFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *handleExtendActionFunc = proxy->handleExtendActionFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetGetLeftTextOfCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetLeftTextOfCursorFunc *getLeftTextOfCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (getLeftTextOfCursorFunc == nullptr) {
        IMSA_HILOGE("getLeftTextOfCursorFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *getLeftTextOfCursorFunc = proxy->getLeftTextOfCursorFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetGetRightTextOfCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetRightTextOfCursorFunc *getRightTextOfCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (getRightTextOfCursorFunc == nullptr) {
        IMSA_HILOGE("getRightTextOfCursorFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *getRightTextOfCursorFunc = proxy->getRightTextOfCursorFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetGetTextIndexAtCursorFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextIndexAtCursorFunc *getTextIndexAtCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (getTextIndexAtCursorFunc == nullptr) {
        IMSA_HILOGE("getTextIndexAtCursorFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *getTextIndexAtCursorFunc = proxy->getTextIndexAtCursorFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetReceivePrivateCommandFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_ReceivePrivateCommand *receivePrivateCommandFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (receivePrivateCommandFunc == nullptr) {
        IMSA_HILOGE("receivePrivateCommandFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *receivePrivateCommandFunc = proxy->receivePrivateCommandFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetSetPreviewTextFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SetPreviewTextFunc *setPreviewTextFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (setPreviewTextFunc == nullptr) {
        IMSA_HILOGE("setPreviewTextFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *setPreviewTextFunc = proxy->setPreviewTextFunc;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextEditorProxy_GetFinishTextPreviewFunc(
    OH_InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_FinishTextPreview *finishTextPreviewFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (finishTextPreviewFunc == nullptr) {
        IMSA_HILOGE("finishTextPreviewFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *finishTextPreviewFunc = proxy->finishTextPreviewFunc;
    return INPUT_METHOD_ERR_OK;
}

OH_InputMethod_TextConfig *OH_TextConfig_New()
{
    return new OH_InputMethod_TextConfig();
}
void OH_TextConfig_Delete(OH_InputMethod_TextConfig *config)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return;
    }

    delete config;
}
int32_t OH_TextConfig_SetInputType(OH_InputMethod_TextConfig *config, InputMethod_TextInputType inputType)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    config->inputType = inputType;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextConfig_SetEnterKeyType(OH_InputMethod_TextConfig *config, InputMethod_EnterKeyType enterKeyType)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    config->enterKeyType = enterKeyType;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextConfig_SetIsPreviewTextSupported(OH_InputMethod_TextConfig *config, bool supported)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    config->previewTextSupported = supported;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextConfig_SetSelection(OH_InputMethod_TextConfig *config, int32_t start, int32_t end)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    config->selectionStart = start;
    config->selectionEnd = end;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextConfig_SetWindowId(OH_InputMethod_TextConfig *config, int32_t windowId)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    config->windowId = windowId;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_TextConfig_GetInputType(OH_InputMethod_TextConfig *config, InputMethod_TextInputType *inputType)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (inputType == nullptr) {
        IMSA_HILOGE("inputType is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *inputType = config->inputType;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextConfig_GetEnterKeyType(OH_InputMethod_TextConfig *config, InputMethod_EnterKeyType *enterKeyType)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (enterKeyType == nullptr) {
        IMSA_HILOGE("enterKeyType is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *enterKeyType = config->enterKeyType;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextConfig_IsPreviewTextSupported(OH_InputMethod_TextConfig *config, bool *supported)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (supported == nullptr) {
        IMSA_HILOGE("supported is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *supported = config->previewTextSupported;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextConfig_GetCursorInfo(OH_InputMethod_TextConfig *config, OH_InputMethod_CursorInfo **cursorInfo)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (cursorInfo == nullptr) {
        IMSA_HILOGE("cursorInfo is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *cursorInfo = &config->cursorInfo;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextConfig_GetSelection(OH_InputMethod_TextConfig *config, int32_t *start, int32_t *end)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *start = config->selectionStart;
    *end = config->selectionEnd;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_TextConfig_GetWindowId(OH_InputMethod_TextConfig *config, int32_t *windowId)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *windowId = config->windowId;
    return INPUT_METHOD_ERR_OK;
}

OH_InputMethod_TextAvoidInfo *OH_InputMethod_TextAvoidInfo_New(double positionY, double height)
{
    return new OH_InputMethod_TextAvoidInfo({ positionY, height });
}
void OH_InputMethod_TextAvoidInfo_Delete(OH_InputMethod_TextAvoidInfo *info)
{
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr");
        return;
    }
    delete info;
}
int32_t OH_InputMethod_TextAvoidInfo_SetPositionY(OH_InputMethod_TextAvoidInfo *info, double positionY)
{
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    info->positionY = positionY;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_InputMethod_TextAvoidInfo_SetHeight(OH_InputMethod_TextAvoidInfo *info, double height)
{
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    info->height = height;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_InputMethod_TextAvoidInfo_GetPositionY(OH_InputMethod_TextAvoidInfo *info, double *positionY)
{
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *positionY = info->positionY;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_InputMethod_TextAvoidInfo_GetHeight(OH_InputMethod_TextAvoidInfo *info, double *height)
{
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *height = info->height;
    return INPUT_METHOD_ERR_OK;
}

OH_InputMethod_PrivateCommand *OH_InputMethod_PrivateCommand_New(char key[], size_t keyLength)
{
    return new OH_InputMethod_PrivateCommand({ std::string(key, keyLength), false });
}
void OH_InputMethod_PrivateCommand_Delete(OH_InputMethod_PrivateCommand *command)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return;
    }
    delete command;
}
int32_t OH_InputMethod_PrivateCommand_SetKey(OH_InputMethod_PrivateCommand *command, char key[], size_t keyLength)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (key == nullptr) {
        IMSA_HILOGE("key is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    command->key = std::string(key, keyLength);
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_InputMethod_PrivateCommand_SetBoolValue(OH_InputMethod_PrivateCommand *command, bool value)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    command->value = value;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_InputMethod_PrivateCommand_SetIntValue(OH_InputMethod_PrivateCommand *command, int32_t value)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    command->value = value;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_InputMethod_PrivateCommand_SetStrValue(
    OH_InputMethod_PrivateCommand *command, char value[], size_t valueLength)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (value == nullptr) {
        IMSA_HILOGE("value is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    command->value = std::string(value, valueLength);
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_InputMethod_PrivateCommand_GetKey(OH_InputMethod_PrivateCommand *command, char **key, size_t keyLength)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (key == nullptr) {
        IMSA_HILOGE("key is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *key = const_cast<char *>(command->key.c_str());
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_InputMethod_PrivateCommand_GetValueType(
    OH_InputMethod_PrivateCommand *command, InputMethod_CommandValueType *type)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (type == nullptr) {
        IMSA_HILOGE("type is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (std::holds_alternative<bool>(command->value)) {
        *type = COMMAND_VALUE_TYPE_BOOL;
    } else if (std::holds_alternative<int32_t>(command->value)) {
        *type = COMMAND_VALUE_TYPE_INT32;
    } else if (std::holds_alternative<std::string>(command->value)) {
        *type = COMMAND_VALUE_TYPE_STRING;
    } else {
        IMSA_HILOGE("value is not bool or int or string");
        *type = COMMAND_VALUE_TYPE_NONE;
    }

    return INPUT_METHOD_ERR_OK;
}

int32_t OH_InputMethod_PrivateCommand_GetBoolValue(OH_InputMethod_PrivateCommand *command, bool *value)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (value == nullptr) {
        IMSA_HILOGE("value is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (!std::holds_alternative<bool>(command->value)) {
        IMSA_HILOGE("value is not bool");
        return ErrorCode::ERROR_KEYWORD_NOT_FOUND;
    }
    *value = std::get<bool>(command->value);
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_InputMethod_PrivateCommand_GetIntValue(OH_InputMethod_PrivateCommand *command, int32_t *value)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (value == nullptr) {
        IMSA_HILOGE("value is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (!std::holds_alternative<int32_t>(command->value)) {
        IMSA_HILOGE("value is not int32_t");
        return ErrorCode::ERROR_KEYWORD_NOT_FOUND;
    }

    *value = std::get<int32_t>(command->value);
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_InputMethod_PrivateCommand_GetStrValue(
    OH_InputMethod_PrivateCommand *command, char **value, size_t valueLength)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (value == nullptr) {
        IMSA_HILOGE("value is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (!std::holds_alternative<std::string>(command->value)) {
        IMSA_HILOGE("value is not string");
        return ErrorCode::ERROR_KEYWORD_NOT_FOUND;
    }

    *value = const_cast<char *>(std::get<std::string>(command->value).c_str());
    return INPUT_METHOD_ERR_OK;
}

const std::map<int32_t, int32_t> ERROR_CODE_MAP = {
    { ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED,   INPUT_METHOD_ERR_CONTROLLER        },
    { ErrorCode::ERROR_STATUS_PERMISSION_DENIED,     INPUT_METHOD_ERR_PERMISSION        },
    { ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION,     INPUT_METHOD_ERR_SYSTEM_PERMISSION },
    { ErrorCode::ERROR_REMOTE_CLIENT_DIED,           INPUT_METHOD_ERR_IMCLIENT          },
    { ErrorCode::ERROR_CLIENT_NOT_FOUND,             INPUT_METHOD_ERR_IMCLIENT          },
    { ErrorCode::ERROR_CLIENT_NULL_POINTER,          INPUT_METHOD_ERR_IMCLIENT          },
    { ErrorCode::ERROR_CLIENT_NOT_FOCUSED,           INPUT_METHOD_ERR_IMCLIENT          },
    { ErrorCode::ERROR_CLIENT_NOT_EDITABLE,          INPUT_METHOD_ERR_IMCLIENT          },
    { ErrorCode::ERROR_CLIENT_NOT_BOUND,             INPUT_METHOD_ERR_DETACHED          },
    { ErrorCode::ERROR_CLIENT_ADD_FAILED,            INPUT_METHOD_ERR_IMCLIENT          },
    { ErrorCode::ERROR_NULL_POINTER,                 INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_BAD_PARAMETERS,               INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_SERVICE_START_FAILED,         INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_IME_START_FAILED,             INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_KBD_SHOW_FAILED,              INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_KBD_HIDE_FAILED,              INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_IME_NOT_STARTED,              INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_EX_NULL_POINTER,              INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_PERSIST_CONFIG,               INPUT_METHOD_ERR_CONFPERSIST       },
    { ErrorCode::ERROR_PACKAGE_MANAGER,              INPUT_METHOD_ERR_PACKAGEMANAGER    },
    { ErrorCode::ERROR_EX_UNSUPPORTED_OPERATION,     INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_EX_SERVICE_SPECIFIC,          INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_EX_PARCELABLE,                INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT,          INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_EX_ILLEGAL_STATE,             INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_IME_START_INPUT_FAILED,       INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_NOT_IME,                      INPUT_METHOD_ERR_IME               },
    { ErrorCode::ERROR_IME,                          INPUT_METHOD_ERR_IMENGINE          },
    { ErrorCode::ERROR_PARAMETER_CHECK_FAILED,       INPUT_METHOD_ERR_PARAMCHECK        },
    { ErrorCode::ERROR_ENABLE_IME,                   INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_NOT_CURRENT_IME,              INPUT_METHOD_ERR_IMMS              },
    { ErrorCode::ERROR_GET_TEXT_CONFIG,              INPUT_METHOD_ERR_IMCLIENT          },
    { ErrorCode::ERROR_INVALID_PRIVATE_COMMAND_SIZE, INPUT_METHOD_ERR_PARAMCHECK        },
    { ErrorCode::ERROR_TEXT_LISTENER_ERROR,          INPUT_METHOD_ERR_IMCLIENT          },
    { ErrorCode::ERROR_INVALID_RANGE,                INPUT_METHOD_ERR_PARAMCHECK        },
};

constexpr int32_t ERROR_CODE_QUERY_FAILED = 1;
int32_t ErrorCodeConvert(int32_t code)
{
    IMSA_HILOGD("Convert start.");
    auto iter = ERROR_CODE_MAP.find(code);
    if (iter != ERROR_CODE_MAP.end()) {
        IMSA_HILOGE("ErrorCode: %{public}d", iter->second);
        return iter->second;
    }
    IMSA_HILOGD("Convert end.");
    return ERROR_CODE_QUERY_FAILED;
}