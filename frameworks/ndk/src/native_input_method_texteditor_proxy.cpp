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
InputMethod_TextEditorProxy *OH_TextEditorProxy_New()
{
    return new InputMethod_TextEditorProxy();
}
void OH_TextEditorProxy_Delete(InputMethod_TextEditorProxy *proxy)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return;
    }
    delete proxy;
}

InputMethod_ErrorCode OH_TextEditorProxy_SetGetTextConfigFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextConfigFunc getTextConfigFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (getTextConfigFunc == nullptr) {
        IMSA_HILOGE("getTextConfigFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->getTextConfigFunc = getTextConfigFunc;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextEditorProxy_SetInsertTextFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_InsertTextFunc insertTextFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (insertTextFunc == nullptr) {
        IMSA_HILOGE("insertTextFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    proxy->insertTextFunc = insertTextFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_SetDeleteForwardFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteForwardFunc deleteForwardFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (deleteForwardFunc == nullptr) {
        IMSA_HILOGE("deleteForwardFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->deleteForwardFunc = deleteForwardFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_SetDeleteBackwardFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteBackwardFunc deleteBackwardFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (deleteBackwardFunc == nullptr) {
        IMSA_HILOGE("deleteBackwardFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->deleteBackwardFunc = deleteBackwardFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_SetSendKeyboardStatusFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendKeyboardStatusFunc sendKeyboardStatusFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (sendKeyboardStatusFunc == nullptr) {
        IMSA_HILOGE("sendKeyboardStatusFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->sendKeyboardStatusFunc = sendKeyboardStatusFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_SetSendEnterKeyFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendEnterKeyFunc sendEnterKeyFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (sendEnterKeyFunc == nullptr) {
        IMSA_HILOGE("sendEnterKeyFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->sendEnterKeyFunc = sendEnterKeyFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_SetMoveCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_MoveCursorFunc moveCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (moveCursorFunc == nullptr) {
        IMSA_HILOGE("moveCursorFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->moveCursorFunc = moveCursorFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_SetHandleSetSelectionFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleSetSelectionFunc handleSetSelectionFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (handleSetSelectionFunc == nullptr) {
        IMSA_HILOGE("handleSetSelectionFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->handleSetSelectionFunc = handleSetSelectionFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_SetHandleExtendActionFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleExtendActionFunc handleExtendActionFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (handleExtendActionFunc == nullptr) {
        IMSA_HILOGE("handleExtendActionFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->handleExtendActionFunc = handleExtendActionFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_SetGetLeftTextOfCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetLeftTextOfCursorFunc getLeftTextOfCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (getLeftTextOfCursorFunc == nullptr) {
        IMSA_HILOGE("getLeftTextOfCursorFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->getLeftTextOfCursorFunc = getLeftTextOfCursorFunc;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextEditorProxy_SetGetRightTextOfCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetRightTextOfCursorFunc getRightTextOfCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (getRightTextOfCursorFunc == nullptr) {
        IMSA_HILOGE("getRightTextOfCursorFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->getRightTextOfCursorFunc = getRightTextOfCursorFunc;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextEditorProxy_SetGetTextIndexAtCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextIndexAtCursorFunc getTextIndexAtCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (getTextIndexAtCursorFunc == nullptr) {
        IMSA_HILOGE("getTextIndexAtCursorFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->getTextIndexAtCursorFunc = getTextIndexAtCursorFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_SetReceivePrivateCommandFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_ReceivePrivateCommandFunc receivePrivateCommandFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (receivePrivateCommandFunc == nullptr) {
        IMSA_HILOGE("receivePrivateCommandFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->receivePrivateCommandFunc = receivePrivateCommandFunc;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextEditorProxy_SetSetPreviewTextFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SetPreviewTextFunc setPreviewTextFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (setPreviewTextFunc == nullptr) {
        IMSA_HILOGE("setPreviewTextFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->setPreviewTextFunc = setPreviewTextFunc;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextEditorProxy_SetFinishTextPreviewFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_FinishTextPreviewFunc finishTextPreviewFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (finishTextPreviewFunc == nullptr) {
        IMSA_HILOGE("finishTextPreviewFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->finishTextPreviewFunc = finishTextPreviewFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetGetTextConfigFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextConfigFunc *getTextConfigFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (getTextConfigFunc == nullptr) {
        IMSA_HILOGE("getTextConfigFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *getTextConfigFunc = proxy->getTextConfigFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetInsertTextFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_InsertTextFunc *insertTextFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (insertTextFunc == nullptr) {
        IMSA_HILOGE("insertTextFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    *insertTextFunc = proxy->insertTextFunc;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextEditorProxy_GetDeleteForwardFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteForwardFunc *deleteForwardFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (deleteForwardFunc == nullptr) {
        IMSA_HILOGE("deleteForwardFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *deleteForwardFunc = proxy->deleteForwardFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetDeleteBackwardFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_DeleteBackwardFunc *deleteBackwardFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (deleteBackwardFunc == nullptr) {
        IMSA_HILOGE("deleteBackwardFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *deleteBackwardFunc = proxy->deleteBackwardFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetSendKeyboardStatusFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendKeyboardStatusFunc *sendKeyboardStatusFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (sendKeyboardStatusFunc == nullptr) {
        IMSA_HILOGE("sendKeyboardStatusFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *sendKeyboardStatusFunc = proxy->sendKeyboardStatusFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetSendEnterKeyFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SendEnterKeyFunc *sendEnterKeyFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (sendEnterKeyFunc == nullptr) {
        IMSA_HILOGE("sendEnterKeyFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *sendEnterKeyFunc = proxy->sendEnterKeyFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetMoveCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_MoveCursorFunc *moveCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (moveCursorFunc == nullptr) {
        IMSA_HILOGE("moveCursorFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *moveCursorFunc = proxy->moveCursorFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetHandleSetSelectionFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleSetSelectionFunc *handleSetSelectionFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (handleSetSelectionFunc == nullptr) {
        IMSA_HILOGE("handleSetSelectionFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *handleSetSelectionFunc = proxy->handleSetSelectionFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetHandleExtendActionFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_HandleExtendActionFunc *handleExtendActionFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (handleExtendActionFunc == nullptr) {
        IMSA_HILOGE("handleExtendActionFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *handleExtendActionFunc = proxy->handleExtendActionFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetGetLeftTextOfCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetLeftTextOfCursorFunc *getLeftTextOfCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (getLeftTextOfCursorFunc == nullptr) {
        IMSA_HILOGE("getLeftTextOfCursorFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *getLeftTextOfCursorFunc = proxy->getLeftTextOfCursorFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetGetRightTextOfCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetRightTextOfCursorFunc *getRightTextOfCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (getRightTextOfCursorFunc == nullptr) {
        IMSA_HILOGE("getRightTextOfCursorFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *getRightTextOfCursorFunc = proxy->getRightTextOfCursorFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetGetTextIndexAtCursorFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_GetTextIndexAtCursorFunc *getTextIndexAtCursorFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (getTextIndexAtCursorFunc == nullptr) {
        IMSA_HILOGE("getTextIndexAtCursorFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *getTextIndexAtCursorFunc = proxy->getTextIndexAtCursorFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetReceivePrivateCommandFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_ReceivePrivateCommandFunc *receivePrivateCommandFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (receivePrivateCommandFunc == nullptr) {
        IMSA_HILOGE("receivePrivateCommandFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *receivePrivateCommandFunc = proxy->receivePrivateCommandFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetSetPreviewTextFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_SetPreviewTextFunc *setPreviewTextFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (setPreviewTextFunc == nullptr) {
        IMSA_HILOGE("setPreviewTextFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *setPreviewTextFunc = proxy->setPreviewTextFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextEditorProxy_GetFinishTextPreviewFunc(
    InputMethod_TextEditorProxy *proxy, OH_TextEditorProxy_FinishTextPreviewFunc *finishTextPreviewFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (finishTextPreviewFunc == nullptr) {
        IMSA_HILOGE("finishTextPreviewFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *finishTextPreviewFunc = proxy->finishTextPreviewFunc;
    return IME_ERR_OK;
}

InputMethod_TextConfig *OH_TextConfig_New()
{
    return new InputMethod_TextConfig();
}
void OH_TextConfig_Delete(InputMethod_TextConfig *config)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return;
    }

    delete config;
}
InputMethod_ErrorCode OH_TextConfig_SetInputType(InputMethod_TextConfig *config, InputMethod_TextInputType inputType)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    config->inputType = inputType;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextConfig_SetEnterKeyType(
    InputMethod_TextConfig *config, InputMethod_EnterKeyType enterKeyType)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    config->enterKeyType = enterKeyType;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextConfig_SetIsPreviewTextSupported(InputMethod_TextConfig *config, bool supported)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    config->previewTextSupported = supported;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextConfig_SetSelection(InputMethod_TextConfig *config, int32_t start, int32_t end)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    config->selectionStart = start;
    config->selectionEnd = end;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextConfig_SetWindowId(InputMethod_TextConfig *config, int32_t windowId)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    config->windowId = windowId;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextConfig_GetInputType(InputMethod_TextConfig *config, InputMethod_TextInputType *inputType)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (inputType == nullptr) {
        IMSA_HILOGE("inputType is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *inputType = config->inputType;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextConfig_GetEnterKeyType(
    InputMethod_TextConfig *config, InputMethod_EnterKeyType *enterKeyType)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (enterKeyType == nullptr) {
        IMSA_HILOGE("enterKeyType is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *enterKeyType = config->enterKeyType;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextConfig_IsPreviewTextSupported(InputMethod_TextConfig *config, bool *supported)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (supported == nullptr) {
        IMSA_HILOGE("supported is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *supported = config->previewTextSupported;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextConfig_GetCursorInfo(InputMethod_TextConfig *config, InputMethod_CursorInfo **cursorInfo)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (cursorInfo == nullptr) {
        IMSA_HILOGE("cursorInfo is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *cursorInfo = &config->cursorInfo;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextConfig_GetTextAvoidInfo(
    InputMethod_TextConfig *config, InputMethod_TextAvoidInfo **avoidInfo)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (avoidInfo == nullptr) {
        IMSA_HILOGE("avoidInfo is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *avoidInfo = &config->avoidInfo;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextConfig_GetSelection(InputMethod_TextConfig *config, int32_t *start, int32_t *end)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *start = config->selectionStart;
    *end = config->selectionEnd;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextConfig_GetWindowId(InputMethod_TextConfig *config, int32_t *windowId)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *windowId = config->windowId;
    return IME_ERR_OK;
}

InputMethod_TextAvoidInfo *OH_TextAvoidInfo_New(double positionY, double height)
{
    return new InputMethod_TextAvoidInfo({ positionY, height });
}
void OH_TextAvoidInfo_Delete(InputMethod_TextAvoidInfo *info)
{
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr");
        return;
    }
    delete info;
}
InputMethod_ErrorCode OH_TextAvoidInfo_SetPositionY(InputMethod_TextAvoidInfo *info, double positionY)
{
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    info->positionY = positionY;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextAvoidInfo_SetHeight(InputMethod_TextAvoidInfo *info, double height)
{
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    info->height = height;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextAvoidInfo_GetPositionY(InputMethod_TextAvoidInfo *info, double *positionY)
{
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *positionY = info->positionY;
    return IME_ERR_OK;
}
InputMethod_ErrorCode OH_TextAvoidInfo_GetHeight(InputMethod_TextAvoidInfo *info, double *height)
{
    if (info == nullptr) {
        IMSA_HILOGE("info is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *height = info->height;
    return IME_ERR_OK;
}

InputMethod_PrivateCommand *OH_PrivateCommand_New(char key[], size_t keyLength)
{
    return new InputMethod_PrivateCommand({ std::string(key, keyLength), false });
}
void OH_PrivateCommand_Delete(InputMethod_PrivateCommand *command)
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

InputMethod_ErrorCode OH_PrivateCommand_GetKey(InputMethod_PrivateCommand *command, char **key, size_t keyLength)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (key == nullptr) {
        IMSA_HILOGE("key is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *key = const_cast<char *>(command->key.c_str());
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
    InputMethod_PrivateCommand *command, char **value, size_t *valueLength)
{
    if (command == nullptr) {
        IMSA_HILOGE("command is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (value == nullptr) {
        IMSA_HILOGE("value is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (!std::holds_alternative<std::string>(command->value)) {
        IMSA_HILOGE("value is not string");
        return IME_ERR_QUERY_FAILED;
    }

    *value = const_cast<char *>(std::get<std::string>(command->value).c_str());
    *valueLength = std::get<std::string>(command->value).length();
    return IME_ERR_OK;
}

const std::map<int32_t, InputMethod_ErrorCode> ERROR_CODE_MAP = {
    { ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED,   IME_ERR_CONTROLLER     },
    { ErrorCode::ERROR_REMOTE_CLIENT_DIED,           IME_ERR_IMCLIENT       },
    { ErrorCode::ERROR_CLIENT_NOT_FOUND,             IME_ERR_IMCLIENT       },
    { ErrorCode::ERROR_CLIENT_NULL_POINTER,          IME_ERR_IMCLIENT       },
    { ErrorCode::ERROR_CLIENT_NOT_FOCUSED,           IME_ERR_IMCLIENT       },
    { ErrorCode::ERROR_CLIENT_NOT_EDITABLE,          IME_ERR_IMCLIENT       },
    { ErrorCode::ERROR_CLIENT_NOT_BOUND,             IME_ERR_DETACHED       },
    { ErrorCode::ERROR_CLIENT_ADD_FAILED,            IME_ERR_IMCLIENT       },
    { ErrorCode::ERROR_NULL_POINTER,                 IME_ERR_IMMS           },
    { ErrorCode::ERROR_BAD_PARAMETERS,               IME_ERR_IMMS           },
    { ErrorCode::ERROR_SERVICE_START_FAILED,         IME_ERR_IMMS           },
    { ErrorCode::ERROR_IME_START_FAILED,             IME_ERR_IMMS           },
    { ErrorCode::ERROR_KBD_SHOW_FAILED,              IME_ERR_IMMS           },
    { ErrorCode::ERROR_KBD_HIDE_FAILED,              IME_ERR_IMMS           },
    { ErrorCode::ERROR_IME_NOT_STARTED,              IME_ERR_IMMS           },
    { ErrorCode::ERROR_EX_NULL_POINTER,              IME_ERR_IMMS           },
    { ErrorCode::ERROR_PACKAGE_MANAGER,              IME_ERR_PACKAGEMANAGER },
    { ErrorCode::ERROR_EX_UNSUPPORTED_OPERATION,     IME_ERR_IMMS           },
    { ErrorCode::ERROR_EX_SERVICE_SPECIFIC,          IME_ERR_IMMS           },
    { ErrorCode::ERROR_EX_PARCELABLE,                IME_ERR_IMMS           },
    { ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT,          IME_ERR_IMMS           },
    { ErrorCode::ERROR_EX_ILLEGAL_STATE,             IME_ERR_IMMS           },
    { ErrorCode::ERROR_IME_START_INPUT_FAILED,       IME_ERR_IMMS           },
    { ErrorCode::ERROR_IME,                          IME_ERR_IMENGINE       },
    { ErrorCode::ERROR_PARAMETER_CHECK_FAILED,       IME_ERR_PARAMCHECK     },
    { ErrorCode::ERROR_ENABLE_IME,                   IME_ERR_IMMS           },
    { ErrorCode::ERROR_NOT_CURRENT_IME,              IME_ERR_IMMS           },
    { ErrorCode::ERROR_GET_TEXT_CONFIG,              IME_ERR_IMCLIENT       },
    { ErrorCode::ERROR_INVALID_PRIVATE_COMMAND_SIZE, IME_ERR_PARAMCHECK     },
    { ErrorCode::ERROR_TEXT_LISTENER_ERROR,          IME_ERR_IMCLIENT       },
    { ErrorCode::ERROR_INVALID_RANGE,                IME_ERR_PARAMCHECK     },
};

InputMethod_ErrorCode ErrorCodeConvert(int32_t code)
{
    IMSA_HILOGD("Convert start.");
    auto iter = ERROR_CODE_MAP.find(code);
    if (iter != ERROR_CODE_MAP.end()) {
        IMSA_HILOGE("ErrorCode: %{public}d", iter->second);
        return iter->second;
    }
    IMSA_HILOGD("Convert end.");
    return IME_ERR_QUERY_FAILED;
}