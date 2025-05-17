/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "global.h"
#include "input_method_manager_command.h"
#include "tdd_util.h"

namespace OHOS {
namespace MiscServices {
namespace {
using namespace testing::ext;
} // namespace
class InputMethodManagerCommandTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void) {}
};

void InputMethodManagerCommandTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodManagerCommandTest::SetUpTestCase");
    sleep(2);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_001
 * @tc.desc: Test handling missing argument for -e option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_001, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_001 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -e com.example.newTestIme:InputMethodExtAbility", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Succeeded in enabling IME. IME:com.example.newTestIme:InputMethodExtAbility\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_002
 * @tc.desc: Test handling missing argument for -e option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_002, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_002 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -e com.example.test:InputMethodExtAbility", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Error: The input method does not exist.\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_003
 * @tc.desc: Test handling missing argument for -e option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_003, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_003 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -d com.example.newTestIme:InputMethodExtAbility", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Succeeded in disabling IME. IME:com.example.newTestIme:InputMethodExtAbility\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_004
 * @tc.desc: Test handling missing argument for -e option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_004, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_004 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -f com.example.newTestIme:InputMethodExtAbility", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Succeeded in setting IME to basic mode. IME:com.example.newTestIme:InputMethodExtAbility\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_005
 * @tc.desc: Test handling missing argument for -e option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_005, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_005 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -b com.example.newTestIme:InputMethodExtAbility", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Succeeded in setting IME to full mode. IME:com.example.newTestIme:InputMethodExtAbility\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_006
 * @tc.desc: Test handling missing argument for -e option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_006, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_006 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -s com.example.newTestIme:InputMethodExtAbility", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Could not switch the input method. IME:com.example.newTestIme:InputMethodExtAbility\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_007
 * @tc.desc: Test handling missing argument for -e option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_007, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_007 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -d com.example.newTestIme:InputMethodExtAbility", result);
    EXPECT_TRUE(ret);
    sleep(1);
    ret = TddUtil::ExecuteCmd("ime -e com.example.newTestIme:InputMethodExtAbility", result);
    EXPECT_TRUE(ret);
    sleep(1);
    ret = TddUtil::ExecuteCmd("ime -s com.example.newTestIme:lowerInput", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Succeeded in switching the input method. IME:com.example.newTestIme:lowerInput\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_008
 * @tc.desc: Test handling missing argument for -e option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_008, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_008 START");
    sleep(2);
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -g", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("The current input method is: com.example.newTestIme\n", result);
}
} // namespace MiscServices
} // namespace OHOS