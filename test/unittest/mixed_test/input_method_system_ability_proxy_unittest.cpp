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

#include "element_name.h"
#include "iinput_client.h"
#include "iinput_method_core.h"
#include "ime_window_info.h"
#include "input_method_system_ability_proxy.h"
#include "input_type.h"
#include "input_window_status.h"
#include "inputmethod_service_ipc_interface_code.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "panel_info.h"
#include "property.h"
#include "sub_property.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace OHOS;
using namespace MiscServices;
using namespace testing;

class MockRemoteObject : public IRemoteObject {
public:
    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
};

class MockMessageParcel : public MessageParcel {
public:
    MOCK_METHOD1(WriteInterfaceToken, bool(const std::string &));
    MOCK_METHOD1(WriteRemoteObject, bool(const sptr<IRemoteObject> &));
    MOCK_METHOD1(ReadRemoteObject, sptr<IRemoteObject>());
    MOCK_METHOD1(ReadInt32, int32_t(int32_t *));
};

class MockMessageOption : public MessageOption {
public:
    MOCK_METHOD1(SetFlags, void(int32_t));
    MOCK_METHOD0(GetFlags, int32_t());
};

class MockITypesUtil {
public:
    MOCK_METHOD3(Marshal, bool(MessageParcel &, const std::string &, const std::string &, SwitchTrigger));
    MOCK_METHOD2(Marshal, bool(MessageParcel &, const std::string &));
    MOCK_METHOD3(Marshal, bool(MessageParcel &, uint32_t, const sptr<IRemoteObject> &));
    MOCK_METHOD4(
        Marshal, bool(MessageParcel &, InputClientInfo &, const sptr<IRemoteObject> &, const sptr<IRemoteObject> &));
    MOCK_METHOD2(Marshal, bool(MessageParcel &, bool));
    MOCK_METHOD2(Marshal, bool(MessageParcel &, uint32_t));
    MOCK_METHOD2(Marshal, bool(MessageParcel &, InputType));
    MOCK_METHOD2(Marshal, bool(MessageParcel &, const PanelInfo &));
    MOCK_METHOD2(Marshal, bool(MessageParcel &, int32_t));
    MOCK_METHOD2(Unmarshal, bool(MessageParcel &, bool &));
    MOCK_METHOD2(Unmarshal, bool(MessageParcel &, int32_t &));
    MOCK_METHOD2(Unmarshal, bool(MessageParcel &, std::shared_ptr<Property> &));
    MOCK_METHOD2(Unmarshal, bool(MessageParcel &, std::shared_ptr<SubProperty> &));
    MOCK_METHOD2(Unmarshal, bool(MessageParcel &, OHOS::AppExecFwk::ElementName &));
    MOCK_METHOD2(Unmarshal, bool(MessageParcel &, std::vector<Property> &));
    MOCK_METHOD2(Unmarshal, bool(MessageParcel &, std::vector<SubProperty> &));
};

class InputMethodSystemAbilityProxyTest : public Test {
protected:
    void SetUp() override
    {
        remoteObject = new MockRemoteObject();
        proxy = new InputMethodSystemAbilityProxy(remoteObject);
    }

    void TearDown() override
    {
        delete proxy;
        delete remoteObject;
    }

    MockRemoteObject *remoteObject;
    InputMethodSystemAbilityProxy *proxy;
};

