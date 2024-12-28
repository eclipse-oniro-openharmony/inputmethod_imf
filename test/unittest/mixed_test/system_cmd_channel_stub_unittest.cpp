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

#include "ime_system_channel.h"
#include "ipc_object_stub.h"
#include "ipc_skeleton.h"
#include "itypes_util.h"
#include "message_option.h"
#include "message_parcel.h"
#include "private_data_value.h"
#include "sys_panel_status.h"
#include "system_cmd_channel_stub.h"
#include <gtest/gtest.h>

using namespace OHOS;
using namespace MiscServices;

class SystemCmdChannelStubTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        // 设置测试环境
    }

    static void TearDownTestCase()
    {
        // 清理测试环境
    }

    void SetUp() override
    {
        // 在每个测试之前设置
        stub_ = std::make_shared<SystemCmdChannelStub>();
        mockImeSystemCmdChannel_ = std::make_shared<MockImeSystemCmdChannel>();
        mockITypesUtil_ = std::make_shared<MockITypesUtil>();
        mockIPCObjectStub_ = std::make_shared<MockIPCObjectStub>();
        mockMessageParcel_ = std::make_shared<MockMessageParcel>();
        mockMessageOption_ = std::make_shared<MockMessageOption>();
    }

    void TearDown() override
    {
        // 在每个测试之后清理
        stub_.reset();
        mockImeSystemCmdChannel_.reset();
        mockITypesUtil_.reset();
        mockIPCObjectStub_.reset();
        mockMessageParcel_.reset();
        mockMessageOption_.reset();
    }

    std::shared_ptr<SystemCmdChannelStub> stub_;
    std::shared_ptr<MockImeSystemCmdChannel> mockImeSystemCmdChannel_;
    std::shared_ptr<MockITypesUtil> mockITypesUtil_;
    std::shared_ptr<MockIPCObjectStub> mockIPCObjectStub_;
    std::shared_ptr<MockMessageParcel> mockMessageParcel_;
    std::shared_ptr<MockMessageOption> mockMessageOption_;
};

TEST_F(SystemCmdChannelStubTest, SendPrivateCommandOnRemote_Success)
{
    // 设置
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    privateCommand["key"] = PrivateDataValue("value");
    EXPECT_CALL(*mockITypesUtil_, Unmarshal(testing::Ref(*mockMessageParcel_), testing::Ref(privateCommand)))
        .WillOnce(testing::Return(true));
    EXPECT_CALL(*mockImeSystemCmdChannel_, ReceivePrivateCommand(testing::Ref(privateCommand)))
        .WillOnce(testing::Return(0));
    EXPECT_CALL(*mockMessageParcel_, WriteInt32(0)).WillOnce(testing::Return(true));

    // 操作
    int32_t result = stub_->SendPrivateCommandOnRemote(*mockMessageParcel_, *mockMessageParcel_);

    // 验证
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(SystemCmdChannelStubTest, SendPrivateCommandOnRemote_UnmarshalFailure)
{
    // 设置
    EXPECT_CALL(*mockITypesUtil_, Unmarshal(testing::Ref(*mockMessageParcel_), testing::_))
        .WillOnce(testing::Return(false));

    // 操作
    int32_t result = stub_->SendPrivateCommandOnRemote(*mockMessageParcel_, *mockMessageParcel_);

    // 验证
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

TEST_F(SystemCmdChannelStubTest, NotifyPanelStatusOnRemote_Success)
{
    // 设置
    SysPanelStatus sysPanelStatus;
    EXPECT_CALL(*mockITypesUtil_, Unmarshal(testing::Ref(*mockMessageParcel_), testing::Ref(sysPanelStatus)))
        .WillOnce(testing::Return(true));
    EXPECT_CALL(*mockImeSystemCmdChannel_, NotifyPanelStatus(testing::Ref(sysPanelStatus)))
        .WillOnce(testing::Return(0));
    EXPECT_CALL(*mockMessageParcel_, WriteInt32(0)).WillOnce(testing::Return(true));

    // 操作
    int32_t result = stub_->NotifyPanelStatusOnRemote(*mockMessageParcel_, *mockMessageParcel_);

    // 验证
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(SystemCmdChannelStubTest, NotifyPanelStatusOnRemote_UnmarshalFailure)
{
    // 设置
    EXPECT_CALL(*mockITypesUtil_, Unmarshal(testing::Ref(*mockMessageParcel_), testing::_))
        .WillOnce(testing::Return(false));

    // 操作
    int32_t result = stub_->NotifyPanelStatusOnRemote(*mockMessageParcel_, *mockMessageParcel_);

    // 验证
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

TEST_F(SystemCmdChannelStubTest, OnRemoteRequest_DescriptorMismatch)
{
    // 设置
    EXPECT_CALL(*mockMessageParcel_, ReadInterfaceToken()).WillOnce(testing::Return("wrong_descriptor"));

    // 操作
    int32_t result = stub_->OnRemoteRequest(1, *mockMessageParcel_, *mockMessageParcel_, *mockMessageOption_);

    // 验证
    EXPECT_EQ(result, ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION);
}

TEST_F(SystemCmdChannelStubTest, OnRemoteRequest_CodeOutOfRange)
{
    // 设置
    EXPECT_CALL(*mockMessageParcel_, ReadInterfaceToken())
        .WillOnce(testing::Return(ISystemCmdChannel::GetDescriptor()));
    EXPECT_CALL(*mockIPCObjectStub_,
        OnRemoteRequest(100, testing::Ref(*mockMessageParcel_), testing::Ref(*mockMessageParcel_),
            testing::Ref(*mockMessageOption_)))
        .WillOnce(testing::Return(0));

    // 操作
    int32_t result = stub_->OnRemoteRequest(100, *mockMessageParcel_, *mockMessageParcel_, *mockMessageOption_);

    // 验证
    EXPECT_EQ(result, 0);
}

TEST_F(SystemCmdChannelStubTest, OnRemoteRequest_CodeInRange)
{
    // 设置
    EXPECT_CALL(*mockMessageParcel_, ReadInterfaceToken())
        .WillOnce(testing::Return(ISystemCmdChannel::GetDescriptor()));
    EXPECT_CALL(*mockMessageParcel_, WriteInt32(0)).WillOnce(testing::Return(true));

    // 操作
    int32_t result = stub_->OnRemoteRequest(1, *mockMessageParcel_, *mockMessageParcel_, *mockMessageOption_);

    // 验证
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}