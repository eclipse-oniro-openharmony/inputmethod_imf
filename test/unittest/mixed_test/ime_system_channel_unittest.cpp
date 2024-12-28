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

#include "bundle_info.h"
#include "bundle_mgr_client.h"
#include "extension_ability_info.h"
#include "ime_system_channel.h"
#include "metadata.h"
#include "property.h"
#include "system_ability_definition.h"
#include "system_cmd_channel_stub.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace OHOS;
using namespace MiscServices;
using namespace testing;

class MockBundleMgrClient : public BundleMgrClient {
public:
    MOCK_METHOD3(GetBundleInfo, bool(const std::string &, int, BundleInfo &));
};

class MockSystemAbilityProxy : public IInputMethodSystemAbility {
public:
    MOCK_METHOD2(GetDefaultInputMethod, int32_t(std::shared_ptr<Property> &, bool));
};

class MockSystemCmdChannel : public ImeSystemCmdChannel {
public:
    MOCK_METHOD0(GetSystemAbilityProxy, sptr<IInputMethodSystemAbility>());
};

class ImeSystemCmdChannelTest : public Test {
protected:
    void SetUp() override
    {
        systemCmdChannel = new MockSystemCmdChannel();
        bundleMgrClient = new MockBundleMgrClient();
        systemAbilityProxy = new MockSystemAbilityProxy();
    }

    void TearDown() override
    {
        delete systemCmdChannel;
        delete bundleMgrClient;
        delete systemAbilityProxy;
    }

    MockSystemCmdChannel *systemCmdChannel;
    MockBundleMgrClient *bundleMgrClient;
    MockSystemAbilityProxy *systemAbilityProxy;
};

TEST_F(
    ImeSystemCmdChannelTest, GetSmartMenuCfg_DefaultImeCfgSuccess_BundleInfoSuccess_ExtensionFound_ResConfigFileSuccess)
{
    std::shared_ptr<Property> defaultIme = std::make_shared<Property>();
    defaultIme->name = "defaultImeName";
    BundleInfo bundleInfo;
    ExtensionAbilityInfo extInfo;
    extInfo.metadata = {
        Metadata {SMART_MENU_METADATA_NAME, "configFile"}
    };
    bundleInfo.extensionInfos = { extInfo };

    EXPECT_CALL(*systemCmdChannel, GetSystemAbilityProxy()).WillOnce(Return(systemAbilityProxy));
    EXPECT_CALL(*systemAbilityProxy, GetDefaultInputMethod(_, true)).WillOnce(Return(ErrorCode::NO_ERROR));
    EXPECT_CALL(*bundleMgrClient, GetBundleInfo(defaultIme->name, BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO, _))
        .WillOnce(DoAll(SetArgReferee<2>(bundleInfo), Return(true)));
    EXPECT_CALL(*bundleMgrClient, GetResConfigFile(extInfo, SMART_MENU_METADATA_NAME, _))
        .WillOnce(DoAll(SetArgReferee<2>(std::vector<std::string> { "configFile" }), Return(true)));

    std::string result = systemCmdChannel->GetSmartMenuCfg();
    EXPECT_EQ(result, "configFile");
}

TEST_F(ImeSystemCmdChannelTest, GetSmartMenuCfg_DefaultImeCfgFailure)
{
    EXPECT_CALL(*systemCmdChannel, GetSystemAbilityProxy()).WillOnce(Return(systemAbilityProxy));
    EXPECT_CALL(*systemAbilityProxy, GetDefaultInputMethod(_, true)).WillOnce(Return(ErrorCode::ERROR_NULL_POINTER));

    std::string result = systemCmdChannel->GetSmartMenuCfg();
    EXPECT_EQ(result, "");
}

TEST_F(ImeSystemCmdChannelTest, GetSmartMenuCfg_BundleInfoFailure)
{
    std::shared_ptr<Property> defaultIme = std::make_shared<Property>();
    defaultIme->name = "defaultImeName";

    EXPECT_CALL(*systemCmdChannel, GetSystemAbilityProxy()).WillOnce(Return(systemAbilityProxy));
    EXPECT_CALL(*systemAbilityProxy, GetDefaultInputMethod(_, true)).WillOnce(Return(ErrorCode::NO_ERROR));
    EXPECT_CALL(*bundleMgrClient, GetBundleInfo(defaultIme->name, BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO, _))
        .WillOnce(Return(false));

    std::string result = systemCmdChannel->GetSmartMenuCfg();
    EXPECT_EQ(result, "");
}

TEST_F(ImeSystemCmdChannelTest, GetSmartMenuCfg_ExtensionNotFound)
{
    std::shared_ptr<Property> defaultIme = std::make_shared<Property>();
    defaultIme->name = "defaultImeName";
    BundleInfo bundleInfo;

    EXPECT_CALL(*systemCmdChannel, GetSystemAbilityProxy()).WillOnce(Return(systemAbilityProxy));
    EXPECT_CALL(*systemAbilityProxy, GetDefaultInputMethod(_, true)).WillOnce(Return(ErrorCode::NO_ERROR));
    EXPECT_CALL(*bundleMgrClient, GetBundleInfo(defaultIme->name, BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO, _))
        .WillOnce(DoAll(SetArgReferee<2>(bundleInfo), Return(true)));

    std::string result = systemCmdChannel->GetSmartMenuCfg();
    EXPECT_EQ(result, "");
}

TEST_F(ImeSystemCmdChannelTest, GetSmartMenuCfg_ResConfigFileFailure)
{
    std::shared_ptr<Property> defaultIme = std::make_shared<Property>();
    defaultIme->name = "defaultImeName";
    BundleInfo bundleInfo;
    ExtensionAbilityInfo extInfo;
    extInfo.metadata = {
        Metadata {SMART_MENU_METADATA_NAME, "configFile"}
    };
    bundleInfo.extensionInfos = { extInfo };

    EXPECT_CALL(*systemCmdChannel, GetSystemAbilityProxy()).WillOnce(Return(systemAbilityProxy));
    EXPECT_CALL(*systemAbilityProxy, GetDefaultInputMethod(_, true)).WillOnce(Return(ErrorCode::NO_ERROR));
    EXPECT_CALL(*bundleMgrClient, GetBundleInfo(defaultIme->name, BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO, _))
        .WillOnce(DoAll(SetArgReferee<2>(bundleInfo), Return(true)));
    EXPECT_CALL(*bundleMgrClient, GetResConfigFile(extInfo, SMART_MENU_METADATA_NAME, _)).WillOnce(Return(false));

    std::string result = systemCmdChannel->GetSmartMenuCfg();
    EXPECT_EQ(result, "");
}