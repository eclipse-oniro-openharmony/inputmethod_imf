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

HWTEST_F(OH_AttachOptions_Create, CreateWithTrue_ShouldSetShowKeyboardTrue)
{
    InputMethod_AttachOptions *options = OH_AttachOptions_Create(true);
    bool showKeyboard = false;
    InputMethod_ErrorCode errorCode = OH_AttachOptions_IsShowKeyboard(options, &showKeyboard);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    EXPECT_TRUE(showKeyboard);
    OH_AttachOptions_Destroy(options);
}

HWTEST_F(OH_AttachOptions_Create, CreateWithFalse_ShouldSetShowKeyboardFalse)
{
    InputMethod_AttachOptions *options = OH_AttachOptions_Create(false);
    bool showKeyboard = true;
    InputMethod_ErrorCode errorCode = OH_AttachOptions_IsShowKeyboard(options, &showKeyboard);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    EXPECT_FALSE(showKeyboard);
    OH_AttachOptions_Destroy(options);
}

HWTEST_F(OH_AttachOptions_Destroy, DestroyNullPointer_ShouldDoNothing)
{
    // 不应该抛出异常或崩溃
    OH_AttachOptions_Destroy(nullptr);
}

HWTEST_F(OH_AttachOptions_IsShowKeyboard, OptionsNull_ShouldReturnNullPointerError)
{
    bool showKeyboard = false;
    InputMethod_ErrorCode errorCode = OH_AttachOptions_IsShowKeyboard(nullptr, &showKeyboard);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
}

HWTEST_F(OH_AttachOptions_IsShowKeyboard, ShowKeyboardNull_ShouldReturnNullPointerError)
{
    InputMethod_AttachOptions *options = OH_AttachOptions_Create(true);
    InputMethod_ErrorCode errorCode = OH_AttachOptions_IsShowKeyboard(options, nullptr);
    EXPECT_EQ(errorCode, IME_ERR_NULL_POINTER);
    OH_AttachOptions_Destroy(options);
}

HWTEST_F(OH_AttachOptions_IsShowKeyboard, ValidOptionsAndShowKeyboard_ShouldReturnOk)
{
    InputMethod_AttachOptions *options = OH_AttachOptions_Create(true);
    bool showKeyboard = false;
    InputMethod_ErrorCode errorCode = OH_AttachOptions_IsShowKeyboard(options, &showKeyboard);
    EXPECT_EQ(errorCode, IME_ERR_OK);
    EXPECT_TRUE(showKeyboard);
    OH_AttachOptions_Destroy(options);
}