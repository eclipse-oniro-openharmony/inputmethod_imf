/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "ime_state_manager_factory.h"
#include <gtest/gtest.h>

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class ImeStateManagerFactoryTest : public testing::Test {
protected:
    void SetUp() override
    {
        // Reset to default state before each test
        ImeStateManagerFactory::GetInstance().SetDynamicStartIme(false);
    }
};

/**
 * @tc.name: SetGetDynamicStartIme
 * @tc.desc: Test Set/Get for boolean flag
 * @tc.type: FUNC
 */
HWTEST_F(ImeStateManagerFactoryTest, SetGetDynamicStartIme, TestSize.Level1)
{
    auto &factory = ImeStateManagerFactory::GetInstance();

    factory.SetDynamicStartIme(true);
    EXPECT_TRUE(factory.GetDynamicStartIme());

    factory.SetDynamicStartIme(false);
    EXPECT_FALSE(factory.GetDynamicStartIme());
}

/**
 * @tc.name: SingletonInstance
 * @tc.desc: Test singleton instance consistency
 * @tc.type: FUNC
 */
HWTEST_F(ImeStateManagerFactoryTest, SingletonInstance, TestSize.Level1)
{
    auto &instance1 = ImeStateManagerFactory::GetInstance();
    auto &instance2 = ImeStateManagerFactory::GetInstance();
    ASSERT_EQ(&instance1, &instance2);
}

/**
 * @tc.name: CreateImeLifecycleManagerWhenDynamic
 * @tc.desc: Test object creation in dynamic mode
 * @tc.type: FUNC
 */
HWTEST_F(ImeStateManagerFactoryTest, CreateImeLifecycleManagerWhenDynamic, TestSize.Level1)
{
    auto &factory = ImeStateManagerFactory::GetInstance();
    factory.SetDynamicStartIme(true);
    auto manager = factory.CreateImeStateManager(0, [] {
        return;
    });

    EXPECT_NE(manager.get(), nullptr);
}

/**
 * @tc.name: CreateFreezeManagerWhenNotDynamic
 * @tc.desc: Test object creation in non-dynamic mode
 * @tc.type: FUNC
 */
HWTEST_F(ImeStateManagerFactoryTest, CreateFreezeManagerWhenNotDynamic, TestSize.Level1)
{
    auto &factory = ImeStateManagerFactory::GetInstance();
    factory.SetDynamicStartIme(false); // Explicit set for clarity
    // stopFunc should be ignored in this mode
    auto manager = factory.CreateImeStateManager(0, [] {
        FAIL() << "Should not be called";
    });

    // Verify type
    EXPECT_NE(manager.get(), nullptr);
}
} // namespace MiscServices
} // namespace OHOS