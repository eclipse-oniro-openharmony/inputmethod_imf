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

#include "ime_event_monitor_manager_impl.h"
#include "input_client_stub.h"
#include "input_method_controller.h"
#include "message_option.h"
#include "message_parcel.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace testing;
using namespace OHOS::MiscServices;

class MockInputClientStub : public InputClientStub {
public:
    MOCK_METHOD4(
        OnRemoteRequest, int32_t(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));
    MOCK_METHOD2(OnInputReadyOnRemote, void(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(OnInputStopOnRemote, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(OnSwitchInputOnRemote, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(OnPanelStatusChangeOnRemote, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(DeactivateClientOnRemote, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(NotifyInputStartOnRemote, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(NotifyInputStopOnRemote, int32_t(MessageParcel &data, MessageParcel &reply));
};

class InputClientStubTest : public Test {
protected:
    MockInputClientStub stub;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    void SetUp() override
    {
        // 设置测试环境
    }

    void TearDown() override
    {
        // 清理测试环境
    }
};

TEST_F(InputClientStubTest, OnRemoteRequest_InvalidDescriptorToken_ReturnsError)
{
    // 设置
    data.WriteInterfaceToken("InvalidToken");

    // 操作
    int32_t result = stub.OnRemoteRequest(ON_INPUT_READY, data, reply, option);

    // 验证
    EXPECT_EQ(result, ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION);
}

TEST_F(InputClientStubTest, OnRemoteRequest_ValidDescriptorToken_ProcessesRequest)
{
    // 设置
    data.WriteInterfaceToken(InputClientStub::GetDescriptor());

    // 操作
    int32_t result = stub.OnRemoteRequest(ON_INPUT_READY, data, reply, option);

    // 验证
    EXPECT_EQ(result, NO_ERROR);
    EXPECT_CALL(stub, OnInputReadyOnRemote(Ref(data), Ref(reply)));
}

TEST_F(InputClientStubTest, OnRemoteRequest_DefaultCase_CallsBaseMethod)
{
    // 设置
    data.WriteInterfaceToken(InputClientStub::GetDescriptor());

    // 操作
    int32_t result = stub.OnRemoteRequest(9999, data, reply, option);

    // 验证
    EXPECT_CALL(stub, OnRemoteRequest(9999, Ref(data), Ref(reply), Ref(option)));
}