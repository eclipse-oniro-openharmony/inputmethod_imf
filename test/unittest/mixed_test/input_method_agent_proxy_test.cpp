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
#include "input_method_agent_proxy.h"
#include "iinput_method_agent.h"
#include "iremote_object.h"
#include "message_parcel.h"
#include "message_option.h"
#include "types.h"
#include "mmi/key_event.h"
#include "ikey_event_consumer.h"

using namespace testing;
using namespace OHOS::MiscServices;

class MockRemoteObject : public IRemoteObject {
public:
    MockRemoteObject() : IRemoteObject(nullptr) {}
    MOCK_METHOD4(SendRequest, int32_t(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option));
};

class MockKeyEventConsumer : public IKeyEventConsumer {
public:
    MOCK_METHOD1(OnKeyEvent, int32_t(const std::shared_ptr<MMI::KeyEvent> &keyEvent));
};

class InputMethodAgentProxyTest : public Test {
protected:
    void SetUp() override
    {
        remoteObject = new MockRemoteObject();
        proxy = new InputMethodAgentProxy(remoteObject);
    }

    void TearDown() override
    {
        delete proxy;
        proxy = nullptr;
        delete remoteObject;
        remoteObject = nullptr;
    }

    RemoteObject *remoteObject;
    InputMethodAgentProxy *proxy;

    MockRemoteObject *remoteObject;
    InputMethodAgentProxy *proxy;
};

HWTEST_F(InputMethodAgentProxyTest, DispatchKeyEvent_ConsumerIsNull_ReturnsError) {
    std::shared_ptr<MMI::KeyEvent> keyEvent = std::make_shared<MMI::KeyEvent>();
    sptr<IKeyEventConsumer> consumer = nullptr;
    EXPECT_EQ(proxy->DispatchKeyEvent(keyEvent, consumer), ErrorCode::ERROR_EX_NULL_POINTER);
}

HWTEST_F(InputMethodAgentProxyTest, DispatchKeyEvent_ValidConsumer_ReturnsSuccess) {
    std::shared_ptr<MMI::KeyEvent> keyEvent = std::make_shared<MMI::KeyEvent>();
    sptr<IKeyEventConsumer> consumer = new MockKeyEventConsumer();
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));
    EXPECT_EQ(proxy->DispatchKeyEvent(keyEvent, consumer), ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAgentProxyTest, OnCursorUpdate_SendsRequest) {
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));
    proxy->OnCursorUpdate(10, 20, 30);
}

HWTEST_F(InputMethodAgentProxyTest, OnSelectionChange_SendsRequest) {
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));
    proxy->OnSelectionChange(u"test", 0, 1, 2, 3);
}

HWTEST_F(InputMethodAgentProxyTest, SetCallingWindow_SendsRequest) {
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));
    proxy->SetCallingWindow(123);
}

HWTEST_F(InputMethodAgentProxyTest, OnAttributeChange_SendsRequest) {
    InputAttribute attribute;
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));
    proxy->OnAttributeChange(attribute);
}

HWTEST_F(InputMethodAgentProxyTest, SendMessage_SendsRequest) {
    ArrayBuffer arraybuffer;
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));
    EXPECT_EQ(proxy->SendMessage(arraybuffer), ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAgentProxyTest, SendPrivateCommand_SendsRequest) {
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));
    EXPECT_EQ(proxy->SendPrivateCommand(privateCommand), ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAgentProxyTest, SendRequest_RemoteIsNull_ReturnsError) {
    InputMethodAgentProxy nullProxy(nullptr);
    EXPECT_EQ(nullProxy.SendRequest(1, nullptr, nullptr), ErrorCode::ERROR_EX_NULL_POINTER);
}