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

#include "ima_service_proxy.h"

#include "ima_response_channel_impl.h"
#include "input_method_ability.h"
#include "on_demand_start_stop_sa.h"
#include "variant_util.h"

namespace OHOS {
namespace MiscServices {
ImaServiceProxy &ImaServiceProxy::GetInstance()
{
    static ImaServiceProxy ImaServiceProxy;
    return ImaServiceProxy;
}

ImaServiceProxy::ImaServiceProxy()
{
    Init();
}

void ImaServiceProxy::Init()
{
    IMSA_HILOGD("start");
    sptr<ImaResponseChannelStub> channel = new (std::nothrow) ImaResponseChannelImpl();
    if (channel == nullptr) {
        IMSA_HILOGE("failed to create channel!");
        return;
    }
    responseChannelStub_ = channel;
}

RequestId ImaServiceProxy::GetNextRequestId()
{
    static std::atomic<uint64_t> seqId{ 1 };
    return seqId.fetch_add(1, std::memory_order_seq_cst);
}

void ImaServiceProxy::OnResponse(RequestId id, const ServiceResponse &responseData)
{
    std::lock_guard<std::mutex> lock(requestsMutex_);
    auto iter = pendingRequests_.find(id);
    if (iter == pendingRequests_.end()) {
        IMSA_HILOGE("failed to find pending request, id: %{public}d", id);
        return;
    }
    IMSA_HILOGD("id: %{public}d", id);
    iter->second.promise.set_value(responseData);
}

sptr<IInputMethodSystemAbility> ImaServiceProxy::GetSystemAbilityProxy(bool ifRetry)
{
    std::lock_guard<std::mutex> lock(abilityLock_);
    if (abilityManager_ != nullptr) {
        return abilityManager_;
    }
    IMSA_HILOGD("get input method service proxy.");
    auto systemAbility = OnDemandStartStopSa::GetInputMethodSystemAbility(ifRetry);
    if (systemAbility == nullptr) {
        IMSA_HILOGE("systemAbility is nullptr!");
        return nullptr;
    }

    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new (std::nothrow) InputDeathRecipient();
        if (deathRecipient_ == nullptr) {
            IMSA_HILOGE("create death recipient failed!");
            return nullptr;
        }
    }
    deathRecipient_->SetDeathRecipient([this](const wptr<IRemoteObject> &remote) { OnRemoteSaDied(remote); });
    if ((systemAbility->IsProxyObject()) && (!systemAbility->AddDeathRecipient(deathRecipient_))) {
        IMSA_HILOGE("failed to add death recipient!");
        return nullptr;
    }
    abilityManager_ = iface_cast<IInputMethodSystemAbility>(systemAbility);
    return abilityManager_;
}

void ImaServiceProxy::RemoveDeathRecipient()
{
    std::lock_guard<std::mutex> lock(abilityLock_);
    if (abilityManager_ != nullptr && abilityManager_->AsObject() != nullptr && deathRecipient_ != nullptr) {
        abilityManager_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    deathRecipient_ = nullptr;
    abilityManager_ = nullptr;
}

int32_t ImaServiceProxy::SendRequest(const RequestFunc &request, int64_t timeout)
{
    if (request == nullptr) {
        IMSA_HILOGE("request is nullptr");
        return ErrorCode::ERROR_IMC_NULLPTR;
    }
    if (isInterrupted_.load()) {
        IMSA_HILOGW("request is interrupted");
        return ErrorCode::ERROR_REQUEST_INTERRUPTED;
    }
    PendingRequest pendingRequest;
    RequestId id = GetNextRequestId();
    auto future = pendingRequest.promise.get_future();
    AddRequest(id, std::move(pendingRequest));

    // send request
    int32_t ret = request(id);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("request failed, ret: %{public}d", ret);
        RemoveRequest(id);
        return ret;
    }

    if (isInterrupted_.load()) {
        IMSA_HILOGW("request is interrupted");
        return ErrorCode::ERROR_REQUEST_INTERRUPTED;
    }
    // wait till timeout
    if (future.wait_for(std::chrono::milliseconds(timeout)) != std::future_status::ready) {
        IMSA_HILOGE("service handle timeout");
        RemoveRequest(id);
        return ErrorCode::ERROR_IMC_SERVICE_RESPONSE_TIMEOUT;
    }

    // handle response
    ServiceResponse response = future.get();
    ret = response.result;
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("task failed");
        return ret;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImaServiceProxy::SendRequestInner(
    const RequestFunc &request, ServiceResponseData &responseData, int64_t timeout)
{
    if (request == nullptr) {
        IMSA_HILOGE("request is nullptr");
        return ErrorCode::ERROR_IMC_NULLPTR;
    }
    if (isInterrupted_.load()) {
        IMSA_HILOGW("request is interrupted");
        return ErrorCode::ERROR_REQUEST_INTERRUPTED;
    }

    PendingRequest pendingRequest;
    RequestId id = GetNextRequestId();
    auto future = pendingRequest.promise.get_future();
    AddRequest(id, std::move(pendingRequest));

    // send request
    int32_t ret = request(id);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("request failed, ret: %{public}d", ret);
        RemoveRequest(id);
        return ret;
    }

    if (isInterrupted_.load()) {
        IMSA_HILOGW("request is interrupted");
        return ErrorCode::ERROR_REQUEST_INTERRUPTED;
    }
    // wait till timeout
    if (future.wait_for(std::chrono::milliseconds(timeout)) != std::future_status::ready) {
        IMSA_HILOGE("service handle timeout");
        RemoveRequest(id);
        return ErrorCode::ERROR_IMC_SERVICE_RESPONSE_TIMEOUT;
    }

    // handle response
    ServiceResponse response = future.get();
    ret = response.result;
    RemoveRequest(id);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("task failed");
        return ret;
    }
    responseData = response.responseData;
    return ErrorCode::NO_ERROR;
}

template<typename ResultType>
int32_t ImaServiceProxy::SendRequest(const RequestFunc &request, ResultType &resultValue, int64_t timeout)
{
    ServiceResponseData responseData;
    int32_t ret = SendRequestInner(request, responseData, timeout);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    if (!VariantUtil::GetValue(responseData, resultValue)) {
        IMSA_HILOGE("failed to get result value");
        return ErrorCode::ERROR_INVALID_VARIANT_TYPE;
    }
    return ErrorCode::NO_ERROR;
}

void ImaServiceProxy::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
{
    hasRegistered_.store(false);
    {
        std::lock_guard<std::mutex> lock(abilityLock_);
        abilityManager_ = nullptr;
    }
}

void ImaServiceProxy::AddRequest(RequestId id, PendingRequest pendingRequest)
{
    std::lock_guard<std::mutex> lock(requestsMutex_);
    pendingRequests_[id] = std::move(pendingRequest);
    currentRequestId_.store(id);
}

void ImaServiceProxy::RemoveRequest(RequestId requestId)
{
    std::lock_guard<std::mutex> lock(requestsMutex_);
    pendingRequests_.erase(requestId);
    currentRequestId_.store(0);
}
} // namespace MiscServices
} // namespace OHOS