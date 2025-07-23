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

#ifndef IMF_IMA_SERVICE_PROXY_H
#define IMF_IMA_SERVICE_PROXY_H

#include <chrono>
#include <cstdint>
#include <future>
#include <unordered_map>

#include "iima_response_channel.h"
#include "iinput_method_system_ability.h"
#include "input_death_recipient.h"
#include "service_response_data.h"

namespace OHOS {
namespace MiscServices {
class ImaServiceProxy {
private:
    ImaServiceProxy();

public:
    static ImaServiceProxy &GetInstance();

    ~ImaServiceProxy() = default;
    ImaServiceProxy(const ImaServiceProxy &) = delete;
    ImaServiceProxy(ImaServiceProxy &&) = delete;
    ImaServiceProxy &operator=(const ImaServiceProxy &) = delete;
    ImaServiceProxy &operator=(ImaServiceProxy &&) = delete;

    int32_t SendRequest(const RequestFunc &request, int64_t timeout);
    template<typename ResultType>
    int32_t SendRequest(const RequestFunc &request, ResultType &resultValue, int64_t timeout);

    void OnResponse(RequestId id, const ServiceResponse &responseData);

    void RemoveDeathRecipient();

private:
    void Init();
    RequestId GetNextRequestId();
    int32_t SendRequestInner(const RequestFunc &request, ServiceResponseData &responseData, int64_t timeout);

    void OnRemoteSaDied(const wptr<IRemoteObject> &remote);
    sptr<IInputMethodSystemAbility> GetSystemAbilityProxy(bool ifRetry = true);

    void AddRequest(RequestId id, PendingRequest pendingRequest);
    void RemoveRequest(RequestId requestId);

    std::atomic<bool> hasRegistered_{ false };
    std::mutex abilityLock_{};
    sptr<IInputMethodSystemAbility> abilityManager_ = nullptr;
    sptr<InputDeathRecipient> deathRecipient_;

    std::atomic<RequestId> currentRequestId_{ 0 };
    std::atomic<bool> isInterrupted_{ false };

    std::mutex requestsMutex_{};
    std::unordered_map<RequestId, PendingRequest> pendingRequests_;
    std::atomic<uint32_t> lastId_{ 0 };

    sptr<IImaResponseChannel> responseChannelStub_{ nullptr };
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_IMA_SERVICE_PROXY_H
