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

#include "dm_common.h"
#include "input_client_info.h"

namespace OHOS {
namespace MiscServices {
enum ClientAddEvent : int32_t {
    PREPARE_INPUT = 0,
    START_LISTENING,
};

class ClientGroup {
public:
    explicit ClientGroup(Rosen::DisplayId displayGroupId) : displayGroupId_(displayGroupId)
    {
    }

    int32_t AddClientInfo(sptr<IRemoteObject> inputClient, const InputClientInfo &clientInfo, ClientAddEvent event);
    bool IsClientExist(sptr<IRemoteObject> inputClient);
    bool IsNotifyInputStop(const sptr<IInputClient> &client);
    std::shared_ptr<InputClientInfo> GetClientInfo(sptr<IRemoteObject> inputClient);
    std::shared_ptr<InputClientInfo> GetClientInfo(pid_t pid);
    std::shared_ptr<InputClientInfo> GetCurrentClientInfo();
    std::map<sptr<IRemoteObject>, std::shared_ptr<InputClientInfo>> GetClientMap(); // 遍历用的
    void RemoveClientInfo(const sptr<IRemoteObject> &client, bool isClientDied = false);
    void UpdateClientInfo(const sptr<IRemoteObject> &client,
        const std::unordered_map<UpdateFlag,
            std::variant<bool, uint32_t, ImeType, ClientState, TextTotalConfig, ClientType>> &updateInfos);

    sptr<IInputClient> GetCurrentClient();
    void SetCurrentClient(sptr<IInputClient> client);
    sptr<IInputClient> GetInactiveClient();
    void SetInactiveClient(sptr<IInputClient> client);
    int64_t GetCurrentClientPid();
    int64_t GetInactiveClientPid();
    int32_t RemoveCurrentClient();
    void DeactivateClient(const sptr<IInputClient> &client);
    void ReplaceCurrentClient(const sptr<IInputClient> &client);
    int32_t RemoveClient(const sptr<IInputClient> &client, bool isUnbindFromClient = false,
        bool isInactiveClient = false, bool isNotifyClientAsync = false); //
    bool IsCurClientFocused(int32_t pid, int32_t uid);
    bool IsCurClientUnFocused(int32_t pid, int32_t uid);
    // from service notify clients input start and stop
    int32_t NotifyInputStartToClients(uint32_t callingWndId, int32_t requestKeyboardReason = 0);
    int32_t NotifyInputStopToClients();
    int32_t NotifyPanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info);
    int32_t NotifyImeChangeToClients(const Property &property, const SubProperty &subProperty);

private:
    bool IsSameClient(sptr<IInputClient> source, sptr<IInputClient> dest); //
    void OnClientDied(sptr<IInputClient> remote);
    Rosen::DisplayId displayGroupId_;
    std::recursive_mutex mtx;
    std::map<sptr<IRemoteObject>, std::shared_ptr<InputClientInfo>> mapClients_;
    std::mutex clientLock_;
    sptr<IInputClient> currentClient_; // the current input client
    std::mutex inactiveClientLock_;
    sptr<IInputClient> inactiveClient_; // the inactive input client
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_CLIENT_GROUP_H
