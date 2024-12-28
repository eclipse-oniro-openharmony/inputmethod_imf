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

#include "security_mode_parser.h"

#include <gtest/gtest.h>

#include <memory>

#include "ime_info_inquirer.h"
#include "mock_ime_info_inquirer.h"
#include "mock_settings_data_utils.h"
#include "mock_sys_cfg_parser.h"
#include "settings_data_utils.h"
#include "sys_cfg_parser.h"

using namespace OHOS;
using namespace MiscServices;

class SecurityModeParserTest : public testing::Test {
protected:
    void SetUp() override
    {
        // 设置模拟对象
        mockSettingsDataUtils = std::make_shared<MockSettingsDataUtils>();
        mockSysCfgParser = std::make_shared<MockSysCfgParser>();
        mockImeInfoInquirer = std::make_shared<MockImeInfoInquirer>();
        SettingsDataUtils::SetInstance(mockSettingsDataUtils);
        SysCfgParser::SetInstance(mockSysCfgParser);
        ImeInfoInquirer::SetInstance(mockImeInfoInquirer);

        // 获取 SecurityModeParser 实例
        parser = SecurityModeParser::GetInstance();
    }

    void TearDown() override
    {
        // 重置模拟对象
        mockSettingsDataUtils.reset();
        mockSysCfgParser.reset();
        mockImeInfoInquirer.reset();
    }

    std::shared_ptr<MockSettingsDataUtils> mockSettingsDataUtils;
    std::shared_ptr<MockSysCfgParser> mockSysCfgParser;
    std::shared_ptr<MockImeInfoInquirer> mockImeInfoInquirer;
    sptr<SecurityModeParser> parser;
};

/**
 * @tc.name: GetSecurityMode_SystemSpecialIme_ReturnsFull
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SecurityModeParserTest, GetSecurityMode_SystemSpecialIme_ReturnsFull, TestSize.Level0)
{
    // 测试包名是 SYSTEM_SPECIAL_IME 的情况
    EXPECT_EQ(parser->GetSecurityMode(SYSTEM_SPECIAL_IME, 0), SecurityMode::FULL);
}

/**
 * @tc.name: GetSecurityMode_Uninitialized_UpdatesList
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SecurityModeParserTest, GetSecurityMode_Uninitialized_UpdatesList, TestSize.Level0)
{
    // 模拟未初始化状态
    EXPECT_CALL(*mockSettingsDataUtils, GetStringValue(SETTING_URI_PROXY, SECURITY_MODE, testing::_))
        .WillOnce(testing::Return(ErrorCode::NO_ERROR));

    // 模拟包名不在 fullModeList_ 中
    EXPECT_CALL(*mockSysCfgParser, ParseDefaultFullIme(testing::_)).WillOnce(testing::Return(true));

    // 测试未初始化时的行为
    EXPECT_EQ(parser->GetSecurityMode("testBundle", 0), SecurityMode::BASIC);
}

/**
 * @tc.name: GetSecurityMode_FullModeListContains_ReturnsFull
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SecurityModeParserTest, GetSecurityMode_FullModeListContains_ReturnsFull, TestSize.Level0)
{
    // 模拟包名在 fullModeList_ 中
    EXPECT_CALL(*mockSysCfgParser, ParseDefaultFullIme(testing::_)).WillOnce(testing::Return(true));

    // 模拟包名在 fullModeList_ 中
    EXPECT_CALL(*mockImeInfoInquirer, GetImeAppId(0, "testBundle", testing::_)).WillOnce(testing::Return(true));

    // 测试包名在 fullModeList_ 中的情况
    EXPECT_EQ(parser->GetSecurityMode("testBundle", 0), SecurityMode::FULL);
}

/**
 * @tc.name: GetSecurityMode_FullModeListDoesNotContain_ReturnsBasic
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SecurityModeParserTest, GetSecurityMode_FullModeListDoesNotContain_ReturnsBasic, TestSize.Level0)
{
    // 模拟包名不在 fullModeList_ 中
    EXPECT_CALL(*mockSysCfgParser, ParseDefaultFullIme(testing::_)).WillOnce(testing::Return(true));

    // 模拟包名不在 fullModeList_ 中
    EXPECT_CALL(*mockImeInfoInquirer, GetImeAppId(0, "testBundle", testing::_)).WillOnce(testing::Return(false));

    // 测试包名不在 fullModeList_ 中的情况
    EXPECT_EQ(parser->GetSecurityMode("testBundle", 0), SecurityMode::BASIC);
}