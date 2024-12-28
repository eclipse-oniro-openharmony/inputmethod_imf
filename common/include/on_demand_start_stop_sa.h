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

#ifndef ON_DEMAND_START_STOP_SA_H
#define ON_DEMAND_START_STOP_SA_H

#include <condition_variable>

#include "singleton.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace MiscServices {
class OnDemandStartStopSa : public std::enable_shared_from_this<OnDemandStartStopSa> {
public:
    OnDemandStartStopSa() = default;
    ~OnDemandStartStopSa() = default;
    void UnloadInputMethodSystemAbility();
    static sptr<IRemoteObject> GetInputMethodSystemAbility(bool ifRetry = true);
    static void IncreaseProcessingIpcCnt();
    static void DecreaseProcessingIpcCnt();
    static bool IsSaBusy();

private:
    sptr<IRemoteObject> LoadInputMethodSystemAbility();
    class SaLoadCallback : public SystemAbilityLoadCallbackStub {
    public:
        SaLoadCallback(std::shared_ptr<OnDemandStartStopSa> onDemandObj) : onDemandObj_(onDemandObj) {};
        void OnLoadSystemAbilitySuccess(int32_t said, const sptr<IRemoteObject> &object) override;
        void OnLoadSystemAbilityFail(int32_t said) override;

        std::shared_ptr<OnDemandStartStopSa> onDemandObj_ = nullptr;
    };

    sptr<IRemoteObject> remoteObj_;
    std::condition_variable loadSaCv_;
    std::mutex loadSaMtx_;
    static constexpr int32_t LOAD_SA_MAX_WAIT_TIME = 5; // 5s
    static std::atomic<uint32_t> processingIpcCount_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // ON_DEMAND_START_STOP_SA_H