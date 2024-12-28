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

#include <gtest/gtest.h>
#include "native_inputmethod_types.h"

using namespace testing;

HWTEST_F(OH_CursorInfo_Create, ValidInput_CreatesCursorInfo)
{
    InputMethod_CursorInfo *cursorInfo = OH_CursorInfo_Create(10.0, 20.0, 30.0, 40.0);
    EXPECT_NE(cursorInfo, nullptr);
    EXPECT_EQ(cursorInfo->left, 10.0);
    EXPECT_EQ(cursorInfo->top, 20.0);
    EXPECT_EQ(cursorInfo->width, 30.0);
    EXPECT_EQ(cursorInfo->height, 40.0);
    OH_CursorInfo_Destroy(cursorInfo);
}

HWTEST_F(OH_CursorInfo_Destroy, ValidCursorInfo_DestroysWithoutError)
{
    InputMethod_CursorInfo *cursorInfo = OH_CursorInfo_Create(10.0, 20.0, 30.0, 40.0);
    EXPECT_NE(cursorInfo, nullptr);
    OH_CursorInfo_Destroy(cursorInfo);
    // No assertion needed as the function should not throw exceptions
}

HWTEST_F(OH_CursorInfo_Destroy, NullPointer_NoError)
{
    OH_CursorInfo_Destroy(nullptr);
    // No assertion needed as the function should not throw exceptions
}

HWTEST_F(OH_CursorInfo_SetRect, ValidCursorInfo_SetsDimensions)
{
    InputMethod_CursorInfo *cursorInfo = OH_CursorInfo_Create(10.0, 20.0, 30.0, 40.0);
    EXPECT_NE(cursorInfo, nullptr);
    InputMethod_ErrorCode errorCode = OH_CursorInfo_SetRect(cursorInfo, 100.0, 200.0, 300.0, 400.0);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    EXPECT_EQ(cursorInfo->left, 100.0);
    EXPECT_EQ(cursorInfo->top, 200.0);
    EXPECT_EQ(cursorInfo->width, 300.0);
    EXPECT_EQ(cursorInfo->height, 400.0);
    OH_CursorInfo_Destroy(cursorInfo);
}

HWTEST_F(OH_CursorInfo_SetRect, NullCursorInfo_ReturnsError)
{
    InputMethod_ErrorCode errorCode = OH_CursorInfo_SetRect(nullptr, 100.0, 200.0, 300.0, 400.0);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
}

HWTEST_F(OH_CursorInfo_GetRect, ValidCursorInfo_RetrievesDimensions)
{
    InputMethod_CursorInfo *cursorInfo = OH_CursorInfo_Create(10.0, 20.0, 30.0, 40.0);
    EXPECT_NE(cursorInfo, nullptr);
    double left, top, width, height;
    InputMethod_ErrorCode errorCode = OH_CursorInfo_GetRect(cursorInfo, &left, &top, &width, &height);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    EXPECT_EQ(left, 10.0);
    EXPECT_EQ(top, 20.0);
    EXPECT_EQ(width, 30.0);
    EXPECT_EQ(height, 40.0);
    OH_CursorInfo_Destroy(cursorInfo);
}

HWTEST_F(OH_CursorInfo_GetRect, NullCursorInfo_ReturnsError)
{
    double left, top, width, height;
    InputMethod_ErrorCode errorCode = OH_CursorInfo_GetRect(nullptr, &left, &top, &width, &height);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
}

HWTEST_F(OH_CursorInfo_GetRect, NullOutputParameters_ReturnsError)
{
    InputMethod_CursorInfo *cursorInfo = OH_CursorInfo_Create(10.0, 20.0, 30.0, 40.0);
    EXPECT_NE(cursorInfo, nullptr);
    InputMethod_ErrorCode errorCode = OH_CursorInfo_GetRect(cursorInfo, nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
    OH_CursorInfo_Destroy(cursorInfo);
}