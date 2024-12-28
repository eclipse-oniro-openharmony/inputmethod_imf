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

#include "keyevent_consumer_stub.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "itypes_util.h"
#include "message_option.h"
#include "message_parcel.h"
#include "mmi/key_event.h"
#include "mmi/key_event_consumer.h"

using namespace OHOS;
using namespace OHOS::MiscServices;
using namespace OHOS::MMI;
using namespace testing;

namespace {
const int32_t KEY_EVENT_CONSUMER_BEGIN = 1000;
const int32_t KEY_EVENT_CONSUMER_CMD_END = 2000;
const int32_t VALID_CODE = 1500;
const int32_t INVALID_CODE = 3000;
const int32_t ERROR_STATUS_UNKNOWN_TRANSACTION = -1;
const int32_t ERROR_EX_PARCELABLE = -2;
const int32_t NO_ERROR = 0;
} // namespace

class MockMessageParcel : public MessageParcel {
public:
    MOCK_METHOD1(ReadInterfaceToken, bool(const std::u16string &));
    MOCK_METHOD1(WriteInt32, bool(int32_t));
};

class MockMessageOption : public MessageOption {
public:
    MOCK_METHOD0(GetFlags, uint32_t());
};

class MockITypesUtil {
public:
    static bool Unmarshal(MessageParcel &data, bool &isConsumed)
    {
        return true;
    }
};

class MockKeyEventConsumerStub : public KeyEventConsumerStub {
public:
    MockKeyEventConsumerStub(KeyEventCallback callback, std::shared_ptr<KeyEvent> keyEvent)
        : KeyEventConsumerStub(callback, keyEvent)
    {
    }

    MOCK_METHOD4(OnRemoteRequest, int32_t(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
    MOCK_METHOD2(OnKeyEventResultOnRemote, int32_t(MessageParcel &, MessageParcel &));
    MOCK_METHOD1(OnKeyEventResult, int32_t(bool));
};

class KeyEventConsumerStubTest : public Test {
protected:
    void SetUp() override
    {
        keyEvent_ = std::make_shared<KeyEvent>();
        callback_ = [this](std::shared_ptr<KeyEvent> event, bool isConsumed) { callbackCalled_ = true; };
        stub_ = std::make_shared<MockKeyEventConsumerStub>(callback_, keyEvent_);
    }

    void TearDown() override
    {
        stub_.reset();
    }

    std::shared_ptr<KeyEvent> keyEvent_;
    KeyEventCallback callback_;
    std::shared_ptr<MockKeyEventConsumerStub> stub_;
    bool callbackCalled_ = false;
};

HWTEST_F(KeyEventConsumerStubTest, onRemoteRequest_001, TestSize.Level0)
{
    MockMessageParcel data;
    MockMessageParcel reply;
    MockMessageOption option;

    EXPECT_CALL(data, ReadInterfaceToken(_)).WillOnce(Return(false));

    int32_t result = stub_->OnRemoteRequest(VALID_CODE, data, reply, option);
    EXPECT_EQ(result, ERROR_STATUS_UNKNOWN_TRANSACTION);
}

HWTEST_F(KeyEventConsumerStubTest, onRemoteRequest_002, TestSize.Level0)
{
    MockMessageParcel data;
    MockMessageParcel reply;
    MockMessageOption option;

    EXPECT_CALL(data, ReadInterfaceToken(_)).WillOnce(Return(true));

    EXPECT_CALL(*stub_, OnRemoteRequest(VALID_CODE, Ref(data), Ref(reply), Ref(option)))
        .WillOnce(Invoke([this](uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }));

    int32_t result = stub_->OnRemoteRequest(INVALID_CODE, data, reply, option);
    EXPECT_EQ(result, IPCObjectStub::OnRemoteRequest(INVALID_CODE, data, reply, option));
}

HWTEST_F(KeyEventConsumerStubTest, onRemoteRequest_003, TestSize.Level0)
{
    MockMessageParcel data;
    MockMessageParcel reply;
    MockMessageOption option;

    EXPECT_CALL(data, ReadInterfaceToken(_)).WillOnce(Return(true));

    EXPECT_CALL(*stub_, OnKeyEventResultOnRemote(Ref(data), Ref(reply))).WillOnce(Return(NO_ERROR));

    int32_t result = stub_->OnRemoteRequest(VALID_CODE, data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(KeyEventConsumerStubTest, onRemoteRequest_004, TestSize.Level0)
{
    MockMessageParcel data;
    MockMessageParcel reply;

    EXPECT_CALL(data, ReadInterfaceToken(_)).WillOnce(Return(true));

    EXPECT_CALL(data, WriteInt32(_)).WillOnce(Return(false));

    int32_t result = stub_->OnKeyEventResultOnRemote(data, reply);
    EXPECT_EQ(result, ERROR_EX_PARCELABLE);
}

HWTEST_F(KeyEventConsumerStubTest, onRemoteRequest_005, TestSize.Level0)
{
    MockMessageParcel data;
    MockMessageParcel reply;

    EXPECT_CALL(data, ReadInterfaceToken(_)).WillOnce(Return(true));

    EXPECT_CALL(data, WriteInt32(_)).WillOnce(Return(true));

    int32_t result = stub_->OnKeyEventResultOnRemote(data, reply);
    EXPECT_EQ(result, NO_ERROR);
    EXPECT_TRUE(callbackCalled_);
}