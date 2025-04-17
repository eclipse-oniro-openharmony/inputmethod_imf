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

#include "client_group.h"

#include <cinttypes>

#include "event_status_manager.h"
#include "identity_checker_impl.h"
#include "ime_event_listener_manager.h"
#include "variant_util.h"

namespace OHOS {
namespace MiscServices {
uint64_t ClientGroup::GetDisplayGroupId()
{
    return displayGroupId_;
}

int32_t ClientGroup::AddClientInfo(
    const sptr<IRemoteObject> &inputClient, const InputClientInfo &clientInfo)
{
    auto cacheInfo = GetClientInfo(inputClient);
    IMSA_HILOGI("ClientGroup enter");
    if (cacheInfo != nullptr) {
        IMSA_HILOGD("info is existed.");
        if (cacheInfo->uiExtensionTokenId == IMF_INVALID_TOKENID
            && clientInfo.uiExtensionTokenId != IMF_INVALID_TOKENID) {
            UpdateClientInfo(inputClient, { { UpdateFlag::UIEXTENSION_TOKENID, clientInfo.uiExtensionTokenId } });
        }
        UpdateClientInfo(inputClient,
            { { UpdateFlag::TEXT_CONFIG, clientInfo.config }, { UpdateFlag::CLIENT_TYPE, clientInfo.type } });
        return ErrorCode::NO_ERROR;
    }
    auto info = std::make_shared<InputClientInfo>(clientInfo);
    std::weak_ptr<InputClientInfo> weakClientInfo = info;
    info->deathRecipient->SetDeathRecipient([this, weakClientInfo](const wptr<IRemoteObject> &) {
        auto clientInfo = weakClientInfo.lock();
        if (clientInfo == nullptr) {
            IMSA_HILOGD("clientInfo is nullptr.");
            return;
        }
        this->OnClientDied(clientInfo->client);
    });
    auto obj = info->client->AsObject();
    if (obj == nullptr) {
        IMSA_HILOGE("client obj is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    if (obj->IsProxyObject() && !obj->AddDeathRecipient(info->deathRecipient)) {
        IMSA_HILOGE("failed to add client death recipient!");
        return ErrorCode::ERROR_CLIENT_ADD_FAILED;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx_);
    mapClients_.insert({ inputClient, info });
    IMSA_HILOGI(
        "add client with pid: %{public}d displayGroupId: %{public}" PRIu64 " end.", clientInfo.pid, displayGroupId_);
    return ErrorCode::NO_ERROR;
}

void ClientGroup::RemoveClientInfo(const sptr<IRemoteObject> &client, bool isClientDied)
{
    auto clientInfo = GetClientInfo(client);
    if (clientInfo == nullptr) {
        IMSA_HILOGD("client already removed.");
        return;
    }
    if (clientInfo->deathRecipient != nullptr) {
        IMSA_HILOGD("deathRecipient remove.");
        client->RemoveDeathRecipient(clientInfo->deathRecipient);
    }
    std::lock_guard<std::recursive_mutex> lock(mtx_);
    mapClients_.erase(client);
    IMSA_HILOGI("client[%{public}d] is removed.", clientInfo->pid);
}

void ClientGroup::UpdateClientInfo(const sptr<IRemoteObject> &client, const std::unordered_map<UpdateFlag,
    std::variant<bool, uint32_t, ImeType, ClientState, TextTotalConfig, ClientType>> &updateInfos)
{
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr!");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx_);
    auto it = mapClients_.find(client);
    if (it == mapClients_.end() || it->second == nullptr) {
        IMSA_HILOGD("client not found.");
        return;
    }
    for (const auto &updateInfo : updateInfos) {
        switch (updateInfo.first) {
            case UpdateFlag::ISSHOWKEYBOARD: {
                VariantUtil::GetValue(updateInfo.second, it->second->isShowKeyboard);
                break;
            }
            case UpdateFlag::BINDIMETYPE: {
                VariantUtil::GetValue(updateInfo.second, it->second->bindImeType);
                break;
            }
            case UpdateFlag::STATE: {
                VariantUtil::GetValue(updateInfo.second, it->second->state);
                break;
            }
            case UpdateFlag::TEXT_CONFIG: {
                VariantUtil::GetValue(updateInfo.second, it->second->config);
                break;
            }
            case UpdateFlag::UIEXTENSION_TOKENID: {
                VariantUtil::GetValue(updateInfo.second, it->second->uiExtensionTokenId);
                break;
            }
            case UpdateFlag::CLIENT_TYPE: {
                VariantUtil::GetValue(updateInfo.second, it->second->type);
                break;
            }
            default:
                break;
        }
    }
}

std::shared_ptr<InputClientInfo> ClientGroup::GetClientInfo(pid_t pid)
{
    auto iter = std::find_if(
        mapClients_.begin(), mapClients_.end(), [pid](const auto &mapClient) { return mapClient.second->pid == pid; });
    if (iter == mapClients_.end()) {
        IMSA_HILOGD("not found.");
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx_);
    return iter->second;
}

std::shared_ptr<InputClientInfo> ClientGroup::GetCurrentClientInfo()
{
    auto client = GetCurrentClient();
    if (client == nullptr) {
        IMSA_HILOGD("no client in bound state.");
        return nullptr;
    }
    return GetClientInfo(client->AsObject());
}

int64_t ClientGroup::GetCurrentClientPid()
{
    auto clientInfo = GetCurrentClientInfo();
    if (clientInfo == nullptr) {
        IMSA_HILOGD("current client info not found");
        return INVALID_PID;
    }
    return clientInfo->pid;
}

int64_t ClientGroup::GetInactiveClientPid()
{
    auto client = GetInactiveClient();
    if (client == nullptr) {
        IMSA_HILOGD("no inactive client");
        return INVALID_PID;
    }
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        IMSA_HILOGD("client info not found");
        return INVALID_PID;
    }
    return clientInfo->pid;
}

bool ClientGroup::IsClientExist(sptr<IRemoteObject> inputClient)
{
    std::lock_guard<std::recursive_mutex> lock(mtx_);
    return mapClients_.find(inputClient) != mapClients_.end();
}

bool ClientGroup::IsNotifyInputStop(const sptr<IInputClient> &client)
{
    if (IsSameClient(client, GetCurrentClient())) {
        return true;
    }
    if (GetCurrentClient() == nullptr && IsSameClient(client, GetInactiveClient())) {
        return true;
    }
    return false;
}

sptr<IInputClient> ClientGroup::GetCurrentClient()
{
    IMSA_HILOGD("get current client.");
    std::lock_guard<std::mutex> lock(currentClientLock_);
    return currentClient_;
}

void ClientGroup::SetCurrentClient(sptr<IInputClient> client)
{
    IMSA_HILOGD("set current client.");
    std::lock_guard<std::mutex> lock(currentClientLock_);
    currentClient_ = client;
}

sptr<IInputClient> ClientGroup::GetInactiveClient()
{
    std::lock_guard<std::mutex> lock(inactiveClientLock_);
    return inactiveClient_;
}

void ClientGroup::SetInactiveClient(sptr<IInputClient> client)
{
    IMSA_HILOGD("set inactive client.");
    std::lock_guard<std::mutex> lock(inactiveClientLock_);
    inactiveClient_ = client;
}

bool ClientGroup::IsCurClientFocused(int32_t pid, int32_t uid)
{
    auto clientInfo = GetCurrentClientInfo();
    if (clientInfo == nullptr) {
        IMSA_HILOGE("failed to get cur client info!");
        return false;
    }
    auto identityChecker = std::make_shared<IdentityCheckerImpl>();
    if (clientInfo->uiExtensionTokenId != IMF_INVALID_TOKENID
        && identityChecker->IsFocusedUIExtension(clientInfo->uiExtensionTokenId)) {
        IMSA_HILOGI("UIExtension focused");
        return true;
    }
    return clientInfo->pid == pid && clientInfo->uid == uid;
}

bool ClientGroup::IsCurClientUnFocused(int32_t pid, int32_t uid)
{
    auto clientInfo = GetCurrentClientInfo();
    if (clientInfo == nullptr) {
        IMSA_HILOGE("failed to get cur client info!");
        return false;
    }
    auto identityChecker = std::make_shared<IdentityCheckerImpl>();
    if (clientInfo->uiExtensionTokenId != IMF_INVALID_TOKENID
        && !identityChecker->IsFocusedUIExtension(clientInfo->uiExtensionTokenId)) {
        IMSA_HILOGI("UIExtension UnFocused.");
        return true;
    }
    return clientInfo->pid == pid && clientInfo->uid == uid;
}

std::shared_ptr<InputClientInfo> ClientGroup::GetClientInfo(sptr<IRemoteObject> inputClient)
{
    if (inputClient == nullptr) {
        IMSA_HILOGE("inputClient is nullptr!");
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx_);
    auto it = mapClients_.find(inputClient);
    if (it == mapClients_.end()) {
        IMSA_HILOGD("client not found.");
        return nullptr;
    }
    return it->second;
}

std::map<sptr<IRemoteObject>, std::shared_ptr<InputClientInfo>> ClientGroup::GetClientMap()
{
    std::lock_guard<std::recursive_mutex> lock(mtx_);
    return mapClients_;
}

bool ClientGroup::IsSameClient(sptr<IInputClient> source, sptr<IInputClient> dest)
{
    return source != nullptr && dest != nullptr && source->AsObject() == dest->AsObject();
}

void ClientGroup::OnClientDied(sptr<IInputClient> remote)
{
    std::lock_guard<std::mutex> lock(clientDiedLock_);
    if (clientDiedHandler_ == nullptr) {
        return;
    }
    clientDiedHandler_(remote);
}
} // namespace MiscServices
} // namespace OHOS