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
#include "input_method_controller.h"
#include "native_inputmethod_types.h"
#include "native_inputmethod_utils.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
using namespace OHOS::MiscServices;
InputMethod_ErrorCode OH_InputMethodProxy_ShowKeyboard(InputMethod_InputMethodProxy *inputMethodProxy)
{
    return ErrorCodeConvert(InputMethodController::GetInstance()->ShowCurrentInput());
}
InputMethod_ErrorCode OH_InputMethodProxy_HideKeyboard(InputMethod_InputMethodProxy *inputMethodProxy)
{
    return ErrorCodeConvert(InputMethodController::GetInstance()->HideCurrentInput());
}
InputMethod_ErrorCode OH_InputMethodProxy_NotifySelectionChange(
    InputMethod_InputMethodProxy *inputMethodProxy, char16_t text[], size_t length, int start, int end)
{
    return ErrorCodeConvert(
        InputMethodController::GetInstance()->OnSelectionChange(std::u16string(text, length), start, end));
}
InputMethod_ErrorCode OH_InputMethodProxy_NotifyConfigurationChange(InputMethod_InputMethodProxy *inputMethodProxy,
    InputMethod_EnterKeyType enterKey, InputMethod_TextInputType textType)
{
    Configuration info;
    info.SetEnterKeyType(static_cast<EnterKeyType>(enterKey));
    info.SetTextInputType(static_cast<TextInputType>(textType));
    return ErrorCodeConvert(InputMethodController::GetInstance()->OnConfigurationChange(info));
}

InputMethod_ErrorCode OH_InputMethodProxy_NotifyCursorUpdate(
    InputMethod_InputMethodProxy *inputMethodProxy, InputMethod_CursorInfo *cursorInfo)
{
    if (cursorInfo == nullptr) {
        IMSA_HILOGE("cursorInfo is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    return ErrorCodeConvert(InputMethodController::GetInstance()->OnCursorUpdate(
        CursorInfo({ cursorInfo->left, cursorInfo->top, cursorInfo->width, cursorInfo->height })));
}

InputMethod_ErrorCode OH_InputMethodProxy_SendPrivateCommand(InputMethod_PrivateCommand *privateCommand[], size_t size)
{
    if (privateCommand == nullptr) {
        IMSA_HILOGE("privateCommand is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    std::unordered_map<std::string, PrivateDataValue> command;

    for (size_t i = 0; i < size; i++) {
        if (privateCommand[i] == nullptr) {
            IMSA_HILOGE("privateCommand[%zu] is nullptr", i);
            return IME_ERR_NULL_POINTER;
        }
        command.emplace(privateCommand[i]->key, privateCommand[i]->value);
    }
    return ErrorCodeConvert(InputMethodController::GetInstance()->SendPrivateCommand(command));
}
#ifdef __cplusplus
}
#endif /* __cplusplus */