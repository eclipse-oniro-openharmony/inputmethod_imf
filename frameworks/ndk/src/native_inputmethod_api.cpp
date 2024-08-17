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
#include <map>
#include <mutex>

#include "global.h"
#include "input_method_controller.h"
#include "input_method_utils.h"
#include "inputmethod_controller_capi.h"
#include "native_text_editor.h"

using namespace OHOS::MiscServices;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
struct OH_InputMethod_AttachOptions {
    bool showKeyboard;
};

struct OH_InputMethod_InputMethodProxy {
    OH_InputMethod_TextEditorProxy *textEditor = nullptr;
    OHOS::sptr<NativeTextChangedListener> listener = nullptr;
};

OH_InputMethod_InputMethodProxy *g_inputMethodProxy = nullptr;
std::mutex g_textEditorProxyMapMutex;

int32_t OH_InputMethodProxy_ShowKeyboard(OH_InputMethod_InputMethodProxy *inputMethodProxy)
{
    return ErrorCodeConvert(InputMethodController::GetInstance()->ShowCurrentInput());
}
int32_t OH_InputMethodProxy_HideKeyboard(OH_InputMethod_InputMethodProxy *inputMethodProxy)
{
    return ErrorCodeConvert(InputMethodController::GetInstance()->HideCurrentInput());
}
int32_t OH_InputMethodProxy_NotifySelectionChange(
    OH_InputMethod_InputMethodProxy *inputMethodProxy, char16_t text[], size_t length, int start, int end)
{
    return ErrorCodeConvert(
        InputMethodController::GetInstance()->OnSelectionChange(std::u16string(text, length), start, end));
}
int32_t OH_InputMethodProxy_NotifyConfigurationChange(OH_InputMethod_InputMethodProxy *inputMethodProxy,
    InputMethod_EnterKeyType enterKey, InputMethod_TextInputType textType)
{
    Configuration info;
    info.SetEnterKeyType(static_cast<EnterKeyType>(enterKey));
    info.SetTextInputType(static_cast<TextInputType>(textType));
    return ErrorCodeConvert(InputMethodController::GetInstance()->OnConfigurationChange(info));
}

OH_InputMethod_CursorInfo *OH_CursorInfo_New(double left, double top, double width, double height)
{
    return new OH_InputMethod_CursorInfo({ left, top, width, height });
}
void OH_CursorInfo_Delete(OH_InputMethod_CursorInfo *cursorInfo)
{
    if (cursorInfo == nullptr) {
        return;
    }
    delete cursorInfo;
}

