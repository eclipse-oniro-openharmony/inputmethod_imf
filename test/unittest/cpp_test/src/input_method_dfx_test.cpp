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
#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>

#include "global.h"
#include "securec.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr const uint16_t EACH_LINE_LENGTH = 100;
constexpr const uint16_t TOTAL_LENGTH = 1000;
constexpr const char *CMD1 = "hidumper -s 3703 -a -a";
constexpr const char *CMD2 = "hidumper -s 3703 -a -h";
constexpr const char *CMD3 = "hidumper -s 3703 -a -test";
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
    FILE *ptr = popen(cmd.c_str(), "r");
    if (ptr != nullptr) {
        while (fgets(buff, sizeof(buff), ptr) != nullptr) {
            if (strcat_s(output, sizeof(output), buff) != 0) {
                pclose(ptr);
                ptr = nullptr;
                return false;
            }
        }
        pclose(ptr);
        ptr = nullptr;
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
    auto ret = InputMethodDfxTest::ExecuteCmd(CMD1, result);
    EXPECT_TRUE(ret);
    EXPECT_NE(result.find("get input method"), std::string::npos);
    EXPECT_NE(result.find("imeList"), std::string::npos);
    EXPECT_NE(result.find("isCurrentIme"), std::string::npos);
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
    auto ret = InputMethodDfxTest::ExecuteCmd(CMD2, result);
    EXPECT_TRUE(ret);
    EXPECT_NE(result.find("Description:"), std::string::npos);
    EXPECT_NE(result.find("-h show help"), std::string::npos);
    EXPECT_NE(result.find("-a dump all input methods"), std::string::npos);
}

/**
* @tc.name: InputMethodDfxTest_Dump_ShowIllealInformation_001
* @tc.desc: Dump ShowIllealInformation.
* @tc.type: FUNC
* @tc.require: issueI61PMG
* @tc.author: chenyu
*/
HWTEST_F(InputMethodDfxTest, InputMethodDfxTest_Dump_ShowIllealInformation_001, TestSize.Level0)
{
    std::string result;
    auto ret = InputMethodDfxTest::ExecuteCmd(CMD3, result);
    EXPECT_TRUE(ret);
    EXPECT_NE(result.find("input dump parameter error,enter '-h' for usage."), std::string::npos);
}
} // namespace MiscServices
} // namespace OHOS
