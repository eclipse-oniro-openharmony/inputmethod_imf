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
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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
    if (start == nullptr) {
        IMSA_HILOGE("start is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (end == nullptr) {
        IMSA_HILOGE("end is nullptr");
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
    if (windowId == nullptr) {
        IMSA_HILOGE("windowId is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *windowId = config->windowId;
    return IME_ERR_OK;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */