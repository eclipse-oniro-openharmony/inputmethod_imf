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

#include "ime_info_inquirer.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "extension_ability_info.h"
#include "input_type_manager.h"
#include "mock_app_mgr_client.h"
#include "mock_bundle_info.h"
#include "mock_bundle_manager.h"
#include "mock_bundle_mgr.h"
#include "mock_bundle_mgr_client.h"
#include "mock_enable_ime_data_parser.h"
#include "mock_error_code.h"
#include "mock_extension_ability_info.h"
#include "mock_extension_ability_type.h"
#include "mock_full_ime_info_manager.h"
#include "mock_ime_cfg_manager.h"
#include "mock_ime_info_inquirer.h"
#include "mock_ime_target_string.h"
#include "mock_input_method_info.h"
#include "mock_input_method_status.h"
#include "mock_metadata.h"
#include "mock_os_account_adapter.h"
#include "mock_property.h"
#include "mock_res_config.h"
#include "mock_resource_manager.h"
#include "mock_string_retriever.h"
#include "mock_subproperty.h"
#include "mock_subtype.h"
#include "mock_subtype_cfg.h"
#include "mock_subtypes.h"
#include "mock_switch_info.h"
#include "mock_sys_cfg_parser.h"
#include "resource_manager.h"

using namespace testing;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Global::I18n;
namespace {
const int SUBTYPE_PROFILE_NUM = 5;
}
class MockResourceManager : public ResourceManager {
public:
    RState GetStringById(uint32_t labelId, std::string &label) override
    {
        label = "mock_label";
        return RState::SUCCESS;
    }
};

class MockInputTypeManager : public InputTypeManager {
public:
    static std::shared_ptr<InputTypeManager> GetInstance()
    {
        static MockInputTypeManager instance;
        return std::shared_ptr<InputTypeManager>(&instance, [](InputTypeManager *) {});
    }

    bool IsInputType(std::pair<std::string, std::string> imeId) override
    {
        return false;
    }
};

class MockBundleMgrClient {
public:
    static MockBundleMgrClient &GetInstance()
    {
        static MockBundleMgrClient instance;
        return instance;
    }

    std::shared_ptr<ResourceManager> CreateResourceManager()
    {
        return std::make_shared<MockResourceManager>();
    }

    std::unique_ptr<ResConfig> CreateResConfig()
    {
        return std::make_unique<ResConfig>();
    }
};

class MockImeInfoInquirer : public ImeInfoInquirer {
public:
    MockImeInfoInquirer()
    {
    }

    int32_t ParseSubtype(const OHOS::AppExecFwk::ExtensionAbilityInfo &extInfo, std::vector<Subtype> &subtypes) override
    {
        return ErrorCode::NO_ERROR;
    }

    std::shared_ptr<ResourceManager> GetResMgr(const std::string &resourcePath) override
    {
        return std::make_shared<MockResourceManager>();
    }
};

class MockBundleMgrClientImpl : public BundleMgrClientImpl {
public:
    MOCK_METHOD3(GetResConfigFile,
        bool(const ExtensionAbilityInfo &extInfo, const std::string &name, std::vector<std::string> &profiles));
};

class MockImeInfoInquirer : public ImeInfoInquirer {
public:
    MOCK_METHOD2(ParseSubtypeProfile, bool(const std::vector<std::string> &profiles, SubtypeCfg &subtypeCfg));
};

class MockImeInfoInquirer : public ImeInfoInquirer {
public:
    MOCK_METHOD(std::shared_ptr<ImeConfigProp>, GetDefaultImeCfgProp, (), (override));
    MOCK_METHOD(std::shared_ptr<ImeInfo>, GetImeInfo,
        (int32_t userId, const std::string &bundleName, const std::string &imeId), (override));
};

class ImeInfoInquirerTest : public Test {
public:
    static void SetUpTestCase()
    {
        // 设置测试案例
        mockBundleManager = std::make_shared<MockBundleManager>();
        mockAppMgrClient = std::make_shared<MockAppMgrClient>();
        mockImeCfgManager = std::make_shared<MockImeCfgManager>();
        mockFullImeInfoManager = std::make_shared<MockFullImeInfoManager>();
        mockOsAccountAdapter = std::make_shared<MockOsAccountAdapter>();
        mockExtensionAbilityInfo = std::make_shared<MockExtensionAbilityInfo>();
        mockBundleInfo = std::make_shared<MockBundleInfo>();
        mockResConfig = std::make_shared<MockResConfig>();
        mockResourceManager = std::make_shared<MockResourceManager>();
        mockImeInfoInquirer = std::make_shared<MockImeInfoInquirer>();
    }

    static void TearDownTestCase()
    {
        // 清理测试案例
        mockBundleManager.reset();
        mockAppMgrClient.reset();
        mockImeCfgManager.reset();
        mockFullImeInfoManager.reset();
        mockOsAccountAdapter.reset();
        mockExtensionAbilityInfo.reset();
        mockBundleInfo.reset();
        mockResConfig.reset();
        mockResourceManager.reset();
        mockImeInfoInquirer.reset();
    }

    void SetUp() override
    {
        // 设置测试
        mockBundleMgr = std::make_shared<MockBundleMgr>();
        mockFullImeInfoManager = std::make_shared<MockFullImeInfoManager>();
        mockImeCfgManager = std::make_shared<MockImeCfgManager>();
        mockEnableImeDataParser = std::make_shared<MockEnableImeDataParser>();
        mockSysCfgParser = std::make_shared<MockSysCfgParser>();
        mockStringRetriever = std::make_shared<MockStringRetriever>();
        mockExtensionAbilityInfo = std::make_shared<MockExtensionAbilityInfo>();
        mockMetadata = std::make_shared<MockMetadata>();
        mockSubtype = std::make_shared<MockSubtype>();
        mockSubProperty = std::make_shared<MockSubProperty>();
        mockInputMethodInfo = std::make_shared<MockInputMethodInfo>();
        mockProperty = std::make_shared<MockProperty>();
        mockSwitchInfo = std::make_shared<MockSwitchInfo>();
        mockSubtypes = std::make_shared<MockSubtypes>();
        mockExtensionAbilityType = std::make_shared<MockExtensionAbilityType>();
        mockImeTargetString = std::make_shared<MockImeTargetString>();
        mockErrorCode = std::make_shared<MockErrorCode>();
        mockInputMethodStatus = std::make_shared<MockInputMethodStatus>();
        MockBundleMgrClient::GetInstance().CreateResourceManager();
        MockBundleMgrClient::GetInstance().CreateResConfig();
        InputTypeManager::GetInstance();
        imeInfoInquirer = std::make_unique<MockImeInfoInquirer>();
        bundleMgrClientImpl = std::make_unique<MockBundleMgrClientImpl>();
        mockImeInfoInquirer_ = std::make_shared<MockImeInfoInquirer>();
        mockFullImeInfoManager_ = std::make_shared<MockFullImeInfoManager>();
        mockExtensionAbilityInfo_ = std::make_shared<MockExtensionAbilityInfo>();
        mockInquirer = std::make_shared<MockImeInfoInquirer>();
        mockBundleManager = std::make_shared<MockBundleManager>();
        mockAppMgrClient = std::make_shared<MockAppMgrClient>();

        mockFullImeInfoManager = std::make_shared<MockFullImeInfoManager>();
        mockOsAccountAdapter = std::make_shared<MockOsAccountAdapter>();
        mockExtensionAbilityInfo = std::make_shared<MockExtensionAbilityInfo>();
        mockBundleInfo = std::make_shared<MockBundleInfo>();
        mockResConfig = std::make_shared<MockResConfig>();
        mockResourceManager = std::make_shared<MockResourceManager>();
        mockImeInfoInquirer = std::make_shared<MockImeInfoInquirer>();
        ON_CALL(*mockBundleManager, GetBundleInfo(_, _, _, _)).WillByDefault(Return(true));
        ON_CALL(*mockAppMgrClient, GetProcessRunningInfosByUserId(_, _)).WillByDefault(Return(ErrorCode::NO_ERROR));
        ON_CALL(*mockImeCfgManager, IsDefaultImeSet(_)).WillByDefault(Return(true));
        ON_CALL(*mockFullImeInfoManager, Get(_, _, _)).WillByDefault(Return(true));
        ON_CALL(*mockOsAccountAdapter, QueryActiveOsAccountIds()).WillByDefault(Return(std::vector<int32_t>{ 1 }));
        ON_CALL(*mockExtensionAbilityInfo, IsTempInputMethod(_)).WillByDefault(Return(false));
        ON_CALL(*mockBundleInfo, GetBundleInfo(_, _, _, _)).WillByDefault(Return(true));
        ON_CALL(*mockResConfig, SetLocaleInfo(_, _, _)).WillByDefault(Return());
        ON_CALL(*mockResourceManager, AddResource(_)).WillByDefault(Return());
        ON_CALL(*mockResourceManager, UpdateResConfig(_)).WillByDefault(Return());
        ON_CALL(*mockImeInfoInquirer, QueryImeExtInfos(_, _)).WillByDefault(Return(true));
    }

    void TearDown() override
    {
        // 清理测试
        imeInfoInquirer.reset();
        bundleMgrClientImpl.reset();
        mockBundleManager.reset();
        mockAppMgrClient.reset();
        mockImeCfgManager.reset();
        mockFullImeInfoManager.reset();
        mockOsAccountAdapter.reset();
        mockExtensionAbilityInfo.reset();
        mockBundleInfo.reset();
        mockResConfig.reset();
        mockResourceManager.reset();
        mockImeInfoInquirer.reset();
    }

    std::shared_ptr<MockBundleMgr> mockBundleMgr;
    std::shared_ptr<MockFullImeInfoManager> mockFullImeInfoManager;
    std::shared_ptr<MockImeCfgManager> mockImeCfgManager;
    std::shared_ptr<MockEnableImeDataParser> mockEnableImeDataParser;
    std::shared_ptr<MockSysCfgParser> mockSysCfgParser;
    std::shared_ptr<MockStringRetriever> mockStringRetriever;
    std::shared_ptr<MockExtensionAbilityInfo> mockExtensionAbilityInfo;
    std::shared_ptr<MockMetadata> mockMetadata;
    std::shared_ptr<MockSubtype> mockSubtype;
    std::shared_ptr<MockSubProperty> mockSubProperty;
    std::shared_ptr<MockInputMethodInfo> mockInputMethodInfo;
    std::shared_ptr<MockProperty> mockProperty;
    std::shared_ptr<MockSwitchInfo> mockSwitchInfo;
    std::shared_ptr<MockSubtypes> mockSubtypes;
    std::shared_ptr<MockExtensionAbilityType> mockExtensionAbilityType;
    std::shared_ptr<MockImeTargetString> mockImeTargetString;
    std::shared_ptr<MockErrorCode> mockErrorCode;
    std::shared_ptr<MockInputMethodStatus> mockInputMethodStatus;
    std::unique_ptr<MockImeInfoInquirer> imeInfoInquirer;
    std::unique_ptr<MockBundleMgrClientImpl> bundleMgrClientImpl;
    std::shared_ptr<MockImeInfoInquirer> mockImeInfoInquirer_;
    std::shared_ptr<MockImeCfgManager> mockImeCfgManager_;
    std::shared_ptr<MockFullImeInfoManager> mockFullImeInfoManager_;
    std::shared_ptr<MockExtensionAbilityInfo> mockExtensionAbilityInfo_;
    std::shared_ptr<MockImeInfoInquirer> mockInquirer;

    static std::shared_ptr<MockBundleManager> mockBundleManager;
    static std::shared_ptr<MockAppMgrClient> mockAppMgrClient;
    static std::shared_ptr<MockImeCfgManager> mockImeCfgManager;
    static std::shared_ptr<MockFullImeInfoManager> mockFullImeInfoManager;
    static std::shared_ptr<MockOsAccountAdapter> mockOsAccountAdapter;
    static std::shared_ptr<MockExtensionAbilityInfo> mockExtensionAbilityInfo;
    static std::shared_ptr<MockBundleInfo> mockBundleInfo;
    static std::shared_ptr<MockResConfig> mockResConfig;
    static std::shared_ptr<MockResourceManager> mockResourceManager;
    static std::shared_ptr<MockImeInfoInquirer> mockImeInfoInquirer;
};

std::shared_ptr<MockBundleManager> ImeInfoInquirerTest::mockBundleManager;
std::shared_ptr<MockAppMgrClient> ImeInfoInquirerTest::mockAppMgrClient;
std::shared_ptr<MockImeCfgManager> ImeInfoInquirerTest::mockImeCfgManager;
std::shared_ptr<MockFullImeInfoManager> ImeInfoInquirerTest::mockFullImeInfoManager;
std::shared_ptr<MockOsAccountAdapter> ImeInfoInquirerTest::mockOsAccountAdapter;
std::shared_ptr<MockExtensionAbilityInfo> ImeInfoInquirerTest::mockExtensionAbilityInfo;
std::shared_ptr<MockBundleInfo> ImeInfoInquirerTest::mockBundleInfo;
std::shared_ptr<MockResConfig> ImeInfoInquirerTest::mockResConfig;
std::shared_ptr<MockResourceManager> ImeInfoInquirerTest::mockResourceManager;
std::shared_ptr<MockImeInfoInquirer> ImeInfoInquirerTest::mockImeInfoInquirer;

HWTEST_F(ImeInfoInquirerTest, QueryImeExtInfos_BundleMgrNull_ReturnsFalse, TestSize.Level0)
{
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfos(_, _, _)).WillOnce(Return(false));
    bool result = ImeInfoInquirer::GetInstance().QueryImeExtInfos(1, std::vector<ExtensionAbilityInfo>());
    EXPECT_FALSE(result);
}

HWTEST_F(ImeInfoInquirerTest, GetExtInfosByBundleName_QueryFailure_ReturnsError, TestSize.Level0)
{
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfos(_, _, _)).WillOnce(Return(false));
    int32_t result =
        ImeInfoInquirer::GetInstance().GetExtInfosByBundleName(1, "bundleName", std::vector<ExtensionAbilityInfo>());
    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
}

HWTEST_F(ImeInfoInquirerTest, GetImeInfoFromCache_NotFound_ReturnsNull, TestSize.Level0)
{
    EXPECT_CALL(*mockFullImeInfoManager, Get(_)).WillOnce(Return(std::vector<FullImeInfo>()));
    std::shared_ptr<ImeInfo> result = ImeInfoInquirer::GetInstance().GetImeInfoFromCache(1, "bundleName", "subName");
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(ImeInfoInquirerTest, GetImeInfoFromBundleMgr_QueryFailure_ReturnsNull, TestSize.Level0)
{
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfos(_, _, _)).WillOnce(Return(false));
    std::shared_ptr<ImeInfo> result =
        ImeInfoInquirer::GetInstance().GetImeInfoFromBundleMgr(1, "bundleName", "subName");
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(ImeInfoInquirerTest, ListInputMethodInfo_QueryFailure_ReturnsEmpty, TestSize.Level0)
{
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfos(_, _, _)).WillOnce(Return(false));
    std::vector<InputMethodInfo> result = ImeInfoInquirer::GetInstance().ListInputMethodInfo(1);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(ImeInfoInquirerTest, ListInputMethod_StatusAll_ReturnsError, TestSize.Level0)
{
    int32_t result = ImeInfoInquirer::GetInstance().ListInputMethod(1, InputMethodStatus::ALL, std::vector<Property>());
    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
}

HWTEST_F(ImeInfoInquirerTest, ListEnabledInputMethod_EnableOn_ReturnsError, TestSize.Level0)
{
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfos(_, _, _)).WillOnce(Return(false));
    int32_t result = ImeInfoInquirer::GetInstance().ListEnabledInputMethod(1, std::vector<Property>(), true);
    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
}

HWTEST_F(ImeInfoInquirerTest, GetSwitchInfoBySwitchCount_CurrentImeNotFound_ReturnsError, TestSize.Level0)
{
    EXPECT_CALL(*mockBundleMgr, QueryExtensionAbilityInfos(_, _, _)).WillOnce(Return(false));
    SwitchInfo switchInfo;
    int32_t result = ImeInfoInquirer::GetInstance().GetSwitchInfoBySwitchCount(switchInfo, 1, true, 0);
    EXPECT_EQ(result, ErrorCode::ERROR_PACKAGE_MANAGER);
}

HWTEST_F(ImeInfoInquirerTest, ListInputMethodSubtype_NotFound_ReturnsError, TestSize.Level0)
{
    EXPECT_CALL(*mockFullImeInfoManager, Get(_)).WillOnce(Return(std::vector<FullImeInfo>()));
    int32_t result = ImeInfoInquirer::GetInstance().ListInputMethodSubtype(1, "bundleName", std::vector<SubProperty>());
    EXPECT_EQ(result, ErrorCode::ERROR_PACKAGE_MANAGER);
}

