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
#include "securec.h"

#include "global.h"
#include "itypes_util.h"
#include "native_inputmethod_types.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
constexpr int32_t MAX_PLACEHOLDER_SIZE = 256; // 256 utf-16 chars
constexpr int32_t MAX_ABILITY_NAME_SIZE = 32; // 32 utf-16 chars

InputMethod_TextConfig *OH_TextConfig_Create(void)
{
    return new InputMethod_TextConfig();
}
void OH_TextConfig_Destroy(InputMethod_TextConfig *config)
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
InputMethod_ErrorCode OH_TextConfig_SetPreviewTextSupport(InputMethod_TextConfig *config, bool supported)
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

InputMethod_ErrorCode OH_TextConfig_SetPlaceholder(InputMethod_TextConfig *config, const char16_t *placeholder,
    size_t length)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (length <= 0) {
        config->placeholderLength = 0;
        return IME_ERR_OK;
    }
    if (length > MAX_PLACEHOLDER_INPUT_SIZE) {
        IMSA_HILOGE("chars length exceeds limit inputLen:%{public}zu, limit len:%{public}zu", length,
            MAX_PLACEHOLDER_INPUT_SIZE);
        return IME_ERR_PARAMCHECK;
    }
    if (placeholder == nullptr) {
        IMSA_HILOGE("placeholder is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    std::u16string u16Placeholder(placeholder, length);
    int32_t charsLen = OHOS::MiscServices::ITypesUtil::CountUtf16Chars(u16Placeholder);
    if (charsLen > MAX_PLACEHOLDER_SIZE) {
        IMSA_HILOGE("chars length exceeds limit inputLen:%{public}d", charsLen);
        return IME_ERR_PARAMCHECK;
    }
    auto byteLen = length * sizeof(char16_t);
    errno_t err = memcpy_s(config->placeholder, (MAX_PLACEHOLDER_INPUT_SIZE + 1) * sizeof(char16_t),
        placeholder, byteLen);
    if (err != EOK) {
        IMSA_HILOGE("placeholder content copy error:%{public}d", (int32_t)err);
        return IME_ERR_PARAMCHECK;
    }
    config->placeholderLength = length;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextConfig_SetAbilityName(InputMethod_TextConfig *config, const char16_t *abilityName,
    size_t length)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (length <= 0) {
        config->abilityNameLength = 0;
        return IME_ERR_OK;
    }
    if (abilityName == nullptr) {
        IMSA_HILOGE("abilityName is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (length > MAX_ABILITY_NAME_INPUT_SIZE) {
        IMSA_HILOGE("chars length exceeds limit inputLen:%{public}zu", MAX_ABILITY_NAME_INPUT_SIZE);
        return IME_ERR_PARAMCHECK;
    }
    std::u16string u16AbilityName(abilityName, length);
    int32_t charsLen = OHOS::MiscServices::ITypesUtil::CountUtf16Chars(u16AbilityName);
    if (charsLen > MAX_ABILITY_NAME_SIZE) {
        IMSA_HILOGE("length exceeds limit inputLen:%{public}d", charsLen);
        return IME_ERR_PARAMCHECK;
    }
    auto byteLen = length * sizeof(char16_t);
    errno_t err = memcpy_s(config->abilityName, (MAX_ABILITY_NAME_INPUT_SIZE + 1) * sizeof(char16_t),
        abilityName, byteLen);
    if (err != EOK) {
        IMSA_HILOGE("abilityName content copy error:%{public}d", (int32_t)err);
        return IME_ERR_PARAMCHECK;
    }
    config->abilityNameLength = length;
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

InputMethod_ErrorCode OH_TextConfig_GetPlaceholder(InputMethod_TextConfig *config, char16_t *placeholder,
    size_t *length)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (length == nullptr) {
        IMSA_HILOGE("length is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (placeholder == nullptr) {
        IMSA_HILOGE("placeholder is nullptr");
        *length = config->placeholderLength;
        return IME_ERR_NULL_POINTER;
    }
    if ((*length) < config->placeholderLength) {
        IMSA_HILOGE("input memory is less than the length of the obtained memory. actual length:%{public}zu",
            config->placeholderLength);
        *length = config->placeholderLength;
        return IME_ERR_PARAMCHECK;
    }
    if ((*length) > MAX_PLACEHOLDER_INPUT_SIZE) {
        IMSA_HILOGE("input memory exceeds the limit. actual length:%{public}zu",
            config->placeholderLength);
        *length = config->placeholderLength;
        return IME_ERR_PARAMCHECK;
    }
    if (config->placeholderLength <=0) {
        *length = config->placeholderLength;
        return IME_ERR_OK;
    }
    auto byteLen = (*length) * sizeof(char16_t);
    *length = config->placeholderLength;
    errno_t err = memcpy_s(placeholder, byteLen, config->placeholder, config->placeholderLength * sizeof(char16_t));
    if (err != EOK) {
        IMSA_HILOGE("placeholder content copy error:%{public}d", (int32_t)err);
        return IME_ERR_PARAMCHECK;
    }
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_TextConfig_GetAbilityName(InputMethod_TextConfig *config, char16_t *abilityName,
    size_t *length)
{
    if (config == nullptr) {
        IMSA_HILOGE("config is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (length == nullptr) {
        IMSA_HILOGE("length is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (abilityName == nullptr) {
        IMSA_HILOGE("abilityName is nullptr");
        *length = config->abilityNameLength;
        return IME_ERR_NULL_POINTER;
    }
    if ((*length) < config->abilityNameLength) {
        IMSA_HILOGE("input memory is less than the length of the obtained memory. actual length:%{public}zu",
            config->abilityNameLength);
        *length = config->abilityNameLength;
        return IME_ERR_PARAMCHECK;
    }
    if ((*length) > MAX_ABILITY_NAME_INPUT_SIZE) {
        IMSA_HILOGE("input memory exceeds the limit. actual length:%{public}zu",
            config->abilityNameLength);
        *length = config->abilityNameLength;
        return IME_ERR_PARAMCHECK;
    }
    if (config->abilityNameLength <= 0) {
        *length = config->abilityNameLength;
        return IME_ERR_OK;
    }
    auto byteLen = (*length) * sizeof(char16_t);
    *length = config->abilityNameLength;
    errno_t err = memcpy_s(abilityName, byteLen, config->abilityName, config->abilityNameLength * sizeof(char16_t));
    if (err != EOK) {
        IMSA_HILOGE("abilityName content copy error:%{public}d", (int32_t)err);
        return IME_ERR_PARAMCHECK;
    }
    return IME_ERR_OK;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */