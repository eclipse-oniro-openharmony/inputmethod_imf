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

#include "ime_event_listener_manager.h"

#include "input_client_info.h"
#include "peruser_session.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {

ImeEventListenerManager &ImeEventListenerManager::GetInstance()
{
    static ImeEventListenerManager imeEventListenerManager;
    return imeEventListenerManager;
}

int32_t ImeEventListenerManager::UpdateListenerInfo(int32_t userId, const ImeEventListenerInfo &info)
{
    IMSA_HILOGI("enter.");
    std::lock_guard<std::mutex> lock(imeEventListenersLock_);
    ImeEventListenerInfo imeIistenerInfo = info;
    auto it = imeEventListeners_.find(userId);
    if (it == imeEventListeners_.end()) {
        GenerateListenerDeath(userId, imeIistenerInfo);
        imeEventListeners_.insert({ userId, { imeIistenerInfo } });
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("ImeEventListenerManager, pid/eventFlag: %{public}lld/%{public}u",
        imeIistenerInfo.pid, imeIistenerInfo.eventFlag);
    auto iter = std::find_if(it->second.begin(), it->second.end(),
        [&imeIistenerInfo](const ImeEventListenerInfo &listenerInfo) {
        return (listenerInfo.client != nullptr && imeIistenerInfo.client != nullptr
            && listenerInfo.client->AsObject() == imeIistenerInfo.client->AsObject());
    });
    if (iter == it->second.end()) {
        GenerateListenerDeath(userId, imeIistenerInfo);
        it->second.push_back(imeIistenerInfo);
        return ErrorCode::NO_ERROR;
    }
    iter->eventFlag = imeIistenerInfo.eventFlag;
    if (iter->eventFlag == ImeEventListenerInfo::NO_EVENT_ON) {
        it->second.erase(iter);
    }
    if (it->second.empty()) {
        imeEventListeners_.erase(userId);
    }
    return ErrorCode::NO_ERROR;
}

std::vector<ImeEventListenerInfo> ImeEventListenerManager::GetListenerInfo(int32_t userId)
{
    IMSA_HILOGI("ImeEventListenerManager::GetListenerInfo enter");
    std::lock_guard<std::mutex> lock(imeEventListenersLock_);
    std::vector<ImeEventListenerInfo> allListenerInfos;
    auto it = imeEventListeners_.find(userId);
    if (it != imeEventListeners_.end()) {
        allListenerInfos = it->second;
    }
    if (userId == 0) {
        return allListenerInfos;
    }
    auto iter = imeEventListeners_.find(0);
    if (iter == imeEventListeners_.end()) {
        return allListenerInfos;
    }
    allListenerInfos.insert(allListenerInfos.end(), iter->second.begin(), iter->second.end());
    IMSA_HILOGI("ImeEventListenerManager::GetListenerInfo end");
    return allListenerInfos;
}

int32_t ImeEventListenerManager::GenerateListenerDeath(int32_t userId, ImeEventListenerInfo &listenerInfo)
{
    IMSA_HILOGI("enter.");
    if (listenerInfo.client == nullptr) {
        IMSA_HILOGE("listenerInfo.client is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto deathRecipient = new (std::nothrow) InputDeathRecipient();
    if (deathRecipient == nullptr) {
        IMSA_HILOGE("failed to new deathRecipient!");
        return ErrorCode::ERROR_IMSA_MALLOC_FAILED;
    }
    listenerInfo.deathRecipient = deathRecipient;
    listenerInfo.deathRecipient->SetDeathRecipient([this, &userId](const wptr<IRemoteObject> &object) {
        this->OnListenerDied(userId, object);
    });
    auto obj = listenerInfo.client->AsObject();
    if (obj == nullptr) {
        IMSA_HILOGE("client obj is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    if (obj->IsProxyObject() && !obj->AddDeathRecipient(listenerInfo.deathRecipient)) {
        IMSA_HILOGE("failed to add client death recipient!");
        return ErrorCode::ERROR_CLIENT_ADD_DEATH_FAILED;
    }
    return ErrorCode::NO_ERROR;
}

void ImeEventListenerManager::OnListenerDied(int32_t userId, const wptr<IRemoteObject> &object)
{
    IMSA_HILOGI("enter.");
    if (object == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(imeEventListenersLock_);
    auto it = imeEventListeners_.find(userId);
    if (it == imeEventListeners_.end()) {
        return;
    }
    auto &listererInfos = it->second;
    auto iter = std::find_if(listererInfos.begin(), listererInfos.end(),
        [&object](const ImeEventListenerInfo &info) {
            return (info.client != nullptr && info.client->AsObject() == object);
        });
    if (iter == listererInfos.end()) {
        return;
    }
    if (iter->deathRecipient != nullptr) {
        IMSA_HILOGD("deathRecipient remove.");
        object->RemoveDeathRecipient(iter->deathRecipient);
    }
    it->second.erase(iter);
    if (it->second.empty()) {
        imeEventListeners_.erase(userId);
    }
}

int32_t ImeEventListenerManager::NotifyInputStart(int32_t userId, int32_t callingWndId, int32_t requestKeyboardReason)
{
    IMSA_HILOGD("enter.");
    auto listenerInfos = GetListenerInfo(userId);
    for (const auto &listenerInfo : listenerInfos) {
        if (listenerInfo.client == nullptr ||
            !EventStatusManager::IsInputStatusChangedOn(listenerInfo.eventFlag)) {
            IMSA_HILOGE("nullptr listenerInfo or no need to notify");
            continue;
        }
        IMSA_HILOGI("pid/eventFlag: %{public}lld/%{public}u", listenerInfo.pid, listenerInfo.eventFlag);
        int32_t ret = listenerInfo.client->NotifyInputStart(callingWndId, requestKeyboardReason);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("failed to notify OnInputStart, errorCode: %{public}d", ret);
            continue;
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEventListenerManager::NotifyInputStop(int32_t userId)
{
    IMSA_HILOGI("enter.");
    auto listenerInfos = GetListenerInfo(userId);
    for (const auto &listenerInfo : listenerInfos) {
        if (listenerInfo.client == nullptr ||
            !EventStatusManager::IsInputStatusChangedOn(listenerInfo.eventFlag)) {
            IMSA_HILOGE("nullptr clientInfo or no need to notify");
            continue;
        }
        IMSA_HILOGI("pid/eventFlag: %{public}lld/%{public}u", listenerInfo.pid, listenerInfo.eventFlag);
        int32_t ret = listenerInfo.client->NotifyInputStop();
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("failed to notify OnInputStop, errorCode: %{public}d", ret);
            continue;
        }
    }
    IMSA_HILOGI("NotifyInputStopToClients end");
    return ErrorCode::NO_ERROR;
}

int32_t ImeEventListenerManager::NotifyPanelStatusChange(int32_t userId,
    const InputWindowStatus &status, const ImeWindowInfo &info)
{
    IMSA_HILOGI("enter.");
    auto listenerInfos = GetListenerInfo(userId);
    for (const auto &listenerInfo : listenerInfos) {
        if (listenerInfo.client == nullptr) {
            IMSA_HILOGD("client nullptr or no need to notify.");
            continue;
        }
        if (status == InputWindowStatus::SHOW && !EventStatusManager::IsImeShowOn(listenerInfo.eventFlag)) {
            IMSA_HILOGD("has not imeShow callback");
            continue;
        }
        if (status == InputWindowStatus::HIDE && !EventStatusManager::IsImeHideOn(listenerInfo.eventFlag)) {
            IMSA_HILOGD("has not imeHide callback");
            continue;
        }
        int32_t ret = listenerInfo.client->OnPanelStatusChange(static_cast<uint32_t>(status), info);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("failed to NotifyPanelStatusChange, ret: %{public}d", ret);
            continue;
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeEventListenerManager::NotifyImeChange(int32_t userId,
    const Property &property, const SubProperty &subProperty)
{
    IMSA_HILOGI("enter.");
    auto listenerInfos = GetListenerInfo(userId);
    for (const auto &listenerInfo : listenerInfos) {
        if (listenerInfo.client == nullptr || !EventStatusManager::IsImeChangeOn(listenerInfo.eventFlag)) {
            IMSA_HILOGD("client nullptr or no need to notify.");
            continue;
        }
        IMSA_HILOGI("pid/eventFlag: %{public}lld/%{public}u", listenerInfo.pid, listenerInfo.eventFlag);
        int32_t ret = listenerInfo.client->OnSwitchInput(property, subProperty);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("notify failed, ret: %{public}d, uid: %{public}d!",
                ret, static_cast<int32_t>(listenerInfo.pid));
            continue;
        }
    }
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS
