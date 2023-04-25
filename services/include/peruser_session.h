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

#include "ability_connect_callback_proxy.h"
#include "ability_manager_interface.h"
#include "block_data.h"
#include "event_handler.h"
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
#include "inputmethod_sysevent.h"
#include "iremote_object.h"
#include "message.h"
#include "message_handler.h"

namespace OHOS {
namespace MiscServices {
struct ResetManager {
    uint32_t num{ 0 };
    time_t last{};
};

/**@class PerUserSession
 *
 * @brief The class provides session management in input method management service
 *
 * This class manages the sessions between input clients and input method engines for each unlocked user.
 */
class PerUserSession {
    enum : int32_t {
        CURRENT_IME = 0, // index for current ime
        SECURITY_IME,    // index for security ime
        MAX_IME          // the maximum count of ime started for a user
    };

public:
    explicit PerUserSession(int userId);
    ~PerUserSession();

    int32_t OnPrepareInput(const InputClientInfo &clientInfo);
    int32_t OnStartInput(sptr<IInputClient> client, bool isShowKeyboard);
    int32_t OnStopInput(sptr<IInputClient> client);
    int32_t OnReleaseInput(const sptr<IInputClient> &client);
    int32_t OnSetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent);
    int OnHideKeyboardSelf();
    int OnShowKeyboardSelf();
    void StopInputService(std::string imeId);
    int32_t OnSwitchIme(const Property &property, const SubProperty &subProperty, bool isSubtypeSwitch);
    void UpdateCurrentUserId(int32_t userId);
    void OnUnfocused(int32_t pid, int32_t uid);

    BlockData<bool> isImeRemoved_{ MAX_PACKAGE_REMOVE_WAIT_TIME, false };

private:
    int userId_;                                   // the id of the user to whom the object is linking
    std::map<sptr<IRemoteObject>, std::shared_ptr<InputClientInfo>> mapClients_;
    static const int MAX_RESTART_NUM = 3;
    static const int IME_RESET_TIME_OUT = 3;
    static const int MAX_PACKAGE_REMOVE_WAIT_TIME = 100;
    static const int MAX_IME_START_TIME = 350;

    std::mutex imsCoreLock_;
    sptr<IInputMethodCore> imsCore[MAX_IME];       // the remote handlers of input method service

    std::mutex agentLock_;
    sptr<IInputMethodAgent> agent_;
    std::mutex clientLock_;
    sptr<IInputClient> currentClient_;              // the current input client

    sptr<InputDeathRecipient> imsDeathRecipient_ = nullptr;
    std::recursive_mutex mtx;             // mutex to lock the operations among multi work threads
    std::mutex resetLock;
    ResetManager manager[MAX_IME];

    PerUserSession(const PerUserSession &);
    PerUserSession &operator=(const PerUserSession &);
    PerUserSession(const PerUserSession &&);
    PerUserSession &operator=(const PerUserSession &&);
    std::shared_ptr<InputClientInfo> GetClientInfo(sptr<IRemoteObject> inputClient);

    void OnClientDied(sptr<IInputClient> remote);
    void OnImsDied(sptr<IInputMethodCore> remote);

    int AddClient(sptr<IRemoteObject> inputClient, const InputClientInfo &clientInfo);
    void UpdateClient(sptr<IRemoteObject> inputClient, bool isShowKeyboard);
    int32_t RemoveClient(const sptr<IRemoteObject> &client, bool isClientDied);
    int32_t ShowKeyboard(
        const sptr<IInputDataChannel> &channel, const sptr<IInputClient> &inputClient, bool isShowKeyboard);
    int32_t HideKeyboard(const sptr<IInputClient> &inputClient);
    int32_t ClearDataChannel(const sptr<IInputDataChannel> &channel);
    int GetImeIndex(const sptr<IInputClient> &inputClient);
    int32_t SendAgentToSingleClient(const InputClientInfo &clientInfo);
    void InitInputControlChannel();
    void SendAgentToAllClients();
    bool IsRestartIme(uint32_t index);
    void ClearImeData(uint32_t index);
    void SetCurrentClient(sptr<IInputClient> client);
    sptr<IInputClient> GetCurrentClient();
    void SetImsCore(int32_t index, sptr<IInputMethodCore> core);
    sptr<IInputMethodCore> GetImsCore(int32_t index);
    void SetAgent(sptr<IInputMethodAgent> agent);
    sptr<IInputMethodAgent> GetAgent();
    sptr<AAFwk::IAbilityManager> GetAbilityManagerService();
    bool StartCurrentIme(bool isRetry);

    static inline bool IsValid(int32_t index)
    {
        return index >= CURRENT_IME && index <= SECURITY_IME;
    }

    std::mutex propertyLock_;
    SubProperty currentSubProperty;
    BlockData<bool> isImeStarted_{ MAX_IME_START_TIME, false };
    std::shared_ptr<AppExecFwk::EventHandler> imeRestartHandler_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_PERUSER_SESSION_H
