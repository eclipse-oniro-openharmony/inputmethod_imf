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
#include "iinput_method_agent.h"
#include "input_data_channel_service_impl.h"
#include "input_method_agent_service_impl.h"
#include "input_method_engine_listener_impl.h"
#include "input_data_channel_stub.h"
#include "input_method_agent_stub.h"
#include "input_method_engine_listener_impl.h"
#include "key_event_util.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
const std::u16string AGENTSTUB_INTERFACE_TOKEN = u"ohos.miscservices.inputmethod.IInputMethodAgent";
class InputMethodAbilityExceptionTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityExceptionTest::SetUpTestCase");
    }
    static void TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityExceptionTest::TearDownTestCase");
    }
    void SetUp() { }
    void TearDown() { }
    static void ResetMemberVar()
    {
        inputMethodAbility_.dataChannelProxy_ = nullptr;
        inputMethodAbility_.dataChannelObject_ = nullptr;
        inputMethodAbility_.imeListener_ = nullptr;
        inputMethodAbility_.panels_.Clear();
    }
    static InputMethodAbility &inputMethodAbility_;
};
InputMethodAbility &InputMethodAbilityExceptionTest::inputMethodAbility_ = InputMethodAbility::GetInstance();

/**
 * @tc.name: testMoveCursorException
 * @tc.desc: InputMethodAbility MoveCursor
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExceptionTest, testMoveCursorException, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest MoveCursor Test START");
    auto ret = inputMethodAbility_.MoveCursor(4); // move cursor right
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testInsertTextException
 * @tc.desc: InputMethodAbility InsertText
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExceptionTest, testInsertTextException, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest InsertText Test START");
    auto ret = inputMethodAbility_.InsertText("text");
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_CHANNEL_NULLPTR);
}

/**
 * @tc.name: testSendFunctionKeyException
 * @tc.desc: InputMethodAbility SendFunctionKey
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExceptionTest, testSendFunctionKeyException, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest SendFunctionKey Test START");
    auto ret = inputMethodAbility_.SendFunctionKey(0);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testSendExtendActionException
 * @tc.desc: InputMethodAbility SendExtendAction
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testSendExtendActionException, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest SendExtendAction Test START");
    constexpr int32_t action = 1;
    auto ret = inputMethodAbility_.SendExtendAction(action);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testSelectByRangeException
 * @tc.desc: InputMethodAbility SelectByRange
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testSelectByRangeException, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testSelectByRange START");
    // start < 0, end < 0
    int32_t start = -1;
    int32_t end = -2;
    auto ret = inputMethodAbility_.SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    // start < 0, end >0
    start = -1;
    end = 2;
    ret = inputMethodAbility_.SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    // end < 0, start > 0
    start = 1;
    end = -2;
    ret = inputMethodAbility_.SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
    // dataChannel == nullptr
    start = 1;
    end = 2;
    ret = inputMethodAbility_.SelectByRange(start, end);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testSelectByMovementException
 * @tc.desc: InputMethodAbility SelectByMovement
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testSelectByMovementException, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testSelectByMovement START");
    constexpr int32_t direction = 1;
    auto ret = inputMethodAbility_.SelectByMovement(direction);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testDeleteExceptionText
 * @tc.desc: InputMethodAbility DeleteForward & DeleteBackward
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExceptionTest, testDeleteExceptionText, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testDelete Test START");
    int32_t deleteForwardLenth = 1;
    auto ret = inputMethodAbility_.DeleteForward(deleteForwardLenth);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_CHANNEL_NULLPTR);
    int32_t deleteBackwardLenth = 2;
    ret = inputMethodAbility_.DeleteBackward(deleteBackwardLenth);
    EXPECT_EQ(ret, ErrorCode::ERROR_IMA_CHANNEL_NULLPTR);
}

/**
 * @tc.name: testGetTextException001
 * @tc.desc: InputMethodAbility GetTextBeforeCursor & GetTextAfterCursor & GetTextIndexAtCursor
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExceptionTest, testGetTextException001, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testGetText001 START");
    std::u16string text;
    auto ret = inputMethodAbility_.GetTextAfterCursor(8, text);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    ret = inputMethodAbility_.GetTextBeforeCursor(3, text);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    int32_t index;
    ret = inputMethodAbility_.GetTextIndexAtCursor(index);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testGetEnterKeyTypeException
 * @tc.desc: InputMethodAbility GetEnterKeyType & GetInputPattern
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: Hollokin
 */
