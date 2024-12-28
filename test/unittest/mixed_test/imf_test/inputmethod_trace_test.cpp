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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "inputmethod_trace.h"

using ::testing::_;
using ::testing::Return;

namespace OHOS {
namespace MiscServices {

class MockHiTrace {
public:
    MOCK_FUNCTION0(UpdateTraceLabel, void());
    MOCK_FUNCTION3(CountTrace, void(uint64_t, const std::string&, int64_t));
    MOCK_FUNCTION3(StartAsyncTrace, void(uint64_t, const std::string&, int32_t));
    MOCK_FUNCTION3(FinishAsyncTrace, void(uint64_t, const std::string&, int32_t));
    MOCK_FUNCTION2(StartTrace, void(uint64_t, const std::string&));
    MOCK_FUNCTION1(FinishTrace, void(uint64_t));
};

void UpdateTraceLabel()
{
    MockHiTrace::UpdateTraceLabel();
}

void CountTrace(uint64_t tag, const std::string& name, int64_t count)
{
    MockHiTrace::CountTrace(tag, name, count);
}

void StartAsyncTrace(uint64_t tag, const std::string& value, int32_t taskId)
{
    MockHiTrace::StartAsyncTrace(tag, value, taskId);
}

void FinishAsyncTrace(uint64_t tag, const std::string& value, int32_t taskId)
{
    MockHiTrace::FinishAsyncTrace(tag, value, taskId);
}

void StartTrace(uint64_t tag, const std::string& value)
{
    MockHiTrace::StartTrace(tag, value);
}

void FinishTrace(uint64_t tag)
{
    MockHiTrace::FinishTrace(tag);
}

class InputMethodTraceTest : public testing::Test {
protected:
    void SetUp() override
    {
        mockHiTrace = std::make_unique<MockHiTrace>();
    }

    void TearDown() override
    {
        mockHiTrace.reset();
    }

    std::unique_ptr<MockHiTrace> mockHiTrace;
};

TEST_F(InputMethodTraceTest, TestInitHiTrace)
{
    EXPECT_CALL(*mockHiTrace, UpdateTraceLabel()).WillOnce(Return());

    InitHiTrace();
}

TEST_F(InputMethodTraceTest, TestValueTrace)
{
    EXPECT_CALL(*mockHiTrace, CountTrace(HITRACE_TAG_MISC, "test_name", 123)).WillOnce(Return());

    ValueTrace("test_name", 123);
}

TEST_F(InputMethodTraceTest, TestStartAsync)
{
    EXPECT_CALL(*mockHiTrace, StartAsyncTrace(HITRACE_TAG_MISC, "test_value", 456)).WillOnce(Return());

    StartAsync("test_value", 456);
}

TEST_F(InputMethodTraceTest, TestFinishAsync)
{
    EXPECT_CALL(*mockHiTrace, FinishAsyncTrace(HITRACE_TAG_MISC, "test_value", 456)).WillOnce(Return());

    FinishAsync("test_value", 456);
}

TEST_F(InputMethodTraceTest, TestInputMethodSyncTrace)
{
    EXPECT_CALL(*mockHiTrace, StartTrace(HITRACE_TAG_MISC, "test_value")).WillOnce(Return());
    EXPECT_CALL(*mockHiTrace, FinishTrace(HITRACE_TAG_MISC)).WillOnce(Return());

    {
        InputMethodSyncTrace trace("test_value");
    }
}

TEST_F(InputMethodTraceTest, TestInputMethodSyncTraceWithId)
{
    EXPECT_CALL(*mockHiTrace, StartTrace(HITRACE_TAG_MISC, "test_value_id")).WillOnce(Return());
    EXPECT_CALL(*mockHiTrace, FinishTrace(HITRACE_TAG_MISC)).WillOnce(Return());

    {
        InputMethodSyncTrace trace("test_value", "id");
    }
}
} // namespace MiscServices
} // namespace OHOS
