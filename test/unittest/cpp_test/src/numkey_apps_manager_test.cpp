/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "numkey_apps_manager.h"

#include "ime_info_inquirer.h"

#undef private

#include <gtest/gtest.h>

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class NumKeyAppsManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

private:
};

constexpr std::int32_t MAIN_USER_ID = 100;
constexpr std::int32_t INVALID_USER_ID = 10001;
static constexpr const char *WHITE_LIST_APP_NAME = "WHITE_LIST_APP_NAME";
static constexpr const char *BLOCK_LIST_APP_NAME = "BLOCK_LIST_APP_NAME";

void NumKeyAppsManagerTest::SetUpTestCase(void)
{
    IMSA_HILOGI("NumKeyAppsManagerTest::SetUpTestCase");
}

void NumKeyAppsManagerTest::TearDownTestCase(void)
{
    IMSA_HILOGI("NumKeyAppsManagerTest::TearDownTestCase");
}

void NumKeyAppsManagerTest::SetUp()
{
    IMSA_HILOGI("NumKeyAppsManagerTest::SetUp");
}

void NumKeyAppsManagerTest::TearDown()
{
    IMSA_HILOGI("NumKeyAppsManagerTest::TearDown");
}

/**
 * @tc.name: testFeatureNotEnabled_001
 * @tc.desc: when feature not enabled
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testFeatureNotEnabled_001, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testFeatureNotEnabled_001 START");
    NumkeyAppsManager::GetInstance().isFeatureEnabled_ = false;
    ImeInfoInquirer::GetInstance().systemConfig_.enableNumKeyFeature = false;
    int32_t ret = NumkeyAppsManager::GetInstance().Init(MAIN_USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(NumkeyAppsManager::GetInstance().usersBlockList_.find(MAIN_USER_ID),
        NumkeyAppsManager::GetInstance().usersBlockList_.end());
    ret = NumkeyAppsManager::GetInstance().NeedAutoNumKeyInput(MAIN_USER_ID, WHITE_LIST_APP_NAME);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = NumkeyAppsManager::GetInstance().OnUserSwitched(MAIN_USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = NumkeyAppsManager::GetInstance().OnUserRemoved(MAIN_USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testInit_001
 * @tc.desc: test init when feature enabled
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testInit_001, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testInit_001 START");
    NumkeyAppsManager::GetInstance().isFeatureEnabled_ = true;
    ImeInfoInquirer::GetInstance().systemConfig_.enableNumKeyFeature = true;
    int32_t ret = NumkeyAppsManager::GetInstance().Init(MAIN_USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_NE(NumkeyAppsManager::GetInstance().usersBlockList_.find(MAIN_USER_ID),
        NumkeyAppsManager::GetInstance().usersBlockList_.end());
}

/**
 * @tc.name: testNeedAutoNumKeyInput_001
 * @tc.desc: test NeedAutoNumKeyInput when numKeyAppList_ empty or not empty
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testNeedAutoNumKeyInput_001, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testNeedAutoNumKeyInput_001 START");
    NumkeyAppsManager::GetInstance().isFeatureEnabled_ = true;

    NumkeyAppsManager::GetInstance().numKeyAppList_.clear();
    bool ret = NumkeyAppsManager::GetInstance().NeedAutoNumKeyInput(MAIN_USER_ID, WHITE_LIST_APP_NAME);
    EXPECT_FALSE(ret);

    NumkeyAppsManager::GetInstance().numKeyAppList_.insert(WHITE_LIST_APP_NAME);
    NumkeyAppsManager::GetInstance().usersBlockList_.clear();
    ret = NumkeyAppsManager::GetInstance().NeedAutoNumKeyInput(MAIN_USER_ID, WHITE_LIST_APP_NAME);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testNeedAutoNumKeyInput_002
 * @tc.desc: test NeedAutoNumKeyInput when numKeyAppList_ not empty, usersBlockList_ empty or not empty
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testNeedAutoNumKeyInput_002, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testNeedAutoNumKeyInput_002 START");
    NumkeyAppsManager::GetInstance().isFeatureEnabled_ = true;
    NumkeyAppsManager::GetInstance().numKeyAppList_.insert(WHITE_LIST_APP_NAME);

    NumkeyAppsManager::GetInstance().usersBlockList_.clear();
    bool ret = NumkeyAppsManager::GetInstance().NeedAutoNumKeyInput(MAIN_USER_ID, WHITE_LIST_APP_NAME);
    EXPECT_TRUE(ret);

    NumkeyAppsManager::GetInstance().usersBlockList_[MAIN_USER_ID] = { BLOCK_LIST_APP_NAME };
    ret = NumkeyAppsManager::GetInstance().NeedAutoNumKeyInput(MAIN_USER_ID, WHITE_LIST_APP_NAME);
    EXPECT_TRUE(ret);

    NumkeyAppsManager::GetInstance().usersBlockList_[MAIN_USER_ID] = { BLOCK_LIST_APP_NAME };
    ret = NumkeyAppsManager::GetInstance().NeedAutoNumKeyInput(MAIN_USER_ID, BLOCK_LIST_APP_NAME);
    EXPECT_FALSE(ret);

    NumkeyAppsManager::GetInstance().numKeyAppList_.clear();
    NumkeyAppsManager::GetInstance().usersBlockList_.clear();
}

/**
 * @tc.name: testOnUserSwitched_001
 * @tc.desc: already inited, no need to update when user switch
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testOnUserSwitched_001, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testOnUserSwitched_001 START");
    NumkeyAppsManager::GetInstance().isFeatureEnabled_ = true;
    NumkeyAppsManager::GetInstance().usersBlockList_[INVALID_USER_ID] = { BLOCK_LIST_APP_NAME };
    auto ret = NumkeyAppsManager::GetInstance().OnUserSwitched(INVALID_USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(NumkeyAppsManager::GetInstance().usersBlockList_[INVALID_USER_ID].size(), 0);
    EXPECT_EQ(NumkeyAppsManager::GetInstance().usersBlockList_[INVALID_USER_ID].count(BLOCK_LIST_APP_NAME), 0);
}

/**
 * @tc.name: testOnUserSwitched_002
 * @tc.desc: update usersblocklist with user not inited when user switched
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testOnUserSwitched_002, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testOnUserSwitched_002 START");
    NumkeyAppsManager::GetInstance().isFeatureEnabled_ = true;
    NumkeyAppsManager::GetInstance().usersBlockList_.clear();
    auto ret = NumkeyAppsManager::GetInstance().OnUserSwitched(MAIN_USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_NE(NumkeyAppsManager::GetInstance().usersBlockList_.find(MAIN_USER_ID),
        NumkeyAppsManager::GetInstance().usersBlockList_.end());
}

/**
 * @tc.name: testOnUserSwitched_003
 * @tc.desc: test usersBlockList_ not empty after OnUserSwitched
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testOnUserSwitched_003, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testOnUserSwitched_003 START");
    NumkeyAppsManager::GetInstance().usersBlockList_.clear();
    auto ret = NumkeyAppsManager::GetInstance().OnUserSwitched(MAIN_USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    bool result = NumkeyAppsManager::GetInstance().usersBlockList_.find(MAIN_USER_ID)
                  != NumkeyAppsManager::GetInstance().usersBlockList_.end();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: testOnUserRemoved_001
 * @tc.desc: user removed when observers empty
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testOnUserRemoved_001, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testOnUserRemoved_001 START");
    NumkeyAppsManager::GetInstance().isFeatureEnabled_ = true;
    NumkeyAppsManager::GetInstance().observers_.clear();
    auto ret = NumkeyAppsManager::GetInstance().OnUserRemoved(MAIN_USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testOnUserRemoved_002
 * @tc.desc: observers_ not empty, remove valid user
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testOnUserRemoved_002, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testOnUserRemoved_002 START");
    NumkeyAppsManager::GetInstance().isFeatureEnabled_ = true;
    sptr<SettingsDataObserver> observer = new (std::nothrow) SettingsDataObserver("", "", nullptr);
    ASSERT_TRUE(observer != nullptr);
    NumkeyAppsManager::GetInstance().observers_.clear();
    NumkeyAppsManager::GetInstance().observers_[MAIN_USER_ID] = observer;
    NumkeyAppsManager::GetInstance().OnUserRemoved(MAIN_USER_ID);
    EXPECT_EQ(NumkeyAppsManager::GetInstance().observers_.find(MAIN_USER_ID),
        NumkeyAppsManager::GetInstance().observers_.end());
    NumkeyAppsManager::GetInstance().observers_.clear();
}

/**
 * @tc.name: testOnUserRemoved_003
 * @tc.desc: observers_ not empty, remove invalid user
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testOnUserRemoved_003, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testOnUserRemoved_003 START");
    NumkeyAppsManager::GetInstance().isFeatureEnabled_ = true;
    sptr<SettingsDataObserver> observer = new (std::nothrow) SettingsDataObserver("", "", nullptr);
    ASSERT_TRUE(observer != nullptr);
    NumkeyAppsManager::GetInstance().observers_.clear();
    NumkeyAppsManager::GetInstance().observers_[MAIN_USER_ID] = observer;
    NumkeyAppsManager::GetInstance().OnUserRemoved(INVALID_USER_ID);
    EXPECT_NE(NumkeyAppsManager::GetInstance().observers_.find(MAIN_USER_ID),
        NumkeyAppsManager::GetInstance().observers_.end());
    NumkeyAppsManager::GetInstance().observers_.clear();
}

/**
 * @tc.name: testInitWhiteList_001
 * @tc.desc: InitWhiteList when already inited
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testInitWhiteList_001, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testInitWhiteList_001 START");
    bool inited = NumkeyAppsManager::GetInstance().isListInited_.load();
    NumkeyAppsManager::GetInstance().isListInited_.store(true);
    auto ret = NumkeyAppsManager::GetInstance().InitWhiteList();
    NumkeyAppsManager::GetInstance().isListInited_.store(inited);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testInitWhiteList_002
 * @tc.desc: InitWhiteList when not inited
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testInitWhiteList_002, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testInitWhiteList_002 START");
    bool inited = NumkeyAppsManager::GetInstance().isListInited_.load();
    NumkeyAppsManager::GetInstance().isListInited_.store(false);
    auto ret = NumkeyAppsManager::GetInstance().InitWhiteList();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    NumkeyAppsManager::GetInstance().isListInited_.store(inited);
}

/**
 * @tc.name: testUpdateUserBlockList_001
 * @tc.desc: usersBlockList_ not empty update user block list
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testUpdateUserBlockList_001, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testUpdateUserBlockList_001 START");
    NumkeyAppsManager::GetInstance().usersBlockList_.clear();
    auto ret = NumkeyAppsManager::GetInstance().UpdateUserBlockList(MAIN_USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_NE(NumkeyAppsManager::GetInstance().usersBlockList_.find(MAIN_USER_ID),
        NumkeyAppsManager::GetInstance().usersBlockList_.end());
    NumkeyAppsManager::GetInstance().usersBlockList_.clear();
}

/**
 * @tc.name: testParseWhiteList_001
 * @tc.desc: test ParseWhiteList
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testParseWhiteList_001, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testParseWhiteList_001 START");
    std::unordered_set<std::string> list;
    auto ret = NumkeyAppsManager::GetInstance().ParseWhiteList(list);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testParseBlockList_001
 * @tc.desc: test ParseBlockList
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testParseBlockList_001, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testParseBlockList_001 START");
    std::unordered_set<std::string> list;
    auto ret = NumkeyAppsManager::GetInstance().ParseBlockList(MAIN_USER_ID, list);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testRegisterUserBlockListData_001
 * @tc.desc: test RegisterUserBlockListData
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testRegisterUserBlockListData_001, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testRegisterUserBlockListData_001 START");
    NumkeyAppsManager::GetInstance().observers_.clear();
    auto ret = NumkeyAppsManager::GetInstance().RegisterUserBlockListData(MAIN_USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testRegisterUserBlockListData_002
 * @tc.desc: test RegisterUserBlockListData
 * @tc.type: FUNC
 */
HWTEST_F(NumKeyAppsManagerTest, testRegisterUserBlockListData_002, TestSize.Level1)
{
    IMSA_HILOGI("NumKeyAppsManagerTest testRegisterUserBlockListData_002 START");
    NumkeyAppsManager::GetInstance().observers_.clear();
    sptr<SettingsDataObserver> observer = new (std::nothrow) SettingsDataObserver("", "", nullptr);
    ASSERT_TRUE(observer != nullptr);
    NumkeyAppsManager::GetInstance().observers_[MAIN_USER_ID] = observer;
    auto ret = NumkeyAppsManager::GetInstance().RegisterUserBlockListData(MAIN_USER_ID);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    NumkeyAppsManager::GetInstance().observers_.clear();
}
} // namespace MiscServices
} // namespace OHOS
