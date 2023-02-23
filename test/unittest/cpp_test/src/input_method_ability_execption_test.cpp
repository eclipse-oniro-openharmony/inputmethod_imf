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
#define private public
#define protected public
#include "input_method_ability.h"
#undef private

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr int32_t DEALY_TIME = 3;
class InputMethodEngineListenerImpl : public InputMethodEngineListener {
public:
    InputMethodEngineListenerImpl(){};
    ~InputMethodEngineListenerImpl(){};
    void OnKeyboardStatus(bool isShow) override;
    void OnInputStart() override;
    void OnInputStop(const std::string &imeId) override;
    void OnSetCallingWindow(uint32_t windowId) override;
    void OnSetSubtype(const SubProperty &property) override;
};
class InputMethodAbilityExecptionTest : public testing::Test {
public:
    static sptr<InputMethodAbility> inputMethodAbility_;
    static std::shared_ptr<InputMethodEngineListenerImpl> imeListener_;
    static std::mutex lock_;
    static std::condition_variable cv_;
    static bool isInputStart_;
    static void SetUpTestCase(void)
    {
        inputMethodAbility_ = InputMethodAbility::GetInstance();
        inputMethodAbility_->OnImeReady();
        imeListener_ = std::make_shared<InputMethodEngineListenerImpl>();
        inputMethodAbility_->SetImeListener(imeListener_);
        std::unique_lock<std::mutex> lock(lock_);
        cv_.wait_for(lock, std::chrono::milliseconds(DEALY_TIME),
            [] { return InputMethodAbilityExecptionTest::isInputStart_; });
        inputMethodAbility_->dataChannel_ = nullptr;
    }
    static void TearDownTestCase(void)
    {
    }
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};
void InputMethodEngineListenerImpl::OnKeyboardStatus(bool isShow)
{
}
void InputMethodEngineListenerImpl::OnInputStart()
{
    std::unique_lock<std::mutex> lock(InputMethodAbilityExecptionTest::lock_);
    InputMethodAbilityExecptionTest::isInputStart_ = true;
    InputMethodAbilityExecptionTest::cv_.notify_one();
}
void InputMethodEngineListenerImpl::OnInputStop(const std::string &imeId)
{
}
void InputMethodEngineListenerImpl::OnSetCallingWindow(uint32_t windowId)
{
}
void InputMethodEngineListenerImpl::OnSetSubtype(const SubProperty &property)
{
}
sptr<InputMethodAbility> InputMethodAbilityExecptionTest::inputMethodAbility_;
std::shared_ptr<InputMethodEngineListenerImpl> InputMethodAbilityExecptionTest::imeListener_;
std::mutex InputMethodAbilityExecptionTest::lock_;
std::condition_variable InputMethodAbilityExecptionTest::cv_;
bool InputMethodAbilityExecptionTest::isInputStart_ = false;

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
