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

#include "block_queue.h"

#include <gtest/gtest.h>
#include <gtest/hwext/gtest-multithread.h>
#include <unistd.h>

#include <cinttypes>
#include <sstream>
#include <thread>

#include "global.h"

namespace OHOS {
namespace MiscServices {
using namespace testing::ext;
using namespace testing::mt;
using namespace std::chrono;
class ImfBlockQueueTest : public testing::Test {
public:
    static constexpr int32_t MAX_WAIT_TIME = 5000;
    static constexpr int32_t EACH_THREAD_CIRCULATION_TIME = 100;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void TestImfBlockQueue();
    static int64_t GetThreadId();
    static bool timeout_;

private:
    static BlockQueue<std::chrono::system_clock::time_point> timeQueue_;
};
BlockQueue<std::chrono::system_clock::time_point> ImfBlockQueueTest::timeQueue_{ MAX_WAIT_TIME };
bool ImfBlockQueueTest::timeout_{ false };
void ImfBlockQueueTest::SetUpTestCase(void)
{
}

void ImfBlockQueueTest::TearDownTestCase(void)
{
}

void ImfBlockQueueTest::SetUp()
{
}

void ImfBlockQueueTest::TearDown()
{
}

int64_t ImfBlockQueueTest::GetThreadId()
{
    std::thread::id id = std::this_thread::get_id();
    std::ostringstream oss;
    oss << id;
    std::string idStr = oss.str();
    return atol(idStr.c_str());
}

void ImfBlockQueueTest::TestImfBlockQueue()
{
    for (int32_t i = 0; i < EACH_THREAD_CIRCULATION_TIME; i++) {
        if (timeout_) {
            break;
        }
        auto time = std::chrono::system_clock::now();
        timeQueue_.Push(time);
        int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        timeQueue_.Wait(time);
        int64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        auto consume = end - start;
        auto threadId = GetThreadId();
        IMSA_HILOGI("consume:%{public}" PRId64 ",threadId:%{public}" PRId64 "", consume, threadId);
        if (consume >= MAX_WAIT_TIME) {
            timeout_ = true;
        }
        timeQueue_.Pop();
    }
}

/**
 * @tc.name: blockQueueTest_001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: chenyu
 */
HWTEST_F(ImfBlockQueueTest, blockQueueTest_001, TestSize.Level0)
{
    IMSA_HILOGI("ImfBlockQueueTest blockQueueTest_001 START");
    SET_THREAD_NUM(5);
    GTEST_RUN_TASK(TestImfBlockQueue);
    EXPECT_FALSE(timeout_);
}
} // namespace MiscServices
} // namespace OHOS
