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

#define private public
#define protected public
#include "enable_ime_data_parser.h"
#include "ime_cfg_manager.h"
#include "ime_info_inquirer.h"
#include "security_mode_parser.h"
#include "sys_cfg_parser.h"
#undef private

#include <gtest/gtest.h>
#include <unistd.h>

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class JsonOperateTest : public testing::Test {
public:
    static constexpr const char *IME_PERSIST_CFG = "{\"imeCfgList\":[{\"userId\":100,\"currentIme\":\"bundleName/"
                                                   "extName\",\"currentSubName\":\"subName\",\"isDefaultImeSet\":"
                                                   "false},{\"userId\":104,\"currentIme\":\"bundleName1/"
                                                   "extName1\",\"currentSubName\":\"subName1\","
                                                   "\"isDefaultImeSet\":false}]}";
    static constexpr const char *IME_PERSIST_CFG_NULL = "{\"imeCfgList\":[]}";
    static constexpr const char *IME_PERSIST_CFG_VALUE_TYPE_ERROR = "{\"imeCfgList\":[{\"userId\":100,\"currentIme\":"
                                                                    "\"bundleName/"
                                                                    "extName\",\"currentSubName\":\"subName\"},{"
                                                                    "\"userId\":"
                                                                    "\"104\",\"currentIme\":\"bundleName1/"
                                                                    "extName1\",\"currentSubName\":\"subName1\"}]}";
    static constexpr const char *IME_PERSIST_CFG_NAME_LACK = "{\"imeCfgList\":[{\"userId\":100,\"currentSubName\":"
                                                             "\"subName\"}]}";
    static constexpr const char *IME_PERSIST_CFG_NAME_ERROR = "{\"imeCfgList\":[{\"userId\":100, \"bundle\": "
                                                              "\"bundleName/extNme\",\"currentSubName\":"
                                                              "\"subName\"}]}";

    static constexpr const char *ENABLE_IME = "{\"enableImeList\" : {\"100\" : [ \"testIme\", \"testIme1\", "
                                              "\"testIme2\"],\"101\" : [\"testIme3\"], \"102\" : []}}";
    static constexpr const char *ENABLE_KEYBOARD = "{\"enableKeyboardList\" : {\"100\" : [ \"testKeyboard\", "
                                                   "\"testKeyboard1\"],\"101\" : "
                                                   "[\"testKeyboard2\"], \"105\" : []}}";
    static constexpr const char *SECURITY_MODE = "{\"fullExperienceList\" : {\"100\" : [\"testIme\", "
                                                 "\"testIme3\"], \"102\" : []}}";
    static constexpr const char *SUBTYPE = "{\"subtypes\": [{\"icon\": \"$media:icon\",\"id\": "
                                           "\"subtypeId\",\"label\": \"$string:chinese\",\"locale\": "
                                           "\"zh-CN\",\"mode\": \"lower\"},{\"icon\": \"$media:icon1\",\"id\": "
                                           "\"subtypeId1\",\"label\": \"$string:english\",\"locale\": "
                                           "\"en-US\",\"mode\": \"upper\"}]} ";
    static constexpr const char *INPUT_SYS_CGF = "{\"systemConfig\":{\"enableInputMethodFeature\":true,"
                                                 "\"enableFullExperienceFeature\":true,"
                                                 "\"systemInputMethodConfigAbility\":\"setAbility\","
                                                 "\"defaultInputMethod\":\"bundleName/extName\"}, "
                                                 "\"supportedInputTypeList\":[{\"inputType\":0,\"bundleName\":"
                                                 "\"testBundleName\", "
                                                 "\"subtypeId\":\"testSubtypeId\"},{\"inputType\":1,\"bundleName\":"
                                                 "\"\", \"subtypeId\":\"\"}]}";
    static constexpr const char *SYS_PANEL_ADJUST = "{\"sysPanelAdjust\":"
                                                    "[{\"style\": [\"fix\",\"default\",\"landscape\"],"
                                                    "\"top\": 1,\"left\": 2,\"right\": 3,\"bottom\": 4}]}";

    static void SetUpTestCase()
    {
    }
    static void TearDownTestCase()
    {
    }
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

/**
* @tc.name: testParseEnableIme001
* @tc.desc: parse enableIme
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(JsonOperateTest, testParseEnableIme001, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testParseEnableIme001 START");
    std::vector<std::string> enableVec;
    auto ret = EnableImeDataParser::GetInstance()->ParseEnableIme(ENABLE_IME, 100, enableVec);
    ASSERT_TRUE(ret);
    ASSERT_EQ(enableVec.size(), 3);
    EXPECT_EQ(enableVec[0], "testIme");
    EXPECT_EQ(enableVec[1], "testIme1");
    EXPECT_EQ(enableVec[2], "testIme2");

    std::vector<std::string> enableVec1;
    ret = EnableImeDataParser::GetInstance()->ParseEnableIme(ENABLE_IME, 101, enableVec1);
    ASSERT_TRUE(ret);
    ASSERT_EQ(enableVec1.size(), 1);
    EXPECT_EQ(enableVec1[0], "testIme3");

    std::vector<std::string> enableVec2;
    ret = EnableImeDataParser::GetInstance()->ParseEnableIme(ENABLE_IME, 102, enableVec2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(enableVec2.empty());

    std::vector<std::string> enableVec3;
    ret = EnableImeDataParser::GetInstance()->ParseEnableIme(ENABLE_IME, 104, enableVec3);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(enableVec3.empty());
}
/**
* @tc.name: testParseEnableKeyboard001
* @tc.desc: parse enableKeyboard
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(JsonOperateTest, testParseEnableKeyboard001, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testParseEnableKeyboard001 START");
    std::vector<std::string> enableVec;
    auto ret = EnableImeDataParser::GetInstance()->ParseEnableKeyboard(ENABLE_KEYBOARD, 100, enableVec);
    ASSERT_TRUE(ret);
    ASSERT_EQ(enableVec.size(), 2);
    EXPECT_EQ(enableVec[0], "testKeyboard");
    EXPECT_EQ(enableVec[1], "testKeyboard1");

    std::vector<std::string> enableVec1;
    ret = EnableImeDataParser::GetInstance()->ParseEnableKeyboard(ENABLE_KEYBOARD, 101, enableVec1);
    ASSERT_TRUE(ret);
    ASSERT_EQ(enableVec1.size(), 1);
    EXPECT_EQ(enableVec1[0], "testKeyboard2");

    std::vector<std::string> enableVec2;
    ret = EnableImeDataParser::GetInstance()->ParseEnableKeyboard(ENABLE_KEYBOARD, 105, enableVec2);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(enableVec2.empty());

    std::vector<std::string> enableVec3;
    ret = EnableImeDataParser::GetInstance()->ParseEnableKeyboard(ENABLE_KEYBOARD, 104, enableVec3);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(enableVec3.empty());
}

/**
* @tc.name: testParseSecurityMode001
* @tc.desc: parse securityMode
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(JsonOperateTest, testParseSecurityMode001, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testParseSecurityMode001 START");
    SecurityModeParser::GetInstance()->fullModeList_.clear();
    auto ret = SecurityModeParser::GetInstance()->ParseSecurityMode(JsonOperateTest::SECURITY_MODE, 100);
    ASSERT_TRUE(ret);
    auto secMode = SecurityModeParser::GetInstance()->fullModeList_;
    ASSERT_EQ(secMode.size(), 2);
    EXPECT_EQ(secMode[0], "testIme");
    EXPECT_EQ(secMode[1], "testIme3");

    SecurityModeParser::GetInstance()->fullModeList_.clear();
    ret = SecurityModeParser::GetInstance()->ParseSecurityMode(JsonOperateTest::SECURITY_MODE, 102);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(SecurityModeParser::GetInstance()->fullModeList_.empty());

    SecurityModeParser::GetInstance()->fullModeList_.clear();
    ret = SecurityModeParser::GetInstance()->ParseSecurityMode(JsonOperateTest::SECURITY_MODE, 105);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(SecurityModeParser::GetInstance()->fullModeList_.empty());
}

/**
* @tc.name: testParseImePersistCfg001
* @tc.desc: parse imePersistCfg
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(JsonOperateTest, testParseImePersistCfg001, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testParseImePersistCfg001 START");
    ImeCfgManager::GetInstance().imeConfigs_.clear();
    auto ret = ImeCfgManager::GetInstance().ParseImeCfg(JsonOperateTest::IME_PERSIST_CFG);
    ASSERT_TRUE(ret);
    ASSERT_EQ(ImeCfgManager::GetInstance().imeConfigs_.size(), 2);
    auto cfg = ImeCfgManager::GetInstance().imeConfigs_;
    EXPECT_EQ(cfg[0].userId, 100);
    EXPECT_EQ(cfg[0].currentIme, "bundleName/extName");
    EXPECT_EQ(cfg[0].currentSubName, "subName");
    EXPECT_EQ(cfg[1].userId, 104);
    EXPECT_EQ(cfg[1].currentIme, "bundleName1/extName1");
    EXPECT_EQ(cfg[1].currentSubName, "subName1");

    ImeCfgManager::GetInstance().imeConfigs_.clear();
    ret = ImeCfgManager::GetInstance().ParseImeCfg(JsonOperateTest::IME_PERSIST_CFG_NULL);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ImeCfgManager::GetInstance().imeConfigs_.empty());

    ImeCfgManager::GetInstance().imeConfigs_.clear();
    ret = ImeCfgManager::GetInstance().ParseImeCfg(JsonOperateTest::IME_PERSIST_CFG_VALUE_TYPE_ERROR);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(ImeCfgManager::GetInstance().imeConfigs_.empty());

    ImeCfgManager::GetInstance().imeConfigs_.clear();
    ret = ImeCfgManager::GetInstance().ParseImeCfg(JsonOperateTest::IME_PERSIST_CFG_NAME_LACK);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(ImeCfgManager::GetInstance().imeConfigs_.empty());

    ImeCfgManager::GetInstance().imeConfigs_.clear();
    ret = ImeCfgManager::GetInstance().ParseImeCfg(JsonOperateTest::IME_PERSIST_CFG_NAME_ERROR);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(ImeCfgManager::GetInstance().imeConfigs_.empty());

    ret = ImeCfgManager::GetInstance().ParseImeCfg(JsonOperateTest::ENABLE_KEYBOARD);
    EXPECT_FALSE(ret);
}

/**
* @tc.name: testPackageImePersistCfg001
* @tc.desc: package imePersistCfg
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(JsonOperateTest, testPackageImePersistCfg001, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testPackageImePersistCfg001 START");
    const std::string imePersistCfg = "{\"imeCfgList\":[{\"userId\":100,\"currentIme\":\"bundleName/"
                                            "extName\",\"currentSubName\":\"subName\",\"isDefaultImeSet\":0},"
                                            "{\"userId\":104,\"currentIme\":\"bundleName1/"
                                            "extName1\",\"currentSubName\":\"subName1\","
                                            "\"isDefaultImeSet\":0}]}";
    ImeCfgManager::GetInstance().imeConfigs_.clear();
    ImeCfgManager::GetInstance().imeConfigs_.emplace_back(100, "bundleName/extName", "subName", false);
    ImeCfgManager::GetInstance().imeConfigs_.emplace_back(104, "bundleName1/extName1", "subName1", false);
    auto str = ImeCfgManager::GetInstance().PackageImeCfg();
    EXPECT_EQ(str, imePersistCfg);
}

/**
* @tc.name: testParseSystemConfig001
* @tc.desc: parse systemConfig
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(JsonOperateTest, testParseSystemConfig001, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testParseSystemConfig001 START");
    ImeSystemConfig imeSystemConfig;
    auto ret = imeSystemConfig.Unmarshall(INPUT_SYS_CGF);
    ASSERT_TRUE(ret);
    auto systemConfig = imeSystemConfig.systemConfig;
    EXPECT_EQ(systemConfig.systemInputMethodConfigAbility, "setAbility");
    EXPECT_EQ(systemConfig.defaultInputMethod, "bundleName/extName");
    EXPECT_TRUE(systemConfig.enableInputMethodFeature);
    EXPECT_TRUE(systemConfig.enableFullExperienceFeature);
}

/**
* @tc.name: testParseInputType001
* @tc.desc: parse inputType
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(JsonOperateTest, testParseInputType001, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testParseInputType001 START");
    InputTypeCfg inputTypeCfg;
    auto ret = inputTypeCfg.Unmarshall(INPUT_SYS_CGF);
    ASSERT_TRUE(ret);
    auto inputType = inputTypeCfg.inputType;
    ASSERT_EQ(inputType.size(), 2);
    EXPECT_EQ(inputType[0].type, InputType::CAMERA_INPUT);
    EXPECT_EQ(inputType[0].subName, "testSubtypeId");
    EXPECT_EQ(inputType[0].bundleName, "testBundleName");
    EXPECT_EQ(inputType[1].type, InputType::SECURITY_INPUT);
    EXPECT_EQ(inputType[1].subName, "");
    EXPECT_EQ(inputType[1].bundleName, "");
}

/**
* @tc.name: testParseSubtype001
* @tc.desc: parse subtype
* @tc.type: FUNC
* @tc.require:
* @tc.author: chenyu
*/
HWTEST_F(JsonOperateTest, testParseSubtype001, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testParseSubtype001 START");
    std::vector<std::string> profiles{ { JsonOperateTest::SUBTYPE } };
    SubtypeCfg subtype;
    auto ret = ImeInfoInquirer::GetInstance().ParseSubtypeProfile(profiles, subtype);
    ASSERT_TRUE(ret);
    ASSERT_EQ(subtype.subtypes.size(), 2);
    auto subtypes = subtype.subtypes;
    EXPECT_EQ(subtypes[0].icon, "$media:icon");
    EXPECT_EQ(subtypes[0].id, "subtypeId");
    EXPECT_EQ(subtypes[0].label, "$string:chinese");
    EXPECT_EQ(subtypes[0].locale, "zh-CN");
    EXPECT_EQ(subtypes[0].mode, "lower");
    EXPECT_EQ(subtypes[1].icon, "$media:icon1");
    EXPECT_EQ(subtypes[1].id, "subtypeId1");
    EXPECT_EQ(subtypes[1].label, "$string:english");
    EXPECT_EQ(subtypes[1].locale, "en-US");
    EXPECT_EQ(subtypes[1].mode, "upper");

    std::vector<std::string> profiles1{ { JsonOperateTest::SECURITY_MODE } };
    SubtypeCfg subtype1;
    ret = ImeInfoInquirer::GetInstance().ParseSubtypeProfile(profiles1, subtype1);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(subtype1.subtypes.empty());
}

/**
* @tc.name: testParseSysPanelAdjust001
* @tc.desc: parse SysPanelAdjust
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(JsonOperateTest, testParseSysPanelAdjust001, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testParseSysPanelAdjust001 START");
    SysPanelAdjustCfg sysPanelAdjustCfg;
    auto ret = sysPanelAdjustCfg.Unmarshall(SYS_PANEL_ADJUST);
    ASSERT_TRUE(ret);
    auto panelAdjust = sysPanelAdjustCfg.panelAdjust;
    EXPECT_EQ(panelAdjust[0].style[0], "fix");
    EXPECT_EQ(panelAdjust[0].style[1], "default");
    EXPECT_EQ(panelAdjust[0].style[2], "landscape");
    EXPECT_EQ(panelAdjust[0].top, 1);
    EXPECT_EQ(panelAdjust[0].left, 2);
    EXPECT_EQ(panelAdjust[0].right, 3);
    EXPECT_EQ(panelAdjust[0].bottom, 4);
}

/**
* @tc.name: testGetDumpInfo
* @tc.desc: parse GetDumpInfo
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(JsonOperateTest, testGetDumpInfo, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testGetDumpInfo START");
    int32_t userId = 1234567890;
    auto ret = ImeInfoInquirer::GetInstance().GetDumpInfo(userId);
    ASSERT_EQ(ret, "");
}

/**
* @tc.name: testListDisabledInputMethod
* @tc.desc: ListDisabledInputMethod
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(JsonOperateTest, testListDisabledInputMethod, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testListDisabledInputMethod START");
    int32_t userId = 1234567890;
    std::vector<Property> props;
    bool enableOn = false;
    auto ret = ImeInfoInquirer::GetInstance().ListDisabledInputMethod(userId, props, enableOn);
    ASSERT_EQ(ret, ErrorCode::NO_ERROR);
    enableOn = true;
    ret = ImeInfoInquirer::GetInstance().ListDisabledInputMethod(userId, props, enableOn);
    ASSERT_NE(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testGetSwitchInfoBySwitchCount
* @tc.desc: test GetSwitchInfoBySwitchCount
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(JsonOperateTest, testGetSwitchInfoBySwitchCount, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testGetSwitchInfoBySwitchCount START");
    SwitchInfo switchInfo;
    uint32_t cacheCount = 987654321;
    int32_t userId = 1234567890;
    bool enableOn = false;
    auto ret = ImeInfoInquirer::GetInstance().GetSwitchInfoBySwitchCount(switchInfo, userId, enableOn, cacheCount);
    ASSERT_NE(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testGetInputMethodConfig
* @tc.desc: test GetInputMethodConfig
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(JsonOperateTest, testGetInputMethodConfig, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testGetInputMethodConfig START");
    AppExecFwk::ElementName inputMethodConfig;
    int32_t userId = 100;
    auto ret = ImeInfoInquirer::GetInstance().GetInputMethodConfig(userId, inputMethodConfig);
    ASSERT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testFindTargetSubtypeByCondition
* @tc.desc: test FindTargetSubtypeByCondition
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(JsonOperateTest, testFindTargetSubtypeByCondition, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testFindTargetSubtypeByCondition START");
    std::vector<SubProperty> subProps;
    Condition condition = Condition::ENGLISH;
    auto ret = ImeInfoInquirer::GetInstance().FindTargetSubtypeByCondition(subProps, condition);
    ASSERT_EQ(ret, nullptr);
    int32_t invalidNum = 5;
    condition = static_cast<Condition>(invalidNum);
    ret = ImeInfoInquirer::GetInstance().FindTargetSubtypeByCondition(subProps, condition);
    ASSERT_EQ(ret, nullptr);
}

/**
* @tc.name: testParseSubtypeProfile
* @tc.desc: test ParseSubtypeProfile
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(JsonOperateTest, testParseSubtypeProfile, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testParseSubtypeProfile START");
    std::vector<std::string> profiles;
    SubtypeCfg subtypeCfg;
    auto ret = ImeInfoInquirer::GetInstance().ParseSubtypeProfile(profiles, subtypeCfg);
    ASSERT_FALSE(ret);
}

/**
* @tc.name: testGetResMgr
* @tc.desc: test GetResMgr
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(JsonOperateTest, testGetResMgr, TestSize.Level0)
{
    IMSA_HILOGI("JsonOperateTest testGetResMgr START");
    std::string resourcePath = "";
    auto ret = ImeInfoInquirer::GetInstance().GetResMgr(resourcePath);
    ASSERT_FALSE(ret);
}
} // namespace MiscServices
} // namespace OHOS