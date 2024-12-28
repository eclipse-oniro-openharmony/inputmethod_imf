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
#include "iremote_proxy.h"
#include "isystem_cmd_channel.h"
#include "itypes_util.h"
#include "message_option.h"
#include "message_parcel.h"
#include "private_data_value.h"
#include "sys_panel_status.h"
#include "system_cmd_channel_proxy.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace OHOS;
using namespace MiscServices;
using namespace testing;

class MockRemoteObject : public IRemoteObject {
public:
    MOCK_METHOD4(SendRequest, int(int, MessageParcel &, MessageParcel &, MessageOption &));
};

class MockITypesUtil {
public:
    static bool Marshal(MessageParcel &parcel, const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
    {
        return true;
    }
    static bool Marshal(MessageParcel &parcel, const SysPanelStatus &sysPanelStatus)
    {
        return true;
    }
};

class SystemCmdChannelProxyTest : public Test {
protected:
    sptr<MockRemoteObject> remoteObject;
    SystemCmdChannelProxy proxy;

    void SetUp() override
    {
        remoteObject = new MockRemoteObject();
        proxy = SystemCmdChannelProxy(remoteObject);
    }
};

TEST_F(SystemCmdChannelProxyTest, SendPrivateCommand_RemoteIsNull_ReturnsError)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::ERROR_EX_NULL_POINTER));
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    int32_t result = proxy.SendPrivateCommand(privateCommand);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(SystemCmdChannelProxyTest, SendPrivateCommand_SendRequestFails_ReturnsError)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT));
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    int32_t result = proxy.SendPrivateCommand(privateCommand);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT);
}

TEST_F(SystemCmdChannelProxyTest, SendPrivateCommand_Success_ReturnsNoError)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    int32_t result = proxy.SendPrivateCommand(privateCommand);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

TEST_F(SystemCmdChannelProxyTest, NotifyPanelStatus_RemoteIsNull_ReturnsError)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::ERROR_EX_NULL_POINTER));
    SysPanelStatus sysPanelStatus;
    int32_t result = proxy.NotifyPanelStatus(sysPanelStatus);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

TEST_F(SystemCmdChannelProxyTest, NotifyPanelStatus_SendRequestFails_ReturnsError)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT));
    SysPanelStatus sysPanelStatus;
    int32_t result = proxy.NotifyPanelStatus(sysPanelStatus);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT);
}

TEST_F(SystemCmdChannelProxyTest, NotifyPanelStatus_Success_ReturnsNoError)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::NO_ERROR));
    SysPanelStatus sysPanelStatus;
    int32_t result = proxy.NotifyPanelStatus(sysPanelStatus);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}