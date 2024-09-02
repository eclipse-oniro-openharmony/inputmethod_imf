/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
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
#include "task_manager.h"

#include "actions/action_wait.h"
#include "tasks/task_ams.h"
#include "tasks/task_inner.h"
#undef private
#undef protected

#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include "global.h"

namespace OHOS {
namespace MiscServices {

using namespace testing::ext;

class TaskManagerTest : public testing::Test {
public:
    void SetUp() override
    {
        IMSA_HILOGI("TaskManagerTest::SetUp");
        mgr = std::make_unique<TaskManager>();
    }

    void TearDown() override
    {
        IMSA_HILOGI("TaskManagerTest::TearDown");
        mgr.reset();
    }

public:
    std::unique_ptr<TaskManager> mgr;
};

/**
 * @tc.name: PostTask_001
 * @tc.desc: Post a null task
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, PostTask_001, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest PostTask_001 START");
    std::shared_ptr<Task> task { nullptr };
    auto ret = mgr->PostTask(task);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: PostTask_002
 * @tc.desc: Post an AmsInit task
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, PostTask_002, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest PostTask_002 START");
    std::shared_ptr<Task> task = std::make_shared<TaskAmsInit>();
    auto ret = mgr->PostTask(task);
    EXPECT_EQ(ret, task->GetSeqId());

    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(mgr->amsTasks_.size(), 0);
    EXPECT_EQ(mgr->curTask_, task);
    EXPECT_EQ(mgr->curTask_->GetSeqId(), ret);
}

/**
 * @tc.name: Complete_001
 * @tc.desc: Call TaskManager::Complete with seqId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, Complete_001, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest Complete_001 START");

    EXPECT_EQ(mgr->curTask_, nullptr);
    auto seqId = Task::GetNextSeqId();
    mgr->Complete(seqId);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(mgr->innerTasks_.size(), 1);
    EXPECT_EQ(mgr->innerTasks_.front()->GetSeqId(), seqId);
}

/**
 * @tc.name: Pend_001
 * @tc.desc: Pend a NULL action
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, Pend_001, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest Pend_001 START");
    std::unique_ptr<ActionWait> action { nullptr };
    auto ret = mgr->Pend(std::move(action));
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: Pend_002
 * @tc.desc: Pend a action while curTask_ is NULL or not running
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, Pend_002, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest Pend_002 START");
    EXPECT_EQ(mgr->curTask_, nullptr);

    std::unique_ptr<ActionWait> action = std::make_unique<ActionWait>(Task::GetNextSeqId(), 1000);
    auto ret = mgr->Pend(std::move(action));
    EXPECT_EQ(ret, ErrorCode::ERROR_TASK_MANAGER_PEND_FAILED);

    mgr->curTask_ = std::make_shared<TaskAmsInit>();
    action = std::make_unique<ActionWait>(0, 1000);
    ret = mgr->Pend(std::move(action));
    EXPECT_EQ(ret, ErrorCode::ERROR_TASK_MANAGER_PEND_FAILED);
}

/**
 * @tc.name: Pend_003
 * @tc.desc: Pend action success
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, Pend_003, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest Pend_003 START");
    EXPECT_EQ(mgr->curTask_, nullptr);

    mgr->curTask_ = std::make_shared<TaskAmsInit>();
    mgr->curTask_->state_ = RUNNING_STATE_RUNNING;

    auto action = std::make_unique<ActionWait>(Task::GetNextSeqId(), 1000);
    auto ret = mgr->Pend(std::move(action));
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(mgr->curTask_->pendingActions_.size(), 1);
}

/**
 * @tc.name: WaitExec_001
 * @tc.desc: Pend action success
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, WaitExec_001, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest WaitExec_001 START");
    EXPECT_EQ(mgr->curTask_, nullptr);

    auto seqId = Task::GetNextSeqId();
    bool flag = false;
    auto func = [&flag] {
        flag = true;
    };

    auto ret = mgr->WaitExec(seqId, 1000, func);
    EXPECT_EQ(ret, ErrorCode::ERROR_TASK_MANAGER_PEND_FAILED);
}

/**
 * @tc.name: WaitExec_002
 * @tc.desc: Pend action success
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, WaitExec_002, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest WaitExec_002 START");
    EXPECT_EQ(mgr->curTask_, nullptr);

    auto task = std::make_shared<TaskAmsInit>();
    task->state_ = RUNNING_STATE_RUNNING;
    mgr->curTask_ = task;

    EXPECT_EQ(mgr->curTask_->pendingActions_.size(), 0);

    auto seqId = Task::GetNextSeqId();
    bool flag = false;
    auto func = [&flag] {
        flag = true;
    };
    auto ret = mgr->WaitExec(seqId, 1000, func);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(mgr->curTask_->pendingActions_.size(), 2);
}

/**
 * @tc.name: OnNewTask_001
 * @tc.desc: OnNewTask got empty task
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, OnNewTask_001, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest OnNewTask_001 START");

    std::shared_ptr<Task> task { nullptr };
    mgr->OnNewTask(task);
    EXPECT_EQ(mgr->amsTasks_.size(), 0);
    EXPECT_EQ(mgr->imaTasks_.size(), 0);
    EXPECT_EQ(mgr->imsaTasks_.size(), 0);
    EXPECT_EQ(mgr->innerTasks_.size(), 0);
}

/**
 * @tc.name: OnNewTask_002
 * @tc.desc: OnNewTask got a valid task
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, OnNewTask_002, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest OnNewTask_002 START");

    EXPECT_EQ(mgr->curTask_, nullptr);
    EXPECT_EQ(mgr->amsTasks_.size(), 0);
    EXPECT_EQ(mgr->imaTasks_.size(), 0);
    EXPECT_EQ(mgr->imsaTasks_.size(), 0);
    EXPECT_EQ(mgr->innerTasks_.size(), 0);

    std::shared_ptr<Task> task = std::make_shared<TaskAmsInit>();
    mgr->OnNewTask(task);

    EXPECT_EQ(mgr->amsTasks_.size(), 0);
    EXPECT_EQ(mgr->curTask_, task);
}

/**
 * @tc.name: ProcessNextInnerTask_001
 * @tc.desc: ProcessNextInnerTask while curTask_ is NULL
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, ProcessNextInnerTask_001, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest ProcessNextInnerTask_001 START");

    mgr->innerTasks_ = {
        std::make_shared<TaskResume>(Task::GetNextSeqId()),
        std::make_shared<TaskResume>(Task::GetNextSeqId()),
        std::make_shared<TaskResume>(Task::GetNextSeqId()),
    };

    mgr->ProcessNextInnerTask();
    EXPECT_EQ(mgr->innerTasks_.size(), 3);
}

/**
 * @tc.name: ProcessNextInnerTask_002
 * @tc.desc: ProcessNextInnerTask resume success
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, ProcessNextInnerTask_002, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest ProcessNextInnerTask_002 START");

    auto task = std::make_shared<TaskAmsInit>();
    mgr->OnNewTask(task);
    EXPECT_EQ(mgr->curTask_, task);
    EXPECT_EQ(mgr->curTask_->state_, RUNNING_STATE_PAUSED);

    mgr->innerTasks_ = {
        std::make_shared<TaskResume>(task->GetSeqId()),
    };
    mgr->ProcessNextInnerTask();
    EXPECT_EQ(mgr->curTask_, nullptr);
    EXPECT_EQ(task->GetState(), RUNNING_STATE_COMPLETED);
    EXPECT_EQ(mgr->innerTasks_.size(), 0);
}

/**
 * @tc.name: ProcessNextAmsTask_001
 * @tc.desc: ProcessNextAmsTask while curTask_ is not NULL
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, ProcessNextAmsTask_001, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest ProcessNextAmsTask_001 START");

    mgr->curTask_ = std::make_shared<TaskResume>(Task::GetNextSeqId());
    auto task = std::make_shared<TaskAmsInit>();
    mgr->amsTasks_.push_back(task);
    mgr->ProcessNextAmsTask();
    EXPECT_EQ(mgr->curTask_, task);
    EXPECT_EQ(mgr->curTask_->GetState(), RUNNING_STATE_PAUSED);
}

/**
 * @tc.name: ProcessNextAmsTask_002
 * @tc.desc: ProcessNextAmsTask while curTask_ is not NULL
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TaskManagerTest, ProcessNextAmsTask_002, TestSize.Level0)
{
    IMSA_HILOGI("TaskManagerTest ProcessNextAmsTask_002 START");
    EXPECT_EQ(mgr->curTask_, nullptr);
    auto task = std::make_shared<TaskAmsInit>();
    mgr->amsTasks_.push_back(task);
    mgr->ProcessNextAmsTask();
    EXPECT_EQ(mgr->curTask_, task);
    EXPECT_EQ(mgr->curTask_->GetState(), RUNNING_STATE_PAUSED);
}

} // namespace MiscServices
} // namespace OHOS