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

#include "user_session_manager.h"

#include <gtest/gtest.h>

#include <memory>

#include "mock_event_handler.h"
#include "mock_per_user_session.h"

using namespace OHOS::MiscServices;
using namespace testing;

class UserSessionManagerTest : public Test {
protected:
    void SetUp() override
    {
        manager = &UserSessionManager::GetInstance();
        mockEventHandler = std::make_shared<MockEventHandler>();
        mockPerUserSession = std::make_shared<MockPerUserSession>();
    }

    UserSessionManager *manager;
    std::shared_ptr<MockEventHandler> mockEventHandler;
    std::shared_ptr<MockPerUserSession> mockPerUserSession;
};
/**
 * @tc.name: GetUserSession_SessionExists_ReturnsSession
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UserSessionManagerTest, GetUserSession_SessionExists_ReturnsSession, TestSize.Level0)
{
    int32_t userId = 100;
    manager->AddUserSession(userId);
    auto session = manager->GetUserSession(userId);
    EXPECT_NE(session, nullptr);
}

/**
 * @tc.name: GetUserSession_SessionDoesNotExist_ReturnsNull
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UserSessionManagerTest, GetUserSession_SessionDoesNotExist_ReturnsNull, TestSize.Level0)
{
    int32_t userId = 200;
    auto session = manager->GetUserSession(userId);
    EXPECT_EQ(session, nullptr);
}

/**
 * @tc.name: AddUserSession_SessionDoesNotExist_AddsSession
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UserSessionManagerTest, AddUserSession_SessionDoesNotExist_AddsSession, TestSize.Level0)
{
    int32_t userId = 300;
    manager->AddUserSession(userId);
    auto session = manager->GetUserSession(userId);
    EXPECT_NE(session, nullptr);
}

/**
 * @tc.name: AddUserSession_SessionExists_DoesNotAddNewSession
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UserSessionManagerTest, AddUserSession_SessionExists_DoesNotAddNewSession, TestSize.Level0)
{
    int32_t userId = 400;
    manager->AddUserSession(userId);
    manager->AddUserSession(userId);
    auto sessions = manager->GetUserSessions();
    EXPECT_EQ(sessions.size(), 1);
}

/**
 * @tc.name: RemoveUserSession_SessionExists_RemovesSession
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UserSessionManagerTest, RemoveUserSession_SessionExists_RemovesSession, TestSize.Level0)
{
    int32_t userId = 500;
    manager->AddUserSession(userId);
    manager->RemoveUserSession(userId);
    auto session = manager->GetUserSession(userId);
    EXPECT_EQ(session, nullptr);
}

/**
 * @tc.name: RemoveUserSession_SessionDoesNotExist_NoChange
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UserSessionManagerTest, RemoveUserSession_SessionDoesNotExist_NoChange, TestSize.Level0)
{
    int32_t userId = 600;
    manager->RemoveUserSession(userId);
    auto session = manager->GetUserSession(userId);
    EXPECT_EQ(session, nullptr);
}