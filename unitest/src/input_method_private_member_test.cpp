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
#include "input_method_system_ability.h"
#undef private

#include <gtest/gtest.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include "application_info.h"
#include "global.h"
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
* @tc.name: testInputMethodServiceStartAbnormal
* @tc.desc: SystemAbility testInputMethodServiceStartAbnormal.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(InputMethodPrivateMemberTest, testInputMethodServiceStartAbnormal, TestSize.Level0)
{
    IMSA_HILOGI("SystemAbility testInputMethodServiceStartAbnormal Test START");
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
* @tc.name: testGetExtends
* @tc.desc: SystemAbility GetExtends.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(InputMethodPrivateMemberTest, testSystemAbilityGetExtends, TestSize.Level0)
{
    IMSA_HILOGI("SystemAbility testSystemAbilityGetExtends Test START");
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
* @tc.name: testOnHandleMessage
* @tc.desc: SystemAbility OnHandleMessage.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(InputMethodPrivateMemberTest, testOnHandleMessage, TestSize.Level0)
{
    IMSA_HILOGI("SystemAbility testOnHandleMessage Test START");
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
* @tc.name: testOnPackageRemoved
* @tc.desc: SystemAbility OnPackageRemoved.
* @tc.type: FUNC
* @tc.require: issuesI640YZ
*/
HWTEST_F(InputMethodPrivateMemberTest, testOnPackageRemoved, TestSize.Level0)
{
    IMSA_HILOGI("SystemAbility testOnPackageRemoved Test START");
    constexpr int32_t messageId = 5;
    InputMethodSystemAbility service;
    auto *msg = new Message(messageId, nullptr);
    auto ret = service.OnPackageRemoved(msg);
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    delete msg;
    msg = nullptr;
}
} // namespace MiscServices
} // namespace OHOS
