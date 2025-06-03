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

using namespace OHOS::MiscServices;
InputMethod_TextEditorProxy *OH_TextEditorProxy_Create(void)
{
    return new (std::nothrow) InputMethod_TextEditorProxy();
}
void OH_TextEditorProxy_Destroy(InputMethod_TextEditorProxy *proxy)
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