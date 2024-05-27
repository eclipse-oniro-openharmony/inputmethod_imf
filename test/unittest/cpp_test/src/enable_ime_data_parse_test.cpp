/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "ime_info_inquirer.h"

#undef private

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::DataShare;
namespace OHOS {
namespace MiscServices {
const std::string IME_KEY = "settings.inputmethod.enable_ime";
class EnableImeDataParseTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static std::shared_ptr<DataShareHelper> helper_;
    static std::shared_ptr<DataShareResultSet> resultSet_;
    static constexpr int32_t USER_ID = 100;
};
std::shared_ptr<DataShareHelper> EnableImeDataParseTest::helper_;
std::shared_ptr<DataShareResultSet> EnableImeDataParseTest::resultSet_;
constexpr uint32_t USER_100_TOTAL_COUNT = 3;
constexpr uint32_t USER_101_TOTAL_COUNT = 1;
void EnableImeDataParseTest::SetUpTestCase(void)
{
    std::vector<std::string> columns = { "VALUE" };
    helper_ = DataShare::DataShareHelper::Creator(nullptr, "tsetUri", "tsetUri");
    DataSharePredicates predicates;
    Uri uri("tsetUri");
    resultSet_ = helper_->Query(uri, predicates, columns);
    EnableImeDataParser::GetInstance()->Initialize(USER_ID);
    ImeInfoInquirer::currentIme_ = std::make_shared<Property>();
    ImeInfoInquirer::defaultImeProperty_ = std::make_shared<Property>();
    ImeInfoInquirer::defaultIme_ = std::make_shared<ImeInfo>();
    ImeInfoInquirer::GetInstance().GetDefaultImeCfgProp()->name = "defaultImeName";
    ImeInfoInquirer::GetInstance().GetDefaultImeCfgProp()->id = "defaultImeId";
}

void EnableImeDataParseTest::TearDownTestCase(void)
{
}

void EnableImeDataParseTest::SetUp()
{
    resultSet_->strValue_ = "{\"enableImeList\" : {\"100\" : [ \"xiaoyiIme\", \"baiduIme\", "
                            "\"sougouIme\"],\"101\" : [\"sougouIme\"]}}";

    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->name = "defaultImeName";
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->id = "defaultImeId";
    EnableImeDataParser::GetInstance()->enableList_.clear();
}

void EnableImeDataParseTest::TearDown()
{
}

/**
 * @tc.name: testGetEnableData_001
 * @tc.desc: Get 100 user enable ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testGetEnableData_001, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testGetEnableData_001 START");
    std::vector<std::string> enableVec;
    int32_t ret =
        EnableImeDataParser::GetInstance()->GetEnableData(IME_KEY, enableVec, EnableImeDataParseTest::USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(enableVec.size(), USER_100_TOTAL_COUNT);
    if (enableVec.size() == USER_100_TOTAL_COUNT) {
        EXPECT_EQ(enableVec[0], "xiaoyiIme");
        EXPECT_EQ(enableVec[1], "baiduIme");
        EXPECT_EQ(enableVec[2], "sougouIme");
    }
}

/**
 * @tc.name: testGetEnableData_002
 * @tc.desc: Get 101 user enable ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testGetEnableData_002, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testGetEnableData_002 START");
    std::vector<std::string> enableVec;
    int32_t ret = EnableImeDataParser::GetInstance()->GetEnableData(IME_KEY, enableVec, 101);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(enableVec.size(), USER_101_TOTAL_COUNT);
    if (enableVec.size() == USER_101_TOTAL_COUNT) {
        EXPECT_EQ(enableVec[0], "sougouIme");
    }
}

/**
 * @tc.name: testGetEnableData_003
 * @tc.desc: Get 100 user enable ime while enable list is empty.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testGetEnableData_003, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testGetEnableData_003 START");
    EnableImeDataParseTest::resultSet_->strValue_ = "{\"enableImeList\" : {\"100\" : []}}";
    std::vector<std::string> enableVec;
    int32_t ret =
        EnableImeDataParser::GetInstance()->GetEnableData(IME_KEY, enableVec, EnableImeDataParseTest::USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(enableVec.empty());
}

/**
 * @tc.name: testCheckNeedSwitch_001
 * @tc.desc: Check need switch for enable list change, current ime is default
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testCheckNeedSwitch_001, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testCheckNeedSwitch_001 START");
    SwitchInfo switchInfo;
    EnableImeDataParser::GetInstance()->enableList_.clear();
    bool ret =
        EnableImeDataParser::GetInstance()->CheckNeedSwitch(IME_KEY, switchInfo, EnableImeDataParseTest::USER_ID);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: testCheckNeedSwitch_002
 * @tc.desc: Check need switch for enable list change
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testCheckNeedSwitch_002, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testCheckNeedSwitch_002 START");
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->name = "xiaoyiIme";
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->id = "xiaoyiImeId";
    EnableImeDataParseTest::resultSet_->strValue_ = "{\"enableImeList\" : {\"100\" : [\"baiduIme\", "
                                                    "\"sougouIme\"],\"101\" : [\"sougouIme\"]}}";
    SwitchInfo switchInfo;
    EnableImeDataParser::GetInstance()->enableList_[IME_KEY].push_back("xiaoyiIm"
                                                                       "e");
    bool ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(IME_KEY, switchInfo, USER_ID);
    EXPECT_TRUE(ret);
    EXPECT_EQ(switchInfo.bundleName, EnableImeDataParser::GetInstance()->defaultImeInfo_->name);
}

/**
 * @tc.name: testCheckNeedSwitch_003
 * @tc.desc: Check need switch for enable list change
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testCheckNeedSwitch_003, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testCheckNeedSwitch_003 START");
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->name = "xiaoyiIme";
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->id = "xiaoyiImeId";
    EnableImeDataParseTest::resultSet_->strValue_ = "{\"enableImeList\" : {\"100\" : [ \"xiaoyiIme\", \"baiduIme\", "
                                                    "\"sougouIme\"],\"101\" : "
                                                    "[\"sougouIme\"]}}";
    SwitchInfo switchInfo;
    bool ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(IME_KEY, switchInfo, USER_ID);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: testCheckNeedSwitch_004
 * @tc.desc: Check need switch for enable list change
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testCheckNeedSwitch_004, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testCheckNeedSwitch_004 START");
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->name = "xiaoyiIme";
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->id = "xiaoyiImeId";
    EnableImeDataParseTest::resultSet_->strValue_ = "{\"enableImeList\" : {\"100\" : [\"baiduIme\", "
                                                    "\"sougouIme\"],\"101\" : [\"sougouIme\"]}}";
    SwitchInfo switchInfo;
    EnableImeDataParser::GetInstance()->enableList_[IME_KEY].push_back("xiaoyiIme");
    EnableImeDataParser::GetInstance()->enableList_[IME_KEY].push_back("baiduIme");
    bool ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(IME_KEY, switchInfo, USER_ID);
    EXPECT_TRUE(ret);
    EXPECT_EQ(switchInfo.bundleName, "baiduIme");
}

/**
 * @tc.name: testCheckNeedSwitch_005
 * @tc.desc: Check need switch for enable list change
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testCheckNeedSwitch_005, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testCheckNeedSwitch_005 START");
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->name = "xiaoyiIme";
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->id = "xiaoyiImeId";
    EnableImeDataParseTest::resultSet_->strValue_ = "{\"enableImeList\" : {\"100\" : [ \"sougouIme\"],\"101\" : "
                                                    "[\"sougouIme\"]}}";
    SwitchInfo switchInfo;
    EnableImeDataParser::GetInstance()->enableList_[IME_KEY].push_back("xiaoyiIme");
    EnableImeDataParser::GetInstance()->enableList_[IME_KEY].push_back("baiduIme");
    EnableImeDataParser::GetInstance()->enableList_[IME_KEY].push_back("sougouIme");
    bool ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(IME_KEY, switchInfo, USER_ID);
    EXPECT_TRUE(ret);
    EXPECT_EQ(switchInfo.bundleName, "sougouIme");
}

/**
 * @tc.name: testCheckNeedSwitch_006
 * @tc.desc: Check need switch for enable list change
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testCheckNeedSwitch_006, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testCheckNeedSwitch_006 START");
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->name = "xiaoyiIme";
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->id = "xiaoyiImeId";
    SwitchInfo switchInfo;
    bool ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(
        "settings.inputmethod.enable_keyboard", switchInfo, USER_ID);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: testCheckNeedSwitch_007
 * @tc.desc: Check need switch for enable list change
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testCheckNeedSwitch_007, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testCheckNeedSwitch_007 START");
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->name = "defaultImeName";
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->id = "defaultImeId";
    SwitchInfo switchInfo;
    bool ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(
        "settings.inputmethod.enable_keyboard", switchInfo, USER_ID);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: testCheckNeedSwitch_008
 * @tc.desc: Check need switch for switch target ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testCheckNeedSwitch_008, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testCheckNeedSwitch_008 START");
    SwitchInfo switchInfo;
    switchInfo.bundleName = "defaultImeName";
    switchInfo.subName = "defaultImeId";
    bool ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(switchInfo, USER_ID);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testCheckNeedSwitch_009
 * @tc.desc: Check need switch for switch target ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testCheckNeedSwitch_009, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testCheckNeedSwitch_009 START");
    SwitchInfo switchInfo;
    switchInfo.bundleName = "xiaoyiIme";
    bool ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(switchInfo, USER_ID);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testCheckNeedSwitch_010
 * @tc.desc: Check need switch for switch target ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testCheckNeedSwitch_010, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testCheckNeedSwitch_010 START");
    EnableImeDataParseTest::resultSet_->strValue_ = "{\"enableImeList\" : {\"100\" : [ \"sougouIme\"], \"101\" : "
                                                    "[\"sougouIme\"]}}";
    SwitchInfo switchInfo;
    switchInfo.bundleName = "xiaoyiIme";
    bool ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(switchInfo, USER_ID);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: testCheckNeedSwitch_011
 * @tc.desc: Check need switch for enable list change
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testCheckNeedSwitch_011, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testCheckNeedSwitch_011 START");
    EnableImeDataParseTest::resultSet_->strValue_ = "";
    SwitchInfo switchInfo;
    bool ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(IME_KEY, switchInfo, USER_ID);
    EXPECT_FALSE(ret);
    EXPECT_EQ(switchInfo.bundleName, EnableImeDataParser::GetInstance()->defaultImeInfo_->name);
}

/**
 * @tc.name: testCheckNeedSwitch_012
 * @tc.desc: Check need switch for enable list change
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testCheckNeedSwitch_012, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testCheckNeedSwitch_012 START");
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->name = "xiaoyiIme";
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->id = "xiaoyiImeId";
    EnableImeDataParseTest::resultSet_->strValue_ = "{\"enableImeList\" : {\"100\" : []}}";
    SwitchInfo switchInfo;
    EnableImeDataParser::GetInstance()->enableList_[IME_KEY].push_back("xiaoyiIme");
    EnableImeDataParser::GetInstance()->enableList_[IME_KEY].push_back("baiduIme");
    EnableImeDataParser::GetInstance()->enableList_[IME_KEY].push_back("sougouIme");
    bool ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(IME_KEY, switchInfo, USER_ID);
    EXPECT_TRUE(ret);
    EXPECT_EQ(switchInfo.bundleName, EnableImeDataParser::GetInstance()->defaultImeInfo_->name);
}

/**
 * @tc.name: testCheckNeedSwitch_013
 * @tc.desc: Check need switch for enable list change
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testCheckNeedSwitch_013, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testCheckNeedSwitch_013 START");
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->name = "xiaoyiIme";
    ImeInfoInquirer::GetInstance().GetCurrentInputMethod(USER_ID)->id = "xiaoyiImeId";
    SwitchInfo switchInfo;
    bool ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(IME_KEY, switchInfo, USER_ID);
    EXPECT_FALSE(ret);

    EXPECT_EQ(EnableImeDataParser::GetInstance()->enableList_[IME_KEY].size(), USER_100_TOTAL_COUNT);
    if (EnableImeDataParser::GetInstance()->enableList_[IME_KEY].size() == USER_100_TOTAL_COUNT) {
        EXPECT_EQ(EnableImeDataParser::GetInstance()->enableList_[IME_KEY][0], "xiaoyiIme");
        EXPECT_EQ(EnableImeDataParser::GetInstance()->enableList_[IME_KEY][1], "baiduIme");
        EXPECT_EQ(EnableImeDataParser::GetInstance()->enableList_[IME_KEY][2], "sougouIme");
    }
    resultSet_->strValue_ = "{\"enableImeList\" : {\"100\" : [ \"xiaoyiIme\"], \"101\" : [\"sougouIme\"]}}";

    ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(IME_KEY, switchInfo, USER_ID);

    EXPECT_FALSE(ret);
    EXPECT_EQ(EnableImeDataParser::GetInstance()->enableList_[IME_KEY].size(), USER_101_TOTAL_COUNT);
    if (EnableImeDataParser::GetInstance()->enableList_[IME_KEY].size() == USER_101_TOTAL_COUNT) {
        EXPECT_EQ(EnableImeDataParser::GetInstance()->enableList_[IME_KEY][0], "xiaoyiIme");
    }

    ImeInfoInquirer::currentIme_ = nullptr;
    EXPECT_NE(ImeInfoInquirer::defaultImeProperty_, nullptr);
    ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(IME_KEY, switchInfo, USER_ID);
    EXPECT_TRUE(ret);
    EXPECT_EQ(switchInfo.bundleName, ImeInfoInquirer::GetInstance().GetDefaultImeCfgProp()->name);

    ImeInfoInquirer::currentIme_ = std::make_shared<Property>();
    ImeInfoInquirer::defaultImeProperty_ = nullptr;
    ret = EnableImeDataParser::GetInstance()->CheckNeedSwitch(IME_KEY, switchInfo, USER_ID);
    EXPECT_TRUE(ret);
}

/** 
 * @tc.name: testOnUserChanged_001
 * @tc.desc: Test local enable list cache change when user changed.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: mashaoyin
 */
HWTEST_F(EnableImeDataParseTest, testOnUserChanged_001, TestSize.Level0)
{
    IMSA_HILOGI("EnableImeDataParseTest testOnUserChanged_001 START");
    int32_t ret = EnableImeDataParser::GetInstance()->GetEnableData(
        IME_KEY, EnableImeDataParser::GetInstance()->enableList_[IME_KEY], USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(EnableImeDataParser::GetInstance()->enableList_[IME_KEY].size(), USER_100_TOTAL_COUNT);
    // 101 means new user id
    EnableImeDataParser::GetInstance()->OnUserChanged(101);
    EXPECT_EQ(EnableImeDataParser::GetInstance()->enableList_[IME_KEY].size(), USER_101_TOTAL_COUNT);
    if (EnableImeDataParser::GetInstance()->enableList_[IME_KEY].size() == USER_101_TOTAL_COUNT) {
        EXPECT_EQ(EnableImeDataParser::GetInstance()->enableList_[IME_KEY][0], "sougouIme");
    }
}
} // namespace MiscServices
} // namespace OHOS
