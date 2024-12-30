/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "i_input_method_agent.h"
#include "i_system_ability_proxy.h"
#include "input_method_agent_proxy.h"
#include "input_method_controller.h"
#include "key_event.h"
#include "msg_handler_callback_interface.h"
#include "on_text_changed_listener.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace OHOS::MiscServices;
using namespace OHOS::MMI;

class InputMethodControllerTest : public Test {
public:
    void SetUp() override
    {
        controller_ = std::make_shared<InputMethodController>();
        mockAgent_ = std::make_shared<MockInputMethodAgent>();
        mockTextListener_ = std::make_shared<MockTextListener>();
        mockSystemAbilityProxy_ = std::make_shared<MockSystemAbilityProxy>();
        mockKeyEvent_ = std::make_shared<MockKeyEvent>();
        mockConfiguration_ = std::make_shared<MockConfiguration>();

        ON_CALL(*mockAgent_, DispatchKeyEvent(_, _)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockAgent_, SetCallingWindow(_)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockAgent_, SendPrivateCommand(_)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockTextListener_, InsertText(_)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockTextListener_, DeleteBackward(_)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockTextListener_, DeleteForward(_)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockTextListener_, MoveCursor(_)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockTextListener_, SendKeyboardStatus(_)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockTextListener_, NotifyPanelStatusInfo(_)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockTextListener_, NotifyKeyboardHeight(_)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockTextListener_, SendFunctionKey(_)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockTextListener_, ReceivePrivateCommand(_)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockTextListener_, SetPreviewText(_, _)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockTextListener_, FinishTextPreview()).WillByDefault(Return(ErrorCode::NO_ERROR));

        controller_->SetAgent(mockAgent_);
        controller_->SetTextListener(mockTextListener_);
    }

protected:
    std::shared_ptr<InputMethodController> controller_;
    std::shared_ptr<MockInputMethodAgent> mockAgent_;
    std::shared_ptr<MockTextListener> mockTextListener_;
    std::shared_ptr<MockSystemAbilityProxy> mockSystemAbilityProxy_;
    std::shared_ptr<MockKeyEvent> mockKeyEvent_;
    std::shared_ptr<MockConfiguration> mockConfiguration_;
};

class MockTextListener : public OnTextChangedListener {
public:
    MOCK_METHOD1(InsertText, void(const std::u16string &text));
    MOCK_METHOD1(DeleteBackward, void(int32_t length));
    MOCK_METHOD1(MoveCursor, void(Direction direction));
    MOCK_METHOD1(SendKeyboardStatus, void(KeyboardStatus status));
    MOCK_METHOD1(NotifyPanelStatusInfo, void(const PanelStatusInfo &info));
    MOCK_METHOD1(NotifyKeyboardHeight, void(uint32_t height));
    MOCK_METHOD1(SendFunctionKey, void(const FunctionKey &funcKey));
    MOCK_METHOD1(HandleExtendAction, void(int32_t action));
    MOCK_METHOD1(HandleSetSelection, void(int32_t start, int32_t end));
    MOCK_METHOD2(HandleSelect, void(int32_t direction, int32_t cursorMoveSkip));
    MOCK_METHOD1(SetPreviewText, int32_t(const std::u16string &text, const Range &range));
    MOCK_METHOD0(FinishTextPreview, void());
    MOCK_METHOD1(
        ReceivePrivateCommand, int32_t(const std::unordered_map<std::string, PrivateDataValue> &privateCommand));
};

class MockAgent : public IInputMethodAgent {
public:
    MOCK_METHOD1(
        DispatchKeyEvent, int32_t(const std::shared_ptr<KeyEvent> &keyEvent, sptr<IKeyEventConsumer> consumer));
    MOCK_METHOD1(SetCallingWindow, void(uint32_t windowId));
    MOCK_METHOD0(StopInputSession, int32_t());
    MOCK_METHOD0(DisplayOptionalInputMethod, int32_t());
    MOCK_METHOD2(ListInputMethodSubtype, int32_t(const std::string &name, std::vector<SubProperty> &subProps));
    MOCK_METHOD1(ListCurrentInputMethodSubtype, int32_t(std::vector<SubProperty> &subProps));
    MOCK_METHOD3(
        SwitchInputMethod, int32_t(SwitchTrigger trigger, const std::string &name, const std::string &subName));
    MOCK_METHOD1(SendPrivateCommand, int32_t(const std::unordered_map<std::string, PrivateDataValue> &privateCommand));
    MOCK_METHOD1(SendMessage, int32_t(const ArrayBuffer &arrayBuffer));
};

