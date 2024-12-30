/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ability_connect_callback.h"
#include "ability_manager_client.h"
#include "connection_manager.h"
#include "errcode.h"
#include "input_method_extension_context.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_manager_client.h"
#include "start_options.h"
#include "want.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::IMSA;
using namespace testing;
using namespace OHOS::AbilityRuntime;

class MockAbilityManagerClient : public AbilityManagerClient {
public:
    MOCK_METHOD3(StartAbility, ErrCode(const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode));
};

class MockInputMethodExtensionContext : public InputMethodExtensionContext {
public:
    MockInputMethodExtensionContext() : InputMethodExtensionContext() {}
    virtual ~MockInputMethodExtensionContext() {}

    MOCK_METHOD0(GetToken, sptr<IRemoteObject>());
};

class InputMethodExtensionContextTest : public testing::Test {
protected:
    void SetUp() override
    {
        mockAbilityManagerClient = std::make_shared<MockAbilityManagerClient>();
        EXPECT_CALL(*mockAbilityManagerClient, StartAbility(_, _, _)).WillRepeatedly(Return(ERR_OK));
        AbilityManagerClient::SetInstance(mockAbilityManagerClient);
        inputMethodExtensionContext = std::make_shared<InputMethodExtensionContext>();
        context = new MockInputMethodExtensionContext();
        AAFwk::AbilityManagerClient::SetInstance(*mockAbilityManagerClient);
        connectionManager = new MockConnectionManager();
        extensionContext = new InputMethodExtensionContext();
        extensionContext->SetConnectionManager(connectionManager);
    }

    void TearDown() override
    {
        AbilityManagerClient::SetInstance(nullptr);
        delete context;
        delete mockAbilityManagerClient;
        delete connectionManager;
    }

    std::shared_ptr<InputMethodExtensionContext> inputMethodExtensionContext;
    std::shared_ptr<MockAbilityManagerClient> mockAbilityManagerClient;
    MockInputMethodExtensionContext *context;
    MockConnectionManager *connectionManager;
    InputMethodExtensionContext *extensionContext;
};

class MockConnectionManager : public ConnectionManager {
public:
    MOCK_METHOD3(ConnectAbility,
        ErrCode(
            const sptr<IRemoteObject> &token, const Want &want, const sptr<AbilityConnectCallback> &connectCallback));
};

TEST_F(InputMethodExtensionContextTest, StartAbility_Success_ReturnsErrOk)
{
    Want want;
    ErrCode result = inputMethodExtensionContext->StartAbility(want);
    EXPECT_EQ(result, ERR_OK);
}

TEST_F(InputMethodExtensionContextTest, StartAbility_Failure_ReturnsNonErrOk)
{
    EXPECT_CALL(*mockAbilityManagerClient, StartAbility(_, _, _)).WillOnce(Return(ERR_INVALID_VALUE));
    Want want;
    ErrCode result = inputMethodExtensionContext->StartAbility(want);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

TEST_F(InputMethodExtensionContextTest, StartAbility_ReturnsErrOk)
{
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    EXPECT_CALL(*mockAbilityManagerClient, StartAbility(_, _, _, _)).WillOnce(Return(ERR_OK));

    ErrCode result = context->StartAbility(want, startOptions);

    EXPECT_EQ(result, ERR_OK);
}

TEST_F(InputMethodExtensionContextTest, StartAbility_ReturnsNonErrOk)
{
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    ErrCode expectedErr = -1; // 假设-1表示错误
    EXPECT_CALL(*mockAbilityManagerClient, StartAbility(_, _, _, _)).WillOnce(Return(expectedErr));

    ErrCode result = context->StartAbility(want, startOptions);

    EXPECT_EQ(result, expectedErr);
}

TEST_F(InputMethodExtensionContextTest, ConnectAbility_Success_ReturnsTrue)
{
    Want want;
    sptr<MockAbilityConnectCallback> connectCallback = new MockAbilityConnectCallback();

    EXPECT_CALL(*connectionManager, ConnectAbility(_, _, _)).WillOnce(Return(ERR_OK));

    bool result = extensionContext->ConnectAbility(want, connectCallback);

    EXPECT_TRUE(result);
}

TEST_F(InputMethodExtensionContextTest, ConnectAbility_Failure_ReturnsFalse)
{
    Want want;
    sptr<MockAbilityConnectCallback> connectCallback = new MockAbilityConnectCallback();

    EXPECT_CALL(*connectionManager, ConnectAbility(_, _, _)).WillOnce(Return(ERR_INVALID_VALUE));

    bool result = extensionContext->ConnectAbility(want, connectCallback);

    EXPECT_FALSE(result);
}
