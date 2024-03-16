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
constexpr int32_t TASK_NUM = 10;
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
            freezeManager_->BeforeIPC(RequestType::START_INPUT);
            CheckAllState(true, false);
        }
        usleep(5000);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            freezeManager_->AfterIPC(RequestType::START_INPUT, isSuccess);
            CheckAllState(isSuccess, !isSuccess);
        }
    }
    static void TestDetach()
    {
        IMSA_HILOGI("run in");
        {
            std::lock_guard<std::mutex> lock(mtx_);
            freezeManager_->BeforeIPC(RequestType::STOP_INPUT);
            CheckFreezable(false);
        }
        usleep(5000);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            freezeManager_->AfterIPC(RequestType::STOP_INPUT, true);
            CheckAllState(false, true);
        }
    }
    static void TestRequestHideInput(bool isSuccess)
    {
        IMSA_HILOGI("run in, isSuccess: %{public}d", isSuccess);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            bool ret = freezeManager_->BeforeIPC(RequestType::REQUEST_HIDE);
            if (!ret) {
                return;
            }
            CheckFreezable(false);
        }
        usleep(5000);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            freezeManager_->AfterIPC(RequestType::REQUEST_HIDE, isSuccess);
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
            freezeManager_->BeforeIPC(RequestType::NORMAL);
            CheckFreezable(false);
        }
        usleep(5000);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            freezeManager_->AfterIPC(RequestType::NORMAL, true);
        }
    }
    static void ClearState()
    {
        IMSA_HILOGI("run in");
        freezeManager_->imeInUse_ = false;
        freezeManager_->freezable_ = true;
    }
    static void AttachAndDetachTask()
    {
        for (int32_t i = 0; i < TASK_NUM; i++) {
            TestAttach(true);
            TestDetach();
        }
    }
    static void AttachAndRequestHideTask()
    {
        for (int32_t i = 0; i < TASK_NUM; i++) {
            TestAttach(true);
            TestRequestHideInput(true);
        }
    }
    static void NormalIPCTask1()
    {
        for (int32_t i = 0; i < TASK_NUM; i++) {
            TestAttach(true);
            TestDetach();
            TestNormalIPC();
        }
    }
    static void NormalIPCTask2()
    {
        for (int32_t i = 0; i < TASK_NUM; i++) {
            TestAttach(true);
            TestRequestHideInput(true);
            TestNormalIPC();
        }
    }
    static std::shared_ptr<FreezeManager> freezeManager_;
    static std::mutex mtx_;

private:
    static void CheckAllState(bool imeInUse, bool freezable)
    {
        EXPECT_EQ(freezeManager_->imeInUse_, imeInUse);
        EXPECT_EQ(freezeManager_->freezable_, freezable);
    }
    static void CheckImeInUse(bool imeInUse)
    {
        EXPECT_EQ(freezeManager_->imeInUse_, imeInUse);
    }
    static void CheckFreezable(bool freezable)
    {
        EXPECT_EQ(freezeManager_->freezable_, freezable);
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
    EXPECT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
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
    EXPECT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
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
    EXPECT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
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
    EXPECT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
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
 * @tc.name: MultiThread_TestAttachAndDetach_001
 * @tc.desc: test freeze manager
 * @tc.type: FUNC
 */
HWTEST_F(ImeFreezeManagerTest, MultiThread_TestAttachAndDetach_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeFreezeManagerTest::MultiThread_TestAttachAndDetach_001");
    EXPECT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
    SET_THREAD_NUM(5);
    GTEST_RUN_TASK(AttachAndDetachTask);
}

/**
 * @tc.name: MultiThread_TestAttachAndRequestHide_001
 * @tc.desc: test freeze manager
 * @tc.type: FUNC
 */
HWTEST_F(ImeFreezeManagerTest, MultiThread_TestAttachAndRequestHide_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeFreezeManagerTest::MultiThread_TestAttachAndRequestHide_001");
    EXPECT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
    SET_THREAD_NUM(5);
    GTEST_RUN_TASK(AttachAndRequestHideTask);
}

/**
 * @tc.name: MultiThread_TestNormalIPC_001
 * @tc.desc: test freeze manager
 * @tc.type: FUNC
 */
HWTEST_F(ImeFreezeManagerTest, MultiThread_TestNormalIPC_001, TestSize.Level0)
{
    IMSA_HILOGI("ImeFreezeManagerTest::MultiThread_TestNormalIPC_001");
    EXPECT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
    SET_THREAD_NUM(5);
    GTEST_RUN_TASK(NormalIPCTask1);
}

/**
 * @tc.name: MultiThread_TestNormalIPC_002
 * @tc.desc: test freeze manager
 * @tc.type: FUNC
 */
HWTEST_F(ImeFreezeManagerTest, MultiThread_TestNormalIPC_002, TestSize.Level0)
{
    IMSA_HILOGI("ImeFreezeManagerTest::MultiThread_TestNormalIPC_002");
    EXPECT_NE(ImeFreezeManagerTest::freezeManager_, nullptr);
    SET_THREAD_NUM(5);
    GTEST_RUN_TASK(NormalIPCTask2);
}
} // namespace MiscServices
} // namespace OHOS