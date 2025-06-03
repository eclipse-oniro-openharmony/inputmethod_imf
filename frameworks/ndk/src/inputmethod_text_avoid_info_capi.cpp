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

InputMethod_TextAvoidInfo *OH_TextAvoidInfo_Create(double positionY, double height)
{
    return new (std::nothrow) InputMethod_TextAvoidInfo({ positionY, height });
}
void OH_TextAvoidInfo_Destroy(InputMethod_TextAvoidInfo *info)
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
    if (positionY == nullptr) {
        IMSA_HILOGE("positionY is nullptr");
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
    if (height == nullptr) {
        IMSA_HILOGE("height is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *height = info->height;
    return IME_ERR_OK;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */