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
    EXPECT_TRUE(InputMethodSystemAbility::serviceHandler_ == nullptr);

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
    ImeInfoInquirer inquirer;
    std::vector<Metadata> metaData;
    Metadata metadata[metaDataNums] = { { "language", "english", "" }, { "mode", "mode", "" },
        { "locale", "local", "" }, { "icon", "icon", "" }, { "", "", "" } };
    for (auto const &data : metadata) {
        metaData.emplace_back(data);
    }
    auto subProperty = inquirer.GetExtends(metaData);
    EXPECT_EQ(subProperty.language, "english");
    EXPECT_EQ(subProperty.mode, "mode");
    EXPECT_EQ(subProperty.locale, "local");
    EXPECT_EQ(subProperty.icon, "icon");
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
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    delete msg;
    msg = nullptr;
}

/**
* @tc.name: SA_ListInputMethodInfoWithInexistentUserId
* @tc.desc: SA ListInputMethodInfo With Inexistent UserId.
* @tc.type: FUNC
* @tc.require: issuesI669E8
*/
HWTEST_F(InputMethodPrivateMemberTest, SA_ListInputMethodInfoWithInexistentUserId, TestSize.Level0)
{
    ImeInfoInquirer inquirer;
    constexpr int32_t userId = 1;
    auto inputMethodInfos = inquirer.ListInputMethodInfo(userId);
    EXPECT_TRUE(inputMethodInfos.empty());
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