int32_t OH_CursorInfo_SetRect(
    OH_InputMethod_CursorInfo *cursorInfo, double left, double top, double width, double height)
{
    if (cursorInfo == nullptr) {
        IMSA_HILOGE("cursorInfo is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    cursorInfo->left = left;
    cursorInfo->top = top;
    cursorInfo->width = width;
    cursorInfo->height = height;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_CursorInfo_GetRect(
    OH_InputMethod_CursorInfo *cursorInfo, double *left, double *top, double *width, double *height)
{
    if (cursorInfo == nullptr) {
        IMSA_HILOGE("cursorInfo is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (left == nullptr || top == nullptr || width == nullptr || height == nullptr) {
        IMSA_HILOGE("invalid parameter");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    *left = cursorInfo->left;
    *top = cursorInfo->top;
    *width = cursorInfo->width;
    *height = cursorInfo->height;
    return INPUT_METHOD_ERR_OK;
}
int32_t OH_InputMethodProxy_NotifyCursorUpdate(
    OH_InputMethod_InputMethodProxy *inputMethodProxy, OH_InputMethod_CursorInfo *cursorInfo)
{
    if (cursorInfo == nullptr) {
        IMSA_HILOGE("cursorInfo is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    return ErrorCodeConvert(InputMethodController::GetInstance()->OnCursorUpdate(
        CursorInfo({ cursorInfo->left, cursorInfo->top, cursorInfo->width, cursorInfo->height })));
}

int32_t OH_InputMethodProxy_SendPrivateCommand(OH_InputMethod_PrivateCommand *privateCommand[], size_t size)
{
    if (privateCommand == nullptr) {
        IMSA_HILOGE("privateCommand is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    std::unordered_map<std::string, PrivateDataValue> command;

    for (size_t i = 0; i < size; i++) {
        if (privateCommand[i] == nullptr) {
            IMSA_HILOGE("privateCommand[%zu] is nullptr", i);
            return INPUT_METHOD_ERR_NULL_POINTER;
        }
        command.emplace(privateCommand[i]->key, privateCommand[i]->value);
    }
    return ErrorCodeConvert(InputMethodController::GetInstance()->SendPrivateCommand(command));
}

OH_InputMethod_AttachOptions *OH_AttachOptions_New(bool showKeyboard)
{
    return new OH_InputMethod_AttachOptions({ showKeyboard });
}
void OH_AttachOptions_Delete(OH_InputMethod_AttachOptions *options)
{
    if (options == nullptr) {
        return;
    }
    delete options;
}
int32_t OH_AttachOptions_SetShowKeyboard(OH_InputMethod_AttachOptions *options, bool showKeyboard)
{
    if (options == nullptr) {
        IMSA_HILOGE("options is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    options->showKeyboard = showKeyboard;
    return INPUT_METHOD_ERR_OK;
}

int32_t OH_AttachOptions_IsShowKeyboard(OH_InputMethod_AttachOptions *options, bool *showKeyboard)
{
    if (options == nullptr) {
        IMSA_HILOGE("options is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (showKeyboard == nullptr) {
        IMSA_HILOGE("showKeyboard is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    *showKeyboard = options->showKeyboard;
    return INPUT_METHOD_ERR_OK;
}

static int32_t GetInputMethodProxy(OH_InputMethod_TextEditorProxy *textEditor)
{
    std::lock_guard<std::mutex> guard(g_textEditorProxyMapMutex);
    if (g_inputMethodProxy != nullptr && textEditor == g_inputMethodProxy->textEditor) {
        return INPUT_METHOD_ERR_OK;
    }

    if (g_inputMethodProxy != nullptr && textEditor != g_inputMethodProxy->textEditor) {
        g_inputMethodProxy->listener = nullptr;
        delete g_inputMethodProxy;
        g_inputMethodProxy = nullptr;
    }
    OHOS::sptr<NativeTextChangedListener> listener = new NativeTextChangedListener(textEditor);
    if (listener == nullptr) {
        IMSA_HILOGE("new NativeTextChangedListener failed");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    g_inputMethodProxy = new OH_InputMethod_InputMethodProxy({ textEditor, listener });
    if (g_inputMethodProxy == nullptr) {
        IMSA_HILOGE("new OH_InputMethod_InputMethodProxy failed");
        listener = nullptr;
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    return INPUT_METHOD_ERR_OK;
}

static int32_t IsValidTextEditorProxy(OH_InputMethod_TextEditorProxy *textEditor)
{
    if (textEditor == nullptr) {
        IMSA_HILOGE("textEditor is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (textEditor->getTextConfigFunc == nullptr) {
        IMSA_HILOGE("textEditor->getTextConfigFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (textEditor->insertTextFunc == nullptr) {
        IMSA_HILOGE("textEditor->insertTextFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (textEditor->deleteForwardFunc == nullptr) {
        IMSA_HILOGE("textEditor->deleteForwardFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (textEditor->deleteBackwardFunc == nullptr) {
        IMSA_HILOGE("textEditor->deleteBackwardFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (textEditor->sendKeyboardStatusFunc == nullptr) {
        IMSA_HILOGE("textEditor->sendKeyboardStatusFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (textEditor->sendEnterKeyFunc == nullptr) {
        IMSA_HILOGE("textEditor->sendEnterKeyFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (textEditor->moveCursorFunc == nullptr) {
        IMSA_HILOGE("textEditor->moveCursorFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (textEditor->handleSetSelectionFunc == nullptr) {
        IMSA_HILOGE("textEditor->handleSetSelectionFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (textEditor->handleExtendActionFunc == nullptr) {
        IMSA_HILOGE("textEditor->handleExtendActionFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (textEditor->getLeftTextOfCursorFunc == nullptr) {
        IMSA_HILOGE("textEditor->getLeftTextOfCursorFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (textEditor->getRightTextOfCursorFunc == nullptr) {
        IMSA_HILOGE("textEditor->getRightTextOfCursorFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (textEditor->getTextIndexAtCursorFunc == nullptr) {
        IMSA_HILOGE("textEditor->getTextIndexAtCursorFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (textEditor->receivePrivateCommandFunc == nullptr) {
        IMSA_HILOGE("textEditor->receivePrivateCommandFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    if (textEditor->setPreviewTextFunc == nullptr) {
        IMSA_HILOGE("textEditor->setPreviewTextFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    if (textEditor->finishTextPreviewFunc) {
        IMSA_HILOGE("textEditor->finishTextPreviewFunc is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    return INPUT_METHOD_ERR_OK;
}

int32_t OH_InputMethodController_Attach(OH_InputMethod_TextEditorProxy *textEditor,
    OH_InputMethod_AttachOptions *options, OH_InputMethod_InputMethodProxy **inputMethodProxy)
{
    if ((IsValidTextEditorProxy(textEditor) != INPUT_METHOD_ERR_OK) || options == nullptr ||
        inputMethodProxy == nullptr) {
        IMSA_HILOGE("invalid parameter");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }

    int32_t errCode = GetInputMethodProxy(textEditor);
    if (errCode != INPUT_METHOD_ERR_OK) {
        return errCode;
    }

    OH_InputMethod_TextConfig config;
    textEditor->getTextConfigFunc(textEditor, &config);

    TextConfig textConfig = {
        .inputAttribute = {
            .inputPattern = static_cast<InputMethod_TextInputType>(config.inputType),
            .enterKeyType = static_cast<InputMethod_EnterKeyType>(config.enterKeyType),
            .isTextPreviewSupported = config.previewTextSupported,
        },
        .cursorInfo = {
            .left = config.cursorInfo.left,
            .top = config.cursorInfo.top,
            .width = config.cursorInfo.width,
            .height = config.cursorInfo.height,
        },
        .range = {
            .start = config.selectionStart,
            .end = config.selectionEnd,
        },
        .windowId = config.windowId,
        .positionY = config.avoidInfo.positionY,
        .height = config.avoidInfo.height,
    };

    auto controller = InputMethodController::GetInstance();
    OHOS::sptr<NativeTextChangedListener> listener = nullptr;
    {
        std::lock_guard<std::mutex> guard(g_textEditorProxyMapMutex);
        listener = g_inputMethodProxy->listener;
    }
    errCode = controller->Attach(listener, options->showKeyboard, textConfig);
    if (errCode == ErrorCode::NO_ERROR) {
        std::lock_guard<std::mutex> guard(g_textEditorProxyMapMutex);
        *inputMethodProxy = g_inputMethodProxy;
    } else {
        errCode = ErrorCodeConvert(errCode);
    }

    return errCode;
}

int32_t OH_InputMethodController_Detach(OH_InputMethod_InputMethodProxy *inputMethodProxy)
{
    if (inputMethodProxy == nullptr) {
        IMSA_HILOGE("inputMethodProxy is nullptr");
        return INPUT_METHOD_ERR_NULL_POINTER;
    }
    {
        std::lock_guard<std::mutex> guard(g_textEditorProxyMapMutex);
        if (g_inputMethodProxy != nullptr && inputMethodProxy == g_inputMethodProxy) {
            g_inputMethodProxy->listener = nullptr;
            delete g_inputMethodProxy;
            g_inputMethodProxy = nullptr;
        }
    }
    return ErrorCodeConvert(InputMethodController::GetInstance()->Close());
}
#ifdef __cplusplus
}
#endif /* __cplusplus */