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

#define private public
#define protected public
#include "sa_task_manager.h"

#include "response_data_util.h"
#include "service_response_data.h"
#undef private

#include <gtest/gtest.h>
#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <mutex>

#include "identity_checker_mock.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr uint32_t PAUSE_TIMEOUT = 100; // 100ms
constexpr uint32_t DELAY_TIME = 200;    // 200ms
constexpr uint32_t SUCCESS_RESULT = 100;
constexpr uint32_t FAILED_RESULT = 999;
constexpr uint32_t WAIT_EXEC_END = 50000;        // 50ms
constexpr uint32_t WAIT_PAUSE_EXEC_END = 500000; // 500ms
constexpr uint32_t CONSTANTS_1 = 1;
constexpr uint32_t CONSTANTS_2 = 2;
constexpr uint32_t CONSTANTS_5 = 5;
constexpr uint32_t CONSTANTS_12 = 12;
class SaTaskManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        IMSA_HILOGI("SaTaskManagerTest::SetUpTestCase");
        SaTaskManager::GetInstance().Init();
        SaTaskManager::GetInstance().identityChecker_ = std::make_shared<IdentityCheckerMock>();
    }
    static void TearDownTestCase()
    {
        IMSA_HILOGI("SaTaskManagerTest::TearDownTestCase");
    }
    void SetUp()
    {
        IMSA_HILOGI("SaTaskManagerTest::SetUp");
        SaTaskManagerTest::result_ = 0;
    }
    void TearDown()
    {
        IMSA_HILOGI("SaTaskManagerTest::TearDown");
        SaTaskManagerTest::result_ = 0;
    }

    static bool TestSamePriorityTaskOrdering(SaTaskCode codeTypeBegin);
    static bool TestSameInterrupt(SaTaskCode codeTypeBegin, bool isPauseTimeout);
    static bool TestLowInterruptHigh(SaTaskCode higherTaskCode, SaTaskCode lowerTaskCode, bool isPauseTimeout);
    static bool TestHighInterruptLow(SaTaskCode higherTaskCode, SaTaskCode lowerTaskCode, bool isPauseTimeout);

    static bool TestPauseAndExec(SaTaskCode pausedTaskCode, SaTaskCode newTaskCode, bool isPauseTimeout);
    static bool TestPauseAndExecWhiteListRequest(
        SaTaskCode pausedTaskCode, SaTaskCode newTaskCode, bool isPauseTimeout);
    static bool TestPauseAndExecNonWhiteListRequest(
        SaTaskCode pausedTaskCode, SaTaskCode newTaskCode, bool isPauseTimeout);

    static void SetResult(uint32_t result);
    static uint32_t GetResult();
    static int32_t StartPause(const PauseInfo &info, uint32_t timeoutMs);
    static std::mutex mtx_;
    static uint32_t result_;
};
uint32_t SaTaskManagerTest::result_{ 0 };
std::mutex SaTaskManagerTest::mtx_{};

bool SaTaskManagerTest::TestSamePriorityTaskOrdering(SaTaskCode codeTypeBegin)
{
    result_ = CONSTANTS_1;
    uint32_t taskCode1 = static_cast<uint32_t>(codeTypeBegin) + 1;
    uint32_t taskCode2 = static_cast<uint32_t>(codeTypeBegin) + 2;
    IMSA_HILOGI("taskCode1: %{public}u, taskCode2: %{public}u", taskCode1, taskCode2);
    auto action1 = [](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec task1 start");
        result_ += CONSTANTS_5;
        IMSA_HILOGI("exec task1 end");
        return ErrorCode::NO_ERROR;
    };
    auto task1 = std::make_shared<SaTask>(static_cast<SaTaskCode>(taskCode1), action1);
    auto action2 = [](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec task2 start");
        result_ *= CONSTANTS_2;
        IMSA_HILOGI("exec task2 end");
        return ErrorCode::NO_ERROR;
    };
    auto task2 = std::make_shared<SaTask>(static_cast<SaTaskCode>(taskCode2), action2);
    SaTaskManager::GetInstance().PostTask(task1);
    SaTaskManager::GetInstance().PostTask(task2);
    usleep(WAIT_EXEC_END);
    return result_ == CONSTANTS_12; // (1 + 5) * 2 = 12
}

bool SaTaskManagerTest::TestSameInterrupt(SaTaskCode codeTypeBegin, bool isPauseTimeout)
{
    result_ = 0;
    uint32_t taskCode1 = static_cast<uint32_t>(codeTypeBegin) + 1;
    uint32_t taskCode2 = static_cast<uint32_t>(codeTypeBegin) + 2;
    IMSA_HILOGI("taskCode1: %{public}u, taskCode2: %{public}u", taskCode1, taskCode2);
    PauseInfo info = { .type = PauseType::PAUSE_TYPE_START_IME, .target = "TestLowInterruptHigh" };
    auto action1 = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        result_ += CONSTANTS_1;
        IMSA_HILOGI("exec task1 start");
        return StartPause(info, PAUSE_TIMEOUT);
    };
    auto task1 = std::make_shared<SaTask>(static_cast<SaTaskCode>(taskCode1), action1);

    auto action2 = [](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec task2 start");
        result_ *= 2;
        IMSA_HILOGI("exec task2 end");
        return ErrorCode::NO_ERROR;
    };
    auto task2 = std::make_shared<SaTask>(static_cast<SaTaskCode>(taskCode2), action2);

    auto resumeAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec resumeTask start");
        CallerInfo callerInfo = { .bundleName = info.target };
        SaTaskManager::GetInstance().TryResume(info.type, callerInfo);
        IMSA_HILOGI("exec resumeTask end");
        return ErrorCode::NO_ERROR;
    };
    auto resumeTask = std::make_shared<SaTask>(SaTaskCode::SET_CORE_AND_AGENT, resumeAction);
    SaTaskManager::GetInstance().PostTask(task1);
    SaTaskManager::GetInstance().PostTask(task2);
    if (isPauseTimeout) {
        SaTaskManager::GetInstance().PostTask(resumeTask, DELAY_TIME);
    } else {
        SaTaskManager::GetInstance().PostTask(resumeTask);
    }
    usleep(WAIT_PAUSE_EXEC_END);
    uint32_t expectValue = 0;
    if (isPauseTimeout) {
        expectValue = (CONSTANTS_1 + FAILED_RESULT) * CONSTANTS_2;
    } else {
        expectValue = (CONSTANTS_1 + SUCCESS_RESULT) * CONSTANTS_2;
    }
    return result_ == expectValue;
}

bool SaTaskManagerTest::TestLowInterruptHigh(SaTaskCode higherTaskCode, SaTaskCode lowerTaskCode, bool isPauseTimeout)
{
    result_ = 0;
    IMSA_HILOGI("higherTaskCode: %{public}u, lowerTaskCode: %{public}u", static_cast<uint32_t>(higherTaskCode),
        static_cast<uint32_t>(lowerTaskCode));
    PauseInfo info = { .type = PauseType::PAUSE_TYPE_START_IME, .target = "TestLowInterruptHigh" };
    // create higher task
    auto higherAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        result_ += CONSTANTS_1;
        IMSA_HILOGI("exec higherTask start");
        return StartPause(info, PAUSE_TIMEOUT);
    };
    auto higherTask = std::make_shared<SaTask>(higherTaskCode, higherAction);

    // create lower task
    auto lowerAction = [](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec lowerTask start");
        result_ *= CONSTANTS_2;
        IMSA_HILOGI("exec lowerTask end");
        return ErrorCode::NO_ERROR;
    };
    auto lowerTask = std::make_shared<SaTask>(lowerTaskCode, lowerAction);

    // create higher task's resume task
    auto resumeAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec resumeTask start");
        CallerInfo callerInfo = { .bundleName = info.target };
        SaTaskManager::GetInstance().TryResume(info.type, callerInfo);
        IMSA_HILOGI("exec resumeTask end");
        return ErrorCode::NO_ERROR;
    };
    auto resumeTask = std::make_shared<SaTask>(SaTaskCode::SET_CORE_AND_AGENT, resumeAction);

    // post order: higherTask -> lowerTask -> resumeTask
    SaTaskManager::GetInstance().PostTask(higherTask);
    SaTaskManager::GetInstance().PostTask(lowerTask);
    if (isPauseTimeout) {
        SaTaskManager::GetInstance().PostTask(resumeTask, DELAY_TIME);
    } else {
        SaTaskManager::GetInstance().PostTask(resumeTask);
    }

    // expect exec order: higherTask -> resumeTask -> lowerTask
    uint32_t expectValue = 0;
    if (isPauseTimeout) {
        expectValue = (CONSTANTS_1 + FAILED_RESULT) * CONSTANTS_2;
    } else {
        expectValue = (CONSTANTS_1 + SUCCESS_RESULT) * CONSTANTS_2;
    }
    usleep(WAIT_PAUSE_EXEC_END);
    return result_ == expectValue;
}

bool SaTaskManagerTest::TestHighInterruptLow(SaTaskCode higherTaskCode, SaTaskCode lowerTaskCode, bool isPauseTimeout)
{
    result_ = 0;
    IMSA_HILOGI("higherTaskCode: %{public}u, lowerTaskCode: %{public}u", static_cast<uint32_t>(higherTaskCode),
        static_cast<uint32_t>(lowerTaskCode));
    PauseInfo info = { .type = PauseType::PAUSE_TYPE_START_IME, .target = "TestLowInterruptHigh" };
    auto lowerAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        result_ += CONSTANTS_1;
        IMSA_HILOGI("exec lowerTask start");
        return StartPause(info, PAUSE_TIMEOUT);
    };
    auto lowerTask = std::make_shared<SaTask>(static_cast<SaTaskCode>(lowerTaskCode), lowerAction);

    auto higherAction = [](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec higherTask start");
        result_ *= CONSTANTS_2;
        IMSA_HILOGI("exec higherTask end");
        return ErrorCode::NO_ERROR;
    };
    auto higherTask = std::make_shared<SaTask>(static_cast<SaTaskCode>(higherTaskCode), higherAction);

    auto resumeAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec resumeTask start");
        CallerInfo callerInfo = { .bundleName = info.target };
        SaTaskManager::GetInstance().TryResume(info.type, callerInfo);
        IMSA_HILOGI("exec resumeTask end");
        return ErrorCode::NO_ERROR;
    };
    auto resumeTask = std::make_shared<SaTask>(SaTaskCode::SET_CORE_AND_AGENT, resumeAction);

    // post order: lowerTask -> higherTask -> resumeTask
    SaTaskManager::GetInstance().PostTask(lowerTask);
    SaTaskManager::GetInstance().PostTask(higherTask);
    if (isPauseTimeout) {
        SaTaskManager::GetInstance().PostTask(resumeTask, DELAY_TIME);
    } else {
        SaTaskManager::GetInstance().PostTask(resumeTask);
    }

    // expect exec order: lowerTask -> higherTask
    uint32_t expectValue = CONSTANTS_1 * CONSTANTS_2;
    usleep(WAIT_PAUSE_EXEC_END);
    return result_ == expectValue;
}

bool SaTaskManagerTest::TestPauseAndExec(SaTaskCode pausedTaskCode, SaTaskCode newTaskCode, bool isPauseTimeout)
{
    result_ = 0;
    IMSA_HILOGI("SaTaskManagerTest TestPauseAndPermitExecute001 START");
    PauseInfo info = { .type = PauseType::PAUSE_TYPE_START_IME, .target = "TestPostTask006" };
    auto pauseAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec pauseTask start");
        SaTaskManagerTest::result_ = CONSTANTS_1;
        return SaTaskManagerTest::StartPause(info, PAUSE_TIMEOUT);
    };
    auto pauseTask = std::make_shared<SaTask>(pausedTaskCode, pauseAction);

    auto newAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec newTask start");
        SaTaskManagerTest::result_ *= CONSTANTS_2;
        IMSA_HILOGI("exec newTask end");
        return ErrorCode::NO_ERROR;
    };
    auto newTask = std::make_shared<SaTask>(newTaskCode, newAction);

    auto resumeAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec resumeTask start");
        CallerInfo callerInfo = { .bundleName = info.target };
        SaTaskManager::GetInstance().TryResume(info.type, callerInfo);
        IMSA_HILOGI("exec resumeTask end");
        return ErrorCode::NO_ERROR;
    };
    auto resumeTask = std::make_shared<SaTask>(SaTaskCode::SET_CORE_AND_AGENT, resumeAction);

    SaTaskManager::GetInstance().PostTask(pauseTask);
    SaTaskManager::GetInstance().PostTask(newTask);
    if (isPauseTimeout) {
        SaTaskManager::GetInstance().PostTask(resumeTask, DELAY_TIME);
    } else {
        SaTaskManager::GetInstance().PostTask(resumeTask);
    }

    uint32_t expectedValue = 0;
    if (isPauseTimeout) {
        expectedValue = (CONSTANTS_1 * CONSTANTS_2) + FAILED_RESULT;
    } else {
        expectedValue = (CONSTANTS_1 * CONSTANTS_2) + SUCCESS_RESULT;
    }
    usleep(WAIT_PAUSE_EXEC_END);
    return result_ == expectedValue;
}

bool SaTaskManagerTest::TestPauseAndExecWhiteListRequest(
    SaTaskCode pausedTaskCode, SaTaskCode newTaskCode, bool isPauseTimeout)
{
    result_ = 0;
    IMSA_HILOGI("SaTaskManagerTest TestPauseAndPermitExecute001 START");
    PauseInfo info = { .type = PauseType::PAUSE_TYPE_START_IME, .target = "TestPostTask006" };
    auto pauseAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec pauseTask start");
        SaTaskManagerTest::result_ = CONSTANTS_1;
        return SaTaskManagerTest::StartPause(info, PAUSE_TIMEOUT);
    };
    auto pauseTask = std::make_shared<SaTask>(pausedTaskCode, pauseAction);

    auto newAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec newTask start");
        SaTaskManagerTest::result_ *= CONSTANTS_2;
        IMSA_HILOGI("exec newTask end");
        return ErrorCode::NO_ERROR;
    };
    CallerInfo callerInfo = { .bundleName = info.target };
    IdentityCheckerMock::SetBundleName(info.target);
    auto newTask = std::make_shared<SaTask>(newTaskCode, newAction, callerInfo);

    auto resumeAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec resumeTask start");
        CallerInfo callerInfo = { .bundleName = info.target };
        SaTaskManager::GetInstance().TryResume(info.type, callerInfo);
        IMSA_HILOGI("exec resumeTask end");
        return ErrorCode::NO_ERROR;
    };
    auto resumeTask = std::make_shared<SaTask>(SaTaskCode::SET_CORE_AND_AGENT, resumeAction);

    SaTaskManager::GetInstance().PostTask(pauseTask);
    SaTaskManager::GetInstance().PostTask(newTask);
    if (isPauseTimeout) {
        SaTaskManager::GetInstance().PostTask(resumeTask, DELAY_TIME);
    } else {
        SaTaskManager::GetInstance().PostTask(resumeTask);
    }

    uint32_t expectedValue = 0;
    if (isPauseTimeout) {
        expectedValue = (CONSTANTS_1 * CONSTANTS_2) + FAILED_RESULT;
    } else {
        expectedValue = (CONSTANTS_1 * CONSTANTS_2) + SUCCESS_RESULT;
    }
    usleep(WAIT_PAUSE_EXEC_END);
    return result_ == expectedValue;
}

bool SaTaskManagerTest::TestPauseAndExecNonWhiteListRequest(
    SaTaskCode pausedTaskCode, SaTaskCode newTaskCode, bool isPauseTimeout)
{
    result_ = 0;
    IMSA_HILOGI("SaTaskManagerTest TestPauseAndPermitExecute001 START");
    PauseInfo info = { .type = PauseType::PAUSE_TYPE_START_IME, .target = "targetBundleName" };
    auto pauseAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec pauseTask start");
        SaTaskManagerTest::result_ = CONSTANTS_1;
        return SaTaskManagerTest::StartPause(info, PAUSE_TIMEOUT);
    };
    auto pauseTask = std::make_shared<SaTask>(pausedTaskCode, pauseAction);

    auto newAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec newTask start");
        SaTaskManagerTest::result_ *= CONSTANTS_2;
        IMSA_HILOGI("exec newTask end");
        return ErrorCode::NO_ERROR;
    };
    CallerInfo callerInfo = { .bundleName = info.target };
    IdentityCheckerMock::SetBundleName(info.target);
    auto newTask = std::make_shared<SaTask>(newTaskCode, newAction, callerInfo);

    auto resumeAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec resumeTask start");
        CallerInfo callerInfo = { .bundleName = info.target };
        SaTaskManager::GetInstance().TryResume(info.type, callerInfo);
        IMSA_HILOGI("exec resumeTask end");
        return ErrorCode::NO_ERROR;
    };
    auto resumeTask = std::make_shared<SaTask>(SaTaskCode::SET_CORE_AND_AGENT, resumeAction);

    SaTaskManager::GetInstance().PostTask(pauseTask);
    SaTaskManager::GetInstance().PostTask(newTask);
    if (isPauseTimeout) {
        SaTaskManager::GetInstance().PostTask(resumeTask, DELAY_TIME);
    } else {
        SaTaskManager::GetInstance().PostTask(resumeTask);
    }

    uint32_t expectedValue = 0;
    if (isPauseTimeout) {
        expectedValue = CONSTANTS_1 + FAILED_RESULT;
    } else {
        expectedValue = CONSTANTS_1 + SUCCESS_RESULT;
    }
    usleep(WAIT_PAUSE_EXEC_END);
    return result_ == expectedValue;
}

void SaTaskManagerTest::SetResult(uint32_t result)
{
    std::lock_guard<std::mutex> lock(mtx_);
    result_ = result;
    IMSA_HILOGI("result: %{public}u", result_);
}

uint32_t SaTaskManagerTest::GetResult()
{
    std::lock_guard<std::mutex> lock(mtx_);
    IMSA_HILOGI("result: %{public}u", result_);
    return result_;
}

int32_t SaTaskManagerTest::StartPause(const PauseInfo &info, uint32_t timeoutMs)
{
    SaActionFunc onComplete = [](ServiceResponseData &, ActionInnerData &) {
        IMSA_HILOGI("onComplete");
        result_ += SUCCESS_RESULT;
        return ErrorCode::NO_ERROR;
    };
    SaActionFunc onTimeout = [](ServiceResponseData &, ActionInnerData &) {
        IMSA_HILOGI("onTimeout");
        result_ += FAILED_RESULT;
        return ErrorCode::ERROR_IMSA_IME_START_TIMEOUT;
    };
    auto waitAction = std::make_unique<SaActionWait>(timeoutMs, info, onComplete, onTimeout);
    return SaTaskManager::GetInstance().WaitExec(std::move(waitAction));
}

/**
 * @tc.name: TestSamePriorityOrdering
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(SaTaskManagerTest, TestSamePriorityOrdering, TestSize.Level0)
{
    IMSA_HILOGI("SaTaskManagerTest TestSamePriorityOrdering START");
    EXPECT_TRUE(TestSamePriorityTaskOrdering(SaTaskCode::TASK_CRITICAL_CHANGE_BEGIN));
    EXPECT_TRUE(TestSamePriorityTaskOrdering(SaTaskCode::TASK_SWITCH_IME_BEGIN));
    EXPECT_TRUE(TestSamePriorityTaskOrdering(SaTaskCode::TASK_HIGHER_REQUEST_BEGIN));
    EXPECT_TRUE(TestSamePriorityTaskOrdering(SaTaskCode::TASK_NORMAL_REQUEST_BEGIN));
    EXPECT_TRUE(TestSamePriorityTaskOrdering(SaTaskCode::TASK_RESUME_BEGIN));
    EXPECT_TRUE(TestSamePriorityTaskOrdering(SaTaskCode::TASK_QUERY_BEGIN));
}

/**
 * @tc.name: TestPauseAndResume001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(SaTaskManagerTest, TestPauseAndResume001, TestSize.Level0)
{
    IMSA_HILOGI("SaTaskManagerTest TestPauseAndResume001 START");
    PauseInfo info = { .type = PauseType::PAUSE_TYPE_START_IME, .target = "TestPostTask001" };
    auto pauseAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec pauseAction start");
        return SaTaskManagerTest::StartPause(info, PAUSE_TIMEOUT);
    };
    auto pauseTask = std::make_shared<SaTask>(SaTaskCode::SWITCH_INPUT_METHOD, pauseAction);
    auto resumeAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec resumeAction start");
        CallerInfo callerInfo = { .bundleName = info.target };
        SaTaskManager::GetInstance().TryResume(PauseType::PAUSE_TYPE_START_IME, callerInfo);
        IMSA_HILOGI("exec resumeAction end");
        return ErrorCode::NO_ERROR;
    };
    auto resumeTask = std::make_shared<SaTask>(SaTaskCode::SET_CORE_AND_AGENT, resumeAction);
    SaTaskManager::GetInstance().PostTask(pauseTask);
    SaTaskManager::GetInstance().PostTask(resumeTask);
    usleep(WAIT_PAUSE_EXEC_END);
    EXPECT_EQ(SaTaskManagerTest::GetResult(), SUCCESS_RESULT);
}

/**
 * @tc.name: TestPauseAndResume002
 * @tc.desc: resume timeout
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(SaTaskManagerTest, TestPauseAndResume002, TestSize.Level0)
{
    IMSA_HILOGI("SaTaskManagerTest TestPauseAndResume002 START");
    PauseInfo info = { .type = PauseType::PAUSE_TYPE_START_IME, .target = "TestPostTask001" };
    auto pauseAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec pauseAction start");
        return SaTaskManagerTest::StartPause(info, PAUSE_TIMEOUT);
    };
    auto pauseTask = std::make_shared<SaTask>(SaTaskCode::SWITCH_INPUT_METHOD, pauseAction);
    auto resumeAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec resumeAction start");
        CallerInfo callerInfo = { .bundleName = info.target };
        SaTaskManager::GetInstance().TryResume(PauseType::PAUSE_TYPE_START_IME, callerInfo);
        IMSA_HILOGI("exec resumeAction end");
        return ErrorCode::NO_ERROR;
    };
    auto resumeTask = std::make_shared<SaTask>(SaTaskCode::SET_CORE_AND_AGENT, resumeAction);
    SaTaskManager::GetInstance().PostTask(pauseTask);
    SaTaskManager::GetInstance().PostTask(resumeTask, DELAY_TIME);
    usleep(WAIT_PAUSE_EXEC_END);
    EXPECT_EQ(SaTaskManagerTest::GetResult(), FAILED_RESULT);
}

/**
 * @tc.name: TestPauseAndResume003
 * @tc.desc: resume failed
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(SaTaskManagerTest, TestPauseAndResume003, TestSize.Level0)
{
    IMSA_HILOGI("SaTaskManagerTest TestPauseAndResume003 START");
    PauseInfo info = { .type = PauseType::PAUSE_TYPE_START_IME, .target = "TestPostTask001" };
    auto pauseAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec pauseAction start");
        return SaTaskManagerTest::StartPause(info, PAUSE_TIMEOUT);
    };
    auto pauseTask = std::make_shared<SaTask>(SaTaskCode::SWITCH_INPUT_METHOD, pauseAction);
    auto resumeAction = [info](ServiceResponseData &, ActionInnerData &) -> int32_t {
        IMSA_HILOGI("exec resumeAction start");
        CallerInfo callerInfo = { .bundleName = "invalidValue" };
        SaTaskManager::GetInstance().TryResume(PauseType::PAUSE_TYPE_START_IME, callerInfo);
        IMSA_HILOGI("exec resumeAction end");
        return ErrorCode::NO_ERROR;
    };
    auto resumeTask = std::make_shared<SaTask>(SaTaskCode::SET_CORE_AND_AGENT, resumeAction);
    SaTaskManager::GetInstance().PostTask(pauseTask);
    SaTaskManager::GetInstance().PostTask(resumeTask);
    usleep(WAIT_PAUSE_EXEC_END);
    EXPECT_EQ(SaTaskManagerTest::GetResult(), FAILED_RESULT);
}

/**
 * @tc.name: TestPauseAndInterrupt001
 * @tc.desc: Same priority can not interrupt each other. Post order: task1->task2. Exec order: task1->task2.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(SaTaskManagerTest, TestPauseAndInterrupt001, TestSize.Level0)
{
    IMSA_HILOGI("SaTaskManagerTest TestPauseAndInterrupt001 START");
    EXPECT_TRUE(TestSameInterrupt(SaTaskCode::TASK_CRITICAL_CHANGE_BEGIN, true));
    EXPECT_TRUE(TestSameInterrupt(SaTaskCode::TASK_CRITICAL_CHANGE_BEGIN, false));

    EXPECT_TRUE(TestSameInterrupt(SaTaskCode::TASK_SWITCH_IME_BEGIN, true));
    EXPECT_TRUE(TestSameInterrupt(SaTaskCode::TASK_SWITCH_IME_BEGIN, false));

    EXPECT_TRUE(TestSameInterrupt(SaTaskCode::ON_FOCUSED, true));
    EXPECT_TRUE(TestSameInterrupt(SaTaskCode::ON_FOCUSED, false));

    EXPECT_TRUE(TestSameInterrupt(SaTaskCode::START_INPUT, true));
    EXPECT_TRUE(TestSameInterrupt(SaTaskCode::START_INPUT, false));
}

/**
 * @tc.name: TestPauseAndInterrupt002
 * @tc.desc: Post order: higher[paused]->lower->resume. Exec order: higher[paused]->resume->higher[resumed]->lowerTask
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(SaTaskManagerTest, TestPauseAndInterrupt002, TestSize.Level0)
{
    IMSA_HILOGI("SaTaskManagerTest TestPauseAndInterrupt002 START");
    // critical task paused, can not be interrupted by lower task.
    EXPECT_TRUE(TestLowInterruptHigh(SaTaskCode::ON_WMS_SA_STARTED, SaTaskCode::SWITCH_INPUT_METHOD, true));
    EXPECT_TRUE(TestLowInterruptHigh(SaTaskCode::ON_WMS_SA_STARTED, SaTaskCode::SWITCH_INPUT_METHOD, false));

    EXPECT_TRUE(TestLowInterruptHigh(SaTaskCode::ON_WMS_SA_STARTED, SaTaskCode::ON_FOCUSED, true));
    EXPECT_TRUE(TestLowInterruptHigh(SaTaskCode::ON_WMS_SA_STARTED, SaTaskCode::ON_FOCUSED, false));

    EXPECT_TRUE(TestLowInterruptHigh(SaTaskCode::ON_WMS_SA_STARTED, SaTaskCode::START_INPUT, true));
    EXPECT_TRUE(TestLowInterruptHigh(SaTaskCode::ON_WMS_SA_STARTED, SaTaskCode::START_INPUT, false));

    // switch ime task paused, can not be interrupted by lower task.
    EXPECT_TRUE(TestLowInterruptHigh(SaTaskCode::SWITCH_INPUT_METHOD, SaTaskCode::ON_FOCUSED, true));
    EXPECT_TRUE(TestLowInterruptHigh(SaTaskCode::SWITCH_INPUT_METHOD, SaTaskCode::ON_FOCUSED, false));

    EXPECT_TRUE(TestLowInterruptHigh(SaTaskCode::SWITCH_INPUT_METHOD, SaTaskCode::START_INPUT, true));
    EXPECT_TRUE(TestLowInterruptHigh(SaTaskCode::SWITCH_INPUT_METHOD, SaTaskCode::START_INPUT, false));
}

/**
 * @tc.name: TestPauseAndInterrupt003
 * @tc.desc: Post order: lower[paused]->higher->resume, Execute order: lower[paused]->higher->end.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(SaTaskManagerTest, TestPauseAndInterrupt003, TestSize.Level0)
{
    // critical task can interrupt other paused task
    EXPECT_TRUE(TestHighInterruptLow(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::SWITCH_INPUT_METHOD, true));
    EXPECT_TRUE(TestHighInterruptLow(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::SWITCH_INPUT_METHOD, false));

    EXPECT_TRUE(TestHighInterruptLow(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::ON_FOCUSED, true));
    EXPECT_TRUE(TestHighInterruptLow(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::ON_FOCUSED, false));

    EXPECT_TRUE(TestHighInterruptLow(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::START_INPUT, true));
    EXPECT_TRUE(TestHighInterruptLow(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::START_INPUT, false));
}

/**
 * @tc.name: TestPauseAndPermitExecute001
 * @tc.desc: QUERY tasks can be executed when curTask_ is paused.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(SaTaskManagerTest, TestPauseAndPermitExecute001, TestSize.Level0)
{
    IMSA_HILOGI("SaTaskManagerTest TestPauseAndPermitExecute001 START");
    EXPECT_TRUE(TestPauseAndExec(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::GET_CURRENT_INPUT_METHOD, true));
    EXPECT_TRUE(TestPauseAndExec(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::GET_CURRENT_INPUT_METHOD, false));

    EXPECT_TRUE(TestPauseAndExec(SaTaskCode::SWITCH_INPUT_METHOD, SaTaskCode::GET_CURRENT_INPUT_METHOD, true));
    EXPECT_TRUE(TestPauseAndExec(SaTaskCode::SWITCH_INPUT_METHOD, SaTaskCode::GET_CURRENT_INPUT_METHOD, false));

    EXPECT_TRUE(TestPauseAndExec(SaTaskCode::START_INPUT, SaTaskCode::GET_CURRENT_INPUT_METHOD, true));
    EXPECT_TRUE(TestPauseAndExec(SaTaskCode::START_INPUT, SaTaskCode::GET_CURRENT_INPUT_METHOD, false));
}

/**
 * @tc.name: TestPauseAndPermitExecute002
 * @tc.desc: RESUME tasks can be executed when curTask_ is paused.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(SaTaskManagerTest, TestPauseAndPermitExecute002, TestSize.Level0)
{
    IMSA_HILOGI("SaTaskManagerTest TestPauseAndPermitExecute002 START");
    EXPECT_TRUE(TestPauseAndExec(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::SET_CORE_AND_AGENT, true));
    EXPECT_TRUE(TestPauseAndExec(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::SET_CORE_AND_AGENT, false));

    EXPECT_TRUE(TestPauseAndExec(SaTaskCode::SWITCH_INPUT_METHOD, SaTaskCode::SET_CORE_AND_AGENT, true));
    EXPECT_TRUE(TestPauseAndExec(SaTaskCode::SWITCH_INPUT_METHOD, SaTaskCode::SET_CORE_AND_AGENT, false));

    EXPECT_TRUE(TestPauseAndExec(SaTaskCode::START_INPUT, SaTaskCode::SET_CORE_AND_AGENT, true));
    EXPECT_TRUE(TestPauseAndExec(SaTaskCode::START_INPUT, SaTaskCode::SET_CORE_AND_AGENT, false));
}

/**
 * @tc.name: TestPauseAndPermitExecute003
 * @tc.desc: REQUEST tasks from target app in WHITE_LIST can be executed when curTask_ is paused.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(SaTaskManagerTest, TestPauseAndPermitExecute003, TestSize.Level0)
{
    IMSA_HILOGI("SaTaskManagerTest TestPauseAndPermitExecute003 START");
    EXPECT_TRUE(
        TestPauseAndExecWhiteListRequest(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::UPDATE_LISTEN_EVENT_FLAG, true));
    EXPECT_TRUE(
        TestPauseAndExecWhiteListRequest(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::UPDATE_LISTEN_EVENT_FLAG, false));

    EXPECT_TRUE(
        TestPauseAndExecWhiteListRequest(SaTaskCode::SWITCH_INPUT_METHOD, SaTaskCode::UPDATE_LISTEN_EVENT_FLAG, true));
    EXPECT_TRUE(
        TestPauseAndExecWhiteListRequest(SaTaskCode::SWITCH_INPUT_METHOD, SaTaskCode::UPDATE_LISTEN_EVENT_FLAG, false));

    EXPECT_TRUE(TestPauseAndExecWhiteListRequest(SaTaskCode::START_INPUT, SaTaskCode::UPDATE_LISTEN_EVENT_FLAG, true));
    EXPECT_TRUE(TestPauseAndExecWhiteListRequest(SaTaskCode::START_INPUT, SaTaskCode::UPDATE_LISTEN_EVENT_FLAG, false));
}

/**
 * @tc.name: TestPauseAndPermitExecute004
 * @tc.desc: REQUEST tasks from target app not in WHITE_LIST will be dropped when curTask_ is paused.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(SaTaskManagerTest, TestPauseAndPermitExecute004, TestSize.Level0)
{
    IMSA_HILOGI("SaTaskManagerTest TestPauseAndPermitExecute004 START");
    EXPECT_TRUE(TestPauseAndExecNonWhiteListRequest(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::START_INPUT, true));
    EXPECT_TRUE(TestPauseAndExecNonWhiteListRequest(SaTaskCode::ON_WMS_CONNECTED, SaTaskCode::START_INPUT, false));

    EXPECT_TRUE(TestPauseAndExecNonWhiteListRequest(SaTaskCode::SWITCH_INPUT_METHOD, SaTaskCode::START_INPUT, true));
    EXPECT_TRUE(TestPauseAndExecNonWhiteListRequest(SaTaskCode::SWITCH_INPUT_METHOD, SaTaskCode::START_INPUT, false));

    EXPECT_TRUE(TestPauseAndExecNonWhiteListRequest(SaTaskCode::START_INPUT, SaTaskCode::SHOW_INPUT, true));
    EXPECT_TRUE(TestPauseAndExecNonWhiteListRequest(SaTaskCode::START_INPUT, SaTaskCode::SHOW_INPUT, false));
}
} // namespace MiscServices
} // namespace OHOS
