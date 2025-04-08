/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "full_ime_info_manager.h"
#undef private
#include <gtest/gtest.h>
#include <sys/time.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "global.h"
#include "ime_info_inquirer.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class FullImeInfoManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FullImeInfoManagerTest::SetUpTestCase(void)
{
    IMSA_HILOGI("FullImeInfoManagerTest::SetUpTestCase");
}

void FullImeInfoManagerTest::TearDownTestCase(void)
{
    IMSA_HILOGI("FullImeInfoManagerTest::TearDownTestCase");
    FullImeInfoManager::GetInstance().fullImeInfos_.clear();
}

void FullImeInfoManagerTest::SetUp(void)
{
    IMSA_HILOGI("FullImeInfoManagerTest::SetUp");
    FullImeInfoManager::GetInstance().fullImeInfos_.clear();
}

void FullImeInfoManagerTest::TearDown(void)
{
    IMSA_HILOGI("FullImeInfoManagerTest::TearDown");
}

/**
 * @tc.name: test_Init_001
 * @tc.desc: test Init that QueryFullImeInfo failed.
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Init_001, TestSize.Level0)
{
    IMSA_HILOGI("test_Init_001 start");
    // QueryFullImeInfo failed
    std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> fullImeInfos;
    ImeInfoInquirer::GetInstance().SetFullImeInfo(false, fullImeInfos);
    auto ret = FullImeInfoManager::GetInstance().Init();
    EXPECT_EQ(ret, ErrorCode::ERROR_PACKAGE_MANAGER);
}

/**
 * @tc.name: test_Init_002
 * @tc.desc: test Init succeed.
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Init_002, TestSize.Level0)
{
    Property prop;
    prop.name = "bundleName";
    IMSA_HILOGI("test_Init_002 start");
    int32_t userId = 100;
    bool isNewIme = false;
    uint32_t tokenId = 2;
    std::string appId = "appId";
    uint32_t versionCode = 11;
    std::string installTime = "12345";
    std::vector<FullImeInfo> imeInfos;
    imeInfos.push_back({ isNewIme, tokenId, appId, versionCode, installTime, prop });
    std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> fullImeInfos;
    fullImeInfos.emplace_back(std::make_pair(userId, imeInfos));
    ImeInfoInquirer::GetInstance().SetFullImeInfo(true, fullImeInfos);
    auto ret = FullImeInfoManager::GetInstance().Init();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(FullImeInfoManager::GetInstance().fullImeInfos_.size(), 1);
    auto it = FullImeInfoManager::GetInstance().fullImeInfos_.find(userId);
    ASSERT_NE(it, FullImeInfoManager::GetInstance().fullImeInfos_.end());
    ASSERT_EQ(it->second.size(), 1);
    auto imeInfo = it->second[0];
    EXPECT_EQ(imeInfo.isNewIme, isNewIme);
    EXPECT_EQ(imeInfo.tokenId, tokenId);
    EXPECT_EQ(imeInfo.appId, appId);
    EXPECT_EQ(imeInfo.versionCode, versionCode);
    EXPECT_EQ(imeInfo.prop.name, prop.name);
}

/**
 * @tc.name: test_Add_001
 * @tc.desc: test Add in user added that the userId exists
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Add_001, TestSize.Level0)
{
    IMSA_HILOGI("test_Add_001 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);
    auto ret = FullImeInfoManager::GetInstance().Add(userId);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: test_Add_002
 * @tc.desc: test Add in user added that QueryFullImeInfo failed
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Add_002, TestSize.Level0)
{
    IMSA_HILOGI("test_Add_002 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    ImeInfoInquirer::GetInstance().SetFullImeInfo(false, imeInfos);
    auto ret = FullImeInfoManager::GetInstance().Add(userId);
    EXPECT_EQ(ret, ErrorCode::ERROR_PACKAGE_MANAGER);
}

/**
 * @tc.name: test_Add_003
 * @tc.desc: test Add in user added succeed
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Add_003, TestSize.Level0)
{
    Property prop;
    Property prop1;
    IMSA_HILOGI("test_Add_003 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    bool isNewIme = false;
    uint32_t tokenId = 2;
    std::string appId = "appId";
    uint32_t versionCode = 11;
    std::string installTime = "12345";
    prop.name = "bundleName";

    FullImeInfo imeInfo { isNewIme, tokenId, appId, versionCode, installTime, prop };
    uint32_t tokenId1 = 2;
    std::string appId1 = "appId1";
    prop1.name = "bundleName1";
    FullImeInfo imeInfo1 { isNewIme, tokenId1, appId1, versionCode, installTime, prop1 };
    imeInfos.push_back(imeInfo);
    imeInfos.push_back(imeInfo1);
    ImeInfoInquirer::GetInstance().SetFullImeInfo(true, imeInfos);

    int32_t userId1 = 101;
    auto ret = FullImeInfoManager::GetInstance().Add(userId1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(FullImeInfoManager::GetInstance().fullImeInfos_.size(), 2);
    auto it = FullImeInfoManager::GetInstance().fullImeInfos_.find(userId1);
    ASSERT_NE(it, FullImeInfoManager::GetInstance().fullImeInfos_.end());
    ASSERT_EQ(it->second.size(), 2);
    EXPECT_EQ(it->second[0].isNewIme, isNewIme);
    EXPECT_EQ(it->second[0].tokenId, tokenId);
    EXPECT_EQ(it->second[0].appId, appId);
    EXPECT_EQ(it->second[0].versionCode, versionCode);
    EXPECT_EQ(it->second[0].prop.name, prop.name);
    EXPECT_EQ(it->second[1].isNewIme, isNewIme);
    EXPECT_EQ(it->second[1].tokenId, tokenId1);
    EXPECT_EQ(it->second[1].appId, appId1);
    EXPECT_EQ(it->second[1].versionCode, versionCode);
    EXPECT_EQ(it->second[1].prop.name, prop1.name);
}

/**
 * @tc.name: test_Add_004
 * @tc.desc: test Add in package added that GetFullImeInfo failed
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Add_004, TestSize.Level0)
{
    IMSA_HILOGI("test_Add_004 start");
    FullImeInfo imeInfo;
    ImeInfoInquirer::GetInstance().SetFullImeInfo(false, imeInfo);
    int32_t userId = 100;
    std::string bundleName = "bundleName";
    auto ret = FullImeInfoManager::GetInstance().Add(userId, bundleName);
    EXPECT_EQ(ret, ErrorCode::ERROR_PACKAGE_MANAGER);
}

/**
 * @tc.name: test_Add_005
 * @tc.desc: test Add in package added that userId does not exist;
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Add_005, TestSize.Level0)
{
    IMSA_HILOGI("test_Add_005 start");
    FullImeInfo imeInfo;
    ImeInfoInquirer::GetInstance().SetFullImeInfo(true, imeInfo);
    int32_t userId = 100;
    std::string bundleName = "bundleName";
    auto ret = FullImeInfoManager::GetInstance().Add(userId, bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(FullImeInfoManager::GetInstance().fullImeInfos_.size(), 1);
    auto it = FullImeInfoManager::GetInstance().fullImeInfos_.find(userId);
    ASSERT_NE(it, FullImeInfoManager::GetInstance().fullImeInfos_.end());
    EXPECT_EQ(it->second.size(), 1);
}

/**
 * @tc.name: test_Add_006
 * @tc.desc: test Add in package added that bundleName does not exist;
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Add_006, TestSize.Level0)
{
    IMSA_HILOGI("test_Add_006 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfo imeInfo;
    imeInfo.prop.name = "bundleName";
    imeInfos.push_back(imeInfo);
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    FullImeInfo imeInfo1;
    imeInfo1.prop.name = "bundleName1";
    ImeInfoInquirer::GetInstance().SetFullImeInfo(true, imeInfo1);

    std::string bundleName = "bundleName1";
    auto ret = FullImeInfoManager::GetInstance().Add(userId, bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(FullImeInfoManager::GetInstance().fullImeInfos_.size(), 1);
    auto it = FullImeInfoManager::GetInstance().fullImeInfos_.find(userId);
    ASSERT_NE(it, FullImeInfoManager::GetInstance().fullImeInfos_.end());
    ASSERT_EQ(it->second.size(), 2);
    EXPECT_EQ(it->second[0].prop.name, imeInfo.prop.name);
    EXPECT_EQ(it->second[1].prop.name, imeInfo1.prop.name);
}

/**
 * @tc.name: test_Add_007
 * @tc.desc: test Add in package added that bundleName exists;
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Add_007, TestSize.Level0)
{
    IMSA_HILOGI("test_Add_007 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfo imeInfo;
    imeInfo.isNewIme = true;
    imeInfo.prop.name = "bundleName";
    imeInfos.push_back(imeInfo);
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    FullImeInfo imeInfo1;
    imeInfo1.isNewIme = false;
    imeInfo1.prop.name = "bundleName";
    ImeInfoInquirer::GetInstance().SetFullImeInfo(true, imeInfo1);

    std::string bundleName = "bundleName";
    auto ret = FullImeInfoManager::GetInstance().Add(userId, bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(FullImeInfoManager::GetInstance().fullImeInfos_.size(), 1);
    auto it = FullImeInfoManager::GetInstance().fullImeInfos_.find(userId);
    ASSERT_NE(it, FullImeInfoManager::GetInstance().fullImeInfos_.end());
    ASSERT_EQ(it->second.size(), 1);
    EXPECT_FALSE(it->second[0].isNewIme);
}

/**
 * @tc.name: test_Update_001
 * @tc.desc: test Update in language change that Init failed
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Update_001, TestSize.Level0)
{
    IMSA_HILOGI("test_Update_001 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> fullImeInfos;
    ImeInfoInquirer::GetInstance().SetFullImeInfo(false, fullImeInfos);
    auto ret = FullImeInfoManager::GetInstance().Update();
    EXPECT_EQ(ret, ErrorCode::ERROR_PACKAGE_MANAGER);
    EXPECT_TRUE(FullImeInfoManager::GetInstance().fullImeInfos_.empty());
}

/**
 * @tc.name: test_Update_002
 * @tc.desc: test Update in language change succeed
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Update_002, TestSize.Level0)
{
    IMSA_HILOGI("test_Update_002 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    int32_t userId1 = 101;
    std::vector<FullImeInfo> imeInfos1;
    int32_t userId2 = 102;
    std::vector<FullImeInfo> imeInfos2;
    std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> fullImeInfos;
    fullImeInfos.emplace_back(std::make_pair(userId1, imeInfos1));
    fullImeInfos.emplace_back(std::make_pair(userId2, imeInfos2));
    ImeInfoInquirer::GetInstance().SetFullImeInfo(true, fullImeInfos);

    auto ret = FullImeInfoManager::GetInstance().Update();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(FullImeInfoManager::GetInstance().fullImeInfos_.size(), 2);
    auto it = FullImeInfoManager::GetInstance().fullImeInfos_.find(userId);
    ASSERT_EQ(it, FullImeInfoManager::GetInstance().fullImeInfos_.end());
    auto it1 = FullImeInfoManager::GetInstance().fullImeInfos_.find(userId1);
    ASSERT_NE(it1, FullImeInfoManager::GetInstance().fullImeInfos_.end());
    auto it2 = FullImeInfoManager::GetInstance().fullImeInfos_.find(userId2);
    ASSERT_NE(it2, FullImeInfoManager::GetInstance().fullImeInfos_.end());
}

/**
 * @tc.name: test_Delete_001
 * @tc.desc: test Delete in user removed that userId does not exist
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Delete_001, TestSize.Level0)
{
    IMSA_HILOGI("test_Delete_001 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    int32_t userId1 = 101;
    auto ret = FullImeInfoManager::GetInstance().Delete(userId1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(FullImeInfoManager::GetInstance().fullImeInfos_.size(), 1);
    auto it = FullImeInfoManager::GetInstance().fullImeInfos_.find(userId);
    ASSERT_NE(it, FullImeInfoManager::GetInstance().fullImeInfos_.end());
}

/**
 * @tc.name: test_Delete_002
 * @tc.desc: test Delete in user removed that userId exists
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Delete_002, TestSize.Level0)
{
    IMSA_HILOGI("test_Delete_002 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    auto ret = FullImeInfoManager::GetInstance().Delete(userId);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ASSERT_TRUE(FullImeInfoManager::GetInstance().fullImeInfos_.empty());

    int32_t userId1 = 101;
    std::vector<FullImeInfo> imeInfos1;
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId1, imeInfos1);
    int32_t userId2 = 102;
    std::vector<FullImeInfo> imeInfos2;
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId2, imeInfos2);
    ret = FullImeInfoManager::GetInstance().Delete(userId2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(FullImeInfoManager::GetInstance().fullImeInfos_.size(), 1);
    auto it = FullImeInfoManager::GetInstance().fullImeInfos_.find(userId2);
    ASSERT_EQ(it, FullImeInfoManager::GetInstance().fullImeInfos_.end());
}

/**
 * @tc.name: test_Delete_003
 * @tc.desc: test Delete in package removed that userId does not exist
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Delete_003, TestSize.Level0)
{
    IMSA_HILOGI("test_Delete_003 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    int32_t userId1 = 101;
    std::string bundleName = "bundleName";
    auto ret = FullImeInfoManager::GetInstance().Delete(userId1, bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(FullImeInfoManager::GetInstance().fullImeInfos_.size(), 1);
    auto it = FullImeInfoManager::GetInstance().fullImeInfos_.find(userId);
    ASSERT_NE(it, FullImeInfoManager::GetInstance().fullImeInfos_.end());
}

/**
 * @tc.name: test_Delete_004
 * @tc.desc: test Delete in package removed that bundleName does not exist
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Delete_004, TestSize.Level0)
{
    IMSA_HILOGI("test_Delete_004 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfo imeInfo;
    imeInfo.prop.name = "bundleName";
    imeInfos.push_back(imeInfo);
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    std::string bundleName = "bundleName1";
    auto ret = FullImeInfoManager::GetInstance().Delete(userId, bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(FullImeInfoManager::GetInstance().fullImeInfos_.size(), 1);
    auto it = FullImeInfoManager::GetInstance().fullImeInfos_.find(userId);
    ASSERT_NE(it, FullImeInfoManager::GetInstance().fullImeInfos_.end());
    ASSERT_EQ(it->second.size(), 1);
    EXPECT_EQ(it->second[0].prop.name, imeInfo.prop.name);
}

/**
 * @tc.name: test_Delete_005
 * @tc.desc: test Delete in package removed that bundleName exist
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Delete_005, TestSize.Level0)
{
    IMSA_HILOGI("test_Delete_005 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfo imeInfo;
    imeInfo.prop.name = "bundleName";
    imeInfos.push_back(imeInfo);
    FullImeInfo imeInfo1;
    imeInfo1.prop.name = "bundleName1";
    imeInfos.push_back(imeInfo1);
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    std::string bundleName1 = "bundleName1";
    auto ret = FullImeInfoManager::GetInstance().Delete(userId, bundleName1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(FullImeInfoManager::GetInstance().fullImeInfos_.size(), 1);
    auto it = FullImeInfoManager::GetInstance().fullImeInfos_.find(userId);
    ASSERT_NE(it, FullImeInfoManager::GetInstance().fullImeInfos_.end());
    ASSERT_EQ(it->second.size(), 1);
    EXPECT_EQ(it->second[0].prop.name, imeInfo.prop.name);

    std::string bundleName = "bundleName";
    ret = FullImeInfoManager::GetInstance().Delete(userId, bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(FullImeInfoManager::GetInstance().fullImeInfos_.empty());
}

/**
 * @tc.name: test_Get_001
 * @tc.desc: test Get that by userId that userId does not exist
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Get_001, TestSize.Level0)
{
    IMSA_HILOGI("test_Get_001 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfo info;
    imeInfos.push_back(info);
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    int32_t userId1 = 101;
    std::vector<Property> props;
    auto ret = FullImeInfoManager::GetInstance().Get(userId1, props);
    EXPECT_TRUE(props.empty());
}

/**
 * @tc.name: test_Get_002
 * @tc.desc: test Get that by userId that userId exist
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Get_002, TestSize.Level0)
{
    IMSA_HILOGI("test_Get_002 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfo info;
    imeInfos.push_back(info);
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);
    std::vector<Property> props;
    auto ret = FullImeInfoManager::GetInstance().Get(userId, props);
    EXPECT_EQ(props.size(), 1);
}

/**
 * @tc.name: test_Get_003
 * @tc.desc: test Get that by userId and bundleName that userId does not exist
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Get_003, TestSize.Level0)
{
    IMSA_HILOGI("test_Get_003 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    int32_t userId1 = 101;
    std::string bundleName = "bundleName";
    FullImeInfo info;
    auto ret = FullImeInfoManager::GetInstance().Get(userId1, bundleName, info);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: test_Get_004
 * @tc.desc: test Get that by userId and bundleName that bundleName does not exist
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Get_004, TestSize.Level0)
{
    IMSA_HILOGI("test_Get_004 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfo info;
    info.prop.name = "bundleName1";
    imeInfos.push_back(info);
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    int32_t userId1 = 100;
    std::string bundleName = "bundleName";
    FullImeInfo infoRet;
    auto ret = FullImeInfoManager::GetInstance().Get(userId1, bundleName, infoRet);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: test_Get_005
 * @tc.desc: test Get that by userId and bundleName succeed
 * @tc.type: FUNC
 */
HWTEST_F(FullImeInfoManagerTest, test_Get_005, TestSize.Level0)
{
    IMSA_HILOGI("test_Get_005 start");
    int32_t userId = 100;
    std::vector<FullImeInfo> imeInfos;
    FullImeInfo info;
    info.prop.name = "bundleName1";
    imeInfos.push_back(info);
    FullImeInfoManager::GetInstance().fullImeInfos_.insert_or_assign(userId, imeInfos);

    int32_t userId1 = 100;
    std::string bundleName = "bundleName1";
    FullImeInfo infoRet;
    auto ret = FullImeInfoManager::GetInstance().Get(userId1, bundleName, infoRet);
    EXPECT_TRUE(ret);
    EXPECT_EQ(infoRet.prop.name, info.prop.name);
}
} // namespace MiscServices
} // namespace OHOS