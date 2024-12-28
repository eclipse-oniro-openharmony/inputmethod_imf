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

#include "input_data_channel_proxy.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_input_data_channel.h"
#include "mock_message_option.h"
#include "mock_message_parcel.h"
#include "mock_remote_object.h"
#include "mock_types_util.h"

using namespace testing;
using namespace OHOS::MiscServices;

class InputDataChannelProxyTest : public Test {
public:
    static void SetUpTestCase()
    {
    }
    static void TearDownTestCase()
    {
    }
    void SetUp() override
    {
        remoteObject = new MockRemoteObject();
        inputDataChannelProxy = new InputDataChannelProxy(remoteObject);
    }
    void TearDown() override
    {
        delete inputDataChannelProxy;
        delete remoteObject;
    }

    MockRemoteObject *remoteObject;
    InputDataChannelProxy *inputDataChannelProxy;
};

HWTEST_F(InputDataChannelProxyTest, testGetEnableData_001, TestSize.Level0)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockTypesUtil::Instance(), Marshal(_, _)).WillOnce(Return(true));

    int32_t result = inputDataChannelProxy->InsertText(u"test");
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputDataChannelProxyTest, testGetEnableData_002, TestSize.Level0)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockTypesUtil::Instance(), Marshal(_, _)).WillOnce(Return(true));

    int32_t result = inputDataChannelProxy->DeleteForward(5);
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputDataChannelProxyTest, testGetEnableData_003, TestSize.Level0)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockTypesUtil::Instance(), Marshal(_, _)).WillOnce(Return(true));
    EXPECT_CALL(MockTypesUtil::Instance(), Unmarshal(_, _)).WillOnce(Return(true));

    std::u16string text;
    int32_t result = inputDataChannelProxy->GetTextBeforeCursor(5, text);
    EXPECT_EQ(result, NO_ERROR);
}

HWTEST_F(InputDataChannelProxyTest, testGetEnableData_004, TestSize.Level0)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(MockTypesUtil::Instance(), Marshal(_, _)).WillOnce(Return(true));

    inputDataChannelProxy->SendKeyboardStatus(KeyboardStatus::VISIBLE);
}

HWTEST_F(InputDataChannelProxyTest, testGetEnableData_005, TestSize.Level0)
{
    InputDataChannelProxy proxy(nullptr);
    int32_t result = proxy.InsertText(u"test");
    EXPECT_EQ(result, ErrorCode::ERROR_EX_NULL_POINTER);
}

HWTEST_F(InputDataChannelProxyTest, testGetEnableData_006, TestSize.Level0)
{
    EXPECT_CALL(MockTypesUtil::Instance(), Marshal(_, _)).WillOnce(Return(false));

    int32_t result = inputDataChannelProxy->InsertText(u"test");
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

HWTEST_F(InputDataChannelProxyTest, testGetEnableData_007, TestSize.Level0)
{
    EXPECT_CALL(*remoteObject, SendRequest(_, _, _, _)).WillOnce(Return(ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT));

    int32_t result = inputDataChannelProxy->InsertText(u"test");
    EXPECT_EQ(result, ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT);
}