class MockSystemAbilityProxy : public ISystemAbilityProxy {
public:
    MOCK_METHOD1(ShowCurrentInput, int32_t());
    MOCK_METHOD1(HideCurrentInput, int32_t());
    MOCK_METHOD1(IsInputTypeSupported, bool(InputType type));
    MOCK_METHOD1(IsCurrentImeByPid, bool(int32_t pid));
    MOCK_METHOD1(StartInputType, int32_t(InputType type));
    MOCK_METHOD2(IsPanelShown, int32_t(const PanelInfo &panelInfo, bool &isShown));
};

class InputMethodControllerTest : public testing::Test {
protected:
    void SetUp() override
    {
        controller_ = std::make_unique<InputMethodController>();
        mockTextListener_ = std::make_shared<MockTextListener>();
        mockAgent_ = std::make_shared<MockAgent>();
        mockSystemAbilityProxy_ = std::make_shared<MockSystemAbilityProxy>();
    }

    void TearDown() override
    {
        controller_.reset();
        mockTextListener_.reset();
        mockAgent_.reset();
        mockSystemAbilityProxy_.reset();
    }

    std::unique_ptr<InputMethodController> controller_;
    std::shared_ptr<MockTextListener> mockTextListener_;
    std::shared_ptr<MockAgent> mockAgent_;
    std::shared_ptr<MockSystemAbilityProxy> mockSystemAbilityProxy_;
};

TEST_F(InputMethodControllerTest, GetRight_EditableAndListenerNotNull_ReturnsText)
{
    std::u16string text;
    EXPECT_CALL(*mockTextListener_, GetRightTextOfCursor(5)).WillOnce(testing::Return(u"sampleText"));
    controller_->SetTextListener(mockTextListener_);
    controller_->SetAgent(mockAgent_);
    int32_t result = controller_->GetRight(5, text);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
    EXPECT_EQ(text, u"sampleText");
}

TEST_F(InputMethodControllerTest, GetTextIndexAtCursor_EditableAndListenerNotNull_ReturnsIndex)
{
    int32_t index;
    auto indexCursor = 10;
    EXPECT_CALL(*mockTextListener_, GetTextIndexAtCursor()).WillOnce(testing::Return(indexCursor));
    controller_->SetTextListener(mockTextListener_);
    controller_->SetAgent(mockAgent_);
    int32_t result = controller_->GetTextIndexAtCursor(index);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
    EXPECT_EQ(index, indexCursor);
}

