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

#include "input_method_ability.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "input_control_channel.h"
#include "input_method_ability_test_channel.h"
#include "input_method_ability_test_listener.h"
#include "input_method_ability_test_private_command.h"
#include "input_method_ability_test_proxy.h"
#include "input_method_ability_test_system_channel.h"
#include "input_method_agent_stub.h"
#include "input_method_core_stub.h"
#include "input_method_system_ability_proxy.h"
#include "inputmethod_sysevent.h"
#include "inputmethod_trace.h"
#include "iservice_registry.h"
#include "itypes_util.h"
#include "message_parcel.h"
#include "mock_ime_listener.h"
#include "mock_input_control_channel_proxy.h"
#include "mock_input_method_ability.h"
#include "mock_input_method_agent_stub.h"
#include "mock_input_method_panel.h"
#include "mock_system_cmd_channel_proxy.h"
#include "msg_handler_callback_interface.h"
#include "on_demand_start_stop_sa.h"
#include "soft_keyboard_panel.h"
#include "string_ex.h"

using namespace MiscServices;
using namespace OHOS;

class MockInputDataChannelProxy : public InputDataChannelProxy {
public:
    MOCK_METHOD2(SetPreviewText, int32_t(const std::string &, const Range &));
    MOCK_METHOD1(FinishTextPreview, int32_t(bool));
    MOCK_METHOD1(SendMessage, int32_t(const ArrayBuffer &));
};

class MockSoftKeyboardPanel : public SoftKeyboardPanel {
public:
    MOCK_METHOD1(SetCallingWindow, int32_t(int32_t));
    MOCK_METHOD1(GetCallingWindowInfo, int32_t(CallingWindowInfo &));
};

class MockInputControlChannel : public InputControlChannel {
public:
    MOCK_METHOD0(HideKeyboardSelf, void());
};

class MockMsgHandlerCallback : public MsgHandlerCallbackInterface {
public:
    MOCK_METHOD1(OnMessage, int32_t(const ArrayBuffer &));
    MOCK_METHOD0(OnTerminated, void());
};

class InputMethodAbilityTest : public testing::Test {
protected:
    void SetUp() override
    {
        inputMethodAbility_ = std::make_shared<InputMethodAbility>();
        inputDataChannelProxy_ = std::make_shared<MockInputDataChannelProxy>();
        softKeyboardPanel_ = std::make_shared<MockSoftKeyboardPanel>();
        inputControlChannel_ = std::make_shared<MockInputControlChannel>();
        msgHandlerCallback_ = std::make_shared<MockMsgHandlerCallback>();
    }

    void TearDown() override
    {
        inputMethodAbility_ = nullptr;
        inputDataChannelProxy_ = nullptr;
        softKeyboardPanel_ = nullptr;
        inputControlChannel_ = nullptr;
        msgHandlerCallback_ = nullptr;
    }

    std::shared_ptr<InputMethodAbility> inputMethodAbility_;
    std::shared_ptr<MockInputDataChannelProxy> inputDataChannelProxy_;
    std::shared_ptr<MockSoftKeyboardPanel> softKeyboardPanel_;
    std::shared_ptr<MockInputControlChannel> inputControlChannel_;
    std::shared_ptr<MockMsgHandlerCallback> msgHandlerCallback_;
};

class MockInputMethodAbility : public InputMethodAbility {
public:
    MOCK_METHOD(sptr<SoftKeyboardPanel>, GetSoftKeyboardPanel, (), (override));
    MOCK_METHOD(int32_t, InvokeStartInputCallback, (const InputClientConfig &, bool), (override));
    MOCK_METHOD(int32_t, InvokeStartInputCallback, (bool), (override));
    MOCK_METHOD0(GetInputDataChannelProxy, InputDataChannelProxy *());
    MOCK_METHOD0(IsImeTerminating, bool());
    MOCK_METHOD0(PostTaskToEventHandler, void(std::function<void()>, const std::string &));
    MOCK_METHOD0(ForEach, void(std::function<bool(const PanelType &, const std::shared_ptr<InputMethodPanel> &)>));
};

class MockSoftKeyboardPanel : public SoftKeyboardPanel {
public:
    MOCK_METHOD(void, HidePanel, (), (override));
};

class MockTaskManager : public TaskManager {
public:
    MOCK_METHOD(void, WaitExec, (uint64_t, uint32_t, std::function<void()>), (override));
};

class MockInputMethodEngineListener : public InputMethodEngineListener {
public:
    MOCK_METHOD(void, PostTaskToEventHandler, (std::function<void()>, const std::string &), (override));
};

