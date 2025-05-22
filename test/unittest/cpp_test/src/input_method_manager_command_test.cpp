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
 * @tc.desc: Test handling argument for -e option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_001, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_001 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -e com.example.newTestIme", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Succeeded in enabling IME. status:BASIC_MODE\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_002
 * @tc.desc: Test handling argument for -e option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_002, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_002 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -e com.example.test", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Error: The input method does not exist.\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_003
 * @tc.desc: Test handling argument for -d option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_003, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_003 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -d com.example.newTestIme", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Succeeded in disabling IME. status:DISABLED\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_004
 * @tc.desc: Test handling argument for -e -f option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_004, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_004 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -e com.example.newTestIme -f", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Succeeded in enabling IME. status:FULL_EXPERIENCE_MODE\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_005
 * @tc.desc: Test handling argument for -e -b option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_005, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_005 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -e com.example.newTestIme -b", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Succeeded in enabling IME. status:BASIC_MODE\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_006
 * @tc.desc: Test handling invalid mode arguments after -e option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_006, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_006 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -e com.example.newTestIme -a", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Error: Invalid mode after -e. Use -b or -f\n", result);

    ret = TddUtil::ExecuteCmd("ime -e com.example.newTestIme -fa", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Error: Invalid mode after -e. Use -b or -f\n", result);

    ret = TddUtil::ExecuteCmd("ime -e com.example.newTestIme ime -e com.example.newTestIme", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Error: Invalid mode after -e. Use -b or -f\n", result);

    ret = TddUtil::ExecuteCmd("ime -e com.example.newTestIme ime -e -d", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Error: Invalid mode after -e. Use -b or -f\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_007
 * @tc.desc: Test handling argument for -s option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_007, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_007 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -s com.example.TestIme", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Error: The input method does not exist.\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_008
 * @tc.desc: Test handling argument for -s option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_008, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_008 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -d com.example.newTestIme", result);
    EXPECT_TRUE(ret);
    sleep(1);
    ret = TddUtil::ExecuteCmd("ime -e com.example.newTestIme", result);
    EXPECT_TRUE(ret);
    sleep(1);
    ret = TddUtil::ExecuteCmd("ime -s com.example.newTestIme", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Succeeded in switching the input method. IME:com.example.newTestIme\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_009
 * @tc.desc: Test handling argument for -g option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_009, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_009 START");
    sleep(2);
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -g", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("The current input method is: com.example.newTestIme, status: FULL_EXPERIENCE_MODE\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_0010
 * @tc.desc: Test handling situation when the parameters are empty
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_0010, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_0010 START");
    sleep(2);
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -e", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Error: Invalid command!\n", result);
}

/**
 * @tc.name: InputMethodManagerCommand_ExeCmd_011
 * @tc.desc: Test handling invalid mode arguments after -d option
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(InputMethodManagerCommandTest, ExeCmd_011, TestSize.Level1)
{
    IMSA_HILOGI("InputMethodManagerCommandTest ExeCmd_011 START");
    std::string result;
    auto ret = TddUtil::ExecuteCmd("ime -d com.example.k", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Error: The input method does not exist.\n", result);

    ret = TddUtil::ExecuteCmd("ime -d com.example.newTestIme a", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Error: Invalid command!\n", result);

    ret = TddUtil::ExecuteCmd("ime -d com.example.newTestIme -d", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Error: Invalid command!\n", result);

    ret = TddUtil::ExecuteCmd("ime -d com.example.newTestIme ime -d com.example.newTestIme", result);
    EXPECT_TRUE(ret);
    EXPECT_EQ("Error: Invalid command!\n", result);
}
} // namespace MiscServices
} // namespace OHOS