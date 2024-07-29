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

#ifndef SERVICES_INCLUDE_USER_SESSION_MANAGER_H
#define SERVICES_INCLUDE_USER_SESSION_MANAGER_H

#include <chrono>
#include <map>

#include "event_handler.h"
#include "peruser_session.h"
namespace OHOS {
namespace MiscServices {
class UserSessionManager {
public:
    static UserSessionManager &GetInstance();
    std::unordered_map<int32_t, std::shared_ptr<PerUserSession>> GetUserSessions();
    std::shared_ptr<PerUserSession> GetUserSession(int32_t userId);
    void AddUserSession(int32_t userId);
    void RemoveUserSession(int32_t userId);
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler);

private:
    UserSessionManager() = default;
    ~UserSessionManager() = default;
    std::mutex userSessionsLock_;
    std::unordered_map<int32_t, std::shared_ptr<PerUserSession>> userSessions_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_USER_SESSION_MANAGER_H