class InputMethodAbilityTest : public Test {
protected:
    void SetUp() override
    {
        inputMethodAbility = new MockInputMethodAbility();
        softKeyboardPanel = new MockSoftKeyboardPanel();
        taskManager = new MockTaskManager();
        imeListener = new MockInputMethodEngineListener();

        ON_CALL(*inputMethodAbility, GetSoftKeyboardPanel()).WillByDefault(Return(softKeyboardPanel));
        ON_CALL(*inputMethodAbility, InvokeStartInputCallback(_, _)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*inputMethodAbility, InvokeStartInputCallback(_)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*taskManager, WaitExec(_, _, _)).WillByDefault(Invoke([](uint64_t, uint32_t, std::function<void()>) {}));
        ON_CALL(*imeListener, PostTaskToEventHandler(_, _))
            .WillByDefault(Invoke([](std::function<void()>, const std::string &) {}));
    }

    void TearDown() override
    {
        delete inputMethodAbility;
        delete softKeyboardPanel;
        delete taskManager;
        delete imeListener;
    }

    MockInputMethodAbility *inputMethodAbility;
    MockSoftKeyboardPanel *softKeyboardPanel;
    MockTaskManager *taskManager;
    MockInputMethodEngineListener *imeListener;
};

class MockInputDataChannelProxy : public InputDataChannelProxy {
public:
    MOCK_METHOD1(InsertText, int32_t(const std::u16string &));
    MOCK_METHOD1(DeleteForward, int32_t(int32_t));
    MOCK_METHOD1(DeleteBackward, int32_t(int32_t));
    MOCK_METHOD1(SendFunctionKey, int32_t(int32_t));
    MOCK_METHOD1(HandleExtendAction, int32_t(int32_t));
    MOCK_METHOD2(GetTextBeforeCursor, int32_t(int32_t, std::u16string &));
    MOCK_METHOD2(GetTextAfterCursor, int32_t(int32_t, std::u16string &));
    MOCK_METHOD1(MoveCursor, int32_t(int32_t));
    MOCK_METHOD2(SelectByRange, int32_t(int32_t, int32_t));
    MOCK_METHOD2(SelectByMovement, int32_t(int32_t, int32_t));
    MOCK_METHOD1(GetEnterKeyType, int32_t(int32_t &));
    MOCK_METHOD1(GetInputPattern, int32_t(int32_t &));
    MOCK_METHOD1(GetTextIndexAtCursor, int32_t(int32_t &));
};

