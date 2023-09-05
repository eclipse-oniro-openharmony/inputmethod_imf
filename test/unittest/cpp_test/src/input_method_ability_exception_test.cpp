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
#include "input_data_channel_stub.h"
#include "input_method_engine_listener_impl.h"
#include "key_event_util.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class InputMethodAbilityExceptionTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityExceptionTest::SetUpTestCase");
        inputMethodAbility_ = InputMethodAbility::GetInstance();
    }
    static void TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityExceptionTest::TearDownTestCase");
    }
    void SetUp()
    {
    }
    void TearDown()
    {
    }
    static void ResetMemberVar()
    {
        inputMethodAbility_->isImeReady_ = false;
        inputMethodAbility_->dataChannelProxy_ = nullptr;
        inputMethodAbility_->dataChannelObject_ = nullptr;
        inputMethodAbility_->imeListener_ = nullptr;
        inputMethodAbility_->panels_.Clear();
    }
    static sptr<InputMethodAbility> inputMethodAbility_;
};
sptr<InputMethodAbility> InputMethodAbilityExceptionTest::inputMethodAbility_;

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
    // start < 0, end < 0
    int32_t start = -1;
    int32_t end = -2;
    auto ret = inputMethodAbility_->SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    // start < 0, end >0
    start = -1;
    end = 2;
    ret = inputMethodAbility_->SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    //end < 0, start > 0
    start = 1;
    end = -2;
    ret = inputMethodAbility_->SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    // dataChannel == nullptr
    start = 1;
    end = 2;
    ret = inputMethodAbility_->SelectByRange(start, end);
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
    // keyEvent == nullptr;
    std::shared_ptr<MMI::KeyEvent> keyEvent = nullptr;
    auto ret = inputMethodAbility_->DispatchKeyEvent(keyEvent);
    EXPECT_FALSE(ret);

    // kdListener_ == nullptr
    keyEvent = KeyEventUtil::CreateKeyEvent(MMI::KeyEvent::KEYCODE_A, MMI::KeyEvent::KEY_ACTION_DOWN);
    ret = inputMethodAbility_->DispatchKeyEvent(keyEvent);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: testHideKeyboardSelf_001
 * @tc.desc: controlChannel == nullptr
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testHideKeyboardSelf_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testHideKeyboardSelf_001 START");
    auto ret = inputMethodAbility_->HideKeyboardSelf();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testShowKeyboard_001
 * @tc.desc: ShowKeyboard Exception
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testShowKeyboard_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testShowKeyboard_001 START");
    // channelObject == nullptr
    auto ret = inputMethodAbility_->ShowKeyboard(nullptr, false, true);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    // GetTextConfig failed
    sptr<InputDataChannelStub> channelObject = new InputDataChannelStub();
    ret = inputMethodAbility_->ShowKeyboard(channelObject->AsObject(), false, true);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_NOT_READY);

    ResetMemberVar();
}

/**
 * @tc.name: testShowInputWindow_001
 * @tc.desc: ShowInputWindow Exception
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testShowInputWindow_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testShowInputWindow_001 START");
    // isImeReady_ is false
    auto ret = inputMethodAbility_->ShowInputWindow(true);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME_NOT_READY);

    // imeListener_ == nullptr
    inputMethodAbility_->isImeReady_ = true;
    ret = inputMethodAbility_->ShowInputWindow(true);
    EXPECT_EQ(ret, ErrorCode::ERROR_IME);

    // channel == nullptr
    auto imeListener = std::make_shared<InputMethodEngineListenerImpl>();
    inputMethodAbility_->SetImeListener(imeListener);
    ret = inputMethodAbility_->ShowInputWindow(true);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    // panel exist, PanelFlag == FLG_CANDIDATE_COLUMN
    sptr<InputDataChannelStub> channelObject = new InputDataChannelStub();
    inputMethodAbility_->SetInputDataChannel(channelObject->AsObject());
    auto panel = std::make_shared<InputMethodPanel>();
    panel->panelFlag_ = FLG_CANDIDATE_COLUMN;
    inputMethodAbility_->panels_.Insert(SOFT_KEYBOARD, panel);
    ret = inputMethodAbility_->ShowInputWindow(true);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // ShowPanel failed
    inputMethodAbility_->panels_.Clear();
    panel->panelFlag_ = FLG_FIXED;
    inputMethodAbility_->panels_.Insert(SOFT_KEYBOARD, panel);
    ret = inputMethodAbility_->ShowInputWindow(true);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    ResetMemberVar();
}

/**
 * @tc.name: testHideKeyboard_001
 * @tc.desc: HideKeyboard Exception
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testHideKeyboard_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testHideKeyboard_001 START");
    // imeListener_ == nullptr
    auto ret = inputMethodAbility_->HideKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_IME);

    // channel == nullptr
    auto imeListener = std::make_shared<InputMethodEngineListenerImpl>();
    inputMethodAbility_->SetImeListener(imeListener);
    ret = inputMethodAbility_->HideKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    // panel exist, PanelFlag == FLG_CANDIDATE_COLUMN
    sptr<InputDataChannelStub> channelObject = new InputDataChannelStub();
    inputMethodAbility_->SetInputDataChannel(channelObject->AsObject());
    auto panel = std::make_shared<InputMethodPanel>();
    panel->panelFlag_ = FLG_CANDIDATE_COLUMN;
    inputMethodAbility_->panels_.Insert(SOFT_KEYBOARD, panel);
    ret = inputMethodAbility_->HideKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // ShowPanel failed
    inputMethodAbility_->panels_.Clear();
    panel->panelFlag_ = FLG_FIXED;
    inputMethodAbility_->panels_.Insert(SOFT_KEYBOARD, panel);
    ret = inputMethodAbility_->HideKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    ResetMemberVar();
}

} // namespace MiscServices
} // namespace OHOS