HWTEST_F(ImeInfoInquirerTest, IsNewExtInfos_EmptyExtInfos_ReturnsFalse, TestSize.Level0)
{
    bool result = ImeInfoInquirer::GetInstance().IsNewExtInfos(std::vector<ExtensionAbilityInfo>());
    EXPECT_FALSE(result);
}

HWTEST_F(ImeInfoInquirerTest, GetSubProperty_EmptyExtInfos_ReturnsError, TestSize.Level0)
{
    SubProperty subProp;
    int32_t result =
        ImeInfoInquirer::GetInstance().GetSubProperty(1, "subName", std::vector<ExtensionAbilityInfo>(), subProp);
    EXPECT_EQ(result, ErrorCode::ERROR_PACKAGE_MANAGER);
}

HWTEST_F(ImeInfoInquirerTest, ListInputMethodSubtype_EmptySubtypes_ReturnsError, TestSize.Level0)
{
    std::vector<ExtensionAbilityInfo> extInfos;
    extInfos.push_back(*mockExtensionAbilityInfo);
    int32_t result = ImeInfoInquirer::GetInstance().ListInputMethodSubtype(1, extInfos, std::vector<SubProperty>());
    EXPECT_EQ(result, ErrorCode::ERROR_PACKAGE_MANAGER);
}

HWTEST_F(ImeInfoInquirerTest, ListInputMethodSubtype_WithValidInputs_ReturnsSubTypes, TestSize.Level0)
{
    MockImeInfoInquirer inquirer;
    ExtensionAbilityInfo extInfo;
    extInfo.bundleName = "com.example.inputmethod";
    extInfo.resourcePath = "res/ime.xml";
    std::vector<SubProperty> subProps;

    int32_t result = inquirer.ListInputMethodSubtype(1000, extInfo, subProps);

    EXPECT_EQ(result, ErrorCode::NO_ERROR);
    EXPECT_EQ(subProps.size(), 1); // 假设解析为一个子类型
}

HWTEST_F(ImeInfoInquirerTest, ListInputMethodSubtype_WithFailedParse_ReturnsError, TestSize.Level0)
{
    MockImeInfoInquirer inquirer;
    ExtensionAbilityInfo extInfo;
    extInfo.bundleName = "com.example.inputmethod";
    extInfo.resourcePath = "res/ime.xml";
    std::vector<SubProperty> subProps;

    // 模拟解析失败
    inquirer.MockParseSubtypeFailure();

    int32_t result = inquirer.ListInputMethodSubtype(1000, extInfo, subProps);

    EXPECT_EQ(result, ErrorCode::ERROR_BAD_PARAMETERS);
    EXPECT_EQ(subProps.size(), 0);
}

HWTEST_F(ImeInfoInquirerTest, ParseSubtype_MetadataEmpty_ReturnsError, TestSize.Level0)
{
    ExtensionAbilityInfo extInfo;
    std::vector<Subtype> subtypes;
    EXPECT_EQ(imeInfoInquirer->ParseSubtype(extInfo, subtypes), ErrorCode::ERROR_BAD_PARAMETERS);
}

HWTEST_F(ImeInfoInquirerTest, ParseSubtype_MetadataNotFound_ReturnsError, TestSize.Level0)
{
    ExtensionAbilityInfo extInfo;
    Metadata metadata;
    metadata.name = "other_name";
    extInfo.metadata.push_back(metadata);
    std::vector<Subtype> subtypes;
    EXPECT_EQ(imeInfoInquirer->ParseSubtype(extInfo, subtypes), ErrorCode::ERROR_BAD_PARAMETERS);
}