TEST_F(InputMethodControllerTest, DispatchKeyEvent_EditableAndValidKeyEvent_ReturnsNoError)
{
    auto keyEvent = std::make_shared<KeyEvent>();
    EXPECT_CALL(*mockAgent_, DispatchKeyEvent(testing::Ref(keyEvent), testing::_))
        .WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetAgent(mockAgent_);
    int32_t result = controller_->DispatchKeyEvent(keyEvent, nullptr);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, SetCallingWindow_BoundAndEditable_SetsWindowId)
{
    EXPECT_CALL(*mockAgent_, SetCallingWindow(123)).WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetAgent(mockAgent_);
    int32_t result = controller_->SetCallingWindow(123);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, ShowSoftKeyboard_ProxyNotNull_ReturnsNoError)
{
    EXPECT_CALL(*mockSystemAbilityProxy_, ShowCurrentInput()).WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetSystemAbilityProxy(mockSystemAbilityProxy_);
    int32_t result = controller_->ShowSoftKeyboard();
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, HideSoftKeyboard_ProxyNotNull_ReturnsNoError)
{
    EXPECT_CALL(*mockSystemAbilityProxy_, HideCurrentInput()).WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetSystemAbilityProxy(mockSystemAbilityProxy_);
    int32_t result = controller_->HideSoftKeyboard();
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, StopInputSession_ProxyNotNull_ReturnsNoError)
{
    EXPECT_CALL(*mockAgent_, StopInputSession()).WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetAgent(mockAgent_);
    int32_t result = controller_->StopInputSession();
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, ShowOptionalInputMethod_ProxyNotNull_ReturnsNoError)
{
    EXPECT_CALL(*mockAgent_, DisplayOptionalInputMethod()).WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetAgent(mockAgent_);
    int32_t result = controller_->ShowOptionalInputMethod();
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, ListInputMethodSubtype_ProxyNotNull_ReturnsNoError)
{
    std::vector<SubProperty> subProps;
    EXPECT_CALL(*mockAgent_, ListInputMethodSubtype("test", testing::Ref(subProps)))
        .WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetAgent(mockAgent_);
    int32_t result = controller_->ListInputMethodSubtype({ "test" }, subProps);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, ListCurrentInputMethodSubtype_ProxyNotNull_ReturnsNoError)
{
    std::vector<SubProperty> subProps;
    EXPECT_CALL(*mockAgent_, ListCurrentInputMethodSubtype(testing::Ref(subProps)))
        .WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetAgent(mockAgent_);
    int32_t result = controller_->ListCurrentInputMethodSubtype(subProps);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, SwitchInputMethod_ProxyNotNull_ReturnsNoError)
{
    EXPECT_CALL(*mockAgent_, SwitchInputMethod(SwitchTrigger::SWITCH_TRIGGER_USER, "name", "subName"))
        .WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetAgent(mockAgent_);
    int32_t result = controller_->SwitchInputMethod(SwitchTrigger::SWITCH_TRIGGER_USER, "name", "subName");
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, OnInputReady_AgentObjectNotNull_SetsAgent)
{
    sptr<IRemoteObject> agentObject = new OHOS::IRemoteObject();
    EXPECT_CALL(*mockAgent_, SetAgent(testing::Ref(agentObject)));
    controller_->OnInputReady(agentObject);
}

TEST_F(InputMethodControllerTest, OnInputStop_AgentNotNull_ClearsAgent)
{
    controller_->SetAgent(mockAgent_);
    controller_->OnInputStop(false);
    EXPECT_EQ(controller_->GetAgent(), nullptr);
}

TEST_F(InputMethodControllerTest, ClearEditorCache_NewEditor_ClearsCache)
{
    controller_->ClearEditorCache(true, mockTextListener_);
    // 验证缓存是否被清除
}

TEST_F(InputMethodControllerTest, SelectByRange_EditableAndListenerNotNull_SelectsText)
{
    EXPECT_CALL(*mockTextListener_, HandleSetSelection(0, 5));
    controller_->SetTextListener(mockTextListener_);
    controller_->SelectByRange(0, 5);
}

TEST_F(InputMethodControllerTest, SelectByMovement_EditableAndListenerNotNull_SelectsText)
{
    EXPECT_CALL(*mockTextListener_, HandleSelect(1, 2));
    controller_->SetTextListener(mockTextListener_);
    controller_->SelectByMovement(1, 2);
}

