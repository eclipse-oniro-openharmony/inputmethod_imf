/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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

#include "tdd_util.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr int32_t DEALY_TIME = 20;
class InputMethodAbilityExceptionTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityExceptionTest::SetUpTestCase");
        inputMethodAbility_ = InputMethodAbility::GetInstance();
        inputMethodAbility_->SetCoreAndAgent();
        inputMethodAbility_->OnImeReady();
        imeListener_ = std::make_shared<ImeListenerImpl>();
        inputMethodAbility_->SetImeListener(imeListener_);
        std::unique_lock<std::mutex> lock(lock_);
        cv_.wait_for(lock, std::chrono::milliseconds(DEALY_TIME),
            [] { return InputMethodAbilityExceptionTest::isInputStart_; });
        inputMethodAbility_->dataChannelProxy_ = nullptr;
        inputMethodAbility_->dataChannelObject_ = nullptr;
    }
    static void TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityExceptionTest::TearDownTestCase");
        inputMethodAbility_->imeListener_ = nullptr;
    }
    void SetUp()
    {
    }
    void TearDown()
    {
    }
    class ImeListenerImpl : public InputMethodEngineListener {
    public:
        ImeListenerImpl(){};
        ~ImeListenerImpl(){};
        void OnKeyboardStatus(bool isShow) override;
        void OnInputStart() override;
        void OnInputStop(const std::string &imeId) override;
        void OnSetCallingWindow(uint32_t windowId) override;
        void OnSetSubtype(const SubProperty &property) override;
    };
    static sptr<InputMethodAbility> inputMethodAbility_;

private:
    static std::shared_ptr<ImeListenerImpl> imeListener_;
    static std::mutex lock_;
    static std::condition_variable cv_;
    static bool isInputStart_;
};
void InputMethodAbilityExceptionTest::ImeListenerImpl::OnKeyboardStatus(bool isShow)
{
}
void InputMethodAbilityExceptionTest::ImeListenerImpl::OnInputStart()
{
    std::unique_lock<std::mutex> lock(InputMethodAbilityExceptionTest::lock_);
    InputMethodAbilityExceptionTest::isInputStart_ = true;
    InputMethodAbilityExceptionTest::cv_.notify_one();
}
void InputMethodAbilityExceptionTest::ImeListenerImpl::OnInputStop(const std::string &imeId)
{
}
void InputMethodAbilityExceptionTest::ImeListenerImpl::OnSetCallingWindow(uint32_t windowId)
{
}
void InputMethodAbilityExceptionTest::ImeListenerImpl::OnSetSubtype(const SubProperty &property)
{
}
sptr<InputMethodAbility> InputMethodAbilityExceptionTest::inputMethodAbility_;
std::shared_ptr<InputMethodAbilityExceptionTest::ImeListenerImpl> InputMethodAbilityExceptionTest::imeListener_;
std::mutex InputMethodAbilityExceptionTest::lock_;
std::condition_variable InputMethodAbilityExceptionTest::cv_;
bool InputMethodAbilityExceptionTest::isInputStart_ = false;

/**
 * @tc.name: testMoveCursorException
 * @tc.desc: InputMethodAbility MoveCursor
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExceptionTest, testMoveCursorException, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest MoveCursor Test START");
    auto ret = inputMethodAbility_->MoveCursor(4); // move cursor right
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testInsertTextException
 * @tc.desc: InputMethodAbility InsertText
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExceptionTest, testInsertTextException, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest InsertText Test START");
    auto ret = inputMethodAbility_->InsertText("text");
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testSendFunctionKeyException
 * @tc.desc: InputMethodAbility SendFunctionKey
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExceptionTest, testSendFunctionKeyException, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest SendFunctionKey Test START");
    auto ret = inputMethodAbility_->SendFunctionKey(0);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testSendExtendActionException
 * @tc.desc: InputMethodAbility SendExtendAction
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testSendExtendActionException, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest SendExtendAction Test START");
    constexpr int32_t action = 1;
    auto ret = inputMethodAbility_->SendExtendAction(action);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testSelectByRangeException
 * @tc.desc: InputMethodAbility SelectByRange
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testSelectByRangeException, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testSelectByRange START");
    constexpr int32_t start = 1;
    constexpr int32_t end = 2;
    auto ret = inputMethodAbility_->SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testSelectByMovementException
 * @tc.desc: InputMethodAbility SelectByMovement
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testSelectByMovementException, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testSelectByMovement START");
    constexpr int32_t direction = 1;
    auto ret = inputMethodAbility_->SelectByMovement(direction);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testDeleteExceptionText
 * @tc.desc: InputMethodAbility DeleteForward & DeleteBackward
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExceptionTest, testDeleteExceptionText, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testDelete Test START");
    int32_t deleteForwardLenth = 1;
    auto ret = inputMethodAbility_->DeleteForward(deleteForwardLenth);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    int32_t deleteBackwardLenth = 2;
    ret = inputMethodAbility_->DeleteBackward(deleteBackwardLenth);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testGetTextException001
 * @tc.desc: InputMethodAbility GetTextBeforeCursor & GetTextAfterCursor & GetTextIndexAtCursor
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExceptionTest, testGetTextException001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testGetText001 START");
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
 * @tc.name: testGetEnterKeyTypeException
 * @tc.desc: InputMethodAbility GetEnterKeyType & GetInputPattern
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExceptionTest, testGetEnterKeyTypeException, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testGetEnterKeyType START");
    int32_t keyType2;
    auto ret = inputMethodAbility_->GetEnterKeyType(keyType2);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    int32_t inputPattern;
    ret = inputMethodAbility_->GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testDispatchKeyEventException
 * @tc.desc: DispatchKeyEvent Exception
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testDispatchKeyEventException, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest DispatchKeyEvent START");
    auto keyEvent = TddUtil::CreateKeyEvent(MMI::KeyEvent::KEYCODE_A, MMI::KeyEvent::KEY_ACTION_DOWN);
    auto ret = inputMethodAbility_->DispatchKeyEvent(keyEvent);
    EXPECT_FALSE(ret);
}
} // namespace MiscServices
} // namespace OHOS