HWTEST_F(InputMethodAbilityTest, dispatchKeyEvent_001, TestSize.Level0)
{
    InputClientInfo clientInfo;
    clientInfo.channel = new RemoteObject();
    clientInfo.config.inputAttribute.bundleName = "testBundle";

    EXPECT_CALL(*inputMethodAbility, InvokeStartInputCallback(_));

    int32_t result = inputMethodAbility->StartInput(clientInfo, false);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, dispatchKeyEvent_002, TestSize.Level0)
{
    InputClientInfo clientInfo;
    clientInfo.channel = new RemoteObject();
    clientInfo.isShowKeyboard = true;

    inputMethodAbility->SetImeListener(nullptr);

    EXPECT_CALL(*softKeyboardPanel, HidePanel());

    int32_t result = inputMethodAbility->StartInput(clientInfo, false);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, dispatchKeyEvent_003, TestSize.Level0)
{
    InputClientInfo clientInfo;
    clientInfo.channel = new RemoteObject();
    clientInfo.isShowKeyboard = true;

    inputMethodAbility->SetImeListener(std::shared_ptr<InputMethodEngineListener>(imeListener));

    EXPECT_CALL(*taskManager, WaitExec(_, _, _));

    int32_t result = inputMethodAbility->StartInput(clientInfo, false);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, dispatchKeyEvent_004, TestSize.Level0)
{
    InputClientInfo clientInfo;
    clientInfo.channel = nullptr;

    int32_t result = inputMethodAbility->StartInput(clientInfo, false);

    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

HWTEST_F(InputMethodAbilityTest, dispatchKeyEvent_005, TestSize.Level0)
{
    InputClientInfo clientInfo;
    clientInfo.channel = new RemoteObject();
    clientInfo.needHide = true;

    EXPECT_CALL(*softKeyboardPanel, HidePanel());

    int32_t result = inputMethodAbility->StartInput(clientInfo, false);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, dispatchKeyEvent_006, TestSize.Level0)
{
    InputClientInfo clientInfo;
    clientInfo.channel = new RemoteObject();
    clientInfo.config.inputAttribute.bundleName = "testBundle";

    EXPECT_CALL(*inputMethodAbility, InvokeStartInputCallback(_, _));

    int32_t result = inputMethodAbility->StartInput(clientInfo, true);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, dispatchKeyEvent_007, TestSize.Level0)
{
    InputClientInfo clientInfo;
    clientInfo.channel = new RemoteObject();
    clientInfo.config.inputAttribute.bundleName = "testBundle";

    EXPECT_CALL(*inputMethodAbility, InvokeStartInputCallback(_));

    int32_t result = inputMethodAbility->StartInput(clientInfo, false);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, dispatchKeyEvent_008, TestSize.Level0)
{
    InputClientInfo clientInfo;
    clientInfo.channel = new RemoteObject();
    clientInfo.isShowKeyboard = true;

    inputMethodAbility->SetImeListener(nullptr);

    EXPECT_CALL(*softKeyboardPanel, HidePanel());

    int32_t result = inputMethodAbility->StartInput(clientInfo, false);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, dispatchKeyEvent_009, TestSize.Level0)
{
    InputClientInfo clientInfo;
    clientInfo.channel = new RemoteObject();
    clientInfo.isShowKeyboard = true;

    inputMethodAbility->SetImeListener(std::shared_ptr<InputMethodEngineListener>(imeListener));

    EXPECT_CALL(*taskManager, WaitExec(_, _, _));

    int32_t result = inputMethodAbility->StartInput(clientInfo, false);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}
HWTEST_F(InputMethodAbilityTest, dispatchKeyEvent_010, TestSize.Level0)
{
    EXPECT_CALL(*imeListener_, OnInputFinish()).Times(1);
    int32_t result = inputMethodAbility_->StopInput(channelObject_);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, dispatchKeyEvent_011, TestSize.Level0)
{
    inputMethodAbility_->imeListener_ = nullptr;
    int32_t result = inputMethodAbility_->StopInput(channelObject_);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, dispatchKeyEvent_012, TestSize.Level0)
{
    int32_t result = inputMethodAbility_->DispatchKeyEvent(nullptr, consumer_);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

HWTEST_F(InputMethodAbilityTest, dispatchKeyEvent_013, TestSize.Level0)
{
    inputMethodAbility_->kdListener_ = nullptr;
    int32_t result = inputMethodAbility_->DispatchKeyEvent(keyEvent_, consumer_);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

HWTEST_F(InputMethodAbilityTest, onAttributeChange_001, TestSize.Level0)
{
    EXPECT_CALL(*kdListener_, OnDealKeyEvent(keyEvent_, consumer_)).WillOnce(Return(false));
    int32_t result = inputMethodAbility_->DispatchKeyEvent(keyEvent_, consumer_);
    EXPECT_EQ(result, ErrorCode::ERROR_DISPATCH_KEY_EVENT);
}

HWTEST_F(InputMethodAbilityTest, onAttributeChange_002, TestSize.Level0)
{
    EXPECT_CALL(*kdListener_, OnDealKeyEvent(keyEvent_, consumer_)).WillOnce(Return(true));
    int32_t result = inputMethodAbility_->DispatchKeyEvent(keyEvent_, consumer_);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, onAttributeChange_003, TestSize.Level0)
{
    EXPECT_CALL(*imeListener_, OnSetCallingWindow(123)).Times(1);
    inputMethodAbility_->SetCallingWindow(123);
}

HWTEST_F(InputMethodAbilityTest, onAttributeChange_004, TestSize.Level0)
{
    inputMethodAbility_->imeListener_ = nullptr;
    inputMethodAbility_->SetCallingWindow(123);
}

HWTEST_F(InputMethodAbilityTest, onAttributeChange_005, TestSize.Level0)
{
    EXPECT_CALL(*kdListener_, OnCursorUpdate(10, 20, 30)).Times(1);
    inputMethodAbility_->OnCursorUpdate(10, 20, 30);
}

HWTEST_F(InputMethodAbilityTest, onAttributeChange_006, TestSize.Level0)
{
    inputMethodAbility_->kdListener_ = nullptr;
    inputMethodAbility_->OnCursorUpdate(10, 20, 30);
}

HWTEST_F(InputMethodAbilityTest, onAttributeChange_007, TestSize.Level0)
{
    EXPECT_CALL(*kdListener_, OnTextChange(Str16ToStr8(u"test"))).Times(1);
    EXPECT_CALL(*kdListener_, OnSelectionChange(0, 1, 2, 3)).Times(1);
    inputMethodAbility_->OnSelectionChange(u"test", 0, 1, 2, 3);
}

HWTEST_F(InputMethodAbilityTest, onAttributeChange_008, TestSize.Level0)
{
    inputMethodAbility_->kdListener_ = nullptr;
    inputMethodAbility_->OnSelectionChange(u"test", 0, 1, 2, 3);
}

HWTEST_F(InputMethodAbilityTest, onAttributeChange_009, TestSize.Level0)
{
    InputAttribute attribute = { 1, 2 };
    EXPECT_CALL(*kdListener_, OnEditorAttributeChange(attribute)).Times(1);
    inputMethodAbility_->OnAttributeChange(attribute);
}

HWTEST_F(InputMethodAbilityTest, insertText_001, TestSize.Level0)
{
    inputMethodAbility_->kdListener_ = nullptr;
    InputAttribute attribute = { 1, 2 };
    inputMethodAbility_->OnAttributeChange(attribute);
}

HWTEST_F(InputMethodAbilityTest, insertText_002, TestSize.Level0)
{
    EXPECT_CALL(*imeListener_, OnInputStop()).WillOnce(Return(ErrorCode::NO_ERROR));
    int32_t result = inputMethodAbility_->OnStopInputService(true);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, insertText_003, TestSize.Level0)
{
    inputMethodAbility_->imeListener_ = nullptr;
    int32_t result = inputMethodAbility_->OnStopInputService(true);
    EXPECT_EQ(result, ErrorCode::ERROR_IME_NOT_STARTED);
}

HWTEST_F(InputMethodAbilityTest, insertText_004, TestSize.Level0)
{
    int32_t result = inputMethodAbility_->HideKeyboard();
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, insertText_005, TestSize.Level0)
{
    EXPECT_CALL(*imeListener_, OnKeyboardStatus(true)).Times(1);
    int32_t result = inputMethodAbility_->ShowKeyboard();
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, insertText_006, TestSize.Level0)
{
    inputMethodAbility_->imeListener_ = nullptr;
    int32_t result = inputMethodAbility_->ShowKeyboard();
    EXPECT_EQ(result, ErrorCode::ERROR_IME);
}

HWTEST_F(InputMethodAbilityTest, insertText_007, TestSize.Level0)
{
    EXPECT_CALL(*imeListener_, OnInputStart()).Times(1);
    int32_t result = inputMethodAbility_->InvokeStartInputCallback(true);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, insertText_008, TestSize.Level0)
{
    inputMethodAbility_->imeListener_ = nullptr;
    int32_t result = inputMethodAbility_->InvokeStartInputCallback(true);
    EXPECT_EQ(result, ErrorCode::ERROR_IME);
}
HWTEST_F(InputMethodAbilityTest, insertText_009, TestSize.Level0)
{
    // Arrange
    EXPECT_CALL(*mockAbility, GetInputDataChannelProxy()).WillRepeatedly(Return(mockChannel));
    // 模拟panel的遍历

    // Act
    InputMethodConfig textConfig;
    textConfig.windowId = 1;
    mockAbility->SetCallingWindow(textConfig);

    // Assert
    // 验证窗口ID已设置
}

HWTEST_F(InputMethodAbilityTest, insertText_010, TestSize.Level0)
{
    // Arrange
    EXPECT_CALL(*mockAbility, GetInputDataChannelProxy()).WillOnce(Return(nullptr));

    // Act
    int32_t result = mockAbility->InsertText("test");

    // Assert
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

HWTEST_F(InputMethodAbilityTest, insertText_011, TestSize.Level0)
{
    // Arrange
    EXPECT_CALL(*mockAbility, GetInputDataChannelProxy()).WillOnce(Return(mockChannel));
    EXPECT_CALL(*mockChannel, DeleteForward(1)).WillOnce(Return(ErrorCode::NO_ERROR));

    // Act
    int32_t result = mockAbility->DeleteForward(1);

    // Assert
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, clearSystemCmdChannel_001, TestSize.Level0)
{
    // Arrange
    EXPECT_CALL(*mockAbility, IsImeTerminating()).WillOnce(Return(true));

    // Act
    int32_t result = mockAbility->HideKeyboardSelf();

    // Assert
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, clearSystemCmdChannel_002, TestSize.Level0)
{
    // Arrange
    EXPECT_CALL(*mockAbility, GetInputDataChannelProxy()).WillOnce(Return(mockChannel));

    // Act
    int32_t result = mockAbility->SelectByRange(-1, 10);

    // Assert
    EXPECT_EQ(result, ErrorCode::ERROR_PARAMETER_CHECK_FAILED);
}

HWTEST_F(InputMethodAbilityTest, clearSystemCmdChannel_003, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodAbility_, GetInputDataChannelProxy()).WillOnce(Return(nullptr));

    TextTotalConfig textConfig;
    int32_t result = inputMethodAbility_->GetTextConfig(textConfig);

    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

HWTEST_F(InputMethodAbilityTest, clearSystemCmdChannel_004, TestSize.Level0)
{
    auto mockChannel = std::make_shared<MockInputDataChannelProxy>();
    EXPECT_CALL(*inputMethodAbility_, GetInputDataChannelProxy()).WillOnce(Return(mockChannel));
    EXPECT_CALL(*mockChannel, GetTextConfig(_)).WillOnce(Return(ErrorCode::NO_ERROR));
    EXPECT_CALL(*inputMethodAbility_, GetInputAttribute()).WillOnce(Return(InputAttribute{ .bundleName = "testBundle" }));

    TextTotalConfig textConfig;
    int32_t result = inputMethodAbility_->GetTextConfig(textConfig);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
    EXPECT_EQ(textConfig.inputAttribute.bundleName, "testBundle");
}

HWTEST_F(InputMethodAbilityTest, clearSystemCmdChannel_005, TestSize.Level0)
{
    auto mockChannel = std::make_shared<MockInputDataChannelProxy>();
    EXPECT_CALL(*inputMethodAbility_, GetInputDataChannelProxy()).WillOnce(Return(mockChannel));
    EXPECT_CALL(*mockChannel, GetTextConfig(_)).WillOnce(Return(ErrorCode::ERROR_CLIENT_NULL_POINTER));

    TextTotalConfig textConfig;
    int32_t result = inputMethodAbility_->GetTextConfig(textConfig);

    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NULL_POINTER);
    EXPECT_TRUE(textConfig.inputAttribute.bundleName.empty());
}

HWTEST_F(InputMethodAbilityTest, clearSystemCmdChannel_006, TestSize.Level0)
{
    int32_t security = 0;
    ability_->securityMode_.store(1);
    int32_t result = ability_->GetSecurityMode(security);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
    EXPECT_EQ(security, 1);
}

HWTEST_F(InputMethodAbilityTest, clearSystemCmdChannel_007, TestSize.Level0)
{
    int32_t security = 0;
    ability_->securityMode_.store(INVALID_SECURITY_MODE);
    EXPECT_CALL(*mockImsaProxy_, GetSecurityMode(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(ErrorCode::NO_ERROR)));
    EXPECT_CALL(*ability_, GetImsaProxy()).WillOnce(Return(mockImsaProxy_));
    int32_t result = ability_->GetSecurityMode(security);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
    EXPECT_EQ(security, 1);
}

HWTEST_F(InputMethodAbilityTest, clearSystemCmdChannel_008, TestSize.Level0)
{
    int32_t security = 0;
    ability_->securityMode_.store(INVALID_SECURITY_MODE);
    EXPECT_CALL(*ability_, GetImsaProxy()).WillOnce(Return(nullptr));
    int32_t result = ability_->GetSecurityMode(security);
    EXPECT_EQ(result, ErrorCode::ERROR_NULL_POINTER);
}

HWTEST_F(InputMethodAbilityTest, clearSystemCmdChannel_009, TestSize.Level0)
{
    ability_->systemCmdChannelProxy_ = nullptr;
    EXPECT_CALL(*ability_, GetSystemCmdChannelProxy()).WillOnce(Return(nullptr));
    ability_->ClearSystemCmdChannel();
}

HWTEST_F(InputMethodAbilityTest, clearInputAttribute_001, TestSize.Level0)
{
    ability_->systemCmdChannelProxy_ = mockSystemCmdChannelProxy_;
    EXPECT_CALL(*ability_, GetSystemCmdChannelProxy()).WillOnce(Return(mockSystemCmdChannelProxy_));
    ability_->ClearSystemCmdChannel();
    EXPECT_EQ(ability_->systemCmdChannelProxy_, nullptr);
}

HWTEST_F(InputMethodAbilityTest, clearInputAttribute_002, TestSize.Level0)
{
    EXPECT_CALL(*ability_, GetSystemCmdChannelProxy()).WillOnce(Return(nullptr));
    EXPECT_CALL(*ability_, GetInputMethodAgentStub()).WillOnce(Return(mockInputMethodAgentStub_));
    int32_t result = ability_->OnConnectSystemCmd(mockSystemCmdChannelProxy_, mockInputMethodAgentStub_);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, clearInputAttribute_003, TestSize.Level0)
{
    ability_->imeListener_ = nullptr;
    int32_t result = ability_->OnSecurityChange(1);
    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
}

HWTEST_F(InputMethodAbilityTest, clearInputAttribute_004, TestSize.Level0)
{
    ability_->imeListener_ = mockImeListener_;
    EXPECT_CALL(*mockImeListener_, OnSecurityChange(1)).Times(1);
    int32_t result = ability_->OnSecurityChange(1);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, clearInputAttribute_005, TestSize.Level0)
{
    PanelInfo panelInfo;
    panelInfo.panelType = PanelType::SOFT_KEYBOARD;
    EXPECT_CALL(*mockInputMethodPanel_, CreatePanel(_, panelInfo)).WillOnce(Return(ErrorCode::NO_ERROR));
    int32_t result = ability_->CreatePanel(nullptr, panelInfo, mockInputMethodPanel_);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, clearInputAttribute_006, TestSize.Level0)
{
    int32_t result = ability_->DestroyPanel(nullptr);
    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
}

HWTEST_F(InputMethodAbilityTest, clearInputAttribute_007, TestSize.Level0)
{
    int32_t result = ability_->ShowPanel(nullptr);
    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
}

HWTEST_F(InputMethodAbilityTest, clearInputAttribute_008, TestSize.Level0)
{
    int32_t result = ability_->HidePanel(nullptr);
    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
}

HWTEST_F(InputMethodAbilityTest, clearInputAttribute_009, TestSize.Level0)
{
    SysPanelStatus sysPanelStatus;
    EXPECT_CALL(*ability_, GetSystemCmdChannelProxy()).WillOnce(Return(nullptr));
    int32_t result = ability_->NotifyPanelStatus(PanelType::SOFT_KEYBOARD, sysPanelStatus);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, clearInputAttribute_010, TestSize.Level0)
{
    InputAttribute inputAttribute;
    ability_->SetInputAttribute(inputAttribute);
    // 验证属性是否设置
}

HWTEST_F(InputMethodAbilityTest, clearInputAttribute_011, TestSize.Level0)
{
    // 设置
    InputAttribute inputAttribute = { true, false, true };
    inputMethodAbility_->inputAttribute_ = inputAttribute;

    // 操作
    inputMethodAbility_->ClearInputAttribute();

    // 验证
    EXPECT_EQ(inputMethodAbility_->GetInputAttribute(), InputAttribute{});
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_001, TestSize.Level0)
{
    // 设置
    EXPECT_CALL(*mockInputMethodAbility_, GetImsaProxy()).WillOnce(Return(nullptr));

    // 操作
    int32_t result = mockInputMethodAbility_->HideKeyboard(Trigger::IME_APP);

    // 验证
    EXPECT_EQ(result, ErrorCode::ERROR_IME);
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_002, TestSize.Level0)
{
    // 设置
    EXPECT_CALL(*mockInputMethodAbility_, GetImsaProxy()).WillOnce(Return(std::make_shared<MockImsaProxy>()));
    EXPECT_CALL(*mockInputMethodAbility_, GetSoftKeyboardPanel()).WillOnce(Return(nullptr));

    // 操作
    int32_t result = mockInputMethodAbility_->HideKeyboard(Trigger::IME_APP);

    // 验证
    EXPECT_EQ(result, ErrorCode::ERROR_IME);
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_003, TestSize.Level0)
{
    // 设置
    auto mockPanel = std::make_shared<MockInputMethodPanel>();
    EXPECT_CALL(*mockInputMethodAbility_, GetSoftKeyboardPanel()).WillOnce(Return(mockPanel));
    EXPECT_CALL(*mockPanel, GetPanelFlag()).WillOnce(Return(FLG_CANDIDATE_COLUMN));

    // 操作
    int32_t result = mockInputMethodAbility_->HideKeyboard(Trigger::IME_APP);

    // 验证
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_004, TestSize.Level0)
{
    // 设置
    mockInputMethodAbility_->isCurrentIme_ = true;

    // 操作
    bool result = mockInputMethodAbility_->IsCurrentIme();

    // 验证
    EXPECT_TRUE(result);
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_005, TestSize.Level0)
{
    // 设置
    mockInputMethodAbility_->isDefaultIme_ = true;

    // 操作
    bool result = mockInputMethodAbility_->IsDefaultIme();

    // 验证
    EXPECT_TRUE(result);
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_006, TestSize.Level0)
{
    // 设置
    mockInputMethodAbility_->imeListener_ = nullptr;

    // 操作
    bool result = mockInputMethodAbility_->IsEnable();

    // 验证
    EXPECT_FALSE(result);
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_007, TestSize.Level0)
{
    // 设置
    auto mockPanel = std::make_shared<MockInputMethodPanel>();
    EXPECT_CALL(*mockInputMethodAbility_, GetSoftKeyboardPanel()).WillOnce(Return(mockPanel));

    // 操作
    int32_t result = mockInputMethodAbility_->ExitCurrentInputType();

    // 验证
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_008, TestSize.Level0)
{
    // 设置
    PanelInfo panelInfo = { PanelType::SOFT_KEYBOARD, FLG_CANDIDATE_COLUMN };
    bool isShown = false;

    // 操作
    int32_t result = mockInputMethodAbility_->IsPanelShown(panelInfo, isShown);

    // 验证
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
    EXPECT_FALSE(isShown);
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_009, TestSize.Level0)
{
    // 设置
    auto mockChannel = std::make_shared<MockIRemoteObject>();
    EXPECT_CALL(*mockInputMethodAbility_, GetSoftKeyboardPanel()).WillOnce(Return(nullptr));

    // 操作
    mockInputMethodAbility_->OnClientInactive(mockChannel);

    // 验证
    // 需要验证日志或状态变化，但目前没有直接的返回值或状态变化
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_010, TestSize.Level0)
{
    // 设置
    EXPECT_CALL(*mockInputMethodAbility_, GetInputDataChannelProxy())
        .WillOnce(Return(std::make_shared<MockInputDataChannelProxy>()));

    // 操作
    mockInputMethodAbility_->NotifyKeyboardHeight(100, FLG_FIXED);

    // 验证
    // 需要验证日志或状态变化，但目前没有直接的返回值或状态变化
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_011, TestSize.Level0)
{
    // 设置
    std::unordered_map<std::string, PrivateDataValue> privateCommand = {
        { "key", PrivateDataValue{} }; // 假设的私有命令
    EXPECT_CALL(*mockInputMethodAbility_, IsDefaultIme()).WillOnce(Return(true));
    EXPECT_CALL(*mockInputMethodAbility_, GetInputDataChannelProxy())
        .WillOnce(Return(std::make_shared<MockInputDataChannelProxy>()));

    // 操作
    int32_t result = mockInputMethodAbility_->SendPrivateCommand(privateCommand);

    // 验证
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_012, TestSize.Level0)
{
    // 设置
    std::unordered_map<std::string, PrivateDataValue> privateCommand = {
        { "key", PrivateDataValue{} }; // 假设的私有命令
    EXPECT_CALL(*mockInputMethodAbility_, GetImsaProxy()).WillOnce(Return(std::make_shared<MockImsaProxy>()));

    // 操作
    int32_t result = mockInputMethodAbility_->ReceivePrivateCommand(privateCommand);

    // 验证
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_013, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodAbility_, GetInputDataChannelProxy()).WillOnce(testing::Return(nullptr));
    int32_t result = inputMethodAbility_->SetPreviewText("test", Range{ 0, 4 });
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_014, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodAbility_, GetInputDataChannelProxy()).WillOnce(testing::Return(inputDataChannelProxy_));
    EXPECT_CALL(*inputDataChannelProxy_, SetPreviewText("test", Range{ 0, 4 }))
        .WillOnce(testing::Return(ErrorCode::NO_ERROR));
    int32_t result = inputMethodAbility_->SetPreviewText("test", Range{ 0, 4 });
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, finishTextPreview_015, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodAbility_, GetInputDataChannelProxy()).WillOnce(testing::Return(nullptr));
    int32_t result = inputMethodAbility_->FinishTextPreview(true);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

HWTEST_F(InputMethodAbilityTest, getMsgHandlerCallback_001, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodAbility_, GetInputDataChannelProxy()).WillOnce(testing::Return(inputDataChannelProxy_));
    EXPECT_CALL(*inputDataChannelProxy_, FinishTextPreview(true)).WillOnce(testing::Return(ErrorCode::NO_ERROR));
    int32_t result = inputMethodAbility_->FinishTextPreview(true);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, getMsgHandlerCallback_002, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodAbility_, GetInputDataChannelProxy()).WillOnce(testing::Return(nullptr));
    CallingWindowInfo windowInfo;
    int32_t result = inputMethodAbility_->GetCallingWindowInfo(windowInfo);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}

HWTEST_F(InputMethodAbilityTest, getMsgHandlerCallback_003, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodAbility_, GetInputDataChannelProxy()).WillOnce(testing::Return(inputDataChannelProxy_));
    EXPECT_CALL(*inputMethodAbility_, GetSoftKeyboardPanel()).WillOnce(testing::Return(nullptr));
    CallingWindowInfo windowInfo;
    int32_t result = inputMethodAbility_->GetCallingWindowInfo(windowInfo);
    EXPECT_EQ(result, ErrorCode::ERROR_PANEL_NOT_FOUND);
}

