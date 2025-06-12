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
#ifndef IME_STATE_MANAGER_FACTORY_H
#define IME_STATE_MANAGER_FACTORY_H
#include <memory>
#include <functional>

#include "ime_state_manager.h"
namespace OHOS {
namespace MiscServices {
class ImeStateManagerFactory {
public:
    static ImeStateManagerFactory& GetInstance();
    ImeStateManagerFactory(const ImeStateManagerFactory&) = delete;
    ImeStateManagerFactory& operator=(const ImeStateManagerFactory&) = delete;
    void SetDynamicStartIme(bool ifDynamicStartIme);
    bool GetDynamicStartIme();
    std::shared_ptr<ImeStateManager> CreateImeStateManager(pid_t pid, std::function<void()> stopImeFunc);

private:
    ImeStateManagerFactory() = default;
    ~ImeStateManagerFactory() = default;
    std::atomic<bool> ifDynamicStartIme_ { false };
};
} // namespace MiscServices
} // namespace OHOS
#endif // IME_STATE_MANAGER_FACTORY_H