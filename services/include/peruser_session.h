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

#include "block_queue.h"
#include "event_status_manager.h"
#include "freeze_manager.h"
#include "i_input_method_core.h"
#include "ime_cfg_manager.h"
#include "input_type_manager.h"
#include "inputmethod_sysevent.h"
#include "inputmethod_message_handler.h"
#include "input_method_types.h"
#include "want.h"

namespace OHOS {
namespace MiscServices {
enum class ImeStatus : uint32_t { STARTING, READY, EXITING };
enum class ImeEvent : uint32_t {
    START_IME,
    START_IME_TIMEOUT,
    STOP_IME,
    SET_CORE_AND_AGENT,
};
enum class ImeAction : uint32_t {
    DO_NOTHING,
    HANDLE_STARTING_IME,
    FORCE_STOP_IME,
    STOP_READY_IME,
    START_AFTER_FORCE_STOP,
    DO_SET_CORE_AND_AGENT,
    DO_ACTION_IN_NULL_IME_DATA,
    DO_ACTION_IN_IME_EVENT_CONVERT_FAILED,
};
struct ImeData {
    static constexpr int64_t START_TIME_OUT = 8000;
    sptr<IInputMethodCore> core{ nullptr };
    sptr<IRemoteObject> agent{ nullptr };
    sptr<InputDeathRecipient> deathRecipient{ nullptr };
    pid_t pid;
    std::shared_ptr<FreezeManager> freezeMgr;
    ImeStatus imeStatus{ ImeStatus::STARTING };
    std::pair<std::string, std::string> ime; // first: bundleName  second:extName
    int64_t startTime{ 0 };
    ImeData(sptr<IInputMethodCore> core, sptr<IRemoteObject> agent, sptr<InputDeathRecipient> deathRecipient,
        pid_t imePid)
        : core(std::move(core)), agent(std::move(agent)), deathRecipient(std::move(deathRecipient)), pid(imePid),
          freezeMgr(std::make_shared<FreezeManager>(imePid))
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
    PerUserSession(int32_t userId, const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler);
    ~PerUserSession();

    int32_t OnPrepareInput(const InputClientInfo &clientInfo);
    int32_t OnStartInput(
        const InputClientInfo &inputClientInfo, sptr<IRemoteObject> &agent, std::pair<int64_t, std::string> &imeInfo);
    int32_t OnReleaseInput(const sptr<IInputClient> &client);
    int32_t OnSetCoreAndAgent(const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent);
    int32_t OnHideCurrentInput();
    int32_t OnShowCurrentInput();
    int32_t OnShowInput(sptr<IInputClient> client, int32_t requestKeyboardReason = 0);
    int32_t OnHideInput(sptr<IInputClient> client);
    int32_t OnRequestShowInput();
    int32_t OnRequestHideInput(int32_t callingPid);
    void OnSecurityChange(int32_t security);
    void OnHideSoftKeyBoardSelf();
    void NotifyImeChangeToClients(const Property &property, const SubProperty &subProperty);
    int32_t SwitchSubtype(const SubProperty &subProperty);
    int32_t SwitchSubtypeWithoutStartIme(const SubProperty &subProperty);
    void OnFocused(int32_t pid, int32_t uid);
    void OnUnfocused(int32_t pid, int32_t uid);
    void OnScreenUnlock();
    int64_t GetCurrentClientPid();
    int64_t GetInactiveClientPid();
    int32_t OnPanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info);
    int32_t OnUpdateListenEventFlag(const InputClientInfo &clientInfo);
    int32_t OnRegisterProxyIme(const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent);
    int32_t OnUnRegisteredProxyIme(UnRegisteredType type, const sptr<IInputMethodCore> &core);
    int32_t InitConnect(pid_t pid);

    int32_t StartCurrentIme(bool isStopCurrentIme = false);
    int32_t StartIme(const std::shared_ptr<ImeNativeCfg> &ime, bool isStopCurrentIme = false);
    int32_t StopCurrentIme();
    bool RestartIme();
    void AddRestartIme();

    bool IsProxyImeEnable();
    bool IsBoundToClient();
    bool IsCurrentImeByPid(int32_t pid);
    int32_t RestoreCurrentImeSubType();
    int32_t IsPanelShown(const PanelInfo &panelInfo, bool &isShown);
    bool CheckSecurityMode();
    int32_t OnConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent);
    int32_t RemoveCurrentClient();
    std::shared_ptr<ImeData> GetReadyImeData(ImeType type);
    std::shared_ptr<ImeData> GetImeData(ImeType type);
    BlockQueue<SwitchInfo>& GetSwitchQueue();
    bool IsWmsReady();
    bool CheckPwdInputPatternConv(InputClientInfo &clientInfo);
    int32_t RestoreCurrentIme();
    int32_t SetInputType();
    std::shared_ptr<ImeNativeCfg> GetImeNativeCfg(int32_t userId, const std::string &bundleName,
        const std::string &subName);
    int32_t OnSetCallingWindow(uint32_t callingWindowId, sptr<IInputClient> client);
    int32_t GetInputStartInfo(bool& isInputStart, uint32_t& callingWndId, int32_t& requestKeyboardReason);
    bool IsSaReady(int32_t saId);
    void UpdateUserLockState();
    void TryUnloadSystemAbility();
    void HandleCallingWindowDisplayChanged(const int32_t windowId, const int32_t callingPid, const uint64_t displayId);
protected:
   int32_t SendToIMACallingWindowDisplayChange(uint64_t displayId);
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
#ifdef IMF_ON_DEMAND_START_STOP_SA_ENABLE
    static const int MAX_IME_START_TIME = 2000;
#else
    static const int MAX_IME_START_TIME = 1500;
#endif
    std::mutex clientLock_;
    sptr<IInputClient> currentClient_; // the current input client
    std::mutex resetLock;
    ResetManager manager;
    using IpcExec = std::function<int32_t()>;

    PerUserSession(const PerUserSession &);
    PerUserSession &operator=(const PerUserSession &);
    PerUserSession(const PerUserSession &&);
    PerUserSession &operator=(const PerUserSession &&);

    static constexpr int32_t MAX_WAIT_TIME = 5000;
    BlockQueue<SwitchInfo> switchQueue_{ MAX_WAIT_TIME };

    void OnClientDied(sptr<IInputClient> remote);
    void OnImeDied(const sptr<IInputMethodCore> &remote, ImeType type);

    int AddClientInfo(sptr<IRemoteObject> inputClient, const InputClientInfo &clientInfo, ClientAddEvent event);
    void RemoveClientInfo(const sptr<IRemoteObject> &client, bool isClientDied = false);
    int32_t RemoveClient(const sptr<IInputClient> &client, bool isUnbindFromClient = false,
        bool isInactiveClient = false, bool isNotifyClientAsync = false);
    void DeactivateClient(const sptr<IInputClient> &client);
    std::shared_ptr<InputClientInfo> GetClientInfo(sptr<IRemoteObject> inputClient);
    std::shared_ptr<InputClientInfo> GetClientInfo(pid_t pid);
    std::shared_ptr<InputClientInfo> GetCurClientInfo();
    void UpdateClientInfo(const sptr<IRemoteObject> &client,
        const std::unordered_map<UpdateFlag,
            std::variant<bool, uint32_t, ImeType, ClientState, TextTotalConfig, ClientType>> &updateInfos);

    int32_t InitImeData(const std::pair<std::string, std::string> &ime);
    int32_t UpdateImeData(sptr<IInputMethodCore> core, sptr<IRemoteObject> agent, pid_t pid);
    int32_t AddImeData(ImeType type, sptr<IInputMethodCore> core, sptr<IRemoteObject> agent, pid_t pid);
    void RemoveImeData(ImeType type, bool isImeDied);
    int32_t RemoveIme(const sptr<IInputMethodCore> &core, ImeType type);
    std::shared_ptr<ImeData> GetValidIme(ImeType type);

    int32_t BindClientWithIme(const std::shared_ptr<InputClientInfo> &clientInfo, ImeType type,
        bool isBindFromClient = false);
    void UnBindClientWithIme(const std::shared_ptr<InputClientInfo> &currentClientInfo,
        bool isUnbindFromClient = false, bool isNotifyClientAsync = false);
    void StopClientInput(
        const std::shared_ptr<InputClientInfo> &clientInfo, bool isStopInactiveClient = false, bool isAsync = false);
    void StopImeInput(ImeType currentType, const sptr<IRemoteObject> &currentChannel);

    int32_t HideKeyboard(const sptr<IInputClient> &currentClient);
    int32_t ShowKeyboard(const sptr<IInputClient> &currentClient, int32_t requestKeyboardReason = 0);

    int32_t InitInputControlChannel();
    void StartImeInImeDied();
    void StartImeIfInstalled();
    void SetCurrentClient(sptr<IInputClient> client);
    sptr<IInputClient> GetCurrentClient();
    void ReplaceCurrentClient(const sptr<IInputClient> &client);
    void SetInactiveClient(sptr<IInputClient> client);
    sptr<IInputClient> GetInactiveClient();
    bool IsCurClientFocused(int32_t pid, int32_t uid);
    bool IsCurClientUnFocused(int32_t pid, int32_t uid);
    bool IsSameClient(sptr<IInputClient> source, sptr<IInputClient> dest);

