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

#include "enable_ime_data_parser.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ime_info_inquirer.h"
#include "settings_data_utils.h"

using namespace OHOS::MiscServices;
using namespace testing;

class MockImeInfoInquirer : public ImeInfoInquirer {
public:
    MOCK_METHOD1(GetDefaultImeCfgProp, std::shared_ptr<Property>(int32_t));
    MOCK_METHOD2(GetCurrentInputMethod, std::shared_ptr<Property>(int32_t, const std::string &));
};

class MockSettingsDataUtils : public SettingsDataUtils {
public:
    MOCK_METHOD3(GetStringValue, int32_t(const std::string &, const std::string &, std::string &));
};

class EnableImeDataParserTest : public Test {
protected:
    void SetUp() override
    {
        imeInfoInquirer_ = std::make_shared<MockImeInfoInquirer>();
        settingsDataUtils_ = std::make_shared<MockSettingsDataUtils>();
        EnableImeDataParser::GetInstance()->SetImeInfoInquirer(imeInfoInquirer_);
        EnableImeDataParser::GetInstance()->SetSettingsDataUtils(settingsDataUtils_);
    }

    void TearDown() override
    {
        EnableImeDataParser::GetInstance()->SetImeInfoInquirer(nullptr);
        EnableImeDataParser::GetInstance()->SetSettingsDataUtils(nullptr);
    }

    std::shared_ptr<MockImeInfoInquirer> imeInfoInquirer_;
    std::shared_ptr<MockSettingsDataUtils> settingsDataUtils_;
};

/**
 * @tc.name: CheckNeedSwitch_DefaultImeIsNull_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(EnableImeDataParserTest, CheckNeedSwitch_DefaultImeIsNull_ReturnsFalse, TestSize.Level0)
{
    EXPECT_CALL(*imeInfoInquirer_, GetDefaultImeCfgProp(_)).WillOnce(Return(nullptr));

    SwitchInfo switchInfo;
    bool result = EnableImeDataParser::GetInstance()->CheckNeedSwitch(ENABLE_IME, switchInfo, 0);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: CheckNeedSwitch_CurrentImeIsNull_ReturnsTrue
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(EnableImeDataParserTest, CheckNeedSwitch_CurrentImeIsNull_ReturnsTrue, TestSize.Level0)
{
    auto defaultIme = std::make_shared<Property>();
    defaultIme->name = "defaultIme";
    EXPECT_CALL(*imeInfoInquirer_, GetDefaultImeCfgProp(_)).WillOnce(Return(defaultIme));
    EXPECT_CALL(*imeInfoInquirer_, GetCurrentInputMethod(_, _)).WillOnce(Return(nullptr));

    SwitchInfo switchInfo;
    bool result = EnableImeDataParser::GetInstance()->CheckNeedSwitch(ENABLE_IME, switchInfo, 0);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckNeedSwitch_EnableIme_CurrentImeEqualsDefault_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(EnableImeDataParserTest, CheckNeedSwitch_EnableIme_CurrentImeEqualsDefault_ReturnsFalse, TestSize.Level0)
{
    auto defaultIme = std::make_shared<Property>();
    defaultIme->name = "defaultIme";
    auto currentIme = std::make_shared<Property>();
    currentIme->name = "defaultIme";
    EXPECT_CALL(*imeInfoInquirer_, GetDefaultImeCfgProp(_)).WillOnce(Return(defaultIme));
    EXPECT_CALL(*imeInfoInquirer_, GetCurrentInputMethod(_, _)).WillOnce(Return(currentIme));

    SwitchInfo switchInfo;
    bool result = EnableImeDataParser::GetInstance()->CheckNeedSwitch(ENABLE_IME, switchInfo, 0);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: CheckNeedSwitch_EnableIme_CurrentImeNotEqualsDefault_CallsCheckTargetEnableName
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(EnableImeDataParserTest, CheckNeedSwitch_EnableIme_CurrentImeNotEqualsDefault_CallsCheckTargetEnableName,
    TestSize.Level0)
{
    auto defaultIme = std::make_shared<Property>();
    defaultIme->name = "defaultIme";
    auto currentIme = std::make_shared<Property>();
    currentIme->name = "otherIme";
    EXPECT_CALL(*imeInfoInquirer_, GetDefaultImeCfgProp(_)).WillOnce(Return(defaultIme));
    EXPECT_CALL(*imeInfoInquirer_, GetCurrentInputMethod(_, _)).WillOnce(Return(currentIme));
    EXPECT_CALL(*EnableImeDataParser::GetInstance(), CheckTargetEnableName(_, _, _, _)).WillOnce(Return(true));

    SwitchInfo switchInfo;
    bool result = EnableImeDataParser::GetInstance()->CheckNeedSwitch(ENABLE_IME, switchInfo, 0);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckNeedSwitch_EnableKeyboard_CurrentImeNotEqualsDefaultOrIdSame_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(EnableImeDataParserTest, CheckNeedSwitch_EnableKeyboard_CurrentImeNotEqualsDefaultOrIdSame_ReturnsFalse,
    TestSize.Level0)
{
    auto defaultIme = std::make_shared<Property>();
    defaultIme->name = "defaultIme";
    auto currentIme = std::make_shared<Property>();
    currentIme->name = "otherIme";
    currentIme->id = defaultIme->id;
    EXPECT_CALL(*imeInfoInquirer_, GetDefaultImeCfgProp(_)).WillOnce(Return(defaultIme));
    EXPECT_CALL(*imeInfoInquirer_, GetCurrentInputMethod(_, _)).WillOnce(Return(currentIme));

    SwitchInfo switchInfo;
    bool result = EnableImeDataParser::GetInstance()->CheckNeedSwitch(ENABLE_KEYBOARD, switchInfo, 0);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: CheckNeedSwitch_InvalidKey_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(EnableImeDataParserTest, CheckNeedSwitch_InvalidKey_ReturnsFalse, TestSize.Level0)
{
    auto defaultIme = std::make_shared<Property>();
    defaultIme->name = "defaultIme";
    EXPECT_CALL(*imeInfoInquirer_, GetDefaultImeCfgProp(_)).WillOnce(Return(defaultIme));

    SwitchInfo switchInfo;
    bool result = EnableImeDataParser::GetInstance()->CheckNeedSwitch("INVALID_KEY", switchInfo, 0);

    EXPECT_FALSE(result);
}