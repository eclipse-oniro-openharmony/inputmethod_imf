/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "input_method_controller.h"
#include "input_method_system_ability.h"
#undef private

#include <gtest/gtest.h>
#include <sys/time.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "application_info.h"
#include "global.h"
#include "ime_cfg_manager.h"
#include "os_account_manager.h"
using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
class InputMethodPrivateMemberTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
constexpr std::int32_t MAIN_USER_ID = 100;
void InputMethodPrivateMemberTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SetUpTestCase");
}

void InputMethodPrivateMemberTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::TearDownTestCase");
}

void InputMethodPrivateMemberTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::SetUp");
}

void InputMethodPrivateMemberTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodPrivateMemberTest::TearDown");
}

/**
* @tc.name: SA_ServiceStartAbnormal
* @tc.desc: SA Service Start Abnormal.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_ServiceStartAbnormal, TestSize.Level0)
{
    auto service = new InputMethodSystemAbility();
    service->state_ = ServiceRunningState::STATE_RUNNING;
    service->OnStart();

    EXPECT_NE(service->userId_, MAIN_USER_ID);
    EXPECT_TRUE(service->userSessions.empty());
    EXPECT_TRUE(InputMethodSystemAbility::serviceHandler_ == nullptr);
    EXPECT_TRUE(service->msgHandlers.empty());

    service->OnStop();
    EXPECT_EQ(service->state_, ServiceRunningState::STATE_NOT_START);
    service->OnStop();
    delete service;
    service = nullptr;
}

/**
* @tc.name: SA_GetExtends
* @tc.desc: SA GetExtends.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_GetExtends, TestSize.Level0)
{
    constexpr int32_t metaDataNums = 5;
    InputMethodSystemAbility service;
    std::vector<Metadata> metaData;
    Metadata metadata[metaDataNums] = { { "language", "english", "" }, { "mode", "mode", "" },
        { "locale", "local", "" }, { "icon", "icon", "" }, { "", "", "" } };
    for (auto const &data : metadata) {
        metaData.emplace_back(data);
    }
    auto subProperty = service.GetExtends(metaData);
    EXPECT_EQ(subProperty.language, "english");
    EXPECT_EQ(subProperty.mode, "mode");
    EXPECT_EQ(subProperty.locale, "local");
    EXPECT_EQ(subProperty.icon, "icon");
}

/**
* @tc.name: SA_OnHandleMessageWithoutMessageHandler
* @tc.desc: SA OnHandleMessage Without MessageHandler.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_OnHandleMessageWithoutMessageHandler, TestSize.Level0)
{
    constexpr int32_t messageId = 5;
    InputMethodSystemAbility service;
    auto *parcel = new MessageParcel();
    parcel->WriteInt32(MAIN_USER_ID);
    auto *msg = new Message(messageId, parcel);
    auto ret = service.OnHandleMessage(msg);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    delete msg;
    msg = nullptr;
}

/**
* @tc.name: SA_OnPackageRemovedWithNullMessage
* @tc.desc: SA OnPackageRemoved With Null Message.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_OnPackageRemovedWithNullMessage, TestSize.Level0)
{
    InputMethodSystemAbility service;
    constexpr int32_t messageId = 5;
    auto *msg = new Message(messageId, nullptr);
    auto ret = service.OnPackageRemoved(msg);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    delete msg;
    msg = nullptr;
}

/**
* @tc.name: SA_OnUserStartedWithNullMessage
* @tc.desc: SA OnUserStarted With Null Message.
* @tc.type: FUNC
* @tc.require: issuesI669E8
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_OnUserStartedWithNullMessage, TestSize.Level0)
{
    InputMethodSystemAbility service;
    constexpr int32_t messageId = 5;
    auto *msg = new Message(messageId, nullptr);
    auto ret = service.OnUserStarted(msg);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    delete msg;
    msg = nullptr;
}

/**
* @tc.name: SA_ListDisabledInputMethodWithInexistentUserId
* @tc.desc: SA ListDisabledInputMethod With Inexistent UserId.
* @tc.type: FUNC
* @tc.require: issuesI669E8
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_ListDisabledInputMethodWithInexistentUserId, TestSize.Level0)
{
    InputMethodSystemAbility service;
    constexpr int32_t userId = 1;
    std::vector<Property> props;
    auto ret = service.ListDisabledInputMethod(userId, props);
    EXPECT_EQ(ret, ErrorCode::ERROR_PACKAGE_MANAGER);
    EXPECT_TRUE(props.empty());
}

/**
* @tc.name: SA_ListInputMethodInfoWithInexistentUserId
* @tc.desc: SA ListInputMethodInfo With Inexistent UserId.
* @tc.type: FUNC
* @tc.require: issuesI669E8
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_ListInputMethodInfoWithInexistentUserId, TestSize.Level0)
{
    InputMethodSystemAbility service;
    constexpr int32_t userId = 1;
    auto inputMethodInfos = service.ListInputMethodInfo(userId);
    EXPECT_TRUE(inputMethodInfos.empty());
}

/**
* @tc.name: SA_ListSubtypeByBundleNameWithInexistentUserId
* @tc.desc: SA ListSubtypeByBundleName With Inexistent UserId.
* @tc.type: FUNC
* @tc.require: issuesI669E8
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_ListSubtypeByBundleNameWithInexistentUserId, TestSize.Level0)
{
    InputMethodSystemAbility service;
    constexpr int32_t userId = 1;
    std::vector<SubProperty> subProps;
    auto ret = service.ListSubtypeByBundleName(userId, "", subProps);
    EXPECT_EQ(ret, ErrorCode::ERROR_PACKAGE_MANAGER);
    EXPECT_TRUE(subProps.empty());
}

/**
* @tc.name: SA_GetUserSessionWithInexistentUserId
* @tc.desc: SA GetUserSession With Inexistent UserId.
* @tc.type: FUNC
* @tc.require: issuesI669E8
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_GetUserSessionWithInexistentUserId, TestSize.Level0)
{
    InputMethodSystemAbility service;
    constexpr int32_t userId = 1;
    auto perUserSession = service.GetUserSession(userId);
    EXPECT_TRUE(perUserSession == nullptr);
}

/**
* @tc.name: SA_FindSubPropertyWithInexistentSubLabel
* @tc.desc: SA ListSubtypeByBundleName With Inexistent UserId.
* @tc.type: FUNC
* @tc.require: issuesI669E8
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_FindSubPropertyWithInexistentSubLabel, TestSize.Level0)
{
    InputMethodSystemAbility service;
    std::vector<int32_t> userIds;
    if (AccountSA::OsAccountManager::QueryActiveOsAccountIds(userIds) == ERR_OK && !userIds.empty()) {
        service.userId_ = userIds[0];
    }
    auto ime = ImeCfgManager::GetDefaultIme();
    EXPECT_FALSE(ime.empty());
    auto pos = ime.find("/");
    auto subProp = service.FindSubProperty(ime.substr(0, pos), "");
    EXPECT_EQ(subProp.name, "");
}

/**
* @tc.name: SA_FindSubPropertyByCompareWithNullBundleName
* @tc.desc: SA FindSubPropertyByCompare With Null BundleName
* @tc.type: FUNC
* @tc.require: issuesI669E8
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_FindSubPropertyByCompareWithNullBundleName, TestSize.Level0)
{
    InputMethodSystemAbility service;
    InputMethodSystemAbility::CompareHandler compare;
    auto subProp = service.FindSubPropertyByCompare("", compare);
    EXPECT_EQ(subProp.label, "");
}

/**
* @tc.name: IMC_ListInputMethodCommonWithErrorStatus
* @tc.desc: IMC ListInputMethodCommon With Error Status.
* @tc.type: FUNC
* @tc.require: issuesI669E8
*/
HWTEST_F(InputMethodPrivateMemberTest, IMC_ListInputMethodCommonWithErrorStatus, TestSize.Level0)
{
    std::vector<Property> props;
    auto ret = InputMethodController::GetInstance()->ListInputMethodCommon(static_cast<InputMethodStatus>(5), props);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    EXPECT_TRUE(props.empty());
}
} // namespace MiscServices
} // namespace OHOS
