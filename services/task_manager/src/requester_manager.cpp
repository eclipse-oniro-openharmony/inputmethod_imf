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

#include "requester_manager.h"

#include "global.h"

namespace OHOS {
namespace MiscServices {
constexpr int32_t MAX_REQUEST_COUNT = 6;
RequesterManager &RequesterManager::GetInstance()
{
    static RequesterManager requesterManager;
    return requesterManager;
}

std::shared_ptr<RequesterInfo> RequesterManager::GetRequester(int32_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = requesterMap_.find(pid);
    if (iter == requesterMap_.end() || iter->second == nullptr) {
        IMSA_HILOGE("client: %{public}d not registered or nullptr", pid);
        return nullptr;
    }
    if (iter->second->imaResponseChannel == nullptr && iter->second->imcResponseChannel == nullptr) {
        IMSA_HILOGE("client: %{public}d channel is nullptr", pid);
        return nullptr;
    }
    auto requesterInfo = iter->second;
    if (requesterInfo->requestCount >= MAX_REQUEST_COUNT) {
        IMSA_HILOGE("requests from client: %{public}d, count: %{public}d, too much", pid, requesterInfo->requestCount);
        return nullptr;
    }
    return requesterInfo;
}

int32_t RequesterManager::AddImaChannel(int32_t pid, sptr<IImaResponseChannel> channel)
{
    int32_t ret = AddChannel(pid, channel, nullptr);
    IMSA_HILOGI("pid[%{public}d], register result: %{public}d", pid, ret);
    return ret;
    return ErrorCode::NO_ERROR;
}

int32_t RequesterManager::AddImcChannel(int32_t pid, sptr<IImcResponseChannel> channel)
{
    int32_t ret = AddChannel(pid, nullptr, channel);
    IMSA_HILOGI("pid[%{public}d], register result: %{public}d", pid, ret);
    return ret;
}

void RequesterManager::TaskIn(int32_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = requesterMap_.find(pid);
    if (iter == requesterMap_.end()) {
        IMSA_HILOGE("client: %{public}d not found", pid);
        return;
    }
    if (iter->second == nullptr) {
        IMSA_HILOGE("pid %{public}d info nullptr", pid);
        return;
    }
    ++iter->second->requestCount;
}

void RequesterManager::TaskOut(int32_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = requesterMap_.find(pid);
    if (iter == requesterMap_.end()) {
        IMSA_HILOGE("client: %{public}d not found", pid);
        return;
    }
    if (iter->second == nullptr) {
        IMSA_HILOGE("pid %{public}d info nullptr", pid);
        return;
    }
    if (iter->second->requestCount > 0) {
        --iter->second->requestCount;
    }
}

int32_t RequesterManager::AddChannel(
    int32_t pid, sptr<IImaResponseChannel> imaChannel, sptr<IImcResponseChannel> imcChannel)
{
    const bool isAddingIma = imaChannel != nullptr;
    const bool isAddingImc = imcChannel != nullptr;
    if ((imaChannel && imcChannel) || (!imaChannel && !imcChannel)) {
        IMSA_HILOGE("both nullptr or both not nullptr");
        return ErrorCode::ERROR_IMSA_NULLPTR;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<RequesterInfo> info;
    auto iter = requesterMap_.find(pid);
    if (iter != requesterMap_.end() && iter->second != nullptr) {
        info = iter->second;
        if ((isAddingIma && info->imaResponseChannel != nullptr) ||
            (isAddingImc && info->imcResponseChannel != nullptr)) {
            IMSA_HILOGD("client: %{public}d already registered", pid);
            return ErrorCode::NO_ERROR;
        }
        if (info->imaResponseChannel != nullptr || info->imcResponseChannel != nullptr) {
            info->AddChannel(imaChannel, imcChannel);
            return ErrorCode::NO_ERROR;
        }
    } else {
        info = std::make_shared<RequesterInfo>();
    }

    sptr<InputDeathRecipient> deathRecipient = new (std::nothrow) InputDeathRecipient();
    if (deathRecipient == nullptr) {
        IMSA_HILOGE("failed to new deathRecipient!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    deathRecipient->SetDeathRecipient([this, pid](const wptr<IRemoteObject> &remote) { OnClientDied(pid); });
    auto object = isAddingIma ? imaChannel->AsObject() : imcChannel->AsObject();
    if (object == nullptr || !object->IsProxyObject() || !object->AddDeathRecipient(deathRecipient)) {
        IMSA_HILOGE("failed to add death recipient");
        return ErrorCode::ERROR_ADD_DEATH_RECIPIENT_FAILED;
    }
    info->deathRecipient = deathRecipient;
    info->AddChannel(imaChannel, imcChannel);
    requesterMap_.insert_or_assign(pid, info);
    IMSA_HILOGI("register success, pid: %{public}d", pid);
    return ErrorCode::NO_ERROR;
}

void RequesterManager::OnClientDied(int32_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    IMSA_HILOGI("requester: %{public}d died", pid);
    auto iter = requesterMap_.find(pid);
    if (iter == requesterMap_.end()) {
        IMSA_HILOGD("already removed");
        return;
    }
    auto info = iter->second;
    if (info != nullptr) {
        if (info->imaResponseChannel != nullptr && info->imaResponseChannel->AsObject() != nullptr) {
            info->imaResponseChannel->AsObject()->RemoveDeathRecipient(info->deathRecipient);
        }
        if (info->imcResponseChannel != nullptr && info->imcResponseChannel->AsObject() != nullptr) {
            info->imcResponseChannel->AsObject()->RemoveDeathRecipient(info->deathRecipient);
        }
    }
    requesterMap_.erase(pid);
    IMSA_HILOGI("requester: %{public}d removed", pid);
}
} // namespace MiscServices
} // namespace OHOS