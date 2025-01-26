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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "im_common_event_manager.h"
#include "mock_inputmethod_message_handler.h"
#include "mock_system_ability_manager_client.h"

using namespace OHOS;
using namespace MiscServices;
using namespace testing;

class ImCommonEventManagerTest : public Test {
protected:
    void SetUp() override
    {
        // Initialize mock objects
        mockSystemAbilityManagerClient_ = std::make_shared<MockSystemAbilityManagerClient>();
        mockMessageHandler_ = std::make_shared<MockMessageHandler>();
        SystemAbilityManagerClient::SetInstance(mockSystemAbilityManagerClient_);
        MessageHandler::SetInstance(mockMessageHandler_);
    }

    void TearDown() override
    {
        // Clean up mock objects
        SystemAbilityManagerClient::SetInstance(nullptr);
        MessageHandler::SetInstance(nullptr);
    }

    std::shared_ptr<MockSystemAbilityManagerClient> mockSystemAbilityManagerClient_;
    std::shared_ptr<MockMessageHandler> mockMessageHandler_;
};

/**
 * @tc.name: GetInstanceTest
 * @tc.desc: Verify that the instance returned by GetInstance method is a singleton
 * @tc.type: FUNC
*/
TEST_F(ImCommonEventManagerTest, GetInstanceTest)
{
    sptr<ImCommonEventManager> instance1 = ImCommonEventManager::GetInstance();
    sptr<ImCommonEventManager> instance2 = ImCommonEventManager::GetInstance();
    EXPECT_EQ(instance1, instance2);
}

/**
 * @tc.name: SubscribeEventTest
 * @tc.desc: Verify that SubscribeEvent method successfully subscribes to an event
 * @tc.type: FUNC
*/
TEST_F(ImCommonEventManagerTest, SubscribeEventTest)
{
    EXPECT_CALL(*mockSystemAbilityManagerClient_, GetSystemAbilityManager())
        .WillOnce(Return(mockSystemAbilityManagerClient_->GetSystemAbilityManager()));
    EXPECT_CALL(*mockSystemAbilityManagerClient_->GetSystemAbilityManager(), SubscribeSystemAbility(_, _))
        .WillOnce(Return(ERR_OK));

    EXPECT_TRUE(ImCommonEventManager::GetInstance()->SubscribeEvent());
}

/**
 * @tc.name: SubscribeEventFailureTest
 * @tc.desc: Verify that SubscribeEvent method fails when SystemAbilityManager is null
 *  * @tc.type: FUNC
*/
TEST_F(ImCommonEventManagerTest, SubscribeEventFailureTest)
{
    EXPECT_CALL(*mockSystemAbilityManagerClient_, GetSystemAbilityManager()).WillOnce(Return(nullptr));

    EXPECT_FALSE(ImCommonEventManager::GetInstance()->SubscribeEvent());
}

/**
 * @tc.name: SubscribeKeyboardEventTest
 * @tc.desc: Verify that SubscribeKeyboardEvent method successfully subscribes to a keyboard event
 * @tc.type: FUNC
*/
TEST_F(ImCommonEventManagerTest, SubscribeKeyboardEventTest)
{
    EXPECT_CALL(*mockSystemAbilityManagerClient_, GetSystemAbilityManager())
        .WillOnce(Return(mockSystemAbilityManagerClient_->GetSystemAbilityManager()));
    EXPECT_CALL(*mockSystemAbilityManagerClient_->GetSystemAbilityManager(), SubscribeSystemAbility(_, _))
        .WillOnce(Return(ERR_OK));

    EXPECT_TRUE(ImCommonEventManager::GetInstance()->SubscribeKeyboardEvent([]() {}));
}

/**
 * @tc.name: SubscribeKeyboardEventFailureTest
 * @tc.desc: Verify that SubscribeKeyboardEvent method fails when SystemAbilityManager is null
 * @tc.type: FUNC
*/
TEST_F(ImCommonEventManagerTest, SubscribeKeyboardEventFailureTest)
{
    EXPECT_CALL(*mockSystemAbilityManagerClient_, GetSystemAbilityManager()).WillOnce(Return(nullptr));

    EXPECT_FALSE(ImCommonEventManager::GetInstance()->SubscribeKeyboardEvent([]() {}));
}

/**
 * @tc.name: SubscribeWindowManagerServiceTest
 * @tc.desc: Verify that SubscribeWindowManagerService method successfully subscribes to WindowManagerService event
 * @tc.type: FUNC
*/
TEST_F(ImCommonEventManagerTest, SubscribeWindowManagerServiceTest)
{
    EXPECT_CALL(*mockSystemAbilityManagerClient_, GetSystemAbilityManager())
        .WillOnce(Return(mockSystemAbilityManagerClient_->GetSystemAbilityManager()));
    EXPECT_CALL(*mockSystemAbilityManagerClient_->GetSystemAbilityManager(), SubscribeSystemAbility(_, _))
        .WillOnce(Return(ERR_OK));

    EXPECT_TRUE(ImCommonEventManager::GetInstance()->SubscribeWindowManagerService([]() {}));
}

/**
 * @tc.name: SubscribeWindowManagerServiceFailureTest
 * @tc.desc: Verify that SubscribeWindowManagerService method fails when SystemAbilityManager is null
 * @tc.type: FUNC
*/
TEST_F(ImCommonEventManagerTest, SubscribeWindowManagerServiceFailureTest)
{
    EXPECT_CALL(*mockSystemAbilityManagerClient_, GetSystemAbilityManager()).WillOnce(Return(nullptr));

    EXPECT_FALSE(ImCommonEventManager::GetInstance()->SubscribeWindowManagerService([]() {}));
}

/**
 * @tc.name: SubscribeMemMgrServiceTest
 * @tc.desc: Verify that SubscribeMemMgrService method successfully subscribes to MemMgrService event
 * @tc.type: FUNC
*/
TEST_F(ImCommonEventManagerTest, SubscribeMemMgrServiceTest)
{
    EXPECT_CALL(*mockSystemAbilityManagerClient_, GetSystemAbilityManager())
        .WillOnce(Return(mockSystemAbilityManagerClient_->GetSystemAbilityManager()));
    EXPECT_CALL(*mockSystemAbilityManagerClient_->GetSystemAbilityManager(), SubscribeSystemAbility(_, _))
        .WillOnce(Return(ERR_OK));

    EXPECT_TRUE(ImCommonEventManager::GetInstance()->SubscribeMemMgrService([]() {}));
}

/**
 * @tc.name: SubscribeMemMgrServiceFailureTest
 * @tc.desc: Verify that SubscribeMemMgrService method fails when SystemAbilityManager is null
 * @tc.type: FUNC
*/
TEST_F(ImCommonEventManagerTest, SubscribeMemMgrServiceFailureTest)
{
    EXPECT_CALL(*mockSystemAbilityManagerClient_, GetSystemAbilityManager()).WillOnce(Return(nullptr));

    EXPECT_FALSE(ImCommonEventManager::GetInstance()->SubscribeMemMgrService([]() {}));
}

/**
 * @tc.name: SubscribeAccountManagerServiceTest
 * @tc.desc: Verify that SubscribeAccountManagerService method successfully subscribes to AccountManagerService event
 * @tc.type: FUNC
*/
TEST_F(ImCommonEventManagerTest, SubscribeAccountManagerServiceTest)
{
    EXPECT_CALL(*mockSystemAbilityManagerClient_, GetSystemAbilityManager())
        .WillOnce(Return(mockSystemAbilityManagerClient_->GetSystemAbilityManager()));
    EXPECT_CALL(*mockSystemAbilityManagerClient_->GetSystemAbilityManager(), SubscribeSystemAbility(_, _))
        .WillOnce(Return(ERR_OK));

    EXPECT_TRUE(ImCommonEventManager::GetInstance()->SubscribeAccountManagerService([]() {}));
}

/**
 * @tc.name: SubscribeAccountManagerServiceFailureTest
 * @tc.desc: Verify that SubscribeAccountManagerService method fails when SystemAbilityManager is null
 * @tc.type: FUNC
*/
TEST_F(ImCommonEventManagerTest, SubscribeAccountManagerServiceFailureTest)
{
    EXPECT_CALL(*mockSystemAbilityManagerClient_, GetSystemAbilityManager()).WillOnce(Return(nullptr));

    EXPECT_FALSE(ImCommonEventManager::GetInstance()->SubscribeAccountManagerService([]() {}));
}

/**
 * @tc.name: UnsubscribeEventTest
 * @tc.desc: Verify that UnsubscribeEvent method always returns true
 * @tc.type: FUNC
*/
TEST_F(ImCommonEventManagerTest, UnsubscribeEventTest)
{
    EXPECT_TRUE(ImCommonEventManager::GetInstance()->UnsubscribeEvent());
}

/**
 * @tc.name: PublishPanelStatusChangeEventTest
 * @tc.desc: Verify that PublishPanelStatusChangeEvent method successfully publishes an event
 * @tc.type: FUNC
*/
TEST_F(ImCommonEventManagerTest, PublishPanelStatusChangeEventTest)
{
    EXPECT_CALL(*mockSystemAbilityManagerClient_, GetSystemAbilityManager())
        .WillOnce(Return(mockSystemAbilityManagerClient_->GetSystemAbilityManager()));
    EXPECT_CALL(*mockSystemAbilityManagerClient_->GetSystemAbilityManager(), SubscribeSystemAbility(_, _))
        .WillOnce(Return(ERR_OK));

    InputWindowStatus status = InputWindowStatus::SHOW;
    ImeWindowInfo info;
    info.windowInfo.left = 0;
    info.windowInfo.top = 0;
    info.windowInfo.width = 100;
    info.windowInfo.height = 100;

    EXPECT_EQ(ImCommonEventManager::GetInstance()->PublishPanelStatusChangeEvent(1, status, info), 0);
}