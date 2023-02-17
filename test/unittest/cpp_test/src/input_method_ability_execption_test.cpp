/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "input_method_ability.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class InputMethodAbilityExecptionTest : public testing::Test {
public:
    static sptr<InputMethodAbility> inputMethodAbility_;
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        inputMethodAbility_ = InputMethodAbility::GetInstance();
    }
    void TearDown() {}
};
sptr<InputMethodAbility> InputMethodAbilityExecptionTest::inputMethodAbility_;

/**
 * @tc.name: testMoveCursorExecption
 * @tc.desc: InputMethodAbility MoveCursor
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExecptionTest, testMoveCursorExecption, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility MoveCursor Test START");
    auto ret = inputMethodAbility_->MoveCursor(4); // move cursor right
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testInsertTextExecption
 * @tc.desc: InputMethodAbility InsertText
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExecptionTest, testInsertTextExecption, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility InsertText Test START");
    auto ret = inputMethodAbility_->InsertText("text");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testSendFunctionKeyExecption
 * @tc.desc: InputMethodAbility SendFunctionKey
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExecptionTest, testSendFunctionKeyExecption, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility SendFunctionKey Test START");
    auto ret = inputMethodAbility_->SendFunctionKey(0);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testDeleteExecptionText
 * @tc.desc: InputMethodAbility DeleteForward & DeleteBackward
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExecptionTest, testDeleteExecptionText, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testDelete Test START");
    int32_t deleteForwardLenth = 1;
    auto ret = inputMethodAbility_->DeleteForward(deleteForwardLenth);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    int32_t deleteBackwardLenth = 2;
    ret = inputMethodAbility_->DeleteBackward(deleteBackwardLenth);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testGetTextExecption001
 * @tc.desc: InputMethodAbility GetTextBeforeCursor & GetTextAfterCursor & GetTextIndexAtCursor
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExecptionTest, testGetTextExecption001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetText001 START");
    std::u16string text;
    auto ret = inputMethodAbility_->GetTextAfterCursor(8, text);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    ret = inputMethodAbility_->GetTextBeforeCursor(3, text);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    int32_t index;
    ret = inputMethodAbility_->GetTextIndexAtCursor(index);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testGetEnterKeyTypeExecption
 * @tc.desc: InputMethodAbility GetEnterKeyType & GetInputPattern
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExecptionTest, testGetEnterKeyTypeExecption, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbility testGetEnterKeyType START");
    int32_t keyType2;
    auto ret = inputMethodAbility_->GetEnterKeyType(keyType2);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    int32_t inputPattern;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}
} // namespace MiscServices
} // namespace OHOS
