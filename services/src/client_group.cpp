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

#include "client_group.h"

namespace OHOS {
namespace MiscServices {
int32_t ClientGroup::AddClientInfo(
    sptr<IRemoteObject> inputClient, const InputClientInfo &clientInfo, ClientAddEvent event)
{
    auto cacheInfo = GetClientInfo(inputClient);
    if (cacheInfo != nullptr) {
        IMSA_HILOGD("info is existed.");
        if (cacheInfo->uiExtensionTokenId == IMF_INVALID_TOKENID
            && clientInfo.uiExtensionTokenId != IMF_INVALID_TOKENID) {
            UpdateClientInfo(inputClient, { { UpdateFlag::UIEXTENSION_TOKENID, clientInfo.uiExtensionTokenId } });
        }
        UpdateClientInfo(inputClient,
            { { UpdateFlag::TEXT_CONFIG, clientInfo.config }, { UpdateFlag::CLIENT_TYPE, clientInfo.type } });
        if (event == START_LISTENING) {
            UpdateClientInfo(inputClient, { { UpdateFlag::EVENTFLAG, clientInfo.eventFlag } });
        }
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
    std::lock_guard<std::recursive_mutex> lock(mtx);
    mapClients_.insert({ inputClient, info });
    IMSA_HILOGI("add client end.");
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<InputClientInfo> ClientGroup::GetClientInfo(sptr<IRemoteObject> inputClient)
{
    if (inputClient == nullptr) {
        IMSA_HILOGE("inputClient is nullptr!");
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto it = mapClients_.find(inputClient);
    if (it == mapClients_.end()) {
        IMSA_HILOGD("client not found.");
        return nullptr;
    }
    return it->second;
}

std::shared_ptr<InputClientInfo> ClientGroup::GetClientInfo(pid_t pid)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto iter = std::find_if(
        mapClients_.begin(), mapClients_.end(), [pid](const auto &mapClient) { return mapClient.second->pid == pid; });
    if (iter == mapClients_.end()) {
        IMSA_HILOGD("not found.");
        return nullptr;
    }
    return iter->second;
}

void ClientGroup::UpdateClientInfo(const sptr<IRemoteObject> &client,
    const std::unordered_map<UpdateFlag, std::variant<bool, uint32_t, ImeType, ClientState, TextTotalConfig, ClientType>>
        &updateInfos)
{
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr!");
        return;
    }
    auto info = GetClientInfo(client);
    if (info == nullptr) {
        IMSA_HILOGE("client info is not exist!");
        return;
    }
    for (const auto &updateInfo : updateInfos) {
        switch (updateInfo.first) {
            case UpdateFlag::EVENTFLAG: {
                info->eventFlag = std::get<uint32_t>(updateInfo.second);
                break;
            }
            case UpdateFlag::ISSHOWKEYBOARD: {
                info->isShowKeyboard = std::get<bool>(updateInfo.second);
                break;
            }
            case UpdateFlag::BINDIMETYPE: {
                info->bindImeType = std::get<ImeType>(updateInfo.second);
                break;
            }
            case UpdateFlag::STATE: {
                info->state = std::get<ClientState>(updateInfo.second);
                break;
            }
            case UpdateFlag::TEXT_CONFIG: {
                info->config = std::get<TextTotalConfig>(updateInfo.second);
                break;
            }
            case UpdateFlag::UIEXTENSION_TOKENID: {
                info->uiExtensionTokenId = std::get<uint32_t>(updateInfo.second);
                break;
            }
            case UpdateFlag::CLIENT_TYPE: {
                info->type = std::get<ClientType>(updateInfo.second);
                break;
            }
            default:
                break;
        }
    }
}

void ClientGroup::ReplaceCurrentClient(const sptr<IInputClient> &client)
{
    std::lock_guard<std::mutex> lock(focusedClientLock_);
    if (client == nullptr) {
        return;
    }
    auto clientInfo = GetClientInfo(client->AsObject());
    if (clientInfo == nullptr) {
        return;
    }
    auto replacedClient = GetCurrentClient();
    SetCurrentClient(client);
    if (replacedClient != nullptr) {
        auto replacedClientInfo = GetClientInfo(replacedClient->AsObject());
        if (replacedClientInfo != nullptr && replacedClientInfo->pid != clientInfo->pid) {
            IMSA_HILOGI("remove replaced client: [%{public}d]", replacedClientInfo->pid);
            RemoveClient(replacedClient);
        }
    }
    auto inactiveClient = GetInactiveClient();
    if (inactiveClient != nullptr) {
        auto inactiveClientInfo = GetClientInfo(inactiveClient->AsObject());
        if (inactiveClientInfo != nullptr && inactiveClientInfo->pid != clientInfo->pid) {
            IMSA_HILOGI("remove inactive client: [%{public}d]", inactiveClientInfo->pid);
            RemoveClient(inactiveClient, false);
        }
    }
}

void ClientGroup::RemoveClientInfo(const sptr<IRemoteObject> &client, bool isClientDied)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    auto clientInfo = GetClientInfo(client);
    if (clientInfo == nullptr) {
        IMSA_HILOGD("client already removed.");
        return;
    }
    // if client is subscriber and the release is not because of the client died, do not remove
    if (clientInfo->eventFlag != NO_EVENT_ON && !isClientDied) {
        IMSA_HILOGD("is subscriber, do not remove.");
        auto isShowKeyboard = false;
        auto bindImeType = ImeType::NONE;
        UpdateClientInfo(
            client, { { UpdateFlag::BINDIMETYPE, bindImeType }, { UpdateFlag::ISSHOWKEYBOARD, isShowKeyboard } });
        return;
    }
    if (clientInfo->deathRecipient != nullptr) {
        IMSA_HILOGD("deathRecipient remove.");
        client->RemoveDeathRecipient(clientInfo->deathRecipient);
    }
    mapClients_.erase(client);
    IMSA_HILOGI("client[%{public}d] is removed.", clientInfo->pid);
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

void ClientGroup::SetInactiveClient(sptr<IInputClient> client)
{
    IMSA_HILOGD("set inactive client.");
    std::lock_guard<std::mutex> lock(inactiveClientLock_);
    inactiveClient_ = client;
}

sptr<IInputClient> ClientGroup::GetInactiveClient()
{
    std::lock_guard<std::mutex> lock(inactiveClientLock_);
    return inactiveClient_;
}
} // namespace MiscServices
} // namespace OHOS