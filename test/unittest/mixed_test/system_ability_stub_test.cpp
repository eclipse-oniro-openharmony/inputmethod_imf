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
#include <gtest/gtest.h>

#include "input_method_system_ability_stub.h"
#include "mock_input_client.h"
#include "mock_input_method_core.h"

using namespace OHOS;
using namespace OHOS::MiscServices;
using namespace testing;
using namespace testing::ext;

class InputMethodSystemAbilityStubTest : public Test {
protected:
    void SetUp() override
    {
        stub_ = new InputMethodSystemAbilityStub();
    }

    void TearDown() override
    {
        delete stub_;
    }

    InputMethodSystemAbilityStub *stub_;
};

/**
 * @tc.name: StartInputOnRemote_ValidClient_ReturnsSuccess
 * @tc.desc: Verify that the StartInputOnRemote method returns success when a valid client is provided
 * @tc.type: FUNC
*/
HWTEST_F(InputMethodSystemAbilityStubTest, StartInputOnRemote_ValidClient_ReturnsSuccess, TestSize.Level1)
{
    MessageParcel data, reply;
    MessageOption option;
    InputClientInfo clientInfo;
    sptr<IRemoteObject> client = new MockInputClient();
    ITypesUtil::Marshal(data, clientInfo, client, clientInfo.channel);
    EXPECT_CALL(*stub_, StartInput(clientInfo, _)).WillOnce(Return(0));
    int32_t result = stub_->StartInputOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: StartInputOnRemote_InvalidClient_ReturnsError
 * @tc.desc: Verify that the StartInputOnRemote method returns error when an invalid client is provided
 * @tc.type: FUNC
*/
HWTEST_F(InputMethodSystemAbilityStubTest, StartInputOnRemote_InvalidClient_ReturnsError, TestSize.Level1)
{
    MessageParcel data, reply;
    MessageOption option;
    InputClientInfo clientInfo;
    sptr<IRemoteObject> client = nullptr;
    ITypesUtil::Marshal(data, clientInfo, client, clientInfo.channel);
    int32_t result = stub_->StartInputOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

/**
 * @tc.name: ShowCurrentInputOnRemote_Success_ReturnsSuccess
 * @tc.desc: Verify that the ShowCurrentInputOnRemote method returns success when the operation is successful
 * @tc.type: FUNC
*/
HWTEST_F(InputMethodSystemAbilityStubTest, ShowCurrentInputOnRemote_Success_ReturnsSuccess, TestSize.Level1)
{
    MessageParcel data, reply;
    MessageOption option;
    EXPECT_CALL(*stub_, ShowCurrentInput()).WillOnce(Return(0));
    int32_t result = stub_->ShowCurrentInputOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: ShowCurrentInputOnRemote_Failure_ReturnsError
 * @tc.desc: Verify that the ShowCurrentInputOnRemote method returns error when the operation fails
 * @tc.type: FUNC
*/
HWTEST_F(InputMethodSystemAbilityStubTest, ShowCurrentInputOnRemote_Failure_ReturnsError, TestSize.Level1)
{
    MessageParcel data, reply;
    MessageOption option;
    EXPECT_CALL(*stub_, ShowCurrentInput()).WillOnce(Return(-1));
    int32_t result = stub_->ShowCurrentInputOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

/**
 * @tc.name: ReleaseInputOnRemote_ValidClient_ReturnsSuccess
 * @tc.desc: Verify that the ReleaseInputOnRemote method returns success when a valid client is provided
 * @tc.type: FUNC
*/
HWTEST_F(InputMethodSystemAbilityStubTest, ReleaseInputOnRemote_ValidClient_ReturnsSuccess, TestSize.Level1)
{
    MessageParcel data, reply;
    MessageOption option;
    sptr<IRemoteObject> client = new MockInputClient();
    data.WriteRemoteObject(client);
    EXPECT_CALL(*stub_, ReleaseInput(_)).WillOnce(Return(0));
    int32_t result = stub_->ReleaseInputOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: ReleaseInputOnRemote_InvalidClient_ReturnsError
 * @tc.desc: Verify that the ReleaseInputOnRemote method returns error when an invalid client is provided
 * @tc.type: FUNC
*/
HWTEST_F(InputMethodSystemAbilityStubTest, ReleaseInputOnRemote_InvalidClient_ReturnsError, TestSize.Level1)
{
    MessageParcel data, reply;
    MessageOption option;
    data.WriteRemoteObject(nullptr);
    int32_t result = stub_->ReleaseInputOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

/**
 * @tc.name: GetDefaultInputMethodOnRemote_Success_ReturnsSuccess
 * @tc.desc: Verify that the GetDefaultInputMethodOnRemote method returns success when the operation is successful
 * @tc.type: FUNC
*/
HWTEST_F(InputMethodSystemAbilityStubTest, GetDefaultInputMethodOnRemote_Success_ReturnsSuccess, TestSize.Level1)
{
    MessageParcel data, reply;
    MessageOption option;
    bool isBrief = true;
    data.WriteBool(isBrief);
    std::shared_ptr<Property> prop = std::make_shared<Property>();
    EXPECT_CALL(*stub_, GetDefaultInputMethod(_, _)).WillOnce(DoAll(SetArgReferee<0>(*prop), Return(0)));
    int32_t result = stub_->GetDefaultInputMethodOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetDefaultInputMethodOnRemote_Failure_ReturnsError
 * @tc.desc: Verify that the GetDefaultInputMethodOnRemote method returns error when the operation fails
 * @tc.type: FUNC
*/
HWTEST_F(InputMethodSystemAbilityStubTest, GetDefaultInputMethodOnRemote_Failure_ReturnsError, TestSize.Level1)
{
    MessageParcel data, reply;
    MessageOption option;
    bool isBrief = true;
    data.WriteBool(isBrief);
    EXPECT_CALL(*stub_, GetDefaultInputMethod(_, _)).WillOnce(Return(-1));
    int32_t result = stub_->GetDefaultInputMethodOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

/**
 * @tc.name: IsDefaultImeSetOnRemote_ReturnsSuccess
 * @tc.desc: Verify that the IsDefaultImeSetOnRemote method returns success
 * @tc.type: FUNC
*/
HWTEST_F(InputMethodSystemAbilityStubTest, IsDefaultImeSetOnRemote_ReturnsSuccess, TestSize.Level1)
{
    MessageParcel data, reply;
    MessageOption option;
    EXPECT_CALL(*stub_, IsDefaultImeSet()).WillOnce(Return(true));
    int32_t result = stub_->IsDefaultImeSetOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}
/**
 *  * @tc.name: RequestShowInputOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the RequestShowInputOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, RequestShowInputOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t result = stub.RequestShowInputOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: RequestHideInputOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the RequestHideInputOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, RequestHideInputOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t result = stub.RequestHideInputOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: GetCurrentInputMethodOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the GetCurrentInputMethodOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, GetCurrentInputMethodOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t result = stub.GetCurrentInputMethodOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: GetCurrentInputMethodSubtypeOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the GetCurrentInputMethodSubtypeOnRemote method
 *  returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, GetCurrentInputMethodSubtypeOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t result = stub.GetCurrentInputMethodSubtypeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: ListInputMethodOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the ListInputMethodOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, ListInputMethodOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(1); // Assuming 1 is a valid status for testing
    int32_t result = stub.ListInputMethodOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: SwitchInputMethodOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the SwitchInputMethodOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, SwitchInputMethodOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteString("testName");
    data.WriteString("testSubName");
    data.WriteInt32(0); // Assuming 0 is a valid SwitchTrigger for testing
    int32_t result = stub.SwitchInputMethodOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: DisplayOptionalInputMethodOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the DisplayOptionalInputMethodOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, DisplayOptionalInputMethodOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, DisplayOptionalInputMethod()).WillOnce(Return(0));
    int32_t result = stub_->DisplayOptionalInputMethodOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: SetCoreAndAgentOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the SetCoreAndAgentOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, SetCoreAndAgentOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> coreObject = new MockInputMethodCore();
    sptr<IRemoteObject> agentObject = new MockInputMethodCore();
    data.WriteRemoteObject(coreObject);
    data.WriteRemoteObject(agentObject);
    EXPECT_CALL(*stub_, SetCoreAndAgent(_, _)).WillOnce(Return(0));
    int32_t result = stub_->SetCoreAndAgentOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: UnRegisteredProxyImeOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the UnRegisteredProxyImeOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, UnRegisteredProxyImeOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t type = 0;
    sptr<IRemoteObject> coreObject = new MockInputMethodCore();
    data.WriteInt32(type);
    data.WriteRemoteObject(coreObject);
    EXPECT_CALL(*stub_, UnRegisteredProxyIme(_, _)).WillOnce(Return(0));
    int32_t result = stub_->UnRegisteredProxyImeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: ListInputMethodSubtypeOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the ListInputMethodSubtypeOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, ListInputMethodSubtypeOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);
    EXPECT_CALL(*stub_, ListInputMethodSubtype(_, _)).WillOnce(Return(0));
    int32_t result = stub_->ListInputMethodSubtypeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: ListCurrentInputMethodSubtypeOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the ListCurrentInputMethodSubtypeOnRemote method
 *  returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, ListCurrentInputMethodSubtypeOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, ListCurrentInputMethodSubtype(_)).WillOnce(Return(0));
    int32_t result = stub_->ListCurrentInputMethodSubtypeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: PanelStatusChangeOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the PanelStatusChangeOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, PanelStatusChangeOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t status = 0;
    ImeWindowInfo info;
    data.WriteUint32(status);
    data.WriteParcelable(&info);
    EXPECT_CALL(*stub_, PanelStatusChange(_, _)).WillOnce(Return(0));
    int32_t result = stub_->PanelStatusChangeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: UpdateListenEventFlagOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the UpdateListenEventFlagOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, UpdateListenEventFlagOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    InputClientInfo clientInfo;
    sptr<IRemoteObject> client = new MockInputClient();
    uint32_t eventFlag = 0;
    ITypesUtil::Marshal(data, clientInfo, client, clientInfo.channel, eventFlag);
    EXPECT_CALL(*stub_, UpdateListenEventFlag(_, _)).WillOnce(Return(0));
    int32_t result = stub_->UpdateListenEventFlagOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: IsCurrentImeOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the IsCurrentImeOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, IsCurrentImeOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, IsCurrentIme()).WillOnce(Return(true));
    int32_t result = stub_->IsCurrentImeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: IsInputTypeSupportedOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the IsInputTypeSupportedOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, IsInputTypeSupportedOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    InputType type;
    data.WriteParcelable(&type);
    EXPECT_CALL(*stub_, IsInputTypeSupported(_)).WillOnce(Return(true));
    int32_t result = stub_->IsInputTypeSupportedOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: StartInputTypeOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the StartInputTypeOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, StartInputTypeOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    InputType type;
    data.WriteParcelable(&type);
    EXPECT_CALL(*stub_, StartInputType(_)).WillOnce(Return(0));
    int32_t result = stub_->StartInputTypeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: ExitCurrentInputTypeOnRemote_ValidData_ReturnsNoError
 *  * @tc.desc: Verify that the ExitCurrentInputTypeOnRemote method returns NO_ERROR when valid data is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, ExitCurrentInputTypeOnRemote_ValidData_ReturnsNoError)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, ExitCurrentInputType()).WillOnce(Return(0));
    int32_t result = stub_->ExitCurrentInputTypeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: ConnectSystemCmdOnRemote_ValidSystemCmdStub_ReturnsSuccess
 *  * @tc.desc: Verify that the ConnectSystemCmdOnRemote method
 *  returns success when a valid system command stub is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, ConnectSystemCmdOnRemote_ValidSystemCmdStub_ReturnsSuccess)
{
    InputMethodSystemAbilityStub stub;
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> systemCmdStub = new OHOS::IRemoteObject();
    data.WriteRemoteObject(systemCmdStub);

    int32_t result = stub.ConnectSystemCmdOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: ConnectSystemCmdOnRemote_NullSystemCmdStub_ReturnsError
 *  * @tc.desc: Verify that the ConnectSystemCmdOnRemote method
 *  returns error when a null system command stub is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, ConnectSystemCmdOnRemote_NullSystemCmdStub_ReturnsError)
{
    InputMethodSystemAbilityStub stub;
    MessageParcel data;
    MessageParcel reply;
    data.WriteRemoteObject(nullptr);

    int32_t result = stub.ConnectSystemCmdOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

/**
 *  * @tc.name: IsCurrentImeByPidOnRemote_ValidPid_ReturnsSuccess
 *  * @tc.desc: Verify that the IsCurrentImeByPidOnRemote method returns success when a valid PID is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, IsCurrentImeByPidOnRemote_ValidPid_ReturnsSuccess)
{
    InputMethodSystemAbilityStub stub;
    MessageParcel data;
    MessageParcel reply;
    int32_t pid = 1234;
    data.WriteInt32(pid);

    int32_t result = stub.IsCurrentImeByPidOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: IsCurrentImeByPidOnRemote_InvalidPid_ReturnsError
 *  * @tc.desc: Verify that the IsCurrentImeByPidOnRemote method returns error when an invalid PID is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, IsCurrentImeByPidOnRemote_InvalidPid_ReturnsError)
{
    InputMethodSystemAbilityStub stub;
    MessageParcel data;
    MessageParcel reply;
    int32_t pid = -1;
    data.WriteInt32(pid);

    int32_t result = stub.IsCurrentImeByPidOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

/**
 *  * @tc.name: InitConnectOnRemote_ValidCall_ReturnsSuccess
 *  * @tc.desc: Verify that the InitConnectOnRemote method returns success when called
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, InitConnectOnRemote_ValidCall_ReturnsSuccess)
{
    InputMethodSystemAbilityStub stub;
    MessageParcel data;
    MessageParcel reply;

    int32_t result = stub.InitConnectOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: SetCallingWindowOnRemote_ValidParameters_ReturnsSuccess
 *  * @tc.desc: Verify that the SetCallingWindowOnRemote method returns success when valid parameters are provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, SetCallingWindowOnRemote_ValidParameters_ReturnsSuccess)
{
    InputMethodSystemAbilityStub stub;
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> clientObject = new OHOS::IRemoteObject();
    uint32_t windowId = 1001;
    InputClientInfo clientInfo;
    data.WriteRemoteObject(clientObject);
    data.WriteUint32(windowId);
    data.WriteParcelable(&clientInfo);

    int32_t result = stub.SetCallingWindowOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 *  * @tc.name: SetCallingWindowOnRemote_NullClientObject_ReturnsError
 *  * @tc.desc: Verify that the SetCallingWindowOnRemote method returns error when a null client object is provided
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, SetCallingWindowOnRemote_NullClientObject_ReturnsError)
{
    InputMethodSystemAbilityStub stub;
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> clientObject = nullptr;
    uint32_t windowId = 1001;
    InputClientInfo clientInfo;
    data.WriteRemoteObject(clientObject);
    data.WriteUint32(windowId);
    data.WriteParcelable(&clientInfo);

    int32_t result = stub.SetCallingWindowOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

/**
 *  * @tc.name: GetInputStartInfoOnRemote_ValidCall_ReturnsSuccess
 *  * @tc.desc: Verify that the GetInputStartInfoOnRemote method returns success when called
 *  * @tc.type: FUNC
 */
TEST_F(InputMethodSystemAbilityStubTest, GetInputStartInfoOnRemote_ValidCall_ReturnsSuccess)
{
    InputMethodSystemAbilityStub stub;
    MessageParcel data;
    MessageParcel reply;

    int32_t result = stub.GetInputStartInfoOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}