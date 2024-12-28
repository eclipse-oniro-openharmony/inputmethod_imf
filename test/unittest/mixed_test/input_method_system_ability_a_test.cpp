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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "appexecfwk_event_handler.h"
#include "appexecfwk_event_runner.h"
#include "input_method_system_ability.h"
#include "input_method_system_ability_stub.h"
#include "user_session_manager.h"


using namespace OHOS;
using namespace MiscServices;
using namespace testing;

class MockInputMethodSystemAbility : public InputMethodSystemAbility {
public:
    MOCK_METHOD4(
        OnRemoteRequest, int32_t(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));
    MOCK_METHOD0(GetUserSession, std::shared_ptr<UserSession>());
    MOCK_METHOD1(RemoveTask, void(const std::string &taskName));
    MOCK_METHOD3(PostTask, void(std::function<void()> task, const std::string &taskName, int64_t delay));
    MOCK_METHOD0(GetTickCount, int64_t());
};

class InputMethodSystemAbilityTest : public Test {
protected:
    void SetUp() override
    {
        serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create("test"));
        InputMethodSystemAbility::serviceHandler_ = serviceHandler_;
    }

    void TearDown() override
    {
        InputMethodSystemAbility::serviceHandler_ = nullptr;
    }

    std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
};

TEST_F(InputMethodSystemAbilityTest, OnRemoteRequest_WithReleaseInputCode_TaskPosted)
{
    MockInputMethodSystemAbility mockAbility;
    MessageParcel data, reply;
    MessageOption option;

    EXPECT_CALL(mockAbility, GetUserSession()).WillOnce(Return(std::make_shared<UserSession>()));
    EXPECT_CALL(mockAbility, RemoveTask("unloadInputMethodSaTask"));
    EXPECT_CALL(mockAbility, PostTask(_, "unloadInputMethodSaTask", DELAY_UNLOAD_SA_TIME));

    int32_t result = mockAbility.OnRemoteRequest(
        static_cast<uint32_t>(InputMethodInterfaceCode::RELEASE_INPUT), data, reply, option);
    EXPECT_EQ(result, 0);
}

TEST_F(InputMethodSystemAbilityTest, OnRemoteRequest_WithRequestHideInputCode_TaskPosted)
{
    MockInputMethodSystemAbility mockAbility;
    MessageParcel data, reply;
    MessageOption option;

    EXPECT_CALL(mockAbility, GetUserSession()).WillOnce(Return(std::make_shared<UserSession>()));
    EXPECT_CALL(mockAbility, RemoveTask("unloadInputMethodSaTask"));
    EXPECT_CALL(mockAbility, PostTask(_, "unloadInputMethodSaTask", DELAY_UNLOAD_SA_TIME));

    int32_t result = mockAbility.OnRemoteRequest(
        static_cast<uint32_t>(InputMethodInterfaceCode::REQUEST_HIDE_INPUT), data, reply, option);
    EXPECT_EQ(result, 0);
}

TEST_F(InputMethodSystemAbilityTest, OnRemoteRequest_WithOtherCode_NoTaskPosted)
{
    MockInputMethodSystemAbility mockAbility;
    MessageParcel data, reply;
    MessageOption option;

    EXPECT_CALL(mockAbility, GetUserSession()).WillOnce(Return(std::make_shared<UserSession>()));
    EXPECT_CALL(mockAbility, RemoveTask(_)).Times(0);
    EXPECT_CALL(mockAbility, PostTask(_, _, _)).Times(0);

    int32_t result = mockAbility.OnRemoteRequest(1234, data, reply, option);
    EXPECT_EQ(result, 0);
}

TEST_F(InputMethodSystemAbilityTest, OnRemoteRequest_WithTimeNotPassed_NoTaskPosted)
{
    MockInputMethodSystemAbility mockAbility;
    MessageParcel data, reply;
    MessageOption option;

    EXPECT_CALL(mockAbility, GetUserSession()).WillOnce(Return(std::make_shared<UserSession>()));
    EXPECT_CALL(mockAbility, GetTickCount()).WillOnce(Return(10000));
    EXPECT_CALL(mockAbility, RemoveTask(_)).Times(0);
    EXPECT_CALL(mockAbility, PostTask(_, _, _)).Times(0);

    int32_t result = mockAbility.OnRemoteRequest(
        static_cast<uint32_t>(InputMethodInterfaceCode::RELEASE_INPUT), data, reply, option);
    EXPECT_EQ(result, 0);
}