TEST_F(InputMethodSystemAbilityProxyTest, StartInput_ClientIsNull_ReturnsError)
{
    InputClientInfo inputClientInfo;
    inputClientInfo.client = nullptr;
    sptr<IRemoteObject> agent;

    int32_t result = proxy->StartInput(inputClientInfo, agent);

    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityProxyTest, StartInput_ClientIsNotNull_ReturnsSuccess)
{
    InputClientInfo inputClientInfo;
    inputClientInfo.client = new MockRemoteObject();
    sptr<IRemoteObject> agent;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->StartInput(inputClientInfo, agent);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, ConnectSystemCmd_SuccessfulConnection)
{
    sptr<IRemoteObject> channel = new MockRemoteObject();
    sptr<IRemoteObject> agent;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->ConnectSystemCmd(channel, agent);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, ShowCurrentInput_SuccessfulExecution)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->ShowCurrentInput();

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, HideCurrentInput_SuccessfulExecution)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->HideCurrentInput();

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, StopInputSession_SuccessfulExecution)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->StopInputSession();

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, ShowInput_ClientIsNull_ReturnsError)
{
    sptr<IInputClient> client = nullptr;

    int32_t result = proxy->ShowInput(client);

    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityProxyTest, ShowInput_ClientIsNotNull_ReturnsSuccess)
{
    sptr<IInputClient> client = new MockRemoteObject();

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->ShowInput(client);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, HideInput_ClientIsNull_ReturnsError)
{
    sptr<IInputClient> client = nullptr;

    int32_t result = proxy->HideInput(client);

    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityProxyTest, HideInput_ClientIsNotNull_ReturnsSuccess)
{
    sptr<IInputClient> client = new MockRemoteObject();

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->HideInput(client);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, ReleaseInput_ClientIsNull_ReturnsError)
{
    sptr<IInputClient> client = nullptr;

    int32_t result = proxy->ReleaseInput(client);

    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityProxyTest, ReleaseInput_ClientIsNotNull_ReturnsSuccess)
{
    sptr<IInputClient> client = new MockRemoteObject();

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->ReleaseInput(client);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, RequestShowInput_SuccessfulExecution)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->RequestShowInput();

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, RequestHideInput_SuccessfulExecution)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->RequestHideInput();

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, InitConnect_SuccessfulExecution)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->InitConnect();

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, DisplayOptionalInputMethod_SuccessfulExecution)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->DisplayOptionalInputMethod();

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, SetCoreAndAgent_CoreIsNull_ReturnsError)
{
    sptr<IInputMethodCore> core = nullptr;
    sptr<IRemoteObject> agent;

    int32_t result = proxy->SetCoreAndAgent(core, agent);

    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityProxyTest, SetCoreAndAgent_CoreIsNotNull_ReturnsSuccess)
{
    sptr<IInputMethodCore> core = new MockRemoteObject();
    sptr<IRemoteObject> agent;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->SetCoreAndAgent(core, agent);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, GetDefaultInputMethod_SuccessfulExecution)
{
    std::shared_ptr<Property> property;
    bool isBrief = true;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->GetDefaultInputMethod(property, isBrief);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, GetInputMethodConfig_SuccessfulExecution)
{
    OHOS::AppExecFwk::ElementName inputMethodConfig;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->GetInputMethodConfig(inputMethodConfig);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, GetSecurityMode_SuccessfulExecution)
{
    int32_t security;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->GetSecurityMode(security);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, UnRegisteredProxyIme_CoreIsNull_ReturnsError)
{
    UnRegisteredType type = UnRegisteredType::TYPE_1;
    sptr<IInputMethodCore> core = nullptr;

    int32_t result = proxy->UnRegisteredProxyIme(type, core);

    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityProxyTest, UnRegisteredProxyIme_CoreIsNotNull_ReturnsSuccess)
{
    UnRegisteredType type = UnRegisteredType::TYPE_1;
    sptr<IInputMethodCore> core = new MockRemoteObject();

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->UnRegisteredProxyIme(type, core);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, GetCurrentInputMethod_SuccessfulExecution)
{
    std::shared_ptr<Property> property;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    property = proxy->GetCurrentInputMethod();

    EXPECT_NE(property, nullptr);
}

TEST_F(InputMethodSystemAbilityProxyTest, GetCurrentInputMethodSubtype_SuccessfulExecution)
{
    std::shared_ptr<SubProperty> property;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    property = proxy->GetCurrentInputMethodSubtype();

    EXPECT_NE(property, nullptr);
}

TEST_F(InputMethodSystemAbilityProxyTest, IsDefaultImeSet_SuccessfulExecution)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    bool result = proxy->IsDefaultImeSet();

    EXPECT_TRUE(result);
}