HWTEST_F(ImeInfoInquirerTest, ParseSubtype_GetResConfigFileFails_ReturnsPackageManagerError, TestSize.Level0)
{
    ExtensionAbilityInfo extInfo;
    Metadata metadata;
    metadata.name = SUBTYPE_PROFILE_METADATA_NAME;
    extInfo.metadata.push_back(metadata);

    EXPECT_CALL(*bundleMgrClientImpl, GetResConfigFile(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(false));

    std::vector<Subtype> subtypes;
    EXPECT_EQ(imeInfoInquirer->ParseSubtype(extInfo, subtypes), ErrorCode::ERROR_PACKAGE_MANAGER);
}

HWTEST_F(ImeInfoInquirerTest, ParseSubtype_ParseSubtypeProfileFails_ReturnsError, TestSize.Level0)
{
    ExtensionAbilityInfo extInfo;
    Metadata metadata;
    metadata.name = SUBTYPE_PROFILE_METADATA_NAME;
    extInfo.metadata.push_back(metadata);

    EXPECT_CALL(*bundleMgrClientImpl, GetResConfigFile(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(true));
    EXPECT_CALL(*imeInfoInquirer, ParseSubtypeProfile(testing::_, testing::_)).WillOnce(testing::Return(false));

    std::vector<Subtype> subtypes;
    EXPECT_EQ(imeInfoInquirer->ParseSubtype(extInfo, subtypes), ErrorCode::ERROR_BAD_PARAMETERS);
}

HWTEST_F(ImeInfoInquirerTest, ParseSubtype_Success_ReturnsNoError, TestSize.Level0)
{
    ExtensionAbilityInfo extInfo;
    Metadata metadata;
    metadata.name = SUBTYPE_PROFILE_METADATA_NAME;
    extInfo.metadata.push_back(metadata);

    EXPECT_CALL(*bundleMgrClientImpl, GetResConfigFile(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(true));
    EXPECT_CALL(*imeInfoInquirer, ParseSubtypeProfile(testing::_, testing::_)).WillOnce(testing::Return(true));

    std::vector<Subtype> subtypes;
    EXPECT_EQ(imeInfoInquirer->ParseSubtype(extInfo, subtypes), ErrorCode::NO_ERROR);
}

HWTEST_F(ImeInfoInquirerTest, GetImeProperty_ExtInfosEmpty_ReturnsNull, TestSize.Level0)
{
    int32_t userId = 100;
    std::string bundleName = "com.example.ime";
    std::string extName = "ext1";

    EXPECT_CALL(*mockImeInfoInquirer_, GetExtInfosByBundleName(userId, bundleName, _))
        .WillOnce(DoAll(SetArgReferee<2>(std::vector<ExtensionAbilityInfo>()), Return(ErrorCode::NO_ERROR)));

    auto result = mockImeInfoInquirer_->GetImeProperty(userId, bundleName, extName);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(ImeInfoInquirerTest, GetCurrentInputMethod_FoundInFullImeInfoManager_ReturnsProperty, TestSize.Level0)
{
    int32_t userId = 100;
    std::string bundleName = "com.example.ime";
    std::string extName = "ext1";

    Property prop = { .name = bundleName, .id = extName };
    FullImeInfo fullImeInfo = { .prop = prop };
    std::vector<FullImeInfo> fullImeInfos = { fullImeInfo };

    EXPECT_CALL(*mockFullImeInfoManager_, Get(userId)).WillOnce(Return(fullImeInfos));
    EXPECT_CALL(*mockImeCfgManager_, GetCurrentImeCfg(userId))
        .WillOnce(Return(std::make_shared<ImeCfg>(bundleName, extName)));

    auto result = mockImeInfoInquirer_->GetCurrentInputMethod(userId);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->name, bundleName);
    EXPECT_EQ(result->id, extName);
}

HWTEST_F(ImeInfoInquirerTest, GetCurrentSubtype_FoundInFullImeInfoManager_ReturnsSubProperty, TestSize.Level0)
{
    int32_t userId = 100;
    std::string bundleName = "com.example.ime";
    std::string subName = "sub1";

    SubProperty subProp = { .id = subName };
    Property prop = { .name = bundleName };
    FullImeInfo fullImeInfo = { .prop = prop, .subProps = { subProp } };
    std::vector<FullImeInfo> fullImeInfos = { fullImeInfo };

    EXPECT_CALL(*mockFullImeInfoManager_, Get(userId)).WillOnce(Return(fullImeInfos));
    EXPECT_CALL(*mockImeCfgManager_, GetCurrentImeCfg(userId))
        .WillOnce(Return(std::make_shared<ImeCfg>(bundleName, subName)));

    auto result = mockImeInfoInquirer_->GetCurrentSubtype(userId);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->id, subName);
}

HWTEST_F(ImeInfoInquirerTest, IsImeInstalled_Found_ReturnsTrue, TestSize.Level0)
{
    int32_t userId = 100;
    std::string bundleName = "com.example.ime";
    std::string extName = "ext1";

    ExtensionAbilityInfo extInfo = { .bundleName = bundleName, .name = extName };
    std::vector<ExtensionAbilityInfo> extInfos = { extInfo };

    EXPECT_CALL(*mockImeInfoInquirer_, GetExtInfosByBundleName(userId, bundleName, _))
        .WillOnce(DoAll(SetArgReferee<2>(extInfos), Return(ErrorCode::NO_ERROR)));

    bool result = mockImeInfoInquirer_->IsImeInstalled(userId, bundleName, extName);
    EXPECT_TRUE(result);
}

HWTEST_F(ImeInfoInquirerTest, GetImeToStart_CurrentImeNotInstalled_ReturnsDefaultIme, TestSize.Level0)
{
    int32_t userId = 100;
    std::string bundleName = "com.example.ime";
    std::string extName = "ext1";

    EXPECT_CALL(*mockImeCfgManager_, GetCurrentImeCfg(userId))
        .WillOnce(Return(std::make_shared<ImeCfg>(bundleName, extName)));
    EXPECT_CALL(*mockImeInfoInquirer_, IsImeInstalled(userId, bundleName, extName)).WillOnce(Return(false));

    auto result = mockImeInfoInquirer_->GetImeToStart(userId);
    EXPECT_NE(result, nullptr);
    EXPECT_FALSE(result->imeId.empty());
}

HWTEST_F(ImeInfoInquirerTest, GetInputMethodConfig_SystemInputMethodConfigAbilityEmpty_ReturnsNoError, TestSize.Level0)
{
    int32_t userId = 100;
    AppExecFwk::ElementName inputMethodConfig;

    EXPECT_CALL(*mockImeInfoInquirer_, GetInputMethodConfig(userId, _))
        .WillOnce(DoAll(SetArgReferee<1>(inputMethodConfig), Return(ErrorCode::NO_ERROR)));

    int32_t result = mockImeInfoInquirer_->GetInputMethodConfig(userId, inputMethodConfig);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(ImeInfoInquirerTest, GetDefaultInputMethod_Found_ReturnsProperty, TestSize.Level0)
{
    int32_t userId = 100;
    std::string bundleName = "com.example.ime";
    std::string extName = "ext1";

    Property prop = { .name = bundleName, .id = extName };
    FullImeInfo fullImeInfo = { .prop = prop };
    std::vector<FullImeInfo> fullImeInfos = { fullImeInfo };

    EXPECT_CALL(*mockFullImeInfoManager_, Get(userId)).WillOnce(Return(fullImeInfos));
    EXPECT_CALL(*mockImeInfoInquirer_, GetDefaultImeCfgProp())
        .WillOnce(Return(std::make_shared<ImeCfg>(bundleName, extName)));

    std::shared_ptr<Property> result;
    int32_t errorCode = mockImeInfoInquirer_->GetDefaultInputMethod(userId, result, false);
    EXPECT_EQ(errorCode, ErrorCode::NO_ERROR);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->name, bundleName);
    EXPECT_EQ(result->id, extName);
}

HWTEST_F(ImeInfoInquirerTest, GetDefaultImeInfo_DefaultImeIsNull_ReturnsNull, TestSize.Level0)
{
    EXPECT_CALL(*mockInquirer, GetDefaultImeCfgProp()).WillOnce(Return(nullptr));

    auto result = mockInquirer->GetDefaultImeInfo(0);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(ImeInfoInquirerTest, GetDefaultImeInfo_InfoIsNull_ReturnsNull, TestSize.Level0)
{
    auto mockDefaultIme = std::make_shared<ImeConfigProp>();
    mockDefaultIme->name = "defaultIme";

    EXPECT_CALL(*mockInquirer, GetDefaultImeCfgProp()).WillOnce(Return(mockDefaultIme));
    EXPECT_CALL(*mockInquirer, GetImeInfo(0, "defaultIme", "")).WillOnce(Return(nullptr));

    auto result = mockInquirer->GetDefaultImeInfo(0);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(ImeInfoInquirerTest, GetDefaultImeInfo_InfoIsNotNewImeWithMatchingSubProp, TestSize.Level0)
{
    auto mockDefaultIme = std::make_shared<ImeConfigProp>();
    mockDefaultIme->name = "defaultIme";
    mockDefaultIme->id = 1;

    auto mockImeInfo = std::make_shared<ImeInfo>();
    mockImeInfo->isNewIme = false;
    mockImeInfo->subProps = { SubProperty{ 1, "subProp1" }, SubProperty{ 2, "subProp2" } };

    EXPECT_CALL(*mockInquirer, GetDefaultImeCfgProp()).WillOnce(Return(mockDefaultIme));
    EXPECT_CALL(*mockInquirer, GetImeInfo(0, "defaultIme", "")).WillOnce(Return(mockImeInfo));

    auto result = mockInquirer->GetDefaultImeInfo(0);
    EXPECT_EQ(result->subProp.id, 1);
    EXPECT_EQ(result->subProp.name, "subProp1");
}
HWTEST_F(ImeInfoInquirerTest, GetDefaultImeInfo_InfoIsNotNewImeWithoutMatchingSubProp, TestSize.Level0)
{
    auto mockDefaultIme = std::make_shared<ImeConfigProp>();
    mockDefaultIme->name = "defaultIme";
    mockDefaultIme->id = 3;

    auto mockImeInfo = std::make_shared<ImeInfo>();
    mockImeInfo->isNewIme = false;
    mockImeInfo->subProps = { SubProperty{ 1, "subProp1" }, SubProperty{ 2, "subProp2" } };

    EXPECT_CALL(*mockInquirer, GetDefaultImeCfgProp()).WillOnce(Return(mockDefaultIme));
    EXPECT_CALL(*mockInquirer, GetImeInfo(0, "defaultIme", "")).WillOnce(Return(mockImeInfo));

    auto result = mockInquirer->GetDefaultImeInfo(0);
    EXPECT_EQ(result->subProp.id, 0);
    EXPECT_EQ(result->subProp.name, "");
}

HWTEST_F(ImeInfoInquirerTest, FindTargetSubtypeByCondition_Upper_ReturnsCorrectSubtype, TestSize.Level0)
{
    std::vector<SubProperty> subProps = { { "upper", "english" }, { "lower", "chinese" } };
    std::shared_ptr<SubProperty> result = ImeInfoInquirer::FindTargetSubtypeByCondition(subProps, Condition::UPPER);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->mode, "upper");
    EXPECT_EQ(result->language, "english");
}

HWTEST_F(ImeInfoInquirerTest, FindTargetSubtypeByCondition_Lower_ReturnsCorrectSubtype, TestSize.Level0)
{
    std::vector<SubProperty> subProps = { { "upper", "english" }, { "lower", "chinese" } };
    std::shared_ptr<SubProperty> result = ImeInfoInquirer::FindTargetSubtypeByCondition(subProps, Condition::LOWER);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->mode, "lower");
    EXPECT_EQ(result->language, "chinese");
}

HWTEST_F(ImeInfoInquirerTest, FindTargetSubtypeByCondition_English_ReturnsCorrectSubtype, TestSize.Level0)
{
    std::vector<SubProperty> subProps = { { "lower", "english" }, { "upper", "chinese" } };
    std::shared_ptr<SubProperty> result = ImeInfoInquirer::FindTargetSubtypeByCondition(subProps, Condition::ENGLISH);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->mode, "lower");
    EXPECT_EQ(result->language, "english");
}

HWTEST_F(ImeInfoInquirerTest, FindTargetSubtypeByCondition_Chinese_ReturnsCorrectSubtype, TestSize.Level0)
{
    std::vector<SubProperty> subProps = { { "lower", "english" }, { "upper", "chinese" } };
    std::shared_ptr<SubProperty> result = ImeInfoInquirer::FindTargetSubtypeByCondition(subProps, Condition::CHINESE);
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->mode, "upper");
    EXPECT_EQ(result->language, "chinese");
}

