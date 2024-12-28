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

#include "sys_cfg_parser.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "default_full_ime_cfg.h"
#include "file_operator.h"
#include "ime_system_config.h"
#include "input_type_cfg.h"
#include "sys_panel_adjust_cfg.h"

using namespace OHOS::MiscServices;
using namespace testing;

class SysCfgParserTest : public Test {
public:
    static void SetUpTestCase()
    {
    }

    static void TearDownTestCase()
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    bool MockRead(const std::string &filePath, const std::string &key, std::string &content)
    {
        if (key == "systemConfig") {
            content = "validSystemConfigContent";
        } else if (key == "supportedInputTypeList") {
            content = "validInputTypeContent";
        } else if (key == "sysPanelAdjust") {
            content = "validPanelAdjustContent";
        } else if (key == "defaultFullImeList") {
            content = "validDefaultFullImeContent";
        } else {
            content = "";
        }
        return !content.empty();
    }
};

/**
 * @tc.name: ParseSystemConfig_ContentEmpty_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SysCfgParserTest, ParseSystemConfig_ContentEmpty_ReturnsFalse, TestSize.Level0)
{
    SysCfgParser parser;
    SystemConfig systemConfig;
    EXPECT_CALL(FileOperator, Read(_, _, _)).WillOnce(Invoke(this, &SysCfgParserTest::MockRead));
    EXPECT_CALL(ImeSystemConfig(), Unmarshall(_)).Times(0); // 确保 Unmarshall 不被调用
    bool result = parser.ParseSystemConfig(systemConfig);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ParseSystemConfig_ContentValid_ReturnsTrue
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SysCfgParserTest, ParseSystemConfig_ContentValid_ReturnsTrue, TestSize.Level0)
{
    SysCfgParser parser;
    SystemConfig systemConfig;
    EXPECT_CALL(FileOperator, Read(_, _, _)).WillOnce(Invoke(this, &SysCfgParserTest::MockRead));
    EXPECT_CALL(ImeSystemConfig(), Unmarshall("validSystemConfigContent")).WillOnce(Return(true));
    bool result = parser.ParseSystemConfig(systemConfig);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: ParseInputType_ContentEmpty_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SysCfgParserTest, ParseInputType_ContentEmpty_ReturnsFalse, TestSize.Level0)
{
    SysCfgParser parser;
    std::vector<InputTypeInfo> inputType;
    EXPECT_CALL(FileOperator, Read(_, _, _)).WillOnce(Invoke(this, &SysCfgParserTest::MockRead));
    EXPECT_CALL(InputTypeCfg(), Unmarshall(_)).Times(0); // 确保 Unmarshall 不被调用
    bool result = parser.ParseInputType(inputType);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ParseInputType_ContentValid_ReturnsTrue
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SysCfgParserTest, ParseInputType_ContentValid_ReturnsTrue, TestSize.Level0)
{
    SysCfgParser parser;
    std::vector<InputTypeInfo> inputType;
    EXPECT_CALL(FileOperator, Read(_, _, _)).WillOnce(Invoke(this, &SysCfgParserTest::MockRead));
    EXPECT_CALL(InputTypeCfg(), Unmarshall("validInputTypeContent")).WillOnce(Return(true));
    bool result = parser.ParseInputType(inputType);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: ParsePanelAdjust_ContentEmpty_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SysCfgParserTest, ParsePanelAdjust_ContentEmpty_ReturnsFalse, TestSize.Level0)
{
    SysCfgParser parser;
    std::vector<SysPanelAdjust> sysPanelAdjust;
    EXPECT_CALL(FileOperator, Read(_, _, _)).WillOnce(Invoke(this, &SysCfgParserTest::MockRead));
    EXPECT_CALL(SysPanelAdjustCfg(), Unmarshall(_)).Times(0); // 确保 Unmarshall 不被调用
    bool result = parser.ParsePanelAdjust(sysPanelAdjust);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ParsePanelAdjust_ContentValid_ReturnsTrue
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SysCfgParserTest, ParsePanelAdjust_ContentValid_ReturnsTrue, TestSize.Level0)
{
    SysCfgParser parser;
    std::vector<SysPanelAdjust> sysPanelAdjust;
    EXPECT_CALL(FileOperator, Read(_, _, _)).WillOnce(Invoke(this, &SysCfgParserTest::MockRead));
    EXPECT_CALL(SysPanelAdjustCfg(), Unmarshall("validPanelAdjustContent")).WillOnce(Return(true));
    bool result = parser.ParsePanelAdjust(sysPanelAdjust);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: ParseDefaultFullIme_ContentEmpty_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SysCfgParserTest, ParseDefaultFullIme_ContentEmpty_ReturnsFalse, TestSize.Level0)
{
    SysCfgParser parser;
    std::vector<DefaultFullImeInfo> defaultFullImeList;
    EXPECT_CALL(FileOperator, Read(_, _, _)).WillOnce(Invoke(this, &SysCfgParserTest::MockRead));
    EXPECT_CALL(DefaultFullImeCfg(), Unmarshall(_)).Times(0); // 确保 Unmarshall 不被调用
    bool result = parser.ParseDefaultFullIme(defaultFullImeList);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ParseDefaultFullIme_ContentValid_ReturnsTrue
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SysCfgParserTest, ParseDefaultFullIme_ContentValid_ReturnsTrue, TestSize.Level0)
{
    SysCfgParser parser;
    std::vector<DefaultFullImeInfo> defaultFullImeList;
    EXPECT_CALL(FileOperator, Read(_, _, _)).WillOnce(Invoke(this, &SysCfgParserTest::MockRead));
    EXPECT_CALL(DefaultFullImeCfg(), Unmarshall("validDefaultFullImeContent")).WillOnce(Return(true));
    bool result = parser.ParseDefaultFullIme(defaultFullImeList);
    EXPECT_TRUE(result);
}