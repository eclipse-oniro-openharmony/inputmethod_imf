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
#include "global.h"

class TextConfigTest : public testing::Test {
protected:
    void SetUp() override
    {
        config = OH_TextConfig_Create();
    }

    void TearDown() override
    {
        OH_TextConfig_Destroy(config);
    }

    InputMethod_TextConfig *config = nullptr;
};

HWTEST_F(TextConfigTest, SetAndGetInputType_ValidInput_Success) {
    InputMethod_TextInputType inputType = IM_TEXT_INPUT_TYPE_TEXT;
    EXPECT_EQ(OH_TextConfig_SetInputType(config, inputType), IME_ERR_OK);

    InputMethod_TextInputType retrievedInputType;
    EXPECT_EQ(OH_TextConfig_GetInputType(config, &retrievedInputType), IME_ERR_OK);
    EXPECT_EQ(retrievedInputType, inputType);
}

HWTEST_F(TextConfigTest, SetAndGetEnterKeyType_ValidInput_Success) {
    InputMethod_EnterKeyType enterKeyType = IM_ENTER_KEY_TYPE_GO;
    EXPECT_EQ(OH_TextConfig_SetEnterKeyType(config, enterKeyType), IME_ERR_OK);

    InputMethod_EnterKeyType retrievedEnterKeyType;
    EXPECT_EQ(OH_TextConfig_GetEnterKeyType(config, &retrievedEnterKeyType), IME_ERR_OK);
    EXPECT_EQ(retrievedEnterKeyType, enterKeyType);
}

HWTEST_F(TextConfigTest, SetAndGetPreviewTextSupport_ValidInput_Success) {
    bool supported = true;
    EXPECT_EQ(OH_TextConfig_SetPreviewTextSupport(config, supported), IME_ERR_OK);

    bool retrievedSupported;
    EXPECT_EQ(OH_TextConfig_IsPreviewTextSupported(config, &retrievedSupported), IME_ERR_OK);
    EXPECT_EQ(retrievedSupported, supported);
}

HWTEST_F(TextConfigTest, SetAndGetSelection_ValidInput_Success) {
    int32_t start = 10, end = 20;
    EXPECT_EQ(OH_TextConfig_SetSelection(config, start, end), IME_ERR_OK);

    int32_t retrievedStart, retrievedEnd;
    EXPECT_EQ(OH_TextConfig_GetSelection(config, &retrievedStart, &retrievedEnd), IME_ERR_OK);
    EXPECT_EQ(retrievedStart, start);
    EXPECT_EQ(retrievedEnd, end);
}

HWTEST_F(TextConfigTest, SetAndGetWindowId_ValidInput_Success) {
    int32_t windowId = 123;
    EXPECT_EQ(OH_TextConfig_SetWindowId(config, windowId), IME_ERR_OK);

    int32_t retrievedWindowId;
    EXPECT_EQ(OH_TextConfig_GetWindowId(config, &retrievedWindowId), IME_ERR_OK);
    EXPECT_EQ(retrievedWindowId, windowId);
}

HWTEST_F(TextConfigTest, SetInputType_NullConfig_ReturnsError) {
    InputMethod_TextInputType inputType = IM_TEXT_INPUT_TYPE_TEXT;
    EXPECT_EQ(OH_TextConfig_SetInputType(nullptr, inputType), IME_ERR_NULL_POINTER);
}

HWTEST_F(TextConfigTest, GetInputType_NullConfig_ReturnsError) {
    InputMethod_TextInputType inputType;
    EXPECT_EQ(OH_TextConfig_GetInputType(nullptr, &inputType), IME_ERR_NULL_POINTER);
}

HWTEST_F(TextConfigTest, GetInputType_NullInputType_ReturnsError) {
    EXPECT_EQ(OH_TextConfig_GetInputType(config, nullptr), IME_ERR_NULL_POINTER);
}