TEST_F(InputMethodSystemAbilityProxyTest, EnableIme_SuccessfulExecution)
{
    std::string bundleName = "testBundle";

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    bool result = proxy->EnableIme(bundleName);

    EXPECT_TRUE(result);
}

TEST_F(InputMethodSystemAbilityProxyTest, SetCallingWindow_ClientIsNull_ReturnsError)
{
    uint32_t windowId = 1;
    sptr<IInputClient> client = nullptr;

    int32_t result = proxy->SetCallingWindow(windowId, client);

    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityProxyTest, SetCallingWindow_ClientIsNotNull_ReturnsSuccess)
{
    uint32_t windowId = 1;
    sptr<IInputClient> client = new MockRemoteObject();

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->SetCallingWindow(windowId, client);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, GetInputStartInfo_SuccessfulExecution)
{
    bool isInputStart;
    uint32_t callingWndId;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->GetInputStartInfo(isInputStart, callingWndId);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, ListInputMethod_SuccessfulExecution)
{
    InputMethodStatus status = InputMethodStatus::STATUS_ENABLED;
    std::vector<Property> props;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->ListInputMethod(status, props);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, ShowCurrentInputDeprecated_SuccessfulExecution)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->ShowCurrentInputDeprecated();

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, HideCurrentInputDeprecated_SuccessfulExecution)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->HideCurrentInputDeprecated();

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, ListInputMethodSubtype_SuccessfulExecution)
{
    std::string name = "testName";
    std::vector<SubProperty> subProps;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->ListInputMethodSubtype(name, subProps);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, ListCurrentInputMethodSubtype_SuccessfulExecution)
{
    std::vector<SubProperty> subProps;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->ListCurrentInputMethodSubtype(subProps);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, SwitchInputMethod_SuccessfulExecution)
{
    std::string name = "testName";
    std::string subName = "testSubName";
    SwitchTrigger trigger = SwitchTrigger::TRIGGER_USER;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->SwitchInputMethod(name, subName, trigger);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, PanelStatusChange_SuccessfulExecution)
{
    InputWindowStatus status = InputWindowStatus::STATUS_SHOWN;
    ImeWindowInfo info;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->PanelStatusChange(status, info);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, UpdateListenEventFlag_ClientIsNull_ReturnsError)
{
    InputClientInfo clientInfo;
    clientInfo.client = nullptr;
    uint32_t eventFlag = 1;

    int32_t result = proxy->UpdateListenEventFlag(clientInfo, eventFlag);

    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(InputMethodSystemAbilityProxyTest, UpdateListenEventFlag_ClientIsNotNull_ReturnsSuccess)
{
    InputClientInfo clientInfo;
    clientInfo.client = new MockRemoteObject();
    uint32_t eventFlag = 1;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->UpdateListenEventFlag(clientInfo, eventFlag);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, IsCurrentIme_SuccessfulExecution)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    bool result = proxy->IsCurrentIme();

    EXPECT_TRUE(result);
}

TEST_F(InputMethodSystemAbilityProxyTest, IsCurrentImeByPid_SuccessfulExecution)
{
    int32_t pid = 1;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    bool result = proxy->IsCurrentImeByPid(pid);

    EXPECT_TRUE(result);
}

TEST_F(InputMethodSystemAbilityProxyTest, IsInputTypeSupported_SuccessfulExecution)
{
    InputType type = InputType::TYPE_TEXT;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    bool result = proxy->IsInputTypeSupported(type);

    EXPECT_TRUE(result);
}

TEST_F(InputMethodSystemAbilityProxyTest, StartInputType_SuccessfulExecution)
{
    InputType type = InputType::TYPE_TEXT;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->StartInputType(type);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, ExitCurrentInputType_SuccessfulExecution)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->ExitCurrentInputType();

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, IsPanelShown_SuccessfulExecution)
{
    PanelInfo panelInfo;
    bool isShown;

    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->IsPanelShown(panelInfo, isShown);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputMethodSystemAbilityProxyTest, IsDefaultIme_SuccessfulExecution)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));

    int32_t result = proxy->IsDefaultIme();

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}