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
InputMethod_CursorInfo *OH_CursorInfo_Create(double left, double top, double width, double height)
{
    return new (std::nothrow) InputMethod_CursorInfo({ left, top, width, height });
}
void OH_CursorInfo_Destroy(InputMethod_CursorInfo *cursorInfo)
{
    if (cursorInfo == nullptr) {
        return;
    }
    delete cursorInfo;
}

InputMethod_ErrorCode OH_CursorInfo_SetRect(
    InputMethod_CursorInfo *cursorInfo, double left, double top, double width, double height)
{
    if (cursorInfo == nullptr) {
        IMSA_HILOGE("cursorInfo is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    cursorInfo->left = left;
    cursorInfo->top = top;
    cursorInfo->width = width;
    cursorInfo->height = height;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_CursorInfo_GetRect(
    InputMethod_CursorInfo *cursorInfo, double *left, double *top, double *width, double *height)
{
    if (cursorInfo == nullptr) {
        IMSA_HILOGE("cursorInfo is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (left == nullptr || top == nullptr || width == nullptr || height == nullptr) {
        IMSA_HILOGE("invalid parameter");
        return IME_ERR_NULL_POINTER;
    }
    *left = cursorInfo->left;
    *top = cursorInfo->top;
    *width = cursorInfo->width;
    *height = cursorInfo->height;
    return IME_ERR_OK;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */