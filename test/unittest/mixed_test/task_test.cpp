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

#include "tasks/task.h"

#include <gtest/gtest.h>
#include <list>
#include <memory>

#include "global.h"
#include "tasks/action.h"

using namespace OHOS::MiscServices;

class MockAction : public Action {
public:
    MockAction() : state_(RUNNING_STATE_COMPLETED)
    {
    }

    RunningState Execute() override
    {
        return state_;
    }

    RunningState Resume(uint64_t seqId) override
    {
        return state_;
    }

    void SetState(RunningState state)
    {
        state_ = state;
    }

private:
    RunningState state_;
};

class TaskTest : public testing::Test {
protected:
    void SetUp() override
    {
        task = std::make_shared<Task>(TASK_TYPE_IMSA);
        action = std::make_unique<MockAction>();
        task->Pend(std::move(action));
    }

    std::shared_ptr<Task> task;
    std::unique_ptr<MockAction> action;
};

HWTEST_F(TaskTest, executeIdleState_001, TestSize.Level0)
{
    task->state_ = RUNNING_STATE_IDLE;
    EXPECT_EQ(task->Execute(), RUNNING_STATE_RUNNING);
}

HWTEST_F(TaskTest, executeIdleState_002, TestSize.Level0)
{
    task->state_ = RUNNING_STATE_PAUSED;
    EXPECT_EQ(task->Execute(), RUNNING_STATE_ERROR);
}

HWTEST_F(TaskTest, executeIdleState_003, TestSize.Level0)
{
    task->curAction_ = nullptr;
    EXPECT_EQ(task->Resume(1), RUNNING_STATE_ERROR);
}

HWTEST_F(TaskTest, executeIdleState_004, TestSize.Level0)
{
    task->state_ = RUNNING_STATE_RUNNING;
    task->curAction_ = std::make_unique<MockAction>();
    EXPECT_EQ(task->Resume(1), RUNNING_STATE_ERROR);
}

HWTEST_F(TaskTest, executeIdleState_005, TestSize.Level0)
{
    task->state_ = RUNNING_STATE_PAUSED;
    task->curAction_ = std::make_unique<MockAction>();
    task->curAction_->SetState(RUNNING_STATE_COMPLETED);
    EXPECT_EQ(task->Resume(1), RUNNING_STATE_RUNNING);
}

HWTEST_F(TaskTest, executeIdleState_006, TestSize.Level0)
{
    EXPECT_EQ(task->OnTask(nullptr), RUNNING_STATE_IDLE);
}

HWTEST_F(TaskTest, executeIdleState_007, TestSize.Level0)
{
    auto innerTask = std::make_shared<Task>(TASK_TYPE_INNER);
    EXPECT_EQ(task->OnTask(innerTask), RUNNING_STATE_IDLE);
}

HWTEST_F(TaskTest, executeIdleState_008, TestSize.Level0)
{
    auto imaTask = std::make_shared<Task>(TASK_TYPE_IMA);
    EXPECT_EQ(task->OnTask(imaTask), RUNNING_STATE_IDLE);
}

HWTEST_F(TaskTest, executeIdleState_009, TestSize.Level0)
{
    task->type_ = TASK_TYPE_AMS;
    EXPECT_EQ(task->GetSourceType(), SOURCE_TYPE_AMS);
}

HWTEST_F(TaskTest, executeIdleState_010, TestSize.Level0)
{
    task->type_ = TASK_TYPE_IMA;
    EXPECT_EQ(task->GetSourceType(), SOURCE_TYPE_IMA);
}

HWTEST_F(TaskTest, executeIdleState_011, TestSize.Level0)
{
    task->type_ = TASK_TYPE_IMSA;
    EXPECT_EQ(task->GetSourceType(), SOURCE_TYPE_IMSA);
}

HWTEST_F(TaskTest, executeIdleState_012, TestSize.Level0)
{
    task->type_ = TASK_TYPE_OTHER;
    EXPECT_EQ(task->GetSourceType(), SOURCE_TYPE_INNER);
}

HWTEST_F(TaskTest, executeIdleState_013, TestSize.Level0)
{
    task->actions_.clear();
    EXPECT_EQ(task->ExecuteInner(), RUNNING_STATE_COMPLETED);
}

HWTEST_F(TaskTest, executeIdleState_014, TestSize.Level0)
{
    task->actions_.push_back(std::make_unique<MockAction>());
    EXPECT_EQ(task->ExecuteInner(), RUNNING_STATE_RUNNING);
}

HWTEST_F(TaskTest, executeIdleState_015, TestSize.Level0)
{
    task->actions_.push_back(std::make_unique<MockAction>());
    task->actions_.front()->SetState(RUNNING_STATE_PAUSED);
    EXPECT_EQ(task->ExecuteInner(), RUNNING_STATE_PAUSED);
}