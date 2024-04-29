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
#define private public
#define protected public
#include "freeze_manager.h"
#include "peruser_session.h"
#undef private
#include <gtest/gtest.h>
#include <gtest/hwext/gtest-multithread.h>
#include <sys/time.h>

#include "global.h"
using namespace testing::ext;
using namespace testing::mt;
namespace OHOS {
namespace MiscServices {
constexpr int32_t TASK_NUM = 100;
constexpr int32_t IPC_COST_TIME = 5000;
class ImeFreezeManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        freezeManager_ = std::make_shared<FreezeManager>(-1);
    }
    static void TearDownTestCase(void)
    {
    }
    void SetUp()
    {
        IMSA_HILOGI("ImeFreezeManagerTest::SetUp");
    }
    void TearDown()
    {
        IMSA_HILOGI("ImeFreezeManagerTest::TearDown");
        ClearState();
    }
    static void TestAttach(bool isSuccess)
    {
        IMSA_HILOGI("run in, isSuccess: %{public}d", isSuccess);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            freezeManager_->BeforeIpc(RequestType::START_INPUT);
            CheckAllState(true, false);
        }
        usleep(IPC_COST_TIME);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            freezeManager_->AfterIpc(RequestType::START_INPUT, isSuccess);
            CheckAllState(isSuccess, !isSuccess);
        }
    }
    static void TestDetach()
    {
        IMSA_HILOGI("run in");
        {
            std::lock_guard<std::mutex> lock(mtx_);
            freezeManager_->BeforeIpc(RequestType::STOP_INPUT);
            CheckFreezable(false);
        }
        usleep(IPC_COST_TIME);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            freezeManager_->AfterIpc(RequestType::STOP_INPUT, true);
            CheckAllState(false, true);
        }
    }
    static void TestRequestShowInput(bool isSuccess)
    {
        IMSA_HILOGI("run in, isSuccess: %{public}d", isSuccess);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            bool ret = freezeManager_->IsIpcNeeded(RequestType::REQUEST_SHOW);
            if (!ret) {
                return;
            }
            freezeManager_->BeforeIpc(RequestType::REQUEST_SHOW);
            CheckFreezable(false);
        }
        usleep(IPC_COST_TIME);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            freezeManager_->AfterIpc(RequestType::REQUEST_SHOW, isSuccess);
            if (isSuccess) {
                CheckAllState(true, false);
            }
        }
    }
    static void TestRequestHideInput(bool isSuccess)
    {
        IMSA_HILOGI("run in, isSuccess: %{public}d", isSuccess);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            bool ret = freezeManager_->IsIpcNeeded(RequestType::REQUEST_HIDE);
            if (!ret) {
                return;
            }
            freezeManager_->BeforeIpc(RequestType::REQUEST_HIDE);
            CheckFreezable(false);
        }
        usleep(IPC_COST_TIME);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            freezeManager_->AfterIpc(RequestType::REQUEST_HIDE, isSuccess);
            if (isSuccess) {
                CheckAllState(false, true);
            }
        }
    }
    static void TestNormalIPC()
    {
        IMSA_HILOGI("run in");
        {
            std::lock_guard<std::mutex> lock(mtx_);
            freezeManager_->BeforeIpc(RequestType::NORMAL);
            CheckFreezable(false);
        }
        usleep(IPC_COST_TIME);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            freezeManager_->AfterIpc(RequestType::NORMAL, true);
        }
    }
    static void ClearState()
    {
        IMSA_HILOGI("run in");
        freezeManager_->isImeInUse_ = false;
        freezeManager_->isFrozen_ = true;
    }
    static void FullTestTask()
    {
        for (int32_t i = 0; i < TASK_NUM; i++) {
            TestAttach(false);
            TestAttach(true);
            TestDetach();
            TestRequestHideInput(true);
            TestRequestHideInput(false);
            TestNormalIPC();
        }
    }
    static std::shared_ptr<FreezeManager> freezeManager_;
    static std::mutex mtx_;

private:
    static void CheckAllState(bool imeInUse, bool freezable)
    {
        EXPECT_EQ(freezeManager_->isImeInUse_, imeInUse);
        EXPECT_EQ(freezeManager_->isFrozen_, freezable);
    }
    static void CheckImeInUse(bool imeInUse)
    {
        EXPECT_EQ(freezeManager_->isImeInUse_, imeInUse);
    }
    static void CheckFreezable(bool freezable)
    {
        EXPECT_EQ(freezeManager_->isFrozen_, freezable);
    }
};
std::shared_ptr<FreezeManager> ImeFreezeManagerTest::freezeManager_{ nullptr };
std::mutex ImeFreezeManagerTest::mtx_;

/**
 * @tc.name: SingleThread_StartInput_001
 * @tc.desc: test start input
 * @tc.type: FUNC
 */
HWTEST_F(ImeFreezeManagerTest, SingleThread_StartInput_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeFreezeManagerTest::SingleThread_StartInput_001");
    ASSERT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
    ClearState();
    TestAttach(false);
    TestAttach(true);

    ClearState();
    TestAttach(true);
    TestAttach(false);

    ClearState();
    TestAttach(true);
    TestAttach(true);

    ClearState();
    TestAttach(false);
    TestAttach(false);
}

/**
 * @tc.name: SingleThread_StartAndStopInput_001
 * @tc.desc: test freeze manager
 * @tc.type: FUNC
 */
HWTEST_F(ImeFreezeManagerTest, SingleThread_StartAndStopInput_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeFreezeManagerTest::SingleThread_StartAndStopInput_001");
    ASSERT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
    ClearState();
    TestAttach(true);
    TestDetach();

    ClearState();
    TestAttach(false);
    TestDetach();

    ClearState();
    TestAttach(true);
    TestDetach();
    TestAttach(true);
}

/**
 * @tc.name: SingleThread_StartInputAndRequestHide_001
 * @tc.desc: test freeze manager
 * @tc.type: FUNC
 */
HWTEST_F(ImeFreezeManagerTest, SingleThread_StartInputAndRequestHide_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeFreezeManagerTest::SingleThread_StartInputAndRequestHide_001");
    ASSERT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
    ClearState();
    TestAttach(true);
    TestRequestHideInput(true);

    ClearState();
    TestAttach(false);
    TestRequestHideInput(true);
}

/**
 * @tc.name: SingleThread_StartInputAndNormalIPC_001
 * @tc.desc: test freeze manager
 * @tc.type: FUNC
 */
HWTEST_F(ImeFreezeManagerTest, SingleThread_StartInputAndNormalIPC_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeFreezeManagerTest::SingleThread_StartInputAndNormalIPC_001");
    ASSERT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
    ClearState();
    TestNormalIPC();

    ClearState();
    TestAttach(false);
    TestNormalIPC();

    ClearState();
    TestAttach(true);
    TestNormalIPC();
}

/**
 * @tc.name: MultiThread_FullTest_001
 * @tc.desc: test freeze manager
 * @tc.type: FUNC
 */
HWTEST_F(ImeFreezeManagerTest, MultiThread_FullTest_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeFreezeManagerTest::MultiThread_FullTest_001");
    ASSERT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
    SET_THREAD_NUM(5);
    GTEST_RUN_TASK(FullTestTask);
}

/**
 * @tc.name: SingleThread_RequestShow_001
 * @tc.desc: test freeze manager
 * @tc.type: FUNC
 */
HWTEST_F(ImeFreezeManagerTest, SingleThread_RequestShow_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeFreezeManagerTest::SingleThread_RequestShow_001");
    ASSERT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
    ClearState();
    TestRequestShowInput(true);
}
} // namespace MiscServices
} // namespace OHOS