HWTEST_F(InputMethodAbilityTest, getMsgHandlerCallback_004, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodAbility_, GetInputDataChannelProxy()).WillOnce(testing::Return(inputDataChannelProxy_));
    EXPECT_CALL(*inputMethodAbility_, GetSoftKeyboardPanel()).WillOnce(testing::Return(softKeyboardPanel_));
    EXPECT_CALL(*inputMethodAbility_, GetTextConfig(testing::_)).WillOnce(testing::Return(ErrorCode::NO_ERROR));
    EXPECT_CALL(*softKeyboardPanel_, SetCallingWindow(testing::_)).WillOnce(testing::Return(ErrorCode::NO_ERROR));
    EXPECT_CALL(*softKeyboardPanel_, GetCallingWindowInfo(testing::_)).WillOnce(testing::Return(ErrorCode::NO_ERROR));
    CallingWindowInfo windowInfo;
    int32_t result = inputMethodAbility_->GetCallingWindowInfo(windowInfo);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAbilityTest, getMsgHandlerCallback_005, TestSize.Level0)
{
    PanelStatusInfo info;
    info.panelInfo.panelFlag = PanelFlag::FLG_CANDIDATE_COLUMN;
    inputMethodAbility_->NotifyPanelStatusInfo(info, inputDataChannelProxy_);
    // 预期没有交互
}

