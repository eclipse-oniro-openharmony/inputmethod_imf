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
#ifndef IME_MIRROR_MANAGER_H
#define IME_MIRROR_MANAGER_H
#include <atomic>
#include <functional>
#include <mutex>
#include <unordered_map>

#include "iservice_registry.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace MiscServices {
class ImeMirrorManager {
public:
    bool IsImeMirrorEnable();
    void SetImeMirrorEnable(bool isRegistered);
    bool SubscribeSaStart(std::function<void()> handler, int32_t saId);
    bool UnSubscribeSaStart(int32_t saId);

private:
    class SaMgrListener : public SystemAbilityStatusChangeStub {
    public:
        explicit SaMgrListener(std::function<void()> handler) : func_(handler) {};
        ~SaMgrListener() = default;
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override { }

    private:
        std::function<void()> func_;
    };
    std::atomic<bool> isEnable_ = false;

    std::mutex listenerMapMutex_;
    std::unordered_map<int32_t, sptr<ISystemAbilityStatusChange>> saMgrListenerMap_; // saId -> listener
};
}
}
#endif // IME_MIRROR_MANAGER_H