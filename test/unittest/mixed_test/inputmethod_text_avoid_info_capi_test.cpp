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

#include "gtest/gtest.h"
#include "native_inputmethod_types.h"

namespace OHOS {
namespace MiscServices {
namespace {
    const double POSITION_Y = 10.0;
    const double HEIGHT = 20.0;
    const double NEW_POSITION_Y = 30.0;
    const double NEW_HEIGHT = 40.0;
}

HWTEST_F(TextAvoidInfoTest, Create_ValidInputs_ObjectCreated)
{
    InputMethod_TextAvoidInfo *info = OH_TextAvoidInfo_Create(POSITION_Y, HEIGHT);
    EXPECT_NE(info, nullptr);
    EXPECT_EQ(info->positionY, POSITION_Y);
    EXPECT_EQ(info->height, HEIGHT);
    OH_TextAvoidInfo_Destroy(info);
}

HWTEST_F(TextAvoidInfoTest, Destroy_ValidObject_ObjectDestroyed)
{
    InputMethod_TextAvoidInfo *info = OH_TextAvoidInfo_Create(POSITION_Y, HEIGHT);
    EXPECT_NE(info, nullptr);
    OH_TextAvoidInfo_Destroy(info);
    // No way to directly verify destruction, but ensure no memory leaks
}

HWTEST_F(TextAvoidInfoTest, Destroy_NullPointer_NoError)
{
    OH_TextAvoidInfo_Destroy(nullptr);
    // Ensure no error occurs
}

HWTEST_F(TextAvoidInfoTest, SetPositionY_ValidObject_PositionSet)
{
    InputMethod_TextAvoidInfo *info = OH_TextAvoidInfo_Create(POSITION_Y, HEIGHT);
    EXPECT_NE(info, nullptr);
    InputMethod_ErrorCode errorCode = OH_TextAvoidInfo_SetPositionY(info, NEW_POSITION_Y);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    EXPECT_EQ(info->positionY, NEW_POSITION_Y);
    OH_TextAvoidInfo_Destroy(info);
}

HWTEST_F(TextAvoidInfoTest, SetPositionY_NullPointer_ReturnsError)
{
    InputMethod_ErrorCode errorCode = OH_TextAvoidInfo_SetPositionY(nullptr, NEW_POSITION_Y);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
}

HWTEST_F(TextAvoidInfoTest, SetHeight_ValidObject_HeightSet)
{
    InputMethod_TextAvoidInfo *info = OH_TextAvoidInfo_Create(POSITION_Y, HEIGHT);
    EXPECT_NE(info, nullptr);
    InputMethod_ErrorCode errorCode = OH_TextAvoidInfo_SetHeight(info, NEW_HEIGHT);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    EXPECT_EQ(info->height, NEW_HEIGHT);
    OH_TextAvoidInfo_Destroy(info);
}

HWTEST_F(TextAvoidInfoTest, SetHeight_NullPointer_ReturnsError)
{
    InputMethod_ErrorCode errorCode = OH_TextAvoidInfo_SetHeight(nullptr, NEW_HEIGHT);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
}

HWTEST_F(TextAvoidInfoTest, GetPositionY_ValidInputs_PositionRetrieved)
{
    InputMethod_TextAvoidInfo *info = OH_TextAvoidInfo_Create(POSITION_Y, HEIGHT);
    EXPECT_NE(info, nullptr);
    double retrievedPositionY;
    InputMethod_ErrorCode errorCode = OH_TextAvoidInfo_GetPositionY(info, &retrievedPositionY);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    EXPECT_EQ(retrievedPositionY, POSITION_Y);
    OH_TextAvoidInfo_Destroy(info);
}

HWTEST_F(TextAvoidInfoTest, GetPositionY_NullObject_ReturnsError)
{
    double retrievedPositionY;
    InputMethod_ErrorCode errorCode = OH_TextAvoidInfo_GetPositionY(nullptr, &retrievedPositionY);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
}

HWTEST_F(TextAvoidInfoTest, GetPositionY_NullPointer_ReturnsError)
{
    InputMethod_TextAvoidInfo *info = OH_TextAvoidInfo_Create(POSITION_Y, HEIGHT);
    EXPECT_NE(info, nullptr);
    InputMethod_ErrorCode errorCode = OH_TextAvoidInfo_GetPositionY(info, nullptr);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
    OH_TextAvoidInfo_Destroy(info);
}

HWTEST_F(TextAvoidInfoTest, GetHeight_ValidInputs_HeightRetrieved)
{
    InputMethod_TextAvoidInfo *info = OH_TextAvoidInfo_Create(POSITION_Y, HEIGHT);
    EXPECT_NE(info, nullptr);
    double retrievedHeight;
    InputMethod_ErrorCode errorCode = OH_TextAvoidInfo_GetHeight(info, &retrievedHeight);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    EXPECT_EQ(retrievedHeight, HEIGHT);
    OH_TextAvoidInfo_Destroy(info);
}

HWTEST_F(TextAvoidInfoTest, GetHeight_NullObject_ReturnsError)
{
    double retrievedHeight;
    InputMethod_ErrorCode errorCode = OH_TextAvoidInfo_GetHeight(nullptr, &retrievedHeight);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
}

HWTEST_F(TextAvoidInfoTest, GetHeight_NullPointer_ReturnsError)
{
    InputMethod_TextAvoidInfo *info = OH_TextAvoidInfo_Create(POSITION_Y, HEIGHT);
    EXPECT_NE(info, nullptr);
    InputMethod_ErrorCode errorCode = OH_TextAvoidInfo_GetHeight(info, nullptr);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
    OH_TextAvoidInfo_Destroy(info);
}
} // namespace MiscServices
} // namespace OHOS