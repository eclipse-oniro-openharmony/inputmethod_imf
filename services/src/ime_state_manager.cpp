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

#include "ime_state_manager.h"

#include "global.h"

namespace OHOS {
namespace MiscServices {
std::shared_ptr<AppExecFwk::EventHandler> ImeStateManager::eventHandler_ = nullptr;
bool ImeStateManager::IsIpcNeeded(RequestType type)
{
    // If ime is in use, no need to request hide.
    std::lock_guard<std::mutex> lock(mutex_);
    return !(type == RequestType::REQUEST_HIDE && !isImeInUse_);
}

void ImeStateManager::BeforeIpc(RequestType type)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (type == RequestType::START_INPUT || type == RequestType::REQUEST_SHOW) {
            isImeInUse_ = true;
        }
        if (!isFrozen_) {
            IMSA_HILOGD("not frozen already.");
            return;
        }
        isFrozen_ = false;
    }
    ControlIme(false);
}

void ImeStateManager::AfterIpc(RequestType type, bool isSuccess)
{
    bool shouldFreeze = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (type == RequestType::START_INPUT || type == RequestType::REQUEST_SHOW) {
            isImeInUse_ = isSuccess;
        }
        if (type == RequestType::REQUEST_HIDE && isImeInUse_) {
            isImeInUse_ = !isSuccess;
        }
        if (type == RequestType::STOP_INPUT) {
            isImeInUse_ = false;
        }
        if (isFrozen_ == !isImeInUse_) {
            IMSA_HILOGD("frozen state already: %{public}d.", isFrozen_);
            return;
        }
        isFrozen_ = !isImeInUse_;
        shouldFreeze = isFrozen_;
    }
    ControlIme(shouldFreeze);
}

void ImeStateManager::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler)
{
    eventHandler_ = eventHandler;
}

bool ImeStateManager::IsImeInUse()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isImeInUse_;
}
} // namespace MiscServices
} // namespace OHOS