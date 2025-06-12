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

#ifndef IME_LIFECYCLE_MANAEGR_H
#define IME_LIFECYCLE_MANAEGR_H
#include <functional>

#include "ime_state_manager.h"
namespace OHOS {
namespace MiscServices {
class ImeLifecycleManager : public ImeStateManager {
public:
    explicit ImeLifecycleManager(pid_t pid, std::function<void()> stopImeFunc)
        : ImeStateManager(pid), stopImeFunc_(stopImeFunc) { };
    ~ImeLifecycleManager() = default;

private:
    void ControlIme(bool shouldStop) override;
    std::function<void()> stopImeFunc_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // IME_LIFECYCLE_MANAEGR_H