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

#include "settings_data_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "data_share_helper.h"
#include "system_ability_manager.h"
#include "system_ability_manager_client.h"

using namespace OHOS;
using namespace MiscServices;
using namespace DataShare;
using namespace testing;

class MockSettingsDataUtils : public SettingsDataUtils {
public:
    MOCK_METHOD3(GetStringValue, int32_t(const std::string &, const std::string &, std::string &));
    MOCK_METHOD3(SetStringValue, bool(const std::string &, const std::string &, const std::string &));
};

class MockSystemAbilityManagerClient : public SystemAbilityManagerClient {
public:
    static sptr<ISystemAbilityManager> GetSystemAbilityManager()
    {
        return mockSystemAbilityManager;
    }
    static sptr<ISystemAbilityManager> mockSystemAbilityManager;
};

class MockSystemAbilityManager : public ISystemAbilityManager {
public:
    MOCK_METHOD1(GetSystemAbility, sptr<IRemoteObject>(int32_t));
};

class MockDataShareHelper : public DataShareHelper {
public:
    MOCK_METHOD3(Query, std::shared_ptr<DataShareResultSet>(
                            const Uri &, const DataSharePredicates &, const std::vector<std::string> &));
    MOCK_METHOD3(Update, int32_t(const Uri &, const DataSharePredicates &, const DataShareValuesBucket &));
    MOCK_METHOD2(Insert, int32_t(const Uri &, const DataShareValuesBucket &));
    MOCK_METHOD0(Release, bool());
};

sptr<ISystemAbilityManager> MockSystemAbilityManagerClient::mockSystemAbilityManager = new MockSystemAbilityManager();

/**
 * @tc.name: EnableIme_MainUser_ReturnsTrue
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SettingsDataUtilsTest, EnableIme_MainUser_ReturnsTrue, TestSize.Level0)
{
    MockSettingsDataUtils mockUtils;
    EXPECT_CALL(mockUtils, GetStringValue(_, _, _)).WillOnce(DoAll(SetArgReferee<2>(""), Return(0)));
    EXPECT_CALL(mockUtils, SetStringValue(_, _, _)).WillOnce(Return(true));

    bool result = mockUtils.EnableIme(100, "com.example.ime");
    EXPECT_TRUE(result);
}

/**
 * @tc.name: EnableIme_NonMainUser_ReturnsFalse
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SettingsDataUtilsTest, EnableIme_NonMainUser_ReturnsFalse, TestSize.Level0)
{
    MockSettingsDataUtils mockUtils;
    bool result = mockUtils.EnableIme(101, "com.example.ime");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: EnableIme_SettingValueNotEmpty_UpdatesJson
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SettingsDataUtilsTest, EnableIme_SettingValueNotEmpty_UpdatesJson, TestSize.Level0)
{
    MockSettingsDataUtils mockUtils;
    EXPECT_CALL(mockUtils, GetStringValue(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>("{\"enableImeList\" : {\"100\" : [\"existing.ime\"]}}"), Return(0)));
    EXPECT_CALL(mockUtils, SetStringValue(_, _, _)).WillOnce(Return(true));

    bool result = mockUtils.EnableIme(100, "com.example.ime");
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SetSettingValues_AddsBundleNameToJson
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SettingsDataUtilsTest, SetSettingValues_AddsBundleNameToJson, TestSize.Level0)
{
    MockSettingsDataUtils mockUtils;
    std::string settingValue = "{\"enableImeList\" : {\"100\" : [\"existing.ime\"]}}";
    std::string bundleName = "com.example.ime";
    std::string expectedValue = "{\"enableImeList\" : {\"100\" : [\"existing.ime\",\"com.example.ime\"]}}";

    std::string result = mockUtils.SetSettingValues(settingValue, bundleName);
    EXPECT_EQ(result, expectedValue);
}