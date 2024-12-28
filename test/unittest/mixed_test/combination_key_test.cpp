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

#include "combination_key.h"

#include <gtest/gtest.h>

#include "keyboard_event.h"

namespace OHOS {
namespace MiscServices {
namespace {
using namespace testing;

class CombinationKeyTest : public Test {
public:
    static void SetUpTestCase()
    {
    }
    static void TearDownTestCase()
    {
    }
};

/**
 * @tc.name: IsMatch_UnknownCombinationKey_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CombinationKeyTest, IsMatch_UnknownCombinationKey_ReturnsFalse, TestSize.Level0)
{
    CombinationKeyFunction unknownKey = static_cast<CombinationKeyFunction>(100); // 假设100不在映射中
    uint32_t state = KeyboardEvent::SHIFT_LEFT_MASK;
    EXPECT_FALSE(CombinationKey::IsMatch(unknownKey, state));
}

/**
 * @tc.name: IsMatch_KnownCombinationKeyStateNotMatch_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CombinationKeyTest, IsMatch_KnownCombinationKeyStateNotMatch_ReturnsFalse, TestSize.Level0)
{
    CombinationKeyFunction combinationKey = CombinationKeyFunction::SWITCH_LANGUAGE;
    uint32_t state = KeyboardEvent::CTRL_LEFT_MASK; // 不在映射中
    EXPECT_FALSE(CombinationKey::IsMatch(combinationKey, state));
}

/**
 * @tc.name: IsMatch_KnownCombinationKeyStateMatch_ReturnsTrue
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CombinationKeyTest, IsMatch_KnownCombinationKeyStateMatch_ReturnsTrue, TestSize.Level0)
{
    CombinationKeyFunction combinationKey = CombinationKeyFunction::SWITCH_LANGUAGE;
    uint32_t state = KeyboardEvent::SHIFT_LEFT_MASK; // 在映射中
    EXPECT_TRUE(CombinationKey::IsMatch(combinationKey, state));
}
} // namespace
} // namespace MiscServices
} // namespace OHOS