HWTEST_F(InputMethodAbilityTest, getMsgHandlerCallback_006, TestSize.Level0)
{
    PanelStatusInfo info;
    info.panelInfo.panelFlag = PanelFlag::FLG_NORMAL;
    EXPECT_CALL(*inputDataChannelProxy_, NotifyPanelStatusInfo(info));
    inputMethodAbility_->NotifyPanelStatusInfo(info, inputDataChannelProxy_);
}

HWTEST_F(InputMethodAbilityTest, getMsgHandlerCallback_007, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodAbility_, GetSecurityMode(testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<0>(1), testing::Return(ErrorCode::NO_ERROR)));
    ArrayBuffer arrayBuffer;
    int32_t result = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(result, ErrorCode::ERROR_SECURITY_MODE_OFF);
}

HWTEST_F(InputMethodAbilityTest, getMsgHandlerCallback_008, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodAbility_, GetSecurityMode(testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<0>(static_cast<int32_t>(SecurityMode::FULL)),
            testing::Return(ErrorCode::NO_ERROR)));
    EXPECT_CALL(*inputMethodAbility_, GetInputDataChannelProxy()).WillOnce(testing::Return(nullptr));
    ArrayBuffer arrayBuffer;
    int32_t result = inputMethodAbility_->SendMessage(arrayBuffer);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