HWTEST_F(InputMethodAbilityExceptionTest, testGetEnterKeyTypeException, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testGetEnterKeyType START");
    int32_t keyType2;
    auto ret = inputMethodAbility_.GetEnterKeyType(keyType2);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    int32_t inputPattern;
    ret = inputMethodAbility_.GetInputPattern(inputPattern);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testDispatchKeyEventException
 * @tc.desc: DispatchKeyEvent Exception
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testDispatchKeyEventException, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest DispatchKeyEvent START");
    // keyEvent == nullptr;
    std::shared_ptr<MMI::KeyEvent> keyEvent = nullptr;
    sptr<KeyEventConsumerProxy> consumer = new (std::nothrow) KeyEventConsumerProxy(nullptr);
    auto ret = inputMethodAbility_.DispatchKeyEvent(keyEvent, consumer);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);

    // kdListener_ == nullptr
    keyEvent = KeyEventUtil::CreateKeyEvent(MMI::KeyEvent::KEYCODE_A, MMI::KeyEvent::KEY_ACTION_DOWN);
    ret = inputMethodAbility_.DispatchKeyEvent(keyEvent, consumer);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testShowKeyboard_001
 * @tc.desc: ShowKeyboard Exception
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testShowKeyboard_001, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testShowKeyboard_001 START");
    // channelObject == nullptr
    auto ret = inputMethodAbility_.ShowKeyboard(static_cast<int32_t>(RequestKeyboardReason::NONE));
    EXPECT_EQ(ret, ErrorCode::ERROR_IME);

    ResetMemberVar();
}

/**
 * @tc.name: testShowKeyboard_002
 * @tc.desc: ShowKeyBoard Exception
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testShowKeyboard_002, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testShowKeyboard_002 START");
    // imeListener_ == nullptr
    auto ret = inputMethodAbility_.ShowKeyboard(static_cast<int32_t>(RequestKeyboardReason::NONE));
    EXPECT_EQ(ret, ErrorCode::ERROR_IME);

    auto imeListener = std::make_shared<InputMethodEngineListenerImpl>();
    inputMethodAbility_.SetImeListener(imeListener);
    sptr<InputDataChannelStub> channelObject = new InputDataChannelServiceImpl();
    inputMethodAbility_.SetInputDataChannel(channelObject->AsObject());
    // panel exist, PanelFlag == FLG_CANDIDATE_COLUMN
    auto panel = std::make_shared<InputMethodPanel>();
    panel->panelFlag_ = FLG_CANDIDATE_COLUMN;
    panel->windowId_ = 2;
    inputMethodAbility_.panels_.Insert(SOFT_KEYBOARD, panel);
    ret = inputMethodAbility_.ShowKeyboard(static_cast<int32_t>(RequestKeyboardReason::NONE));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    // panel not exist
    inputMethodAbility_.panels_.Clear();
    ret = inputMethodAbility_.ShowKeyboard(static_cast<int32_t>(RequestKeyboardReason::NONE));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ResetMemberVar();
}

/**
 * @tc.name: testHideKeyboard_001
 * @tc.desc: HideKeyboard Exception
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(InputMethodAbilityExceptionTest, testHideKeyboard_001, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testHideKeyboard_001 START");
    // imeListener_ == nullptr
    auto ret = inputMethodAbility_.HideKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_IME);

    // panel exist, PanelFlag == FLG_CANDIDATE_COLUMN
    auto imeListener = std::make_shared<InputMethodEngineListenerImpl>();
    inputMethodAbility_.SetImeListener(imeListener);
    sptr<InputDataChannelStub> channelObject = new InputDataChannelServiceImpl();
    inputMethodAbility_.SetInputDataChannel(channelObject->AsObject());
    auto panel = std::make_shared<InputMethodPanel>();
    panel->panelFlag_ = FLG_CANDIDATE_COLUMN;
    panel->windowId_ = 2;
    inputMethodAbility_.panels_.Insert(SOFT_KEYBOARD, panel);
    ret = inputMethodAbility_.HideKeyboard();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // ShowPanel failed
    inputMethodAbility_.panels_.Clear();
    panel->panelFlag_ = FLG_FIXED;
    inputMethodAbility_.panels_.Insert(SOFT_KEYBOARD, panel);
    ret = inputMethodAbility_.HideKeyboard();
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    ResetMemberVar();
}

/**
 * @tc.name: testDispatchKeyEvent_001
 * @tc.desc: DispatchKeyEvent Exception
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(InputMethodAbilityExceptionTest, testDispatchKeyEvent_001, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodAbilityExceptionTest testDispatchKeyEvent_001 START");
    sptr<InputMethodAgentStub> agentStub = new InputMethodAgentServiceImpl();
    MessageParcel data;
    data.WriteInterfaceToken(AGENTSTUB_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    keyEvent->WriteToParcel(data);
    data.WriteRemoteObject(nullptr);
    auto ret =
        agentStub->OnRemoteRequest(static_cast<uint32_t>(IInputMethodAgentIpcCode::COMMAND_DISPATCH_KEY_EVENT),
        data, reply, option);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);
}
} // namespace MiscServices
} // namespace OHOS
