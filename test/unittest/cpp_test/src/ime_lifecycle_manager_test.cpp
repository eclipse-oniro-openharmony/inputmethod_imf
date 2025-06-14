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

#define private   public
#define protected public
#include "ime_lifecycle_manager.h"
#undef private
#include <gtest/gtest.h>
#include <gtest/hwext/gtest-multithread.h>
#include <sys/time.h>

#include "event_handler.h"
#include "global.h"
using namespace testing::ext;
using namespace testing::mt;
namespace OHOS {
namespace MiscServices {
constexpr int32_t TEST_STOP_DELAY_TIME = 100; // 100ms
constexpr int32_t WAIT_FOR_OTHERS = 50; // 50ms
constexpr int32_t MS_TO_US = 1000;
class ImeLifecycleManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        imeLifecycleManager_ = std::make_shared<ImeLifecycleManager>(-1, StopImeCb, TEST_STOP_DELAY_TIME);
        auto runner = AppExecFwk::EventRunner::Create("test_imeLifecycleManager");
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    static void TearDownTestCase()
    {
        imeLifecycleManager_->SetEventHandler(nullptr);
        eventHandler_ = nullptr;
    }

    void SetUp()
    {
        IMSA_HILOGI("ImeLifecycleManagerTest::SetUp");
        imeLifecycleManager_->SetEventHandler(eventHandler_);
        isStopImeCalled_ = false;
    }
    void TearDown()
    {
        IMSA_HILOGI("ImeLifecycleManagerTest::TearDown");
    }
    static void StopImeCb()
    {
        IMSA_HILOGI("StopImeCb");
        isStopImeCalled_ = true;
    }
    static std::shared_ptr<ImeLifecycleManager> imeLifecycleManager_;
    static std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    static std::atomic_bool isStopImeCalled_;
};

std::shared_ptr<ImeLifecycleManager> ImeLifecycleManagerTest::imeLifecycleManager_ = nullptr;
std::shared_ptr<AppExecFwk::EventHandler> ImeLifecycleManagerTest::eventHandler_ = nullptr;
std::atomic_bool ImeLifecycleManagerTest::isStopImeCalled_ = false;

/**
 * @tc.name: ControlIme_EventHandlerNull
 * @tc.desc: test ime lifecycle manager
 * @tc.type: FUNC
 */
HWTEST_F(ImeLifecycleManagerTest, ControlIme_EventHandlerNull, TestSize.Level1)
{
    ASSERT_NE(ImeLifecycleManagerTest::imeLifecycleManager_, nullptr);
    ImeLifecycleManagerTest::imeLifecycleManager_->SetEventHandler(nullptr);
    ImeLifecycleManagerTest::imeLifecycleManager_->ControlIme(true);
    EXPECT_FALSE(ImeLifecycleManagerTest::isStopImeCalled_);
}

/**
 * @tc.name: ControlIme_shouldStopIme
 * @tc.desc: test ime lifecycle manager
 * @tc.type: FUNC
 */
HWTEST_F(ImeLifecycleManagerTest, ControlIme_shouldStopIme, TestSize.Level1)
{
    auto manager = std::make_shared<ImeLifecycleManager>(-1, ImeLifecycleManagerTest::StopImeCb, TEST_STOP_DELAY_TIME);
    manager->ControlIme(true);
    usleep((TEST_STOP_DELAY_TIME + WAIT_FOR_OTHERS) * MS_TO_US);
    EXPECT_TRUE(ImeLifecycleManagerTest::isStopImeCalled_);
}

/**
 * @tc.name: ControlIme_shouldNotStopIme
 * @tc.desc: test ime lifecycle manager
 * @tc.type: FUNC
 */
HWTEST_F(ImeLifecycleManagerTest, ControlIme_shouldNotStopIme, TestSize.Level1)
{
    ASSERT_NE(ImeLifecycleManagerTest::imeLifecycleManager_, nullptr);
    ImeLifecycleManagerTest::imeLifecycleManager_->ControlIme(false);
    usleep((TEST_STOP_DELAY_TIME + WAIT_FOR_OTHERS) * MS_TO_US);
    EXPECT_FALSE(ImeLifecycleManagerTest::isStopImeCalled_);
}
} // namespace MiscServices
} // namespace OHOS