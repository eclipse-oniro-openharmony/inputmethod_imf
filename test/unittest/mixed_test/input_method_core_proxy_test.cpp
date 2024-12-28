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
#include <gmock/gmock.h>
#include "input_method_core_proxy.h"
#include "iinput_control_channel.h"
#include "iinput_method_core.h"
#include "message_parcel.h"
#include "message_option.h"
#include "input_client_info.h"
#include "input_type.h"
#include "sub_property.h"
#include "panel_info.h"

using namespace testing;
using namespace OHOS;
using namespace OHOS::MiscServices;

class MockRemoteObject : public IRemoteObject {
public:
    MOCK_METHOD4(SendRequest, int32_t(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
};

class MockITypesUtil {
public:
    MOCK_METHOD2(Marshal, bool(MessageParcel &, const sptr<IInputControlChannel> &));
    MOCK_METHOD2(Marshal, bool(MessageParcel &, bool, const InputClientInfo &, const sptr<IInputControlChannel> &));
    MOCK_METHOD2(Marshal, bool(MessageParcel &, int32_t));
    MOCK_METHOD2(Marshal, bool(MessageParcel &, InputType));
    MOCK_METHOD2(Marshal, bool(MessageParcel &, const sptr<IRemoteObject> &));
    MOCK_METHOD2(Marshal, bool(MessageParcel &, const SubProperty &));
    MOCK_METHOD2(Marshal, bool(MessageParcel &, const PanelInfo &));
    MOCK_METHOD2(Unmarshal, bool(MessageParcel &, bool &));
    MOCK_METHOD2(Unmarshal, bool(MessageParcel &, sptr<IRemoteObject> &));
};

class InputMethodCoreProxyTest : public Test {
protected:
    void SetUp() override
    {
        remoteObject = new MockRemoteObject();
        proxy = new InputMethodCoreProxy(remoteObject);
    }

    void TearDown() override
    {
        delete proxy;
        proxy = nullptr;
        delete remoteObject;
        remoteObject = nullptr;
    }

    RemoteObject *remoteObject;
    InputMethodCoreProxy *proxy;

    MockRemoteObject *remoteObject;
    InputMethodCoreProxy *proxy;
};

HWTEST_F(InputMethodCoreProxyTest, InitInputControlChannel_ValidChannel_ReturnsSuccess) {
    sptr<IInputControlChannel> inputControlChannel = new IInputControlChannel();
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockITypesUtil(), Marshal(_, inputControlChannel)).WillOnce(Return(true));

    int32_t result = proxy->InitInputControlChannel(inputControlChannel);
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputMethodCoreProxyTest, InitInputControlChannel_NullChannel_ReturnsError) {
    int32_t result = proxy->InitInputControlChannel(nullptr);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

HWTEST_F(InputMethodCoreProxyTest, StartInput_ValidClientInfo_ReturnsSuccess) {
    InputClientInfo clientInfo;
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockITypesUtil(), Marshal(_, true, clientInfo, clientInfo.channel)).WillOnce(Return(true));

    int32_t result = proxy->StartInput(clientInfo, true);
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputMethodCoreProxyTest, OnSecurityChange_ValidSecurity_ReturnsSuccess) {
    int32_t security = 1;
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockITypesUtil(), Marshal(_, security)).WillOnce(Return(true));

    int32_t result = proxy->OnSecurityChange(security);
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputMethodCoreProxyTest, OnSetInputType_ValidInputType_ReturnsSuccess) {
    InputType inputType;
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockITypesUtil(), Marshal(_, inputType)).WillOnce(Return(true));

    int32_t result = proxy->OnSetInputType(inputType);
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputMethodCoreProxyTest, OnConnectSystemCmd_ValidChannel_ReturnsSuccess) {
    sptr<IRemoteObject> channel = new IRemoteObject();
    sptr<IRemoteObject> agent;
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockITypesUtil(), Marshal(_, channel)).WillOnce(Return(true));
    EXPECT_CALL(MockITypesUtil(), Unmarshal(_, agent)).WillOnce(Return(true));

    int32_t result = proxy->OnConnectSystemCmd(channel, agent);
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputMethodCoreProxyTest, StopInputService_TerminateIme_ReturnsSuccess) {
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockITypesUtil(), Marshal(_, true)).WillOnce(Return(true));

    int32_t result = proxy->StopInputService(true);
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputMethodCoreProxyTest, ShowKeyboard_ReturnsSuccess) {
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));

    int32_t result = proxy->ShowKeyboard();
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputMethodCoreProxyTest, HideKeyboard_ReturnsSuccess) {
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));

    int32_t result = proxy->HideKeyboard();
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputMethodCoreProxyTest, SetSubtype_ValidProperty_ReturnsSuccess) {
    SubProperty property;
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockITypesUtil(), Marshal(_, property)).WillOnce(Return(true));

    int32_t result = proxy->SetSubtype(property);
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputMethodCoreProxyTest, StopInput_ValidChannel_ReturnsSuccess) {
    sptr<IRemoteObject> channel = new IRemoteObject();
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockITypesUtil(), Marshal(_, channel)).WillOnce(Return(true));

    int32_t result = proxy->StopInput(channel);
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputMethodCoreProxyTest, IsEnable_ReturnsTrue) {
    bool isEnable = true;
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockITypesUtil(), Unmarshal(_, isEnable)).WillOnce(Return(true));

    bool result = proxy->IsEnable();
    EXPECT_TRUE(result);
}

HWTEST_F(InputMethodCoreProxyTest, IsPanelShown_ValidPanelInfo_ReturnsSuccess) {
    PanelInfo panelInfo;
    bool isShown = false;
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockITypesUtil(), Marshal(_, panelInfo)).WillOnce(Return(true));
    EXPECT_CALL(MockITypesUtil(), Unmarshal(_, isShown)).WillOnce(Return(true));

    int32_t result = proxy->IsPanelShown(panelInfo, isShown);
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputMethodCoreProxyTest, OnClientInactive_ValidChannel_ReturnsSuccess) {
    sptr<IRemoteObject> channel = new IRemoteObject();
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockITypesUtil(), Marshal(_, channel)).WillOnce(Return(true));

    proxy->OnClientInactive(channel);
}