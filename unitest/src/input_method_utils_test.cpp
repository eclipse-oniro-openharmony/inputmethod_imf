/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "input_method_utils.h"

#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>

#include "global.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class InputMethodUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void InputMethodUtilsTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodUtilsTest::SetUpTestCase");
}

void InputMethodUtilsTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodUtilsTest::TearDownTestCase");
}

void InputMethodUtilsTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodUtilsTest::SetUp");
}

void InputMethodUtilsTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodUtilsTest::TearDown");
}

/**
* @tc.name: inputMethodUtils_keyboardStatus_001
* @tc.desc: Checkout KeyboardStatus.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodUtilsTest, inputMethodUtils_keyboardStatus_001, TestSize.Level0)
{
    KeyboardInfo info;
    info.SetKeyboardStatus(int32_t(KeyboardStatus::SHOW));
    KeyboardStatus status = info.GetKeyboardStatus();
    EXPECT_EQ(status, KeyboardStatus::SHOW);
}

/**
* @tc.name: inputMethodUtils_functionKey_001
* @tc.desc: Checkout FunctionKey.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodUtilsTest, inputMethodUtils_functionKey_001, TestSize.Level0)
{
    KeyboardInfo info;
    info.SetFunctionKey(int32_t(FunctionKey::CONFIRM));
    FunctionKey key = info.GetFunctionKey();
    EXPECT_EQ(key, FunctionKey::CONFIRM);
}
} // namespace MiscServices
} // namespace OHOS