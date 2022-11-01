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
#include "input_method_setting.h"
#include "inputmethod_sysevent.h"
#include "iremote_object.h"
#include "keyboard_type.h"
#include "message.h"
#include "message_handler.h"
#include "platform.h"

namespace OHOS {
namespace MiscServices {
class RemoteObjectDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    void SetDeathRecipient(RemoteDiedHandler handler);
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

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
};

struct ResetManager {
    uint32_t num{ 0 };
    time_t last{};
};

/*! \class PerUserSession
        \brief The class provides session management in input method management service

        This class manages the sessions between input clients and input method engines for each unlocked user.
    */
class PerUserSession {
    enum : int32_t {
        DEFAULT_IME = 0, // index for default input method service
        SECURITY_IME,    // index for security input method service
        MAX_IME          // the maximum count of ims started for a user
    };

public:
    explicit PerUserSession(int userId);
    ~PerUserSession();

    void SetCurrentIme(InputMethodInfo *ime);
    void SetSecurityIme(InputMethodInfo *ime);
    void SetInputMethodSetting(InputMethodSetting *setting);
    void ResetIme(InputMethodInfo *defaultIme, InputMethodInfo *securityIme);
    void OnPackageRemoved(const std::u16string &packageName);
    int32_t OnPrepareInput(const ClientInfo &clientInfo);
    int32_t OnStartInput(sptr<IInputClient> client, bool isShowKeyboard);
    int32_t OnStopInput(sptr<IInputClient> client);
    int32_t OnReleaseInput(sptr<IInputClient> client);
    int32_t OnSetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent);
    int OnHideKeyboardSelf(int flags);
    int OnShowKeyboardSelf();
    int OnGetKeyboardWindowHeight(int &retHeight);
    KeyboardType *GetCurrentKeyboardType();

    int OnSettingChanged(const std::u16string &key, const std::u16string &value);
    void CreateWorkThread(MessageHandler &handler);
    void JoinWorkThread();
    void StopInputService(std::string imeId);
    static bool StartInputService();
    int32_t OnInputMethodSwitched(const Property &property, const SubProperty &subProperty);

    void SetCurrentSubProperty(const SubProperty &subProperty);
    SubProperty GetCurrentSubProperty();

private:
    int userId_;                                   // the id of the user to whom the object is linking
    int userState = UserState::USER_STATE_STARTED; // the state of the user to whom the object is linking
    int displayId;                                 // the id of the display screen on which the user is
    int currentIndex;
    std::map<sptr<IRemoteObject>, std::shared_ptr<ClientInfo>> mapClients;
    static const int MIN_IME = 2;
    static const int MAX_RESTART_NUM = 3;
    static const int IME_RESET_TIME_OUT = 300;
    static const int MAX_RESET_WAIT_TIME = 1600000;
    static const int SLEEP_TIME = 300000;

    InputMethodInfo *currentIme[MAX_IME] = { nullptr, nullptr }; // 0 - the default ime. 1 - security ime

    InputControlChannelStub *localControlChannel[MAX_IME];
    sptr<IInputControlChannel> inputControlChannel[MAX_IME];
    sptr<IInputMethodCore> imsCore[MAX_IME];       // the remote handlers of input method service
    sptr<IRemoteObject> inputMethodToken[MAX_IME]; // the window token of keyboard
    int currentKbdIndex[MAX_IME];                  // current keyboard index
    int lastImeIndex;                              // The last ime which showed keyboard
    InputMethodSetting *inputMethodSetting;        // The pointer referred to the object in PerUserSetting
    int currentDisplayMode;                        // the display mode of the current keyboard

    sptr<IInputMethodAgent> imsAgent;
    InputChannel *imsChannel; // the write channel created by input method service
    std::mutex clientLock_;
    sptr<IInputClient> currentClient;              // the current input client
    sptr<IInputClient> needReshowClient = nullptr; // the input client for which keyboard need to re-show

    sptr<RemoteObjectDeathRecipient> imsDeathRecipient = nullptr;
    MessageHandler *msgHandler = nullptr; // message handler working with Work Thread
    std::thread workThreadHandler;        // work thread handler
    std::recursive_mutex mtx;             // mutex to lock the operations among multi work threads
    sptr<AAFwk::AbilityConnectionProxy> connCallback;
    std::mutex resetLock;
    ResetManager manager[MAX_IME];

    PerUserSession(const PerUserSession &);
    PerUserSession &operator=(const PerUserSession &);
    PerUserSession(const PerUserSession &&);
    PerUserSession &operator=(const PerUserSession &&);
    KeyboardType *GetKeyboardType(int imeIndex, int typeIndex);
    void ResetCurrentKeyboardType(int imeIndex);
    int OnCurrentKeyboardTypeChanged(int index, const std::u16string &value);
    void CopyInputMethodService(int imeIndex);
    std::shared_ptr<ClientInfo> GetClientInfo(sptr<IRemoteObject> inputClient);
    void WorkThread();

    void OnClientDied(sptr<IInputClient> remote);
    void OnImsDied(sptr<IInputMethodCore> remote);

    void OnAdvanceToNext();
    void OnSetDisplayMode(int mode);
    void OnRestartIms(int index, const std::u16string &imeId);
    void OnUserLocked();
    int AddClient(sptr<IRemoteObject> inputClient, const ClientInfo &clientInfo);
    void RemoveClient(sptr<IRemoteObject> inputClient);
    int StartInputMethod(int index);
    int StopInputMethod(int index);
    int ShowKeyboard(const sptr<IInputClient> &inputClient, bool isShowKeyboard);
    int HideKeyboard(const sptr<IInputClient> &inputClient);
    void SetDisplayId(int displayId);
    int GetImeIndex(const sptr<IInputClient> &inputClient);
    static sptr<AAFwk::IAbilityManager> GetAbilityManagerService();
    void SendAgentToSingleClient(const ClientInfo &clientInfo);
    void InitInputControlChannel();
    void SendAgentToAllClients();
    void ResetImeError(uint32_t index);
    bool IsRestartIme(uint32_t index);
    void ClearImeData(uint32_t index);
    void SetCurrentClient(sptr<IInputClient> client);
    sptr<IInputClient> GetCurrentClient();

    std::mutex propertyLock_;
    SubProperty currentSubProperty;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_PERUSER_SESSION_H
