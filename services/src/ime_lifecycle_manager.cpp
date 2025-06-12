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
constexpr std::int32_t STOP_DELAY_TIME = 20000L;
void ImeLifecycleManager::ControlIme(bool shouldStop)
{
    if (eventHandler_ == nullptr) {
        IMSA_HILOGE("eventHandler_ is nullptr.");
        return;
    }
    if (shouldStop) {
        // Delay the stop report by 20s.
        eventHandler_->PostTask(
            [this]() {
                if (stopImeFunc_ != nullptr) {
                    stopImeFunc_();
                }
            },
            STOP_IME_TASK_NAME, STOP_DELAY_TIME);
    } else {
        // Cancel the unexecuted FREEZE task.
        eventHandler_->RemoveTask(STOP_IME_TASK_NAME);
    }
}
} // namespace MiscServices
} // namespace OHOS