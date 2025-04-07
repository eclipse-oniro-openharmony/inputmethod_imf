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

#ifndef SERVICES_INCLUDE_CLIENT_GROUP_H
#define SERVICES_INCLUDE_CLIENT_GROUP_H

#include <map>
#include <utility>

#include "input_client_info.h"
#include "input_death_recipient.h"

namespace OHOS {
namespace MiscServices {
enum ClientAddEvent : int32_t {
    PREPARE_INPUT = 0,
    START_LISTENING,
};

class ClientGroup {
public:
    using ClientDiedHandler = std::function<void(const sptr<IInputClient> &)>;
    ClientGroup(uint64_t displayGroupId, ClientDiedHandler diedHandler)
        : displayGroupId_(displayGroupId), clientDiedHandler_(std::move(diedHandler))
    {
    }

    int32_t AddClientInfo(
        const sptr<IRemoteObject> &inputClient, const InputClientInfo &clientInfo, ClientAddEvent event);
    void RemoveClientInfo(const sptr<IRemoteObject> &client, bool isClientDied = false);
    void UpdateClientInfo(const sptr<IRemoteObject> &client,
        const std::unordered_map<UpdateFlag,
            std::variant<bool, uint32_t, ImeType, ClientState, TextTotalConfig, ClientType>> &updateInfos);

    std::shared_ptr<InputClientInfo> GetClientInfo(sptr<IRemoteObject> inputClient);
    std::shared_ptr<InputClientInfo> GetClientInfo(pid_t pid);
    std::shared_ptr<InputClientInfo> GetCurrentClientInfo();
    int64_t GetCurrentClientPid();
    int64_t GetInactiveClientPid();

    bool IsClientExist(sptr<IRemoteObject> inputClient);
    bool IsNotifyInputStop(const sptr<IInputClient> &client);

    sptr<IInputClient> GetCurrentClient();
    void SetCurrentClient(sptr<IInputClient> client);
    sptr<IInputClient> GetInactiveClient();
    void SetInactiveClient(sptr<IInputClient> client);

    bool IsCurClientFocused(int32_t pid, int32_t uid);
    bool IsCurClientUnFocused(int32_t pid, int32_t uid);

    // from service notify clients
    int32_t NotifyInputStartToClients(uint32_t callingWndId, int32_t requestKeyboardReason = 0);
    int32_t NotifyInputStopToClients();
    int32_t NotifyPanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info);
    int32_t NotifyImeChangeToClients(const Property &property, const SubProperty &subProperty);

private:
    std::map<sptr<IRemoteObject>, std::shared_ptr<InputClientInfo>> GetClientMap();
    bool IsSameClient(sptr<IInputClient> source, sptr<IInputClient> dest);
    void OnClientDied(sptr<IInputClient> remote);
    uint64_t displayGroupId_{ DEFAULT_DISPLAY_ID };

    std::recursive_mutex mtx_;
    std::map<sptr<IRemoteObject>, std::shared_ptr<InputClientInfo>> mapClients_;

    std::mutex currentClientLock_{};
    sptr<IInputClient> currentClient_; // the current input client

    std::mutex inactiveClientLock_{};
    sptr<IInputClient> inactiveClient_; // the inactive input client

    std::mutex clientDiedLock_{};
    ClientDiedHandler clientDiedHandler_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_CLIENT_GROUP_H
