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
#include "global.h"
#include "i_input_client.h"
#include "i_input_control_channel.h"
#include "i_input_data_channel.h"
#include "i_input_method_agent.h"
#include "i_input_method_core.h"
#include "input_attribute.h"
#include "input_control_channel_stub.h"
#include "input_method_info.h"
#include "input_method_property.h"
#include "inputmethod_sysevent.h"
#include "iremote_object.h"
#include "message.h"
#include "message_handler.h"

namespace OHOS {
namespace MiscServices {
    class RemoteObjectDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
        void SetDeathRecipient(RemoteDiedHandler handler);
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    private:
        RemoteDiedHandler handler_;
    };

    struct ClientInfo {
        int pid;                         // the process id of the process in which the input client is running
        int uid;                         // the uid of the process in which the input client is running
        int userId;                      // the user if of the user under which the input client is running
        int displayId;                   // the display id on which the input client is showing
        sptr<IInputClient> client;       // the remote object handler for the service to callback to the input client
        sptr<IInputDataChannel> channel; // the remote object handler for IMSA callback to input client
        sptr<RemoteObjectDeathRecipient> deathRecipient;
        InputAttribute attribute; // the input attribute of the input client
        bool isShowKeyBoard{ false };
    };

    struct ResetManager {
        uint32_t num {0};
        time_t last {};
    };

    /*! \class PerUserSession
        \brief The class provides session management in input method management service

        This class manages the sessions between input clients and input method engines for each unlocked user.
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

        int32_t OnPrepareInput(const ClientInfo &clientInfo);
        int32_t OnStartInput(sptr<IInputClient> client, bool isShowKeyboard);
        int32_t OnStopInput(sptr<IInputClient> client);
        int32_t OnReleaseInput(sptr<IInputClient> client);
        int32_t OnSetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent);
        int OnHideKeyboardSelf(int flags);
        int OnShowKeyboardSelf();

        void CreateWorkThread(MessageHandler& handler);
        void JoinWorkThread();
        void StopInputService(std::string imeId);
        void OnInputMethodSwitched(const Property &property, const SubProperty &subProperty);

        void SetCurrentSubProperty(const SubProperty &subProperty);
        SubProperty GetCurrentSubProperty();
        void UpdateCurrentUserId(int32_t userId);

    private:
        int userId_; // the id of the user to whom the object is linking
        int userState = UserState::USER_STATE_STARTED; // the state of the user to whom the object is linking
        int displayId; // the id of the display screen on which the user is
        std::map<sptr<IRemoteObject>, std::shared_ptr<ClientInfo>> mapClients;
        static const int MAX_RESTART_NUM = 3;
        static const int IME_RESET_TIME_OUT = 300;
        static const int MAX_RESET_WAIT_TIME = 1600000;

        sptr<IInputControlChannel> inputControlChannel[MAX_IME];
        std::mutex imsCoreLock_;
        sptr<IInputMethodCore> imsCore[MAX_IME]; // the remote handlers of input method service

        sptr<IInputMethodAgent> imsAgent;
        std::mutex clientLock_;
        sptr<IInputClient> currentClient; // the current input client

        sptr<RemoteObjectDeathRecipient> imsDeathRecipient = nullptr;
        MessageHandler *msgHandler = nullptr; // message handler working with Work Thread
        std::thread workThreadHandler; // work thread handler
        std::recursive_mutex mtx; // mutex to lock the operations among multi work threads
        std::mutex resetLock;
        ResetManager manager[MAX_IME];

        PerUserSession(const PerUserSession&);
        PerUserSession& operator =(const PerUserSession&);
        PerUserSession(const PerUserSession&&);
        PerUserSession& operator =(const PerUserSession&&);
        std::shared_ptr<ClientInfo> GetClientInfo(sptr<IRemoteObject> inputClient);
        void WorkThread();

        void OnClientDied(sptr<IInputClient> remote);
        void OnImsDied(sptr<IInputMethodCore> remote);

        int AddClient(sptr<IRemoteObject> inputClient, const ClientInfo &clientInfo);
        void UpdateClient(sptr<IRemoteObject> inputClient, bool isShowKeyboard);
        void RemoveClient(sptr<IRemoteObject> inputClient);
        int ShowKeyboard(const sptr<IInputClient>& inputClient, bool isShowKeyboard);
        int HideKeyboard(const sptr<IInputClient>& inputClient);
        int GetImeIndex(const sptr<IInputClient>& inputClient);
        void SendAgentToSingleClient(const ClientInfo &clientInfo);
        void InitInputControlChannel();
        void SendAgentToAllClients();
        bool IsRestartIme(uint32_t index);
        void ClearImeData(uint32_t index);
        void SetCurrentClient(sptr<IInputClient> client);
        sptr<IInputClient> GetCurrentClient();
        void SetImsCore(int32_t index, sptr<IInputMethodCore> core);
        sptr<IInputMethodCore> GetImsCore(int32_t index);
        static inline bool IsValid(int32_t index)
        {
            return index >= CURRENT_IME && index <= SECURITY_IME;
        }

        std::mutex propertyLock_;
        SubProperty currentSubProperty;
    };
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_PERUSER_SESSION_H