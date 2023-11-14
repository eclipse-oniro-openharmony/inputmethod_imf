/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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
#include "ime_cfg_manager.h"
#include "input_method_system_ability.h"
#undef private

#include <gtest/gtest.h>
#include <sys/time.h>
#include <unistd.h>

#include <cstdint>
#include <regex>
#include <sstream>
#include <string>

#include "global.h"
using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class IdentityCheckerTest : public testing::Test {
public:
    class IdentityCheckerMock : public IdentityChecker {
    public:
        IdentityCheckerMock() = default;
        virtual ~IdentityCheckerMock() = default;
        bool IsFocused(int64_t callingPid, uint32_t callingTokenId, int64_t focusedPid = INVALID_PID) override
        {
            return isFocused_;
        }
        bool IsSystemApp(uint64_t fullTokenId) override
        {
            return isSystemApp_;
        }
        bool IsBundleNameValid(uint32_t tokenId, const std::string &validBundleName) override
        {
            return isBundleNameValid_;
        }
        bool HasPermission(uint32_t tokenId, const std::string &permission) override
        {
            return hasPermission_;
        }
        bool IsBroker(Security::AccessToken::AccessTokenID tokenId) override
        {
            return isBroker_;
        }
        bool IsNativeSa(Security::AccessToken::AccessTokenID tokenId) override
        {
            return isNativeSa_;
        }
        static bool isFocused_;
        static bool isSystemApp_;
        static bool isBundleNameValid_;
        static bool hasPermission_;
        static bool isBroker_;
        static bool isNativeSa_;
    };
    static constexpr uint32_t CURRENT_USERID = 101;
    static const constexpr char *CURRENT_IME = "testBundleName/testExtname";
    static const constexpr char *CURRENT_SUBNAME = "testSubName";
    static const constexpr char *CURRENT_BUNDLENAME = "testBundleName";
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static sptr<InputMethodSystemAbility> service_;
    static std::shared_ptr<IdentityCheckerMock> identityCheckerMock_;
    static std::shared_ptr<IdentityCheckerImpl> identityCheckerImpl_;
};
bool IdentityCheckerTest::IdentityCheckerMock::isFocused_ = false;
bool IdentityCheckerTest::IdentityCheckerMock::isSystemApp_ = false;
bool IdentityCheckerTest::IdentityCheckerMock::isBundleNameValid_ = false;
bool IdentityCheckerTest::IdentityCheckerMock::hasPermission_ = false;
bool IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
bool IdentityCheckerTest::IdentityCheckerMock::isNativeSa_ = false;

void IdentityCheckerTest::SetUpTestCase(void)
{
    IMSA_HILOGI("IdentityCheckerTest::SetUpTestCase");
    service_ = new InputMethodSystemAbility();
    service_->OnStart();
    service_->userId_ = CURRENT_USERID;
    ImeCfgManager::GetInstance().imeConfigs_ = { { CURRENT_USERID, CURRENT_IME, CURRENT_SUBNAME } };
    identityCheckerImpl_ = std::make_shared<IdentityCheckerImpl>();
}

void IdentityCheckerTest::TearDownTestCase(void)
{
    IMSA_HILOGI("IdentityCheckerTest::TearDownTestCase");
    service_->OnStop();
}

void IdentityCheckerTest::SetUp(void)
{
    identityCheckerMock_ = std::make_shared<IdentityCheckerMock>();
    service_->identityChecker_ = identityCheckerMock_;
    IMSA_HILOGI("IdentityCheckerTest::SetUp");
}

void IdentityCheckerTest::TearDown(void)
{
    service_->identityChecker_ = identityCheckerImpl_;
    identityCheckerMock_ = nullptr;
    IMSA_HILOGI("IdentityCheckerTest::TearDown");
}

sptr<InputMethodSystemAbility> IdentityCheckerTest::service_;
std::shared_ptr<IdentityCheckerTest::IdentityCheckerMock> IdentityCheckerTest::identityCheckerMock_;
std::shared_ptr<IdentityCheckerImpl> IdentityCheckerTest::identityCheckerImpl_;