TEST_F(InputMethodControllerTest, HandleExtendAction_EditableAndListenerNotNull_HandlesAction)
{
    EXPECT_CALL(*mockTextListener_, HandleExtendAction(10));
    controller_->SetTextListener(mockTextListener_);
    int32_t result = controller_->HandleExtendAction(10);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, InsertText_EditableAndListenerNotNull_InsertsText)
{
    EXPECT_CALL(*mockTextListener_, InsertText(u"sampleText"));
    controller_->SetTextListener(mockTextListener_);
    int32_t result = controller_->InsertText(u"sampleText");
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, DeleteForward_EditableAndListenerNotNull_DeletesText)
{
    EXPECT_CALL(*mockTextListener_, DeleteBackward(5));
    controller_->SetTextListener(mockTextListener_);
    int32_t result = controller_->DeleteForward(5);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, DeleteBackward_EditableAndListenerNotNull_DeletesText)
{
    EXPECT_CALL(*mockTextListener_, DeleteForward(5));
    controller_->SetTextListener(mockTextListener_);
    int32_t result = controller_->DeleteBackward(5);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, MoveCursor_EditableAndListenerNotNull_MovesCursor)
{
    EXPECT_CALL(*mockTextListener_, MoveCursor(Direction::FORWARD));
    controller_->SetTextListener(mockTextListener_);
    int32_t result = controller_->MoveCursor(Direction::FORWARD);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, SendKeyboardStatus_ListenerNotNull_SendsStatus)
{
    EXPECT_CALL(*mockTextListener_, SendKeyboardStatus(KeyboardStatus::SHOW));
    controller_->SetTextListener(mockTextListener_);
    controller_->SendKeyboardStatus(KeyboardStatus::SHOW);
}

TEST_F(InputMethodControllerTest, NotifyPanelStatusInfo_ListenerNotNull_NotifiesStatus)
{
    PanelStatusInfo info;
    EXPECT_CALL(*mockTextListener_, NotifyPanelStatusInfo(testing::Ref(info)));
    controller_->SetTextListener(mockTextListener_);
    controller_->NotifyPanelStatusInfo(info);
}

TEST_F(InputMethodControllerTest, NotifyKeyboardHeight_ListenerNotNull_NotifiesHeight)
{
    EXPECT_CALL(*mockTextListener_, NotifyKeyboardHeight(100));
    controller_->SetTextListener(mockTextListener_);
    controller_->NotifyKeyboardHeight(100);
}

TEST_F(InputMethodControllerTest, SendFunctionKey_EditableAndListenerNotNull_SendsFunctionKey)
{
    FunctionKey funcKey;
    EXPECT_CALL(*mockTextListener_, SendFunctionKey(testing::Ref(funcKey)));
    controller_->SetTextListener(mockTextListener_);
    int32_t result = controller_->SendFunctionKey(1);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, IsInputTypeSupported_ProxyNotNull_ReturnsSupported)
{
    EXPECT_CALL(*mockSystemAbilityProxy_, IsInputTypeSupported(InputType::TEXT)).WillOnce(testing::Return(true));
    controller_->SetSystemAbilityProxy(mockSystemAbilityProxy_);
    bool result = controller_->IsInputTypeSupported(InputType::TEXT);
    EXPECT_TRUE(result);
}

TEST_F(InputMethodControllerTest, IsCurrentImeByPid_ProxyNotNull_ReturnsPid)
{
    EXPECT_CALL(*mockSystemAbilityProxy_, IsCurrentImeByPid(123)).WillOnce(testing::Return(true));
    controller_->SetSystemAbilityProxy(mockSystemAbilityProxy_);
    bool result = controller_->IsCurrentImeByPid(123);
    EXPECT_TRUE(result);
}

