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
#include "on_demand_start_stop_sa.h"
#include "system_ability_manager_client.h"
#include "mock_system_ability_manager.h"

using namespace OHOS;
using namespace OHOS::MiscServices;
using namespace testing;

class MockSystemAbilityManager : public ISystemAbilityManager {
public:
    MOCK_METHOD(sptr<IRemoteObject>, CheckSystemAbility, (int32_t), (override));
    MOCK_METHOD(int32_t, LoadSystemAbility, (int32_t, const sptr<ISystemAbilityLoadCallback> &), (override));
    MOCK_METHOD(int32_t, UnloadSystemAbility, (int32_t), (override));
    MOCK_METHOD(sptr<IRemoteObject>, GetSystemAbility, (int32_t, const std::string &), (override));
};

TEST(OnDemandStartStopSaTest, LoadInputMethodSystemAbility_Success)
{
    auto mockManager = std::make_shared<NiceMock<MockSystemAbilityManager>>();
    EXPECT_CALL(*mockManager, CheckSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID))
        .WillOnce(Return(nullptr));
    EXPECT_CALL(*mockManager, LoadSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, _))
        .WillOnce(Return(ERR_OK));
    EXPECT_CALL(*mockManager, CheckSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID))
        .WillOnce(Return(std::make_shared<IRemoteObject>()));

    SystemAbilityManagerClient::GetInstance().SetSystemAbilityManager(mockManager);

    OnDemandStartStopSa sa;
    sptr<IRemoteObject> remoteObj = sa.LoadInputMethodSystemAbility();
    ASSERT_NE(remoteObj, nullptr);
}

TEST(OnDemandStartStopSaTest, LoadInputMethodSystemAbility_Fail)
{
    auto mockManager = std::make_shared<NiceMock<MockSystemAbilityManager>>();
    EXPECT_CALL(*mockManager, CheckSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID))
        .WillOnce(Return(nullptr));
    EXPECT_CALL(*mockManager, LoadSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, _))
        .WillOnce(Return(ERR_UNKNOWN_ERR));  // 假设ERR_UNKNOWN_ERR表示失败

    SystemAbilityManagerClient::GetInstance().SetSystemAbilityManager(mockManager);

    OnDemandStartStopSa sa;
    sptr<IRemoteObject> remoteObj = sa.LoadInputMethodSystemAbility();
    ASSERT_EQ(remoteObj, nullptr);
}

TEST(OnDemandStartStopSaTest, UnloadInputMethodSystemAbility)
{
    auto mockManager = std::make_shared<NiceMock<MockSystemAbilityManager>>();
    EXPECT_CALL(*mockManager, UnloadSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID))
        .WillOnce(Return(ERR_OK));

    SystemAbilityManagerClient::GetInstance().SetSystemAbilityManager(mockManager);

    OnDemandStartStopSa sa;
    sa.UnloadInputMethodSystemAbility();
}

TEST(OnDemandStartStopSaTest, SaLoadCallback_Success)
{
    auto mockManager = std::make_shared<NiceMock<MockSystemAbilityManager>>();
    EXPECT_CALL(*mockManager, CheckSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID))
        .WillOnce(Return(nullptr));
    EXPECT_CALL(*mockManager, LoadSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, _))
        .WillOnce(Return(ERR_OK));

    SystemAbilityManagerClient::GetInstance().SetSystemAbilityManager(mockManager);

    OnDemandStartStopSa sa;
    sptr<IRemoteObject> remoteObj = std::make_shared<IRemoteObject>();
    sa.SaLoadCallback::OnLoadSystemAbilitySuccess(INPUT_METHOD_SYSTEM_ABILITY_ID, remoteObj);
    ASSERT_EQ(sa.remoteObj_, remoteObj);
}

TEST(OnDemandStartStopSaTest, SaLoadCallback_Fail)
{
    auto mockManager = std::make_shared<NiceMock<MockSystemAbilityManager>>();
    EXPECT_CALL(*mockManager, CheckSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID))
        .WillOnce(Return(nullptr));
    EXPECT_CALL(*mockManager, LoadSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, _))
        .WillOnce(Return(ERR_OK));

    SystemAbilityManagerClient::GetInstance().SetSystemAbilityManager(mockManager);

    OnDemandStartStopSa sa;
    sa.SaLoadCallback::OnLoadSystemAbilityFail(INPUT_METHOD_SYSTEM_ABILITY_ID);
    ASSERT_EQ(sa.remoteObj_, nullptr);
}

TEST(OnDemandStartStopSaTest, GetInputMethodSystemAbility_NoRetry)
{
    auto mockManager = std::make_shared<NiceMock<MockSystemAbilityManager>>();
    EXPECT_CALL(*mockManager, CheckSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID))
        .WillOnce(Return(std::make_shared<IRemoteObject>()));

    SystemAbilityManagerClient::GetInstance().SetSystemAbilityManager(mockManager);

    sptr<IRemoteObject> systemAbility = OnDemandStartStopSa::GetInputMethodSystemAbility(false);
    ASSERT_NE(systemAbility, nullptr);
}

TEST(OnDemandStartStopSaTest, GetInputMethodSystemAbility_Retry)
{
    auto mockManager = std::make_shared<NiceMock<MockSystemAbilityManager>>();
    EXPECT_CALL(*mockManager, CheckSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID))
        .WillOnce(Return(nullptr));
    EXPECT_CALL(*mockManager, LoadSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, _))
        .WillOnce(Return(ERR_OK));
    EXPECT_CALL(*mockManager, CheckSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID))
        .WillOnce(Return(std::make_shared<IRemoteObject>()));

    SystemAbilityManagerClient::GetInstance().SetSystemAbilityManager(mockManager);

    sptr<IRemoteObject> systemAbility = OnDemandStartStopSa::GetInputMethodSystemAbility(true);
    ASSERT_NE(systemAbility, nullptr);
}
