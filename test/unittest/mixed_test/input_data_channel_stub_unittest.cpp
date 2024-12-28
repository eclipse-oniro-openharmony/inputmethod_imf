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

#include "input_data_channel_stub.h"
#include "input_method_controller.h"
#include "itypes_util.h"
#include "message_option.h"
#include "message_parcel.h"
#include "mock_input_method_controller.h"
#include "mock_ipc_object_stub.h"
#include "mock_itypes_util.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace OHOS;
using namespace MiscServices;
using namespace testing;

namespace {
const uint32_t VALID_CODE = 1000;   // 假设这是有效的代码
const uint32_t INVALID_CODE = 2000; // 假设这是无效的代码
const std::u16string TEXT = u"test";
const int32_t LENGTH = 5;
const int32_t DIRECTION = 1;
const int32_t ACTION = 10;
const int32_t STATUS = 1;
const int32_t FUNCTION_KEY = 2;
const int32_t ENTER_KEY_TYPE = 3;
const int32_t INPUT_PATTERN = 4;
const int32_t INDEX = 6;
const int32_t START = 7;
const int32_t END = 8;
const int32_t SKIP = 9;
const uint32_t HEIGHT = 100;
const std::string TEXT_PREVIEW = "preview";
const std::string PRIVATE_COMMAND_KEY = "key";
const std::string PRIVATE_COMMAND_VALUE = "value";
const std::string ARRAY_BUFFER = "buffer";
} // namespace

class InputDataChannelStubTest : public Test {
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
        inputDataChannelStub = new InputDataChannelStub();
        mockInputMethodController = new MockInputMethodController();
        mockITypesUtil = new MockITypesUtil();
        mockIPCObjectStub = new MockIPCObjectStub();
    }

    void TearDown() override
    {
        // 在每个测试之后清理
        delete inputDataChannelStub;
        delete mockInputMethodController;
        delete mockITypesUtil;
        delete mockIPCObjectStub;
    }

    InputDataChannelStub *inputDataChannelStub;
    MockInputMethodController *mockInputMethodController;
    MockITypesUtil *mockITypesUtil;
    MockIPCObjectStub *mockIPCObjectStub;
};

TEST_F(InputDataChannelStubTest, OnRemoteRequest_ValidDescriptorAndCode_ShouldInvokeHandler)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    // 模拟描述符匹配
    EXPECT_CALL(*mockITypesUtil, Unmarshal(data, IInputDataChannel::GetDescriptor())).WillOnce(Return(true));

    // 模拟代码在允许范围内
    EXPECT_CALL(*mockITypesUtil, Unmarshal(data, VALID_CODE)).WillOnce(Return(true));

    // 模拟处理函数
    EXPECT_CALL(*mockInputMethodController, InsertText(TEXT)).WillOnce(Return(ErrorCode::NO_ERROR));

    // 模拟数据序列化
    EXPECT_CALL(*mockITypesUtil, Marshal(reply, ErrorCode::NO_ERROR, TEXT)).WillOnce(Return(true));

    // 调用被测试的方法
    int32_t result = inputDataChannelStub->OnRemoteRequest(VALID_CODE, data, reply, option);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(InputDataChannelStubTest, OnRemoteRequest_InvalidDescriptor_ShouldReturnError)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    // 模拟描述符不匹配
    EXPECT_CALL(*mockITypesUtil, Unmarshal(data, IInputDataChannel::GetDescriptor())).WillOnce(Return(false));

    // 调用被测试的方法
    int32_t result = inputDataChannelStub->OnRemoteRequest(VALID_CODE, data, reply, option);

    EXPECT_EQ(result, ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION);
}

TEST_F(InputDataChannelStubTest, OnRemoteRequest_InvalidCode_ShouldReturnError)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    // 模拟描述符匹配
    EXPECT_CALL(*mockITypesUtil, Unmarshal(data, IInputDataChannel::GetDescriptor())).WillOnce(Return(true));

    // 模拟代码不在允许范围内
    EXPECT_CALL(*mockITypesUtil, Unmarshal(data, INVALID_CODE)).WillOnce(Return(true));

    // 调用被测试的方法
    int32_t result = inputDataChannelStub->OnRemoteRequest(INVALID_CODE, data, reply, option);

    EXPECT_EQ(result, ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION);
}