TEST_F(InputMethodControllerTest, StartInputType_ProxyNotNull_StartsType)
{
    EXPECT_CALL(*mockSystemAbilityProxy_, StartInputType(InputType::TEXT))
        .WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetSystemAbilityProxy(mockSystemAbilityProxy_);
    int32_t result = controller_->StartInputType(InputType::TEXT);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, IsPanelShown_ProxyNotNull_ReturnsShown)
{
    PanelInfo panelInfo;
    bool isShown;
    EXPECT_CALL(*mockSystemAbilityProxy_, IsPanelShown(testing::Ref(panelInfo), testing::Ref(isShown)))
        .WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetSystemAbilityProxy(mockSystemAbilityProxy_);
    int32_t result = controller_->IsPanelShown(panelInfo, isShown);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, ReceivePrivateCommand_ListenerNotNull_ReceivesCommand)
{
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    EXPECT_CALL(*mockTextListener_, ReceivePrivateCommand(testing::Ref(privateCommand)))
        .WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetTextListener(mockTextListener_);
    int32_t result = controller_->ReceivePrivateCommand(privateCommand);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, SendPrivateCommand_ProxyNotNull_SendsCommand)
{
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    EXPECT_CALL(*mockAgent_, SendPrivateCommand(testing::Ref(privateCommand)))
        .WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetAgent(mockAgent_);
    int32_t result = controller_->SendPrivateCommand(privateCommand);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, SetPreviewText_PreviewSupported_SetsText)
{
    EXPECT_CALL(*mockTextListener_, SetPreviewText(u"sampleText", testing::_))
        .WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetTextListener(mockTextListener_);
    int32_t result = controller_->SetPreviewText("sampleText", { 0, 5 });
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, FinishTextPreview_PreviewSupported_FinishesPreview)
{
    EXPECT_CALL(*mockTextListener_, FinishTextPreview());
    controller_->SetTextListener(mockTextListener_);
    int32_t result = controller_->FinishTextPreview();
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, SendMessage_ProxyNotNull_SendsMessage)
{
    ArrayBuffer arrayBuffer;
    EXPECT_CALL(*mockAgent_, SendMessage(testing::Ref(arrayBuffer))).WillOnce(testing::Return(ErrorCode::NO_ERROR));
    controller_->SetAgent(mockAgent_);
    int32_t result = controller_->SendMessage(arrayBuffer);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, OnConfigurationChange_NotBound_ReturnsError)
{
    EXPECT_CALL(*mockAgent_, GetAgent()).WillOnce(Return(nullptr));
    int32_t result = controller_->OnConfigurationChange(*mockConfiguration_);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NOT_BOUND);
}

TEST_F(InputMethodControllerTest, OnConfigurationChange_NotEditable_ReturnsError)
{
    EXPECT_CALL(*mockAgent_, GetAgent()).WillOnce(Return(mockAgent_));
    EXPECT_CALL(*mockTextListener_, IsEditable()).WillOnce(Return(false));
    int32_t result = controller_->OnConfigurationChange(*mockConfiguration_);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
}

TEST_F(InputMethodControllerTest, OnConfigurationChange_SecurityFlagChanged_RestartsInput)
{
    EXPECT_CALL(*mockAgent_, GetAgent()).WillOnce(Return(mockAgent_));
    EXPECT_CALL(*mockTextListener_, IsEditable()).WillOnce(Return(true));
    EXPECT_CALL(*mockAgent_, StartInput(_, _)).WillOnce(Return(ErrorCode::NO_ERROR));
    EXPECT_CALL(*mockAgent_, OnInputReady(_)).WillOnce(Return(ErrorCode::NO_ERROR));

    bool oldSecurityFlag = true;
    controller_->OnConfigurationChange(*mockConfiguration_);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodControllerTest, GetLeft_NotEditable_ReturnsError)
{
    EXPECT_CALL(*mockTextListener_, IsEditable()).WillOnce(Return(false));
    std::u16string text;
    int32_t result = controller_->GetLeft(10, text);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
}

TEST_F(InputMethodControllerTest, GetRight_NotEditable_ReturnsError)
{
    EXPECT_CALL(*mockTextListener_, IsEditable()).WillOnce(Return(false));
    std::u16string text;
    int32_t result = controller_->GetRight(10, text);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
}

TEST_F(InputMethodControllerTest, GetTextIndexAtCursor_NotEditable_ReturnsError)
{
    EXPECT_CALL(*mockTextListener_, IsEditable()).WillOnce(Return(false));
    int32_t index;
    int32_t result = controller_->GetTextIndexAtCursor(index);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
}

TEST_F(InputMethodControllerTest, DispatchKeyEvent_NotEditable_ReturnsError)
{
    EXPECT_CALL(*mockTextListener_, IsEditable()).WillOnce(Return(false));
    int32_t result = controller_->DispatchKeyEvent(mockKeyEvent_, nullptr);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
}

