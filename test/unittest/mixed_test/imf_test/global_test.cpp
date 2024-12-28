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
#include <gtest/gtest.h>
#include "global.h"

namespace OHOS {
namespace MiscServices {
class BlockRetryTest : public testing::Test {
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(BlockRetryTest, TestBlockRetry_Success)
{
    bool result = BlockRetry(100, 3, []() -> bool { return true; });
    EXPECT_TRUE(result);
}

TEST_F(BlockRetryTest, TestBlockRetry_Failure_MaxRetries)
{
    bool result = BlockRetry(100, 3, []() -> bool { return false; });
    EXPECT_FALSE(result);
}

TEST_F(BlockRetryTest, TestBlockRetry_Failure_Then_Success)
{
    int count = 0;
    bool result = BlockRetry(100, 5, [&count]() -> bool {
        count++;
        return count >= 3;
    });
    EXPECT_TRUE(result);
    EXPECT_EQ(count, 3);
}
} // namespace MiscServices
} // namespace OHOS
