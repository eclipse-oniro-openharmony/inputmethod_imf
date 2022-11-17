/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include <sys/time.h>
#include <unistd.h>

#include <cstdint>
#include <string>

#include "global.h"
#include "securec.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr const uint16_t EACH_LINE_LENGTH = 100;
constexpr const uint16_t TOTAL_LENGTH = 1000;
class InputMethodDfxTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static bool ExecuteCmd(const std::string &cmd, std::string &result);
    void SetUp();
    void TearDown();
};

void InputMethodDfxTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodDfxTest::SetUpTestCase");
}

void InputMethodDfxTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodDfxTest::TearDownTestCase");
}

void InputMethodDfxTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodDfxTest::SetUp");
}

void InputMethodDfxTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodDfxTest::TearDown");
}

bool InputMethodDfxTest::ExecuteCmd(const std::string &cmd, std::string &result)
{
    char buff[EACH_LINE_LENGTH] = { 0x00 };
    char output[TOTAL_LENGTH] = { 0x00 };
    FILE *ptr = NULL;
    if ((ptr = popen(cmd.c_str(), "r")) != NULL) {
        while (fgets(buff, sizeof(buff), ptr) != nullptr) {
            if (strcat_s(output, sizeof(output), buff) != 0) {
                pclose(ptr);
                ptr = NULL;
                return false;
            }
        }
        pclose(ptr);
        ptr = NULL;
    } else {
        return false;
    }
    result = std::string(output);
    return true;
}

/**
* @tc.name: InputMethodDfxTest_DumpAllMethod_001
* @tc.desc: DumpAllMethod
* @tc.type: FUNC
* @tc.require: issueI61PMG
* @tc.author: chenyu
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_DumpAllMethod_001, TestSize.Level0)
{
    std::string result;
    constexpr const char *CMD = "hidumper -s 3703 -a -a";
    auto ret = InputMethodDfxTest::ExecuteCmd(CMD, result);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(result.find("get input method") != std::string::npos);
    EXPECT_TRUE(result.find("imeList") != std::string::npos);
    EXPECT_TRUE(result.find("isDefaultIme") != std::string::npos);
}

/**
* @tc.name: InputMethodDfxTest_Dump_ShowHelp_001
* @tc.desc: Dump ShowHelp.
* @tc.type: FUNC
* @tc.require: issueI61PMG
* @tc.author: chenyu
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Dump_ShowHelp_001, TestSize.Level0)
{
    std::string result;
    constexpr const char *CMD = "hidumper -s 3703 -a -h";
    auto ret = InputMethodDfxTest::ExecuteCmd(CMD, result);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(result.find("Description:") != std::string::npos);
    EXPECT_TRUE(result.find("-h show help") != std::string::npos);
    EXPECT_TRUE(result.find("-a dump all input methods") != std::string::npos);
}

/**
* @tc.name: InputMethodDfxTest_Dump_ShowIllealInfomation_001
* @tc.desc: Dump ShowIllealInfomation.
* @tc.type: FUNC
* @tc.require: issueI61PMG
* @tc.author: chenyu
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Dump_ShowIllealInfomation_001, TestSize.Level0)
{
    std::string result;
    constexpr const char *CMD = "hidumper -s 3703 -a -test";
    auto ret = InputMethodDfxTest::ExecuteCmd(CMD, result);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(result.find("input dump parameter error,enter '-h' for usage.") != std::string::npos);
}
} // namespace MiscServices
} // namespace OHOS
