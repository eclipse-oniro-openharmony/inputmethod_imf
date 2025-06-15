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
#include "ime_lifecycle_manager.h"

#include "global.h"

namespace OHOS {
namespace MiscServices {
constexpr const char *STOP_IME_TASK_NAME = "StopImeTask";
void ImeLifecycleManager::ControlIme(bool shouldApply)
{
    if (eventHandler_ == nullptr) {
        IMSA_HILOGE("eventHandler_ is nullptr.");
        return;
    }

    if (!shouldApply) {
        // Cancel the unexecuted stop task.
        eventHandler_->RemoveTask(STOP_IME_TASK_NAME);
        return;
    }

    // Delay the stop report by 20s.
    std::weak_ptr<ImeLifecycleManager> weakThis = shared_from_this();
    eventHandler_->PostTask(
        [weakThis]() {
            auto sharedThis = weakThis.lock();
            if (sharedThis == nullptr) {
                IMSA_HILOGE("sharedThis is nullptr.");
                return;
            }
            if (sharedThis->stopImeFunc_ == nullptr) {
                IMSA_HILOGE("stopImeFunc_ is nullptr.");
                return;
            }
            IMSA_HILOGD("Stop ime pid %{public}d", sharedThis->pid_);
            sharedThis->stopImeFunc_();
        },
        STOP_IME_TASK_NAME, stopDelayTime_);
}
} // namespace MiscServices
} // namespace OHOS