/**
 * @tc.name: testPrepareInput_001
 * @tc.desc: not broker, not focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testPrepareInput_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testPrepareInput_001 start");
    service_->identityChecker_ = identityCheckerImpl_;
    InputClientInfo clientInfo{};
    if (IdentityCheckerTest::service_ == nullptr) {
        IMSA_HILOGI("service_ is nullptr");
    }
    int32_t ret = IdentityCheckerTest::service_->PrepareInput(clientInfo);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testStartInput_001
 * @tc.desc: not broker, not focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testStartInput_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testStartInput_001 start");
    service_->identityChecker_ = identityCheckerImpl_;
    sptr<IRemoteObject> agent = nullptr;
    InputClientInfo inputClientInfo;
    int32_t ret = IdentityCheckerTest::service_->StartInput(inputClientInfo, agent);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);
}

/**
 * @tc.name: testStartInput_002
 * @tc.desc: is broker, not focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testStartInput_002, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testStartInput_002 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = true;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = false;
    sptr<IRemoteObject> agent = nullptr;
    InputClientInfo inputClientInfo;
    int32_t ret = IdentityCheckerTest::service_->StartInput(inputClientInfo, agent);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testStartInput_003
 * @tc.desc: is broker, is focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testStartInput_003, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testStartInput_003 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = true;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = true;
    sptr<IRemoteObject> agent = nullptr;
    InputClientInfo inputClientInfo;
    int32_t ret = IdentityCheckerTest::service_->StartInput(inputClientInfo, agent);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testStartInput_004
 * @tc.desc: not broker, is focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testStartInput_004, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testStartInput_004 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = true;
    sptr<IRemoteObject> agent = nullptr;
    InputClientInfo inputClientInfo;
    int32_t ret = IdentityCheckerTest::service_->StartInput(inputClientInfo, agent);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testStopInput_001
 * @tc.desc: not broker, not focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testStopInput_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testStopInput_001 start");
    service_->identityChecker_ = identityCheckerImpl_;
    int32_t ret = IdentityCheckerTest::service_->HideInput(nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);
}

/**
 * @tc.name: testStopInput_002
 * @tc.desc: is broker, not focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testStopInput_002, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testStopInput_002 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = true;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = false;
    int32_t ret = IdentityCheckerTest::service_->HideInput(nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testStopInput_003
 * @tc.desc: is broker, is focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testStopInput_003, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testStopInput_003 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = true;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = true;
    InputClientInfo clientInfo{};
    int32_t ret = IdentityCheckerTest::service_->HideInput(nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testStopInput_004
 * @tc.desc: not broker, is focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testStopInput_004, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testStopInput_004 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = true;
    InputClientInfo clientInfo{};
    int32_t ret = IdentityCheckerTest::service_->HideInput(nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NULL_POINTER);
}

/**
 * @tc.name: testStopInputSession_001
 * @tc.desc: not broker, not focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testStopInputSession_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testStopInputSession_001 start");
    service_->identityChecker_ = identityCheckerImpl_;
    int32_t ret = IdentityCheckerTest::service_->StopInputSession();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);
}

/**
 * @tc.name: testStopInputSession_002
 * @tc.desc: is broker, not focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testStopInputSession_002, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testStopInputSession_002 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = true;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = false;
    int32_t ret = IdentityCheckerTest::service_->StopInputSession();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}

/**
 * @tc.name: testStopInputSession_003
 * @tc.desc: is broker, is focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testStopInputSession_003, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testStopInputSession_003 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = true;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = true;
    InputClientInfo clientInfo{};
    int32_t ret = IdentityCheckerTest::service_->StopInputSession();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}

/**
 * @tc.name: testStopInputSession_004
 * @tc.desc: not broker, is focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testStopInputSession_004, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testStopInputSession_004 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = true;
    InputClientInfo clientInfo{};
    int32_t ret = IdentityCheckerTest::service_->StopInputSession();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}

/**
 * @tc.name: testSetCoreAndAgent_001
 * @tc.desc: not current ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testSetCoreAndAgent_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testSetCoreAndAgent_001 start");
    service_->identityChecker_ = identityCheckerImpl_;
    int32_t ret = IdentityCheckerTest::service_->SetCoreAndAgent(nullptr, nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_NOT_CURRENT_IME);
}

/**
 * @tc.name: testSetCoreAndAgent_002
 * @tc.desc: current ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testSetCoreAndAgent_002, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testSetCoreAndAgent_002 start");
    IdentityCheckerTest::IdentityCheckerMock::isBundleNameValid_ = true;
    int32_t ret = IdentityCheckerTest::service_->SetCoreAndAgent(nullptr, nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testSetCoreAndAgent_003
 * @tc.desc: not current ime, is a sys_basic native sa
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testSetCoreAndAgent_003, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testSetCoreAndAgent_003 start");
    IdentityCheckerTest::IdentityCheckerMock::isBundleNameValid_ = false;
    IdentityCheckerTest::IdentityCheckerMock::isNativeSa_ = true;
    int32_t ret = IdentityCheckerTest::service_->SetCoreAndAgent(nullptr, nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testUnRegisteredProxyIme_001
 * @tc.desc: not a sys_basic native sa
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testUnRegisteredProxyIme_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testUnRegisteredProxyIme_001 start");
    IdentityCheckerTest::IdentityCheckerMock::isNativeSa_ = false;
    int32_t ret = IdentityCheckerTest::service_->UnRegisteredProxyIme(UnRegisteredType::REMOVE_PROXY_IME, nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
}

/**
 * @tc.name: testUnRegisteredProxyIme_002
 * @tc.desc: a sys_basic native sa
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testUnRegisteredProxyIme_002, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testUnRegisteredProxyIme_002 start");
    IdentityCheckerTest::IdentityCheckerMock::isNativeSa_ = true;
    int32_t ret = IdentityCheckerTest::service_->UnRegisteredProxyIme(UnRegisteredType::NONE, nullptr);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
}

/**
 * @tc.name: testIsCurrentIme_001
 * @tc.desc: not current ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testIsCurrentIme_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testIsCurrentIme_001 start");
    service_->identityChecker_ = identityCheckerImpl_;
    bool ret = IdentityCheckerTest::service_->IsCurrentIme();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: testIsCurrentIme_002
 * @tc.desc: current ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testIsCurrentIme_002, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testIsCurrentIme_002 start");
    IdentityCheckerTest::IdentityCheckerMock::isBundleNameValid_ = true;
    bool ret = IdentityCheckerTest::service_->IsCurrentIme();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: testHideCurrentInput_001
 * @tc.desc: is broker
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testHideCurrentInput_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testHideCurrentInput_001 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = true;
    int32_t ret = IdentityCheckerTest::service_->HideCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}

/**
 * @tc.name: testHideCurrentInput_002
 * @tc.desc: is not broker, has no PERMISSION_CONNECT_IME_ABILITY
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testHideCurrentInput_002, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testHideCurrentInput_002 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
    IdentityCheckerTest::IdentityCheckerMock::hasPermission_ = false;
    int32_t ret = IdentityCheckerTest::service_->HideCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
}

/**
 * @tc.name: testHideCurrentInput_003
 * @tc.desc: is not broker, has PERMISSION_CONNECT_IME_ABILITY, not focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testHideCurrentInput_003, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testHideCurrentInput_003 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
    IdentityCheckerTest::IdentityCheckerMock::hasPermission_ = true;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = false;
    int32_t ret = IdentityCheckerTest::service_->HideCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);
}

/**
 * @tc.name: testHideCurrentInput_004
 * @tc.desc: is not broker, has PERMISSION_CONNECT_IME_ABILITY, is focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testHideCurrentInput_004, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testHideCurrentInput_004 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
    IdentityCheckerTest::IdentityCheckerMock::hasPermission_ = true;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = true;
    int32_t ret = IdentityCheckerTest::service_->HideCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}

/**
 * @tc.name: testShowCurrentInput_001
 * @tc.desc: is broker
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testShowCurrentInput_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testShowCurrentInput_001 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = true;
    int32_t ret = IdentityCheckerTest::service_->ShowCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}

/**
 * @tc.name: testShowCurrentInput_002
 * @tc.desc: is not broker, has no PERMISSION_CONNECT_IME_ABILITY
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testShowCurrentInput_002, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testShowCurrentInput_002 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
    IdentityCheckerTest::IdentityCheckerMock::hasPermission_ = false;
    int32_t ret = IdentityCheckerTest::service_->ShowCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
}

/**
 * @tc.name: testShowCurrentInput_003
 * @tc.desc: is not broker, has PERMISSION_CONNECT_IME_ABILITY, not focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testShowCurrentInput_003, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testShowCurrentInput_003 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
    IdentityCheckerTest::IdentityCheckerMock::hasPermission_ = true;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = false;
    int32_t ret = IdentityCheckerTest::service_->ShowCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);
}

/**
 * @tc.name: testShowCurrentInput_004
 * @tc.desc: is not broker, has PERMISSION_CONNECT_IME_ABILITY, is focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testShowCurrentInput_004, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testShowCurrentInput_004 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
    IdentityCheckerTest::IdentityCheckerMock::hasPermission_ = true;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = true;
    int32_t ret = IdentityCheckerTest::service_->ShowCurrentInput();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}

/**
 * @tc.name: testPanelStatusChange_001
 * @tc.desc: not current ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testPanelStatusChange_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testPanelStatusChange_001 start");
    service_->identityChecker_ = identityCheckerImpl_;
    InputWindowStatus status = InputWindowStatus::SHOW;
    InputWindowInfo windowInfo{};
    int32_t ret = IdentityCheckerTest::service_->PanelStatusChange(status, windowInfo);
    EXPECT_EQ(ret, ErrorCode::ERROR_NOT_CURRENT_IME);
}

/**
 * @tc.name: testPanelStatusChange_002
 * @tc.desc: current ime
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testPanelStatusChange_002, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testPanelStatusChange_002 start");
    IdentityCheckerTest::IdentityCheckerMock::isBundleNameValid_ = true;
    InputWindowStatus status = InputWindowStatus::SHOW;
    InputWindowInfo windowInfo{};
    int32_t ret = IdentityCheckerTest::service_->PanelStatusChange(status, windowInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testUpdateListenEventFlag_001
 * @tc.desc: not system app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testUpdateListenEventFlag_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testUpdateListenEventFlag_001 start");
    service_->identityChecker_ = identityCheckerImpl_;
    InputClientInfo clientInfo{};
    EventType eventType = IME_SHOW;
    int32_t ret = IdentityCheckerTest::service_->UpdateListenEventFlag(clientInfo, eventType);
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION);

    eventType = IME_HIDE;
    ret = IdentityCheckerTest::service_->UpdateListenEventFlag(clientInfo, eventType);
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION);

    eventType = IME_CHANGE;
    ret = IdentityCheckerTest::service_->UpdateListenEventFlag(clientInfo, eventType);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testUpdateListenEventFlag_002
 * @tc.desc: is system app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testUpdateListenEventFlag_002, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testUpdateListenEventFlag_002 start");
    IdentityCheckerTest::IdentityCheckerMock::isSystemApp_ = true;
    InputClientInfo clientInfo{};
    EventType eventType = IME_SHOW;
    int32_t ret = IdentityCheckerTest::service_->UpdateListenEventFlag(clientInfo, eventType);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    eventType = IME_HIDE;
    ret = IdentityCheckerTest::service_->UpdateListenEventFlag(clientInfo, eventType);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);

    eventType = IME_CHANGE;
    ret = IdentityCheckerTest::service_->UpdateListenEventFlag(clientInfo, eventType);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
}

/**
 * @tc.name: testDisplayOptionalInputMethod_001
 * @tc.desc: has no PERMISSION_CONNECT_IME_ABILITY
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testDisplayOptionalInputMethod_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testDisplayOptionalInputMethod_001 start");
    service_->identityChecker_ = identityCheckerImpl_;
    int32_t ret = IdentityCheckerTest::service_->DisplayOptionalInputMethod();
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
}

/**
 * @tc.name: testSwitchInputMethod_001
 * @tc.desc: has no PERMISSION_CONNECT_IME_ABILITY, and not currentIme switch subtype
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testSwitchInputMethod_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testSwitchInputMethod_001 start");
    service_->identityChecker_ = identityCheckerImpl_;
    int32_t ret = IdentityCheckerTest::service_->SwitchInputMethod(CURRENT_BUNDLENAME, CURRENT_SUBNAME);
    EXPECT_EQ(ret, ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
}

/**
 * @tc.name: testSwitchInputMethod_002
 * @tc.desc: has no PERMISSION_CONNECT_IME_ABILITY, currentIme switch subtype
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testSwitchInputMethod_002, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testSwitchInputMethod_002 start");
    IdentityCheckerTest::IdentityCheckerMock::hasPermission_ = false;
    IdentityCheckerTest::IdentityCheckerMock::isBundleNameValid_ = true;
    int32_t ret = IdentityCheckerTest::service_->SwitchInputMethod(CURRENT_BUNDLENAME, CURRENT_SUBNAME);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testSwitchInputMethod_003
 * @tc.desc: has PERMISSION_CONNECT_IME_ABILITY, not currentIme switch subtype
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testSwitchInputMethod_003, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testSwitchInputMethod_003 start");
    IdentityCheckerTest::IdentityCheckerMock::hasPermission_ = true;
    IdentityCheckerTest::IdentityCheckerMock::isBundleNameValid_ = false;
    int32_t ret = IdentityCheckerTest::service_->SwitchInputMethod(CURRENT_BUNDLENAME, CURRENT_SUBNAME);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
 * @tc.name: testHideCurrentInputDeprecated_001
 * @tc.desc: is broker
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testHideCurrentInputDeprecated_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testHideCurrentInputDeprecated_001 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = true;
    int32_t ret = IdentityCheckerTest::service_->HideCurrentInputDeprecated();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}

/**
 * @tc.name: testHideCurrentInputDeprecated_002
 * @tc.desc: is not broker, not focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testHideCurrentInputDeprecated_002, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testHideCurrentInputDeprecated_002 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = false;
    int32_t ret = IdentityCheckerTest::service_->HideCurrentInputDeprecated();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);
}

/**
 * @tc.name: testHideCurrentInputDeprecated_003
 * @tc.desc: is not broker, is focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testHideCurrentInputDeprecated_003, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testHideCurrentInputDeprecated_003 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = true;
    int32_t ret = IdentityCheckerTest::service_->HideCurrentInputDeprecated();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}

/**
 * @tc.name: testShowCurrentInputDeprecated_001
 * @tc.desc: is broker
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testShowCurrentInputDeprecated_001, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testShowCurrentInputDeprecated_001 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = true;
    int32_t ret = IdentityCheckerTest::service_->ShowCurrentInputDeprecated();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}

/**
 * @tc.name: testShowCurrentInputDeprecated_002
 * @tc.desc: is not broker, not focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testShowCurrentInputDeprecated_002, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testShowCurrentInputDeprecated_002 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = false;
    int32_t ret = IdentityCheckerTest::service_->ShowCurrentInputDeprecated();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOCUSED);
}

/**
 * @tc.name: testShowCurrentInputDeprecated_003
 * @tc.desc: is not broker, is focused app
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
*/
HWTEST_F(IdentityCheckerTest, testShowCurrentInputDeprecated_003, TestSize.Level0)
{
    IMSA_HILOGI("IdentityCheckerTest testShowCurrentInputDeprecated_003 start");
    IdentityCheckerTest::IdentityCheckerMock::isBroker_ = false;
    IdentityCheckerTest::IdentityCheckerMock::isFocused_ = true;
    int32_t ret = IdentityCheckerTest::service_->ShowCurrentInputDeprecated();
    EXPECT_EQ(ret, ErrorCode::ERROR_CLIENT_NOT_FOUND);
}
} // namespace MiscServices
} // namespace OHOS