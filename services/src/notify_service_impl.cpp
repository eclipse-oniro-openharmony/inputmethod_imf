/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "notify_service_impl.h"
#include "peruser_session.h"
#include "user_session_manager.h"
#include "os_account_adapter.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace MiscServices {

ErrCode OnInputStopNotifyServiceImpl::NotifyOnInputStopFinished()
{
    IMSA_HILOGI("NotifyOnInputStopFinished is start!");
    if (pid_ != IPCSkeleton::GetCallingPid()) {
        IMSA_HILOGI("pid is invalid!");
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    auto userId = OsAccountAdapter::GetOsAccountLocalIdFromUid(IPCSkeleton::GetCallingUid());
    if (userId == OsAccountAdapter::INVALID_USER_ID) {
        return ErrorCode::ERROR_EX_ILLEGAL_STATE;
    }
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND;
    }
    session->NotifyOnInputStopFinished();
    IMSA_HILOGI("OnInputStop is finished");
    return ERR_OK;
}
} // namespace MiscServices
} // namespace OHOS