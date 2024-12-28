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
#include "identity_checker_impl.h"

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "window_manager.h"

using namespace OHOS;
using namespace OHOS::MiscServices;
using namespace Security::AccessToken;
using namespace testing;

class MockAbilityManagerClient : public AAFwk::AbilityManagerClient {
public:
    MOCK_METHOD2(CheckUIExtensionIsFocused, int(uint32_t, bool &));
};

class MockAccessTokenKit {
public:
    MOCK_METHOD1(IsSystemAppByFullTokenID, bool(uint64_t));
    MOCK_METHOD2(VerifyAccessToken, int(uint32_t, const std::string &));
    MOCK_METHOD2(GetNativeTokenInfo, int(AccessTokenID, NativeTokenInfo &));
    MOCK_METHOD1(GetTokenTypeFlag, TypeATokenTypeEnum(AccessTokenID));
    MOCK_METHOD2(GetHapTokenInfo, int(AccessTokenID, HapTokenInfo &));
};

class MockWindowManager {
public:
    MOCK_METHOD1(GetFocusWindowInfo, void(FocusChangeInfo &));
};

class IdentityCheckerImplTest : public Test {
protected:
    void SetUp() override
    {
        abilityManagerClientMock = std::make_shared<MockAbilityManagerClient>();
        accessTokenKitMock = std::make_shared<MockAccessTokenKit>();
        windowManagerMock = std::make_shared<MockWindowManager>();

        // Replace the real instances with mocks
        AAFwk::AbilityManagerClient::SetInstance(abilityManagerClientMock);
        AccessTokenKit::SetInstance(accessTokenKitMock);
        WindowManager::SetInstance(windowManagerMock);
    }

    void TearDown() override
    {
        AAFwk::AbilityManagerClient::SetInstance(nullptr);
        AccessTokenKit::SetInstance(nullptr);
        WindowManager::SetInstance(nullptr);
    }

    std::shared_ptr<MockAbilityManagerClient> abilityManagerClientMock;
    std::shared_ptr<MockAccessTokenKit> accessTokenKitMock;
    std::shared_ptr<MockWindowManager> windowManagerMock;
};

/**
 * @tc.name: IsFocused_FocusedPidMatches_ReturnsTrue
 * @tc.desc: Verify that IdentityCheckerImpl::IsFocused method returns true when the focused process ID matches
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, IsFocused_FocusedPidMatches_ReturnsTrue)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*windowManagerMock, GetFocusWindowInfo(_)).Times(0);
    EXPECT_TRUE(identityChecker.IsFocused(123, 456, 123));
}

/**
 * @tc.name: IsFocused_FocusedPidDoesNotMatch_ReturnsFalse
 * @tc.desc: Verify that IdentityCheckerImpl::IsFocused method
 * returns false when the focused process ID does not match
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, IsFocused_FocusedPidDoesNotMatch_ReturnsFalse)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*windowManagerMock, GetFocusWindowInfo(_)).Times(1);
    EXPECT_CALL(*abilityManagerClientMock, CheckUIExtensionIsFocused(456, _))
        .WillOnce(DoAll(SetArgReferee<1>(false), Return(ErrorCode::NO_ERROR)));
    EXPECT_FALSE(identityChecker.IsFocused(123, 456, -1));
}

/**
 * @tc.name: IsSystemApp_ValidTokenId_ReturnsTrue
 * @tc.desc: Verify that IdentityCheckerImpl::IsSystemApp method returns true when the Token ID is valid
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, IsSystemApp_ValidTokenId_ReturnsTrue)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*accessTokenKitMock, IsSystemAppByFullTokenID(123456)).WillOnce(Return(true));
    EXPECT_TRUE(identityChecker.IsSystemApp(123456));
}

/**
 * @tc.name: IsSystemApp_InvalidTokenId_ReturnsFalse
 * @tc.desc: Verify that IdentityCheckerImpl::IsSystemApp method returns false when the Token ID is invalid
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, IsSystemApp_InvalidTokenId_ReturnsFalse)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*accessTokenKitMock, IsSystemAppByFullTokenID(123456)).WillOnce(Return(false));
    EXPECT_FALSE(identityChecker.IsSystemApp(123456));
}

/**
 * @tc.name: IsBundleNameValid_ValidBundleName_ReturnsTrue
 * @tc.desc: Verify that IdentityCheckerImpl::IsBundleNameValid method returns true when the bundle name is valid
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, IsBundleNameValid_ValidBundleName_ReturnsTrue)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*accessTokenKitMock, GetTokenTypeFlag(456)).WillOnce(Return(TOKEN_HAP));
    EXPECT_CALL(*accessTokenKitMock, GetHapTokenInfo(456, _))
        .WillOnce(DoAll(SetArgReferee<1>(HapTokenInfo{ .bundleName = "validBundle" }), Return(ErrorCode::NO_ERROR)));
    EXPECT_TRUE(identityChecker.IsBundleNameValid(456, "validBundle"));
}

/**
 * @tc.name: IsBundleNameValid_InvalidBundleName_ReturnsFalse
 * @tc.desc: Verify that IdentityCheckerImpl::IsBundleNameValid method returns false when the bundle name is invalid
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, IsBundleNameValid_InvalidBundleName_ReturnsFalse)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*accessTokenKitMock, GetTokenTypeFlag(456)).WillOnce(Return(TOKEN_HAP));
    EXPECT_CALL(*accessTokenKitMock, GetHapTokenInfo(456, _))
        .WillOnce(DoAll(SetArgReferee<1>(HapTokenInfo{ .bundleName = "invalidBundle" }), Return(ErrorCode::NO_ERROR)));
    EXPECT_FALSE(identityChecker.IsBundleNameValid(456, "validBundle"));
}

/**
 * @tc.name: HasPermission_PermissionGranted_ReturnsTrue
 * @tc.desc: Verify that IdentityCheckerImpl::HasPermission method returns true when the permission is granted
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, HasPermission_PermissionGranted_ReturnsTrue)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*accessTokenKitMock, VerifyAccessToken(456, "permission")).WillOnce(Return(PERMISSION_GRANTED));
    EXPECT_TRUE(identityChecker.HasPermission(456, "permission"));
}

/**
 * @tc.name: HasPermission_PermissionNotGranted_ReturnsFalse
 * @tc.desc: Verify that IdentityCheckerImpl::HasPermission method returns false when the permission is not granted
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, HasPermission_PermissionNotGranted_ReturnsFalse)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*accessTokenKitMock, VerifyAccessToken(456, "permission")).WillOnce(Return(PERMISSION_DENIED));
    EXPECT_FALSE(identityChecker.HasPermission(456, "permission"));
}

/**
 * @tc.name: IsBroker_NativeSaBroker_ReturnsTrue
 * @tc.desc: Verify that IdentityCheckerImpl::IsBroker method returns true when it is a native service broker
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, IsBroker_NativeSaBroker_ReturnsTrue)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*accessTokenKitMock, GetTokenTypeFlag(456)).WillOnce(Return(TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock, GetNativeTokenInfo(456, _))
        .WillOnce(DoAll(SetArgReferee<1>(NativeTokenInfo{ .processName = "broker" }), Return(ErrorCode::NO_ERROR)));
    EXPECT_TRUE(identityChecker.IsBroker(456));
}

/**
 * @tc.name: IsBroker_NotNativeSa_ReturnsFalse
 * @tc.desc: Verify that IdentityCheckerImpl::IsBroker method returns false when it is not a native service
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, IsBroker_NotNativeSa_ReturnsFalse)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*accessTokenKitMock, GetTokenTypeFlag(456)).WillOnce(Return(TOKEN_HAP));
    EXPECT_FALSE(identityChecker.IsBroker(456));
}

/**
 * @tc.name: IsBroker_NativeSaNotBroker_ReturnsFalse
 * @tc.desc: Verify that IdentityCheckerImpl::IsBroker method
 * returns false when it is a native service but not a broker
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, IsBroker_NativeSaNotBroker_ReturnsFalse)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*accessTokenKitMock, GetTokenTypeFlag(456)).WillOnce(Return(TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock, GetNativeTokenInfo(456, _))
        .WillOnce(DoAll(SetArgReferee<1>(NativeTokenInfo{ .processName = "notbroker" }), Return(ErrorCode::NO_ERROR)));
    EXPECT_FALSE(identityChecker.IsBroker(456));
}

/**
 * @tc.name: IsNativeSa_NativeToken_ReturnsTrue
 * @tc.desc: Verify that IdentityCheckerImpl::IsNativeSa method returns true when it is a native service token
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, IsNativeSa_NativeToken_ReturnsTrue)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*accessTokenKitMock, GetTokenTypeFlag(456)).WillOnce(Return(TOKEN_NATIVE));
    EXPECT_TRUE(identityChecker.IsNativeSa(456));
}

/**
 * @tc.name: IsNativeSa_NotNativeToken_ReturnsFalse
 * @tc.desc: Verify that IdentityCheckerImpl::IsNativeSa method returns false when it is not a native service token
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, IsNativeSa_NotNativeToken_ReturnsFalse)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*accessTokenKitMock, GetTokenTypeFlag(456)).WillOnce(Return(TOKEN_HAP));
    EXPECT_FALSE(identityChecker.IsNativeSa(456));
}

/**
 * @tc.name: IsFocusedUIExtension_Focused_ReturnsTrue
 * @tc.desc: Verify that IdentityCheckerImpl::IsFocusedUIExtension method
 * returns true when the UI extension is focused
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, IsFocusedUIExtension_Focused_ReturnsTrue)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*abilityManagerClientMock, CheckUIExtensionIsFocused(456, _))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(ErrorCode::NO_ERROR)));
    EXPECT_TRUE(identityChecker.IsFocusedUIExtension(456));
}

/**
 * @tc.name: IsFocusedUIExtension_NotFocused_ReturnsFalse
 * @tc.desc: Verify that IdentityCheckerImpl::IsFocusedUIExtension method
 * returns false when the UI extension is not focused
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, IsFocusedUIExtension_NotFocused_ReturnsFalse)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*abilityManagerClientMock, CheckUIExtensionIsFocused(456, _))
        .WillOnce(DoAll(SetArgReferee<1>(false), Return(ErrorCode::NO_ERROR)));
    EXPECT_FALSE(identityChecker.IsFocusedUIExtension(456));
}

/**
 * @tc.name: GetBundleNameByToken_ValidToken_ReturnsBundleName
 * @tc.desc: Verify that IdentityCheckerImpl::GetBundleNameByToken method
 * returns the correct bundle name when the token is valid
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, GetBundleNameByToken_ValidToken_ReturnsBundleName)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*accessTokenKitMock, GetTokenTypeFlag(456)).WillOnce(Return(TOKEN_HAP));
    EXPECT_CALL(*accessTokenKitMock, GetHapTokenInfo(456, _))
        .WillOnce(DoAll(SetArgReferee<1>(HapTokenInfo{ .bundleName = "validBundle" }), Return(ErrorCode::NO_ERROR)));
    EXPECT_EQ(identityChecker.GetBundleNameByToken(456), "validBundle");
}

/**
 * @tc.name: GetBundleNameByToken_InvalidToken_ReturnsEmptyString
 * @tc.desc: Verify that IdentityCheckerImpl::GetBundleNameByToken method
 * returns an empty string when the token is invalid
 * @tc.type: FUNC
 */
TEST_F(IdentityCheckerImplTest, GetBundleNameByToken_InvalidToken_ReturnsEmptyString)
{
    IdentityCheckerImpl identityChecker;
    EXPECT_CALL(*accessTokenKitMock, GetTokenTypeFlag(456)).WillOnce(Return(TOKEN_NATIVE));
    EXPECT_EQ(identityChecker.GetBundleNameByToken(456), "");
}