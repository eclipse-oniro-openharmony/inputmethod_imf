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

#include "input_method_ability_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "iinput_method_system_ability.h"
#include "on_demand_start_stop_sa.h"

using namespace OHOS;
using namespace MiscServices;
using namespace testing;

class MockOnDemandStartStopSa : public OnDemandStartStopSa {
public:
    MOCK_METHOD(sptr<IRemoteObject>, GetInputMethodSystemAbility, (), (override));
};

class MockIInputMethodSystemAbility : public IInputMethodSystemAbility {
public:
    MOCK_METHOD(int, GetInputMethodAbilityId, (), (override));
};

class ImaUtilsTest : public Test {
protected:
    void SetUp() override
    {
        mockOnDemandStartStopSa = std::make_shared<MockOnDemandStartStopSa>();
        mockIInputMethodSystemAbility = std::make_shared<MockIInputMethodSystemAbility>();
    }

    void TearDown() override
    {
        ImaUtils::abilityManager_ = nullptr;
    }

    std::shared_ptr<MockOnDemandStartStopSa> mockOnDemandStartStopSa;
    std::shared_ptr<MockIInputMethodSystemAbility> mockIInputMethodSystemAbility;
};

HWTEST_F(ImaUtilsTest, getImsaProxy_001, TestSize.Level0)
{
    ImaUtils::abilityManager_ = mockIInputMethodSystemAbility;
    EXPECT_EQ(ImaUtils::GetImsaProxy(), mockIInputMethodSystemAbility);
}

HWTEST_F(ImaUtilsTest, getImsaProxy_002, TestSize.Level0)
{
    ImaUtils::abilityManager_ = nullptr;
    EXPECT_CALL(*mockOnDemandStartStopSa, GetInputMethodSystemAbility()).WillOnce(Return(nullptr));
    EXPECT_EQ(ImaUtils::GetImsaProxy(), nullptr);
}

HWTEST_F(ImaUtilsTest, getImsaProxy_003, TestSize.Level0)
{
    ImaUtils::abilityManager_ = nullptr;
    EXPECT_CALL(*mockOnDemandStartStopSa, GetInputMethodSystemAbility()).WillOnce(Return(mockIInputMethodSystemAbility));
    EXPECT_EQ(ImaUtils::GetImsaProxy(), mockIInputMethodSystemAbility);
}