HWTEST_F(ImeInfoInquirerTest, FindTargetSubtypeByCondition_NoMatch_ReturnsNull, TestSize.Level0)
{
    std::vector<SubProperty> subProps = { { "lower", "english" }, { "upper", "chinese" } };
    std::shared_ptr<SubProperty> result = ImeInfoInquirer::FindTargetSubtypeByCondition(subProps, Condition::DEFAULT);
    EXPECT_TRUE(result == nullptr);
}

HWTEST_F(ImeInfoInquirerTest, ParseSubtypeProfile_EmptyProfiles_ReturnsFalse, TestSize.Level0)
{
    std::vector<std::string> profiles = {};
    SubtypeCfg subtypeCfg;
    EXPECT_FALSE(ImeInfoInquirer::ParseSubtypeProfile(profiles, subtypeCfg));
}

HWTEST_F(ImeInfoInquirerTest, ParseSubtypeProfile_InvalidSize_ReturnsFalse, TestSize.Level0)
{
    std::vector<std::string> profiles = { "profile1" };
    SubtypeCfg subtypeCfg;
    EXPECT_FALSE(ImeInfoInquirer::ParseSubtypeProfile(profiles, subtypeCfg));
}

HWTEST_F(ImeInfoInquirerTest, ParseSubtypeProfile_ValidProfiles_ReturnsTrue, TestSize.Level0)
{
    std::vector<std::string> profiles = { "profile1", "profile2", "profile3", "profile4", "profile5" };
    MockSubtypeCfg mockSubtypeCfg;
    EXPECT_CALL(mockSubtypeCfg, Unmarshall("profile1")).WillOnce(Return(true));
    EXPECT_TRUE(ImeInfoInquirer::ParseSubtypeProfile(profiles, mockSubtypeCfg));
}