HWTEST_F(InputMethodAbilityTest, getMsgHandlerCallback_009, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodAbility_, GetSecurityMode(testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<0>(1), testing::Return(ErrorCode::NO_ERROR)));
    ArrayBuffer arrayBuffer;
    int32_t result = inputMethodAbility_->RecvMessage(arrayBuffer);
    EXPECT_EQ(result, ErrorCode::ERROR_SECURITY_MODE_OFF);
}

HWTEST_F(InputMethodAbilityTest, getMsgHandlerCallback_010, TestSize.Level0)
{
    EXPECT_CALL(*inputMethodAbility_, GetSecurityMode(testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<0>(static_cast<int32_t>(SecurityMode::FULL)),
            testing::Return(ErrorCode::NO_ERROR)));
    EXPECT_CALL(*inputMethodAbility_, GetMsgHandlerCallback()).WillOnce(testing::Return(nullptr));
    ArrayBuffer arrayBuffer;
    int32_t result = inputMethodAbility_->RecvMessage(arrayBuffer);
    EXPECT_EQ(result, ErrorCode::ERROR_MSG_HANDLER_NOT_REGIST);
}

HWTEST_F(InputMethodAbilityTest, getMsgHandlerCallback_011, TestSize.Level0)
{
    EXPECT_CALL(*msgHandlerCallback_, OnTerminated());
    inputMethodAbility_->RegisterMsgHandler(msgHandlerCallback_);
    inputMethodAbility_->RegisterMsgHandler(nullptr);
}

HWTEST_F(InputMethodAbilityTest, getMsgHandlerCallback_012, TestSize.Level0)
{
    inputMethodAbility_->RegisterMsgHandler(msgHandlerCallback_);
    std::shared_ptr<MsgHandlerCallbackInterface> handler = inputMethodAbility_->GetMsgHandlerCallback();
    EXPECT_EQ(handler, msgHandlerCallback_);
}