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

#include "os_account_adapter.h"

#include <gtest/gtest.h>

#include "mock_os_account_manager.h"
#include "os_account_manager.h"

using namespace OHOS;
using namespace MiscServices;
using namespace AccountSA;

class OsAccountAdapterTest : public testing::Test {
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
};

/**
 * @tc.name: IsOsAccountForeground_Success
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(OsAccountAdapterTest, IsOsAccountForeground_Success, TestSize.Level0)
{
    MockOsAccountManager mockOsAccountManager;
    OsAccountAdapter osAccountAdapter;

    int32_t userId = 100;
    bool isForeground = true;
    EXPECT_CALL(mockOsAccountManager, IsOsAccountForeground(userId, _))
        .WillOnce(testing::DoAll(testing::SetArgReferee<1>(isForeground), testing::Return(ERR_OK)));

    bool result = osAccountAdapter.IsOsAccountForeground(userId);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsOsAccountForeground_Failure
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(OsAccountAdapterTest, IsOsAccountForeground_Failure, TestSize.Level0)
{
    MockOsAccountManager mockOsAccountManager;
    OsAccountAdapter osAccountAdapter;

    int32_t userId = 100;
    EXPECT_CALL(mockOsAccountManager, IsOsAccountForeground(userId, _)).WillOnce(testing::Return(ERR_NO_PERMISSION));

    bool result = osAccountAdapter.IsOsAccountForeground(userId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: QueryActiveOsAccountIds_Success
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(OsAccountAdapterTest, QueryActiveOsAccountIds_Success, TestSize.Level0)
{
    MockOsAccountManager mockOsAccountManager;
    OsAccountAdapter osAccountAdapter;

    std::vector<int32_t> userIds = { 100, 101 };
    EXPECT_CALL(mockOsAccountManager, QueryActiveOsAccountIds(_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<0>(userIds), testing::Return(ERR_OK)));

    std::vector<int32_t> result = osAccountAdapter.QueryActiveOsAccountIds();
    EXPECT_EQ(result, userIds);
}

/**
 * @tc.name: QueryActiveOsAccountIds_Failure
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(OsAccountAdapterTest, QueryActiveOsAccountIds_Failure, TestSize.Level0)
{
    MockOsAccountManager mockOsAccountManager;
    OsAccountAdapter osAccountAdapter;

    EXPECT_CALL(mockOsAccountManager, QueryActiveOsAccountIds(_)).WillOnce(testing::Return(ERR_NO_PERMISSION));

    std::vector<int32_t> result = osAccountAdapter.QueryActiveOsAccountIds();
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetForegroundOsAccountLocalId_Success
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(OsAccountAdapterTest, GetForegroundOsAccountLocalId_Success, TestSize.Level0)
{
    MockOsAccountManager mockOsAccountManager;
    OsAccountAdapter osAccountAdapter;

    int32_t userId = 100;
    EXPECT_CALL(mockOsAccountManager, GetForegroundOsAccountLocalId(_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<0>(userId), testing::Return(ERR_OK)));

    int32_t result = osAccountAdapter.GetForegroundOsAccountLocalId();
    EXPECT_EQ(result, userId);
}

/**
 * @tc.name: GetForegroundOsAccountLocalId_Failure
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(OsAccountAdapterTest, GetForegroundOsAccountLocalId_Failure, TestSize.Level0)
{
    MockOsAccountManager mockOsAccountManager;
    OsAccountAdapter osAccountAdapter;

    EXPECT_CALL(mockOsAccountManager, GetForegroundOsAccountLocalId(_)).WillOnce(testing::Return(ERR_NO_PERMISSION));

    int32_t result = osAccountAdapter.GetForegroundOsAccountLocalId();
    EXPECT_EQ(result, MAIN_USER_ID);
}

/**
 * @tc.name: GetOsAccountLocalIdFromUid_Success
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(OsAccountAdapterTest, GetOsAccountLocalIdFromUid_Success, TestSize.Level0)
{
    MockOsAccountManager mockOsAccountManager;
    OsAccountAdapter osAccountAdapter;

    int32_t uid = 1000;
    int32_t userId = 100;
    EXPECT_CALL(mockOsAccountManager, GetOsAccountLocalIdFromUid(uid, _))
        .WillOnce(testing::DoAll(testing::SetArgReferee<1>(userId), testing::Return(ERR_OK)));

    int32_t result = osAccountAdapter.GetOsAccountLocalIdFromUid(uid);
    EXPECT_EQ(result, userId);
}

/**
 * @tc.name: GetOsAccountLocalIdFromUid_Failure
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(OsAccountAdapterTest, GetOsAccountLocalIdFromUid_Failure, TestSize.Level0)
{
    MockOsAccountManager mockOsAccountManager;
    OsAccountAdapter osAccountAdapter;

    int32_t uid = 1000;
    EXPECT_CALL(mockOsAccountManager, GetOsAccountLocalIdFromUid(uid, _)).WillOnce(testing::Return(ERR_NO_PERMISSION));

    int32_t result = osAccountAdapter.GetOsAccountLocalIdFromUid(uid);
    EXPECT_EQ(result, INVALID_USER_ID);
}

/**
 * @tc.name: IsOsAccountVerified_Success
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(OsAccountAdapterTest, IsOsAccountVerified_Success, TestSize.Level0)
{
    MockOsAccountManager mockOsAccountManager;
    OsAccountAdapter osAccountAdapter;

    int32_t userId = 100;
    bool isUnlocked = true;
    EXPECT_CALL(mockOsAccountManager, IsOsAccountVerified(userId, _))
        .WillOnce(testing::DoAll(testing::SetArgReferee<1>(isUnlocked), testing::Return(ERR_OK)));

    int32_t result = osAccountAdapter.IsOsAccountVerified(userId, isUnlocked);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: IsOsAccountVerified_Failure
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(OsAccountAdapterTest, IsOsAccountVerified_Failure, TestSize.Level0)
{
    MockOsAccountManager mockOsAccountManager;
    OsAccountAdapter osAccountAdapter;

    int32_t userId = 100;
    bool isUnlocked = false;
    EXPECT_CALL(mockOsAccountManager, IsOsAccountVerified(userId, _)).WillOnce(testing::Return(ERR_NO_PERMISSION));

    int32_t result = osAccountAdapter.IsOsAccountVerified(userId, isUnlocked);
    EXPECT_EQ(result, ErrorCode::ERROR_OS_ACCOUNT);
}