HWTEST_F(ImeInfoInquirerTest, GetResMgr_EmptyResourcePath_ReturnsNull, TestSize.Level0)
{
    std::shared_ptr<ResourceManager> resMgr = mockImeInfoInquirer->GetResMgr("");
    EXPECT_EQ(resMgr, nullptr);
}

HWTEST_F(ImeInfoInquirerTest, GetResMgr_ResourceManagerCreationFails_ReturnsNull, TestSize.Level0)
{
    ON_CALL(*mockImeInfoInquirer, CreateResourceManager()).WillByDefault(Return(nullptr));
    std::shared_ptr<ResourceManager> resMgr = mockImeInfoInquirer->GetResMgr("path");
    EXPECT_EQ(resMgr, nullptr);
}

HWTEST_F(ImeInfoInquirerTest, GetResMgr_ResConfigCreationFails_ReturnsNull, TestSize.Level0)
{
    ON_CALL(*mockImeInfoInquirer, CreateResConfig()).WillByDefault(Return(nullptr));
    std::shared_ptr<ResourceManager> resMgr = mockImeInfoInquirer->GetResMgr("path");
    EXPECT_EQ(resMgr, nullptr);
}

HWTEST_F(ImeInfoInquirerTest, GetResMgr_Success_ReturnsResourceManager, TestSize.Level0)
{
    std::shared_ptr<ResourceManager> resMgr = mockImeInfoInquirer->GetResMgr("path");
    EXPECT_NE(resMgr, nullptr);
}

HWTEST_F(ImeInfoInquirerTest, QueryFullImeInfo_NoActiveOsAccountIds_ReturnsError, TestSize.Level0)
{
    ON_CALL(*mockOsAccountAdapter, QueryActiveOsAccountIds()).WillByDefault(Return(std::vector<int32_t>{}));
    std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> fullImeInfos;
    int32_t result = mockImeInfoInquirer->QueryFullImeInfo(fullImeInfos);
    EXPECT_EQ(result, ErrorCode::ERROR_OS_ACCOUNT);
}

HWTEST_F(ImeInfoInquirerTest, QueryFullImeInfo_Success_ReturnsNoError, TestSize.Level0)
{
    std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> fullImeInfos;
    int32_t result = mockImeInfoInquirer->QueryFullImeInfo(fullImeInfos);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(ImeInfoInquirerTest, QueryFullImeInfoForUserId_QueryFails_ReturnsError, TestSize.Level0)
{
    ON_CALL(*mockImeInfoInquirer, QueryImeExtInfos(_, _)).WillByDefault(Return(false));
    std::vector<FullImeInfo> imeInfo;
    int32_t result = mockImeInfoInquirer->QueryFullImeInfo(1, imeInfo);
    EXPECT_EQ(result, ErrorCode::ERROR_PACKAGE_MANAGER);
}

HWTEST_F(ImeInfoInquirerTest, QueryFullImeInfoForUserId_Success_ReturnsNoError, TestSize.Level0)
{
    std::vector<FullImeInfo> imeInfo;
    int32_t result = mockImeInfoInquirer->QueryFullImeInfo(1, imeInfo);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(ImeInfoInquirerTest, GetFullImeInfo_QueryFails_ReturnsError, TestSize.Level0)
{
    ON_CALL(*mockImeInfoInquirer, QueryImeExtInfos(_, _)).WillByDefault(Return(false));
    FullImeInfo imeInfo;
    int32_t result = mockImeInfoInquirer->GetFullImeInfo(1, "bundleName", imeInfo);
    EXPECT_EQ(result, ErrorCode::ERROR_PACKAGE_MANAGER);
}

HWTEST_F(ImeInfoInquirerTest, GetFullImeInfo_Success_ReturnsNoError, TestSize.Level0)
{
    FullImeInfo imeInfo;
    int32_t result = mockImeInfoInquirer->GetFullImeInfo(1, "bundleName", imeInfo);
    EXPECT_EQ(result, ErrorCode::NO_ERROR);
}

HWTEST_F(ImeInfoInquirerTest, IsInputMethod_CorrespondsToInputMethod_ReturnsTrue, TestSize.Level0)
{
    ON_CALL(*mockBundleManager, GetBundleInfo(_, _, _, _)).WillByDefault(Return(true));
    bool result = mockImeInfoInquirer->IsInputMethod(1, "bundleName");
    EXPECT_TRUE(result);
}

HWTEST_F(ImeInfoInquirerTest, IsInputMethod_DoesNotCorrespondToInputMethod_ReturnsFalse, TestSize.Level0)
{
    ON_CALL(*mockBundleManager, GetBundleInfo(_, _, _, _)).WillByDefault(Return(false));
    bool result = mockImeInfoInquirer->IsInputMethod(1, "bundleName");
    EXPECT_FALSE(result);
}

HWTEST_F(ImeInfoInquirerTest, IsTempInputMethod_TemporaryMetadataExists_ReturnsTrue, TestSize.Level0)
{
    ON_CALL(*mockExtensionAbilityInfo, IsTempInputMethod(_)).WillByDefault(Return(true));
    bool result = mockImeInfoInquirer->IsTempInputMethod(mockExtensionAbilityInfo);
    EXPECT_TRUE(result);
}

HWTEST_F(ImeInfoInquirerTest, IsTempInputMethod_TemporaryMetadataDoesNotExist_ReturnsFalse, TestSize.Level0)
{
    bool result = mockImeInfoInquirer->IsTempInputMethod(mockExtensionAbilityInfo);
    EXPECT_FALSE(result);
}

HWTEST_F(ImeInfoInquirerTest, GetRunningIme_Success_ReturnsBundleNames, TestSize.Level0)
{
    ON_CALL(*mockAppMgrClient, GetProcessRunningInfosByUserId(_, _)).WillByDefault(Return(ErrorCode::NO_ERROR));
    std::vector<std::string> bundleNames = mockImeInfoInquirer->GetRunningIme(1);
    EXPECT_FALSE(bundleNames.empty());
}

HWTEST_F(ImeInfoInquirerTest, IsDefaultImeSet_DefaultImeSet_ReturnsTrue, TestSize.Level0)
{
    bool result = mockImeInfoInquirer->IsDefaultImeSet(1);
    EXPECT_TRUE(result);
}

HWTEST_F(ImeInfoInquirerTest, IsDefaultImeSet_DefaultImeNotSet_ReturnsFalse, TestSize.Level0)
{
    ON_CALL(*mockImeCfgManager, IsDefaultImeSet(_)).WillByDefault(Return(false));
    bool result = mockImeInfoInquirer->IsDefaultImeSet(1);
    EXPECT_FALSE(result);
}

HWTEST_F(ImeInfoInquirerTest, IsRunningIme_ImeIsRunning_ReturnsTrue, TestSize.Level0)
{
    ON_CALL(*mockImeInfoInquirer, GetRunningIme(_)).WillByDefault(Return(std::vector<std::string>{ "bundleName" }));
    bool result = mockImeInfoInquirer->IsRunningIme(1, "bundleName");
    EXPECT_TRUE(result);
}

HWTEST_F(ImeInfoInquirerTest, IsRunningIme_ImeIsNotRunning_ReturnsFalse, TestSize.Level0)
{
    bool result = mockImeInfoInquirer->IsRunningIme(1, "bundleName");
    EXPECT_FALSE(result);
}

HWTEST_F(ImeInfoInquirerTest, GetImeAppId_Success_ReturnsAppId, TestSize.Level0)
{
    std::string appId;
    bool result = mockImeInfoInquirer->GetImeAppId(1, "bundleName", appId);
    EXPECT_TRUE(result);
    EXPECT_FALSE(appId.empty());
}

HWTEST_F(ImeInfoInquirerTest, GetImeAppId_Failure_ReturnsFalse, TestSize.Level0)
{
    ON_CALL(*mockFullImeInfoManager, Get(_, _, _)).WillByDefault(Return(false));
    std::string appId;
    bool result = mockImeInfoInquirer->GetImeAppId(1, "bundleName", appId);
    EXPECT_FALSE(result);
}

HWTEST_F(ImeInfoInquirerTest, GetImeVersionCode_Success_ReturnsVersionCode, TestSize.Level0)
{
    uint32_t versionCode;
    bool result = mockImeInfoInquirer->GetImeVersionCode(1, "bundleName", versionCode);
    EXPECT_TRUE(result);
    EXPECT_NE(versionCode, 0);
}

HWTEST_F(ImeInfoInquirerTest, GetImeVersionCode_Failure_ReturnsFalse, TestSize.Level0)
{
    ON_CALL(*mockFullImeInfoManager, Get(_, _, _)).WillByDefault(Return(false));
    uint32_t versionCode;
    bool result = mockImeInfoInquirer->GetImeVersionCode(1, "bundleName", versionCode);
    EXPECT_FALSE(result);
}

HWTEST_F(ImeInfoInquirerTest, GetBundleInfoByBundleName_Success_ReturnsBundleInfo, TestSize.Level0)
{
    BundleInfo bundleInfo;
    bool result = mockImeInfoInquirer->GetBundleInfoByBundleName(1, "bundleName", bundleInfo);
    EXPECT_TRUE(result);
}

HWTEST_F(ImeInfoInquirerTest, GetBundleInfoByBundleName_Failure_ReturnsFalse, TestSize.Level0)
{
    ON_CALL(*mockBundleManager, GetBundleInfo(_, _, _, _)).WillByDefault(Return(false));
    BundleInfo bundleInfo;
    bool result = mockImeInfoInquirer->GetBundleInfoByBundleName(1, "bundleName", bundleInfo);
    EXPECT_FALSE(result);
}

HWTEST_F(ImeInfoInquirerTest, GetTargetString_Success_ReturnsString, TestSize.Level0)
{
    std::string result = mockImeInfoInquirer->GetTargetString(mockExtensionAbilityInfo, ImeTargetString::LABEL, 1);
    EXPECT_FALSE(result.empty());
}