TEST_F(InputMethodControllerTest, DispatchKeyEvent_NullKeyEvent_ReturnsError)
{
    EXPECT_CALL(*mockTextListener_, IsEditable()).WillOnce(Return(true));
    int32_t result = controller_->DispatchKeyEvent(nullptr, nullptr);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodControllerTest, SetCallingWindow_NotEditable_ReturnsError)
{
    EXPECT_CALL(*mockTextListener_, IsEditable()).WillOnce(Return(false));
    int32_t result = controller_->SetCallingWindow(1);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
}

TEST_F(InputMethodControllerTest, ShowSoftKeyboard_NullProxy_ReturnsError)
{
    EXPECT_CALL(*mockSystemAbilityProxy_, GetSystemAbilityProxy()).WillOnce(Return(nullptr));
    int32_t result = controller_->ShowSoftKeyboard();
    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodControllerTest, HideSoftKeyboard_NullProxy_ReturnsError)
{
    EXPECT_CALL(*mockSystemAbilityProxy_, GetSystemAbilityProxy()).WillOnce(Return(nullptr));
    int32_t result = controller_->HideSoftKeyboard();
    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodControllerTest, StopInputSession_NullProxy_ReturnsError)
{
    EXPECT_CALL(*mockSystemAbilityProxy_, GetSystemAbilityProxy()).WillOnce(Return(nullptr));
    int32_t result = controller_->StopInputSession();
    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodControllerTest, ShowOptionalInputMethod_NullProxy_ReturnsError)
{
    EXPECT_CALL(*mockSystemAbilityProxy_, GetSystemAbilityProxy()).WillOnce(Return(nullptr));
    int32_t result = controller_->ShowOptionalInputMethod();
    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodControllerTest, ListInputMethodSubtype_NullProxy_ReturnsError)
{
    EXPECT_CALL(*mockSystemAbilityProxy_, GetSystemAbilityProxy()).WillOnce(Return(nullptr));
    std::vector<SubProperty> subProps;
    int32_t result = controller_->ListInputMethodSubtype(Property(), subProps);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodControllerTest, ListCurrentInputMethodSubtype_NullProxy_ReturnsError)
{
    EXPECT_CALL(*mockSystemAbilityProxy_, GetSystemAbilityProxy()).WillOnce(Return(nullptr));
    std::vector<SubProperty> subProps;
    int32_t result = controller_->ListCurrentInputMethodSubtype(subProps);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodControllerTest, SwitchInputMethod_NullProxy_ReturnsError)
{
    EXPECT_CALL(*mockSystemAbilityProxy_, GetSystemAbilityProxy()).WillOnce(Return(nullptr));
    int32_t result = controller_->SwitchInputMethod(SwitchTrigger::SWITCH_TRIGGER_USER, "name", "subName");
    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodControllerTest, SendPrivateCommand_NotBound_ReturnsError)
{
    EXPECT_CALL(*mockAgent_, GetAgent()).WillOnce(Return(nullptr));
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    int32_t result = controller_->SendPrivateCommand(privateCommand);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NOT_BOUND);
}

TEST_F(InputMethodControllerTest, SendPrivateCommand_NotEditable_ReturnsError)
{
    EXPECT_CALL(*mockAgent_, GetAgent()).WillOnce(Return(mockAgent_));
    EXPECT_CALL(*mockTextListener_, IsEditable()).WillOnce(Return(false));
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    int32_t result = controller_->SendPrivateCommand(privateCommand);
    EXPECT_EQ(result, ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
}

TEST_F(InputMethodControllerTest, SetPreviewText_NotSupported_ReturnsError)
{
    controller_->textConfig_.inputAttribute.isTextPreviewSupported = false;
    int32_t result = controller_->SetPreviewText("text", Range());
    EXPECT_EQ(result, ErrorCode::ERROR_TEXT_PREVIEW_NOT_SUPPORTED);
}

TEST_F(InputMethodControllerTest, FinishTextPreview_NotSupported_ReturnsError)
{
    controller_->textConfig_.inputAttribute.isTextPreviewSupported = false;
    int32_t result = controller_->FinishTextPreview();
    EXPECT_EQ(result, ErrorCode::ERROR_TEXT_PREVIEW_NOT_SUPPORTED);
}