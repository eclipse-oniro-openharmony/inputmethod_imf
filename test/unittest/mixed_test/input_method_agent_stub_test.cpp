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
#include "input_method_agent_stub.h"
#include "input_method_ability.h"
#include "task_manager.h"
#include "itypes_util.h"
#include "message_parcel.h"
#include "ipc_skeleton.h"
#include "mmi/key_event.h"
#include "private_data_value.h"
#include "input_attribute.h"
#include "array_buffer.h"
#include "task_imsa.h"

using namespace OHOS;
using namespace MiscServices;
using namespace testing;

class InputMethodAgentStubTest : public Test {
public:
    static void SetUpTestCase()
    {
        // Setup any shared resources or configurations
    }

    static void TearDownTestCase()
    {
        // Clean up any shared resources or configurations
    }

    void SetUp() override
    {
        // Setup any resources or configurations for each test
    }

    void TearDown() override
    {
        // Clean up any resources or configurations for each test
    }

    // Mocking objects
    MockMessageParcel mockData;
    MockMessageParcel mockReply;
    MockMessageOption mockOption;
    MockInputMethodAbility mockInputMethodAbility;
    MockTaskManager mockTaskManager;
    MockITypesUtil mockITypesUtil;
    MockMMIKeyEvent mockMMIKeyEvent;
    MockTaskImsaSendPrivateCommand mockTaskImsaSendPrivateCommand;
    MockTaskImsaOnCursorUpdate mockTaskImsaOnCursorUpdate;
    MockTaskImsaOnSelectionChange mockTaskImsaOnSelectionChange;
    MockTaskImsaAttributeChange mockTaskImsaAttributeChange;
};

HWTEST_F(InputMethodAgentStubTest, OnRemoteRequest_ValidDescriptor_Success) {
    InputMethodAgentStub stub;
    EXPECT_CALL(mockData, ReadInterfaceToken()).WillOnce(Return(stub.GetDescriptor()));
    EXPECT_CALL(mockData, ReadUint32()).WillOnce(Return(123));
    EXPECT_CALL(mockReply, WriteNoException()).Times(1);

    int32_t result = stub.OnRemoteRequest(SET_CALLING_WINDOW_ID, mockData, mockReply, mockOption);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(InputMethodAgentStubTest, OnRemoteRequest_InvalidDescriptor_Error) {
    InputMethodAgentStub stub;
    EXPECT_CALL(mockData, ReadInterfaceToken()).WillOnce(Return("InvalidDescriptor"));

    int32_t result = stub.OnRemoteRequest(SET_CALLING_WINDOW_ID, mockData, mockReply, mockOption);
    EXPECT_EQ(result, ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION);
}

HWTEST_F(InputMethodAgentStubTest, DispatchKeyEventOnRemote_NullKeyEvent_Error) {
    InputMethodAgentStub stub;
    EXPECT_CALL(mockData, ReadInterfaceToken()).WillOnce(Return(stub.GetDescriptor()));
    EXPECT_CALL(mockMMIKeyEvent, Create()).WillOnce(Return(nullptr));

    int32_t result = stub.DispatchKeyEventOnRemote(mockData, mockReply);
    EXPECT_EQ(result, ErrorCode::ERROR_NULL_POINTER);
}

HWTEST_F(InputMethodAgentStubTest, SendPrivateCommandOnRemote_NotDefaultIME_Error) {
    InputMethodAgentStub stub;
    EXPECT_CALL(mockData, ReadInterfaceToken()).WillOnce(Return(stub.GetDescriptor()));
    EXPECT_CALL(mockInputMethodAbility, IsDefaultIme()).WillOnce(Return(false));

    int32_t result = stub.SendPrivateCommandOnRemote(mockData, mockReply);
    EXPECT_EQ(result, ErrorCode::ERROR_NOT_DEFAULT_IME);
}

HWTEST_F(InputMethodAgentStubTest, OnAttributeChangeOnRemote_UnmarshallingFails_Error) {
    InputMethodAgentStub stub;
    EXPECT_CALL(mockData, ReadInterfaceToken()).WillOnce(Return(stub.GetDescriptor()));
    EXPECT_CALL(mockITypesUtil, Unmarshal(mockData, _)).WillOnce(Return(false));

    int32_t result = stub.OnAttributeChangeOnRemote(mockData, mockReply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}

HWTEST_F(InputMethodAgentStubTest, RecvMessageOnRemote_UnmarshallingFails_Error) {
    InputMethodAgentStub stub;
    EXPECT_CALL(mockData, ReadInterfaceToken()).WillOnce(Return(stub.GetDescriptor()));
    EXPECT_CALL(mockITypesUtil, Unmarshal(mockData, _)).WillOnce(Return(false));

    int32_t result = stub.RecvMessageOnRemote(mockData, mockReply);
    EXPECT_EQ(result, ErrorCode::ERROR_EX_PARCELABLE);
}