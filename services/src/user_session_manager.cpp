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

namespace OHOS {
namespace MiscServices {

UserSessionManager &UserSessionManager::GetInstance()
{
    static UserSessionManager manager;
    return manager;
}

std::unordered_map<int32_t, std::shared_ptr<PerUserSession>> UserSessionManager::GetUserSessions()
{
    std::lock_guard<std::mutex> lock(userSessionsLock_);
    return userSessions_;
}

std::shared_ptr<PerUserSession> UserSessionManager::GetUserSession(int32_t userId)
{
    std::lock_guard<std::mutex> lock(userSessionsLock_);
    auto session = userSessions_.find(userId);
    if (session == userSessions_.end()) {
        return nullptr;
    }
    return session->second;
}

void UserSessionManager::AddUserSession(int32_t userId)
{
    std::lock_guard<std::mutex> lock(userSessionsLock_);
    auto session = userSessions_.find(userId);
    if (session != userSessions_.end()) {
        return;
    }
    auto sessionTemp = std::make_shared<PerUserSession>(userId, eventHandler_);
    userSessions_.insert({ userId, sessionTemp });
}

void UserSessionManager::RemoveUserSession(int32_t userId)
{
    std::lock_guard<std::mutex> lock(userSessionsLock_);
    userSessions_.erase(userId);
}

void UserSessionManager::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler)
{
    eventHandler_ = eventHandler;
}
} // namespace MiscServices
} // namespace OHOS