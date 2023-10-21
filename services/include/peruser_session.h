/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef SERVICES_INCLUDE_PERUSER_SESSION_H
#define SERVICES_INCLUDE_PERUSER_SESSION_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <variant>

#include "block_data.h"
#include "event_handler.h"
#include "event_status_manager.h"
#include "global.h"
#include "i_input_client.h"
#include "i_input_control_channel.h"
#include "i_input_data_channel.h"
#include "i_input_method_agent.h"
#include "i_input_method_core.h"
#include "input_attribute.h"
#include "input_client_info.h"
#include "input_control_channel_stub.h"
#include "input_death_recipient.h"
#include "input_method_info.h"
#include "input_method_property.h"
#include "input_window_info.h"
#include "inputmethod_sysevent.h"
#include "iremote_object.h"
#include "message.h"
#include "message_handler.h"
#include "unRegistered_type.h"
namespace OHOS {
namespace MiscServices {
struct ImeData {
    sptr<IInputMethodCore> core{ nullptr };
    sptr<IInputMethodAgent> agent{ nullptr };
    sptr<InputDeathRecipient> deathRecipient{ nullptr };
    ImeData(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent, sptr<InputDeathRecipient> deathRecipient)
        : core(std::move(core)), agent(std::move(agent)), deathRecipient(std::move(deathRecipient))
    {
    }
};
/**@class PerUserSession
 *
 * @brief The class provides session management in input method management service
 *
 * This class manages the sessions between input clients and input method engines for each unlocked user.
 */
class PerUserSession {
public:
    explicit PerUserSession(int userId);
    ~PerUserSession();

    int32_t OnPrepareInput(const InputClientInfo &clientInfo);
    int32_t OnStartInput(const sptr<IInputClient> &client, bool isShowKeyboard, sptr<IRemoteObject> &agent);
    int32_t OnReleaseInput(const sptr<IInputClient> &client);
    int32_t OnSetCoreAndAgent(const sptr<IInputMethodCore> &core, const sptr<IInputMethodAgent> &agent);
    int32_t OnHideCurrentInput();
    int32_t OnShowCurrentInput();
    int32_t OnShowInput(sptr<IInputClient> client);
    int32_t OnHideInput(sptr<IInputClient> client);
    void StopInputService();
    void NotifyImeChangeToClients(const Property &property, const SubProperty &subProperty);
    int32_t SwitchSubtype(const SubProperty &subProperty);
    void UpdateCurrentUserId(int32_t userId);
    void OnFocused(int32_t pid, int32_t uid);
    void OnUnfocused(int32_t pid, int32_t uid);
    int64_t GetCurrentClientPid();
    int32_t OnPanelStatusChange(const InputWindowStatus &status, const InputWindowInfo &windowInfo);
    int32_t OnUpdateListenEventFlag(const InputClientInfo &clientInfo);
    bool StartInputService(const std::string &imeName, bool isRetry);
    int32_t OnRegisterProxyIme(const sptr<IInputMethodCore> &core, const sptr<IInputMethodAgent> &agent);
    int32_t OnUnRegisteredProxyIme(UnRegisteredType type, const sptr<IInputMethodCore> &core);
    bool IsProxyImeEnable();
    bool IsBoundToClient();
    int32_t ExitCurrentInputType();

private:
    struct ResetManager {
        uint32_t num{ 0 };
        time_t last{};
    };
    enum ClientAddEvent : int32_t {
        PREPARE_INPUT = 0,
        START_LISTENING,
    };
    int32_t userId_; // the id of the user to whom the object is linking
    std::recursive_mutex mtx;
    std::map<sptr<IRemoteObject>, std::shared_ptr<InputClientInfo>> mapClients_;
    static const int MAX_RESTART_NUM = 3;
    static const int IME_RESET_TIME_OUT = 3;
    static const int MAX_IME_START_TIME = 1000;
    std::mutex clientLock_;
    sptr<IInputClient> currentClient_; // the current input client
    std::mutex resetLock;
    ResetManager manager;

    PerUserSession(const PerUserSession &);
    PerUserSession &operator=(const PerUserSession &);
    PerUserSession(const PerUserSession &&);
    PerUserSession &operator=(const PerUserSession &&);

    void OnClientDied(sptr<IInputClient> remote);
    void OnImeDied(const sptr<IInputMethodCore> &remote, ImeType type);

    int AddClientInfo(sptr<IRemoteObject> inputClient, const InputClientInfo &clientInfo, ClientAddEvent event);
    std::shared_ptr<InputClientInfo> GetClientInfo(sptr<IRemoteObject> inputClient);
    void UpdateClientInfo(const sptr<IRemoteObject> &client,
        const std::unordered_map<UpdateFlag, std::variant<bool, uint32_t, ImeType>> &updateInfos);
    void RemoveClientInfo(const sptr<IRemoteObject> &client, bool isClientDied = false);
    int32_t AddImeData(ImeType type, sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent);
    std::shared_ptr<ImeData> GetImeData(ImeType type);
    void RemoveImeData(ImeType type);

    int32_t RemoveClient(const sptr<IInputClient> &client, bool isUnbindFromClient = false);
    int32_t RemoveIme(const sptr<IInputMethodCore> &core, ImeType type);

    int32_t BindClientWithIme(
        const std::shared_ptr<InputClientInfo> &clientInfo, ImeType type, bool isBindFromClient = false);
    void UnBindClientWithIme(
        const std::shared_ptr<InputClientInfo> &currentClientInfo, bool isUnbindFromClient = false);
    void StopClientInput(const sptr<IInputClient> &currentClient);
    void StopImeInput(ImeType currentType, const sptr<IInputDataChannel> &currentChannel);

    int32_t HideKeyboard(const sptr<IInputClient> &currentClient);
    int32_t ShowKeyboard(const sptr<IInputClient> &currentClient);

    int32_t InitInputControlChannel();
    bool IsReadyToStartIme();
    bool IsRestartIme();
    void RestartIme();

    void SetCurrentClient(sptr<IInputClient> client);
    sptr<IInputClient> GetCurrentClient();
    bool IsCurrentClient(int32_t pid, int32_t uid);
    bool IsCurrentClient(sptr<IInputClient> client);

    bool IsImeStartInBind(ImeType bindImeType, ImeType startImeType);
    bool IsProxyImeStartInBind(ImeType bindImeType, ImeType startImeType);
    bool IsProxyImeStartInImeBind(ImeType bindImeType, ImeType startImeType);
    bool IsBindProxyImeInImeBind(ImeType bindImeType);
    bool IsBindImeInProxyImeBind(ImeType bindImeType);

    BlockData<bool> isImeStarted_{ MAX_IME_START_TIME, false };
    std::mutex imeDataLock_;
    std::unordered_map<ImeType, std::shared_ptr<ImeData>> imeData_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_PERUSER_SESSION_H
