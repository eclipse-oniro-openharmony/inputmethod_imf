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

#include "iremote_object.h"
#include "keyevent_consumer_proxy.h"
#include "message_option.h"
#include "message_parcel.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace OHOS;
using namespace OHOS::MiscServices;
using namespace testing;

class MockRemoteObject : public IRemoteObject {
public:
    MOCK_METHOD4(SendRequest, int(int, MessageParcel &, MessageParcel &, MessageOption &));
};

class MockMessageParcel : public MessageParcel {
public:
    MOCK_METHOD1(WriteInterfaceToken, bool(const std::u16string &));
    MOCK_METHOD1(ReadInt32, int32_t(int32_t *));
};

class MockMessageOption : public MessageOption {
public:
    MOCK_METHOD0(GetFlags, int());
};

class KeyEventConsumerProxyTest : public Test {
protected:
    void SetUp() override
    {
        remoteObject = new MockRemoteObject();
        proxy = new KeyEventConsumerProxy(remoteObject);
    }

    void TearDown() override
    {
        delete proxy;
        delete remoteObject;
    }

    KeyEventConsumerProxy *proxy;
    MockRemoteObject *remoteObject;
};

TEST_F(KeyEventConsumerProxyTest, OnKeyEventResult_SendRequestCalled)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).Times(1);
    proxy->OnKeyEventResult(true);
}

TEST_F(KeyEventConsumerProxyTest, OnKeyEventConsumeResult_UpdatesAndCallsOnKeyEventResult)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).Times(1);
    proxy->OnKeyEventConsumeResult(true);
    proxy->OnKeyCodeConsumeResult(true);
}

TEST_F(KeyEventConsumerProxyTest, OnKeyCodeConsumeResult_UpdatesAndCallsOnKeyEventResult)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).Times(1);
    proxy->OnKeyCodeConsumeResult(true);
    proxy->OnKeyEventConsumeResult(true);
}

TEST_F(KeyEventConsumerProxyTest, SendRequest_InterfaceTokenWriteFailure)
{
    MockMessageParcel data;
    MockMessageOption option;
    EXPECT_CALL(data, WriteInterfaceToken(_)).WillOnce(Return(false));
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).Times(0);
    int result = proxy->SendRequest(1, nullptr, nullptr, option);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT);
}

TEST_F(KeyEventConsumerProxyTest, SendRequest_InputDataHandlingFailure)
{
    MockMessageParcel data;
    MockMessageOption option;
    EXPECT_CALL(data, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(data, WriteInt32(_)).WillOnce(Return(false));
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).Times(0);
    int result = proxy->SendRequest(
        1,
        [](MessageParcel &parcel) {
            return parcel.WriteInt32(1);
        },
        nullptr, option);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

TEST_F(KeyEventConsumerProxyTest, SendRequest_RemoteObjectIsNull)
{
    MockMessageParcel data;
    MockMessageOption option;
    EXPECT_CALL(data, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::ERROR_EX_NULL_POINTER));
    int result = proxy->SendRequest(1, nullptr, nullptr, option);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(KeyEventConsumerProxyTest, SendRequest_SendRequestFailure)
{
    MockMessageParcel data;
    MockMessageParcel reply;
    MockMessageOption option;
    EXPECT_CALL(data, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(1));
    int result = proxy->SendRequest(1, nullptr, nullptr, option);
    EXPECT_EQ(result, 1);
}

TEST_F(KeyEventConsumerProxyTest, SendRequest_AsyncMessageOption)
{
    MockMessageParcel data;
    MockMessageParcel reply;
    MockMessageOption option;
    EXPECT_CALL(data, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(option, GetFlags()).WillOnce(Return(MessageOption::TF_ASYNC));
    int result = proxy->SendRequest(1, nullptr, nullptr, option);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(KeyEventConsumerProxyTest, SendRequest_SyncMessageOption)
{
    MockMessageParcel data;
    MockMessageParcel reply;
    MockMessageOption option;
    EXPECT_CALL(data, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(option, GetFlags()).WillOnce(Return(MessageOption::TF_SYNC));
    EXPECT_CALL(reply, ReadInt32(_)).WillOnce(Return(1));
    int result = proxy->SendRequest(1, nullptr, nullptr, option);
    EXPECT_EQ(result, 1);
}

TEST_F(KeyEventConsumerProxyTest, SendRequest_OutputDataHandlingFailure)
{
    MockMessageParcel data;
    MockMessageParcel reply;
    MockMessageOption option;
    EXPECT_CALL(data, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(option, GetFlags()).WillOnce(Return(MessageOption::TF_SYNC));
    EXPECT_CALL(reply, ReadInt32(_)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(reply, ReadInt32(_)).WillOnce(Return(false));
    int result = proxy->SendRequest(
        1, nullptr,
        [](MessageParcel &parcel) {
            return parcel.ReadInt32(nullptr);
        },
        option);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}