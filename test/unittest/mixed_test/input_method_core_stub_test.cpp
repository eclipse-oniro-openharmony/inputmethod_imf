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

#include "input_method_core_stub.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "i_input_method_core.h"
#include "input_method_ability.h"
#include "ipc_object_stub.h"
#include "itypes_util.h"
#include "message_option.h"
#include "message_parcel.h"
#include "task_manager.h"

using namespace OHOS;
using namespace MiscServices;

class InputMethodCoreStubTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        // 设置测试案例
    }

    static void TearDownTestCase()
    {
        // 清理测试案例
    }

    void SetUp() override
    {
        // 设置测试
        stub_ = std::make_shared<InputMethodCoreStub>();
    }

    void TearDown() override
    {
        // 清理测试
        stub_ = nullptr;
    }

    std::shared_ptr<InputMethodCoreStub> stub_;
};

HWTEST_F(InputMethodCoreStubTest, onRemoteRequest_001, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken("InvalidToken");
    int32_t result = stub_->OnRemoteRequest(1, data, reply, option);
    EXPECT_EQ(result, ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION);
}

HWTEST_F(InputMethodCoreStubTest, onRemoteRequest_002, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IInputMethodCore::GetDescriptor());
    int32_t result = stub_->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(result, ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION);
}

HWTEST_F(InputMethodCoreStubTest, onRemoteRequest_003, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IInputMethodCore::GetDescriptor());
    int32_t result = stub_->OnRemoteRequest(1, data, reply, option);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodCoreStubTest, onRemoteRequest_004, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteRemoteObject(nullptr);
    int32_t result = stub_->InitInputControlChannelOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

HWTEST_F(InputMethodCoreStubTest, onRemoteRequest_005, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> channelObject = new IPCObjectStub();
    data.WriteRemoteObject(channelObject);
    int32_t result = stub_->InitInputControlChannelOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_NULL_POINTER);
}

HWTEST_F(InputMethodCoreStubTest, onRemoteRequest_006, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> channelObject = new IPCObjectStub();
    data.WriteRemoteObject(channelObject);
    int32_t result = stub_->InitInputControlChannelOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodCoreStubTest, onRemoteRequest_007, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟反序列化失败
    int32_t result = stub_->StartInputOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

HWTEST_F(InputMethodCoreStubTest, onRemoteRequest_008, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟成功反序列化
    bool isBindFromClient = false;
    InputClientInfo clientInfo = {};
    ITypesUtil::Marshal(data, isBindFromClient, clientInfo);
    int32_t result = stub_->StartInputOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodCoreStubTest, onRemoteRequest_009, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟反序列化失败
    int32_t result = stub_->SecurityChangeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

HWTEST_F(InputMethodCoreStubTest, securityChangeOnRemote_001, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟成功反序列化
    int32_t security = 1;
    ITypesUtil::Marshal(data, security);
    int32_t result = stub_->SecurityChangeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodCoreStubTest, securityChangeOnRemote_002, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟反序列化失败
    int32_t result = stub_->OnConnectSystemCmdOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

HWTEST_F(InputMethodCoreStubTest, securityChangeOnRemote_003, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟成功反序列化
    sptr<IRemoteObject> channelObject = new IPCObjectStub();
    ITypesUtil::Marshal(data, channelObject);
    int32_t result = stub_->OnConnectSystemCmdOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodCoreStubTest, securityChangeOnRemote_004, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟反序列化失败
    int32_t result = stub_->SetSubtypeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

HWTEST_F(InputMethodCoreStubTest, securityChangeOnRemote_005, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟成功反序列化
    SubProperty property = {};
    ITypesUtil::Marshal(data, property);
    int32_t result = stub_->SetSubtypeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodCoreStubTest, securityChangeOnRemote_006, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟反序列化失败
    int32_t result = stub_->OnSetInputTypeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

HWTEST_F(InputMethodCoreStubTest, securityChangeOnRemote_007, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟成功反序列化
    InputType inputType = {};
    ITypesUtil::Marshal(data, inputType);
    int32_t result = stub_->OnSetInputTypeOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodCoreStubTest, securityChangeOnRemote_008, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟反序列化失败
    int32_t result = stub_->StopInputOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

HWTEST_F(InputMethodCoreStubTest, securityChangeOnRemote_009, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟成功反序列化
    sptr<IRemoteObject> channel = new IPCObjectStub();
    ITypesUtil::Marshal(data, channel);
    int32_t result = stub_->StopInputOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodCoreStubTest, isEnableOnRemote_001, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t result = stub_->IsEnableOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodCoreStubTest, isEnableOnRemote_002, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t result = stub_->ShowKeyboardOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodCoreStubTest, isEnableOnRemote_003, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t result = stub_->HideKeyboardOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodCoreStubTest, isEnableOnRemote_004, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟反序列化失败
    int32_t result = stub_->StopInputServiceOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

HWTEST_F(InputMethodCoreStubTest, isEnableOnRemote_005, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟成功反序列化
    bool isTerminateIme = false;
    ITypesUtil::Marshal(data, isTerminateIme);
    int32_t result = stub_->StopInputServiceOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodCoreStubTest, isEnableOnRemote_006, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟反序列化失败
    int32_t result = stub_->IsPanelShownOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

HWTEST_F(InputMethodCoreStubTest, isEnableOnRemote_007, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟成功反序列化
    PanelInfo info = {};
    ITypesUtil::Marshal(data, info);
    int32_t result = stub_->IsPanelShownOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodCoreStubTest, isEnableOnRemote_008, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟反序列化失败
    int32_t result = stub_->OnClientInactiveOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

HWTEST_F(InputMethodCoreStubTest, isEnableOnRemote_009, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    // 模拟成功反序列化
    sptr<IRemoteObject> channel = new IPCObjectStub();
    ITypesUtil::Marshal(data, channel);
    int32_t result = stub_->OnClientInactiveOnRemote(data, reply);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}