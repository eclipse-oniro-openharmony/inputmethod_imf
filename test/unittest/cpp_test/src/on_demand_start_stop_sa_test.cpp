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

#include <gtest/gtest.h>

#define private public
#include "on_demand_start_stop_sa.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class OnDemandStartStopSaTest : public testing::Test { };

/**
 * @tc.name: OnDemandStartSaTest
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(OnDemandStartStopSaTest, OnDemandStartSaTest, TestSize.Level1) {
    auto remote = OnDemandStartStopSa::GetInputMethodSystemAbility();
    EXPECT_NE(nullptr, remote);

    remote = OnDemandStartStopSa::GetInputMethodSystemAbility(true);
    EXPECT_NE(nullptr, remote);

    remote = OnDemandStartStopSa::GetInputMethodSystemAbility(false);
    EXPECT_NE(nullptr, remote);
}

/**
 * @tc.name: OnDemandStopSaTest
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(OnDemandStartStopSaTest, OnDemandStopSaTest, TestSize.Level1)
{
    auto onDemandStartStopSa = std::make_shared<OnDemandStartStopSa>();
    ASSERT_NE(nullptr, onDemandStartStopSa);

    onDemandStartStopSa->UnloadInputMethodSystemAbility();
}

/**
 * @tc.name: LoadSystemAbilityTest
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(OnDemandStartStopSaTest, LoadSystemAbilityTest, TestSize.Level1)
{
    auto onDemandStartStopSa = std::make_shared<OnDemandStartStopSa>();
    ASSERT_NE(nullptr, onDemandStartStopSa);

    auto systemAbility = onDemandStartStopSa->LoadInputMethodSystemAbility();
    EXPECT_NE(nullptr, systemAbility);
}

/**
 * @tc.name: SaLoadCallBackTest
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(OnDemandStartStopSaTest, SaLoadCallBackTest, TestSize.Level1)
{
    auto onDemandStartStopSa = std::make_shared<OnDemandStartStopSa>();
    ASSERT_NE(nullptr, onDemandStartStopSa);

    sptr<OnDemandStartStopSa::SaLoadCallback> callback =
        new (std::nothrow) OnDemandStartStopSa::SaLoadCallback(onDemandStartStopSa);
    ASSERT_NE(nullptr, callback);

    callback->OnLoadSystemAbilitySuccess(0, nullptr);
    callback->OnLoadSystemAbilityFail(0);
}
} // namespace MiscServices
} // namespace OHOS