    bool IsImeStartInBind(ImeType bindImeType, ImeType startImeType);
    bool IsProxyImeStartInBind(ImeType bindImeType, ImeType startImeType);
    bool IsProxyImeStartInImeBind(ImeType bindImeType, ImeType startImeType);
    bool IsImeBindTypeChanged(ImeType bindImeType);
    std::map<sptr<IRemoteObject>, std::shared_ptr<InputClientInfo>> GetClientMap();
    int32_t RequestIme(const std::shared_ptr<ImeData> &data, RequestType type, const IpcExec &exec);

    bool WaitForCurrentImeStop();
    void NotifyImeStopFinished();
    bool GetCurrentUsingImeId(ImeIdentification &imeId);
    bool CanStartIme();
    int32_t ChangeToDefaultImeIfNeed(
        const std::shared_ptr<ImeNativeCfg> &ime, std::shared_ptr<ImeNativeCfg> &imeToStart);
    AAFwk::Want GetWant(const std::shared_ptr<ImeNativeCfg> &ime);
    int32_t StartCurrentIme(const std::shared_ptr<ImeNativeCfg> &ime);
    int32_t StartNewIme(const std::shared_ptr<ImeNativeCfg> &ime);
    int32_t StartInputService(const std::shared_ptr<ImeNativeCfg> &ime);
    int32_t ForceStopCurrentIme(bool isNeedWait = true);
    int32_t StopReadyCurrentIme();
    int32_t HandleFirstStart(const std::shared_ptr<ImeNativeCfg> &ime, bool isStopCurrentIme);
    int32_t HandleStartImeTimeout(const std::shared_ptr<ImeNativeCfg> &ime);
    bool GetInputTypeToStart(std::shared_ptr<ImeNativeCfg> &imeToStart);
    // from service notify clients input start and stop
    int32_t NotifyInputStartToClients(uint32_t callingWndId, int32_t requestKeyboardReason = 0);
    int32_t NotifyInputStopToClients();
    bool IsNotifyInputStop(const sptr<IInputClient> &client);
    void HandleImeBindTypeChanged(InputClientInfo &newClientInfo);
    std::mutex imeStartLock_;

    BlockData<bool> isImeStarted_{ MAX_IME_START_TIME, false };
    std::mutex imeDataLock_;
    std::unordered_map<ImeType, std::shared_ptr<ImeData>> imeData_;
    std::mutex inactiveClientLock_;
    sptr<IInputClient> inactiveClient_; // the inactive input client
    std::mutex focusedClientLock_;

    std::atomic<bool> isSwitching_ = false;
    std::mutex imeStopMutex_;
    std::condition_variable imeStopCv_;

    std::mutex restartMutex_;
    int32_t restartTasks_ = 0;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_{ nullptr };
    ImeAction GetImeAction(ImeEvent action);
    static inline const std::map<std::pair<ImeStatus, ImeEvent>, std::pair<ImeStatus, ImeAction>> imeEventConverter_ = {
        { { ImeStatus::READY, ImeEvent::START_IME }, { ImeStatus::READY, ImeAction::DO_NOTHING } },
        { { ImeStatus::STARTING, ImeEvent::START_IME }, { ImeStatus::STARTING, ImeAction::HANDLE_STARTING_IME } },
        { { ImeStatus::EXITING, ImeEvent::START_IME }, { ImeStatus::EXITING, ImeAction::START_AFTER_FORCE_STOP } },
        { { ImeStatus::READY, ImeEvent::START_IME_TIMEOUT }, { ImeStatus::READY, ImeAction::DO_NOTHING } },
        { { ImeStatus::STARTING, ImeEvent::START_IME_TIMEOUT },
            { ImeStatus::EXITING, ImeAction::START_AFTER_FORCE_STOP } },
        { { ImeStatus::EXITING, ImeEvent::START_IME_TIMEOUT },
            { ImeStatus::EXITING, ImeAction::START_AFTER_FORCE_STOP } },
        { { ImeStatus::READY, ImeEvent::STOP_IME }, { ImeStatus::EXITING, ImeAction::STOP_READY_IME } },
        { { ImeStatus::STARTING, ImeEvent::STOP_IME }, { ImeStatus::EXITING, ImeAction::FORCE_STOP_IME } },
        { { ImeStatus::EXITING, ImeEvent::STOP_IME }, { ImeStatus::EXITING, ImeAction::FORCE_STOP_IME } },
        { { ImeStatus::READY, ImeEvent::SET_CORE_AND_AGENT }, { ImeStatus::READY, ImeAction::DO_NOTHING } },
        { { ImeStatus::STARTING, ImeEvent::SET_CORE_AND_AGENT },
            { ImeStatus::READY, ImeAction::DO_SET_CORE_AND_AGENT } },
        { { ImeStatus::EXITING, ImeEvent::SET_CORE_AND_AGENT }, { ImeStatus::EXITING, ImeAction::DO_NOTHING } }
    };
    std::string runningIme_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_PERUSER_SESSION_H