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

#include <unordered_set>

#include "block_queue.h"
#include "client_group.h"
#include "event_status_manager.h"
#include "iinput_method_core.h"
#include "ime_cfg_manager.h"
#include "ime_connection.h"
#include "input_method_types.h"
#include "input_type_manager.h"
#include "inputmethod_message_handler.h"
#include "inputmethod_sysevent.h"
#include "want.h"
#include "ime_state_manager.h"
#include "input_method_client_types.h"

namespace OHOS {
namespace Rosen {
struct CallingWindowInfo;
}
}
namespace OHOS {
namespace MiscServices {
const std::string IME_MIRROR_NAME = "proxyIme_IME_MIRROR";
enum class ImeStatus : uint32_t { STARTING, READY, EXITING };
enum class ImeEvent : uint32_t {
    START_IME,
    START_IME_TIMEOUT,
    STOP_IME,
    SET_CORE_AND_AGENT,
};
enum LargeMemoryState : int32_t {
    LARGE_MEMORY_NEED = 2,
    LARGE_MEMORY_NOT_NEED = 3
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
    std::shared_ptr<ImeStateManager> imeStateManager;
    ImeStatus imeStatus{ ImeStatus::STARTING };
    std::pair<std::string, std::string> ime; // first: bundleName  second:extName
    int64_t startTime{ 0 };
    ImeData(sptr<IInputMethodCore> core, sptr<IRemoteObject> agent, sptr<InputDeathRecipient> deathRecipient,
        pid_t imePid)
        : core(std::move(core)), agent(std::move(agent)), deathRecipient(std::move(deathRecipient)), pid(imePid)
    {
    }

    bool IsImeMirror() const
    {
        return ime.first == IME_MIRROR_NAME;
    }
    ImeExtendInfo imeExtendInfo;
};

enum class StartPreDefaultImeStatus : uint32_t { NO_NEED, HAS_STARTED, TO_START };
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
    int32_t OnStartInput(const InputClientInfo &inputClientInfo, std::vector<sptr<IRemoteObject>> &agents,
        std::vector<BindImeInfo> &imeInfos);
    int32_t OnReleaseInput(const sptr<IInputClient> &client, uint32_t sessionId);
    int32_t OnSetCoreAndAgent(const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent);
    int32_t OnHideCurrentInput(uint64_t displayId);
    int32_t OnShowCurrentInput(uint64_t displayId);
    int32_t OnShowInput(sptr<IInputClient> client, int32_t requestKeyboardReason = 0);
    int32_t OnHideInput(sptr<IInputClient> client);
    int32_t OnRequestShowInput(uint64_t displayId);
    int32_t OnRequestHideInput(int32_t callingPid, uint64_t displayId);
    void OnSecurityChange(int32_t security);
    void OnHideSoftKeyBoardSelf();
    void NotifyImeChangeToClients(const Property &property, const SubProperty &subProperty);
    int32_t SwitchSubtype(const SubProperty &subProperty);
    int32_t SwitchSubtypeWithoutStartIme(const SubProperty &subProperty);
    void OnFocused(uint64_t displayId, int32_t pid, int32_t uid);
    void OnUnfocused(uint64_t displayId, int32_t pid, int32_t uid);
    void OnScreenUnlock();
    void OnScreenLock();
    int64_t GetCurrentClientPid(uint64_t displayId);
    int64_t GetInactiveClientPid(uint64_t displayId);
    int32_t OnPanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info, uint64_t displayId);
    int32_t OnUpdateListenEventFlag(const InputClientInfo &clientInfo);
    int32_t OnRegisterProxyIme(const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent, int32_t pid);
    int32_t OnUnRegisteredProxyIme(UnRegisteredType type, const sptr<IInputMethodCore> &core, pid_t pid);
    int32_t OnBindImeMirror(const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent);
    int32_t OnUnbindImeMirror();
    int32_t UpdateLargeMemorySceneState(const int32_t memoryState);
    int32_t OnRegisterProxyIme(
        uint64_t displayId, const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent);
    int32_t OnUnregisterProxyIme(uint64_t displayId);
    int32_t InitConnect(pid_t pid);

    int32_t StartCurrentIme(bool isStopCurrentIme = false);
    int32_t StartIme(const std::shared_ptr<ImeNativeCfg> &ime, bool isStopCurrentIme = false);
    int32_t StopCurrentIme();
    bool RestartIme();
    void AddRestartIme();

    bool IsProxyImeEnable();
    bool IsBoundToClient(uint64_t displayId);
    bool IsCurrentImeByPid(int32_t pid);
    int32_t RestoreCurrentImeSubType(uint64_t callingDisplayId);
    int32_t IsPanelShown(const PanelInfo &panelInfo, bool &isShown);
    bool CheckSecurityMode();
    int32_t OnConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent);
    int32_t RemoveAllCurrentClient();
    std::shared_ptr<ImeData> GetReadyImeData(ImeType type);
    std::shared_ptr<ImeData> GetImeData(ImeType type);
    std::shared_ptr<ImeData> GetImeData(pid_t pid);
    BlockQueue<SwitchInfo>& GetSwitchQueue();
    bool IsWmsReady();
    bool CheckPwdInputPatternConv(InputClientInfo &clientInfo, uint64_t displayId);
    int32_t StartUserSpecifiedIme(uint64_t callingDisplayId);
    int32_t SetInputType();
    std::shared_ptr<ImeNativeCfg> GetImeNativeCfg(int32_t userId, const std::string &bundleName,
        const std::string &subName);
    int32_t OnSetCallingWindow(uint32_t callingWindowId, uint64_t callingDisplayId, sptr<IInputClient> client);
    int32_t GetInputStartInfo(
        uint64_t displayId, bool &isInputStart, uint32_t &callingWndId, int32_t &requestKeyboardReason);
    bool IsSaReady(int32_t saId);
    void TryUnloadSystemAbility();
    void OnCallingDisplayIdChanged(const int32_t windowId, const int32_t callingPid, const uint64_t displayId);
    ImfCallingWindowInfo GetCallingWindowInfo(const InputClientInfo &clientInfo);
    bool SpecialScenarioCheck();
    bool IsScreenLockOrSecurityFlag();
    int32_t SpecialSendPrivateData(const std::unordered_map<std::string, PrivateDataValue> &privateCommand);
    uint64_t GetDisplayGroupId(uint64_t displayId);
    bool IsDefaultDisplayGroup(uint64_t displayId);
    bool IsNumkeyAutoInputApp(const std::string &bundleName);
    std::pair<int32_t, int32_t> GetCurrentInputPattern();
    bool IsPreconfiguredDefaultImeSpecified(const InputClientInfo &inputClientInfo);
    bool IsImeSwitchForbidden();
    std::pair<int32_t, StartPreDefaultImeStatus> StartPreconfiguredDefaultIme(
        uint64_t callingDisplayId, const ImeExtendInfo &imeExtendInfo = {}, bool isStopCurrentIme = false);
    void NotifyOnInputStopFinished();
    void IncreaseAttachCount();
    void DecreaseAttachCount();
    uint32_t GetAttachCount();
    void IncreaseScbStartCount();
    int32_t TryStartIme();
    int32_t TryDisconnectIme();

private:
    struct ResetManager {
        uint32_t num{ 0 };
        time_t last{};
    };
    enum TimeLimitType : uint32_t {
        IME_LIMIT,
        PROXY_IME_LIMIT,
    };
    using CoreMethod = std::function<int32_t(const sptr<IInputMethodCore> &)>;

    int32_t userId_; // the id of the user to whom the object is linking
#ifdef IMF_ON_DEMAND_START_STOP_SA_ENABLE
    static const int MAX_IME_START_TIME = 2000;
#else
    static const int MAX_IME_START_TIME = 1500;
#endif
    static const int MAX_NOTIFY_TIME = 20;
    std::mutex resetLock;
    std::map<TimeLimitType, ResetManager> managers_;
    using IpcExec = std::function<int32_t()>;

    PerUserSession(const PerUserSession &);
    PerUserSession &operator=(const PerUserSession &);
    PerUserSession(const PerUserSession &&);
    PerUserSession &operator=(const PerUserSession &&);

    static constexpr int32_t MAX_WAIT_TIME = 5000;
    BlockQueue<SwitchInfo> switchQueue_{ MAX_WAIT_TIME };

    void OnClientDied(sptr<IInputClient> remote);
    void OnImeDied(const sptr<IInputMethodCore> &remote, ImeType type, pid_t pid);

    int AddClientInfo(sptr<IRemoteObject> inputClient, const InputClientInfo &clientInfo, ClientAddEvent event);
    int32_t RemoveClient(const sptr<IInputClient> &client, const std::shared_ptr<ClientGroup> &clientGroup,
        const DetachOptions &options);
    void DeactivateClient(const sptr<IInputClient> &client);
    std::shared_ptr<InputClientInfo> GetCurrentClientInfo(uint64_t displayId = DEFAULT_DISPLAY_ID);
    std::shared_ptr<ClientGroup> GetClientGroup(uint64_t displayGroupId);
    std::shared_ptr<ClientGroup> GetClientGroup(sptr<IRemoteObject> client);
    std::shared_ptr<ClientGroup> GetClientGroup(ImeType type);
    ImeType GetImeType(uint64_t displayId);

    int32_t InitImeData(const std::pair<std::string, std::string> &ime,
        const std::shared_ptr<ImeNativeCfg> &imeNativeCfg = nullptr);
    int32_t UpdateImeData(sptr<IInputMethodCore> core, sptr<IRemoteObject> agent, pid_t pid);
    int32_t AddImeData(ImeType type, sptr<IInputMethodCore> core, sptr<IRemoteObject> agent, pid_t pid);
    void RemoveImeData(ImeType type);
    void RemoveImeData(pid_t pid);
    int32_t RemoveIme(ImeType type, pid_t pid);

    int32_t BindClientWithIme(const std::shared_ptr<InputClientInfo> &clientInfo, ImeType type,
        bool isBindFromClient = false, uint64_t displayId = DEFAULT_DISPLAY_ID);
    void UnBindClientWithIme(const std::shared_ptr<InputClientInfo> &currentClientInfo, const DetachOptions &options);
    void StopClientInput(
        const std::shared_ptr<InputClientInfo> &clientInfo, bool isStopInactiveClient = false, bool isAsync = false);
    void StopImeInput(ImeType currentType, const sptr<IRemoteObject> &currentChannel, uint32_t sessionId);

    int32_t HideKeyboard(const sptr<IInputClient> &currentClient, const std::shared_ptr<ClientGroup> &clientGroup);
    int32_t ShowKeyboard(const sptr<IInputClient> &currentClient, const std::shared_ptr<ClientGroup> &clientGroup,
        int32_t requestKeyboardReason = 0);

    int32_t InitInputControlChannel();
    void StartImeInImeDied();
    void StartImeIfInstalled();
    void ReplaceCurrentClient(const sptr<IInputClient> &client, const std::shared_ptr<ClientGroup> &clientGroup);
    bool IsSameClient(sptr<IInputClient> source, sptr<IInputClient> dest);

    bool IsImeStartInBind(ImeType bindImeType, ImeType startImeType);
    bool IsProxyImeStartInBind(ImeType bindImeType, ImeType startImeType);
    bool IsProxyImeStartInImeBind(ImeType bindImeType, ImeType startImeType);
    bool IsImeBindTypeChanged(ImeType bindImeType);
    int32_t RequestIme(const std::shared_ptr<ImeData> &data, RequestType type, const IpcExec &exec);
    int32_t RequestAllIme(const std::shared_ptr<ImeData> data, RequestType reqType, const CoreMethod &method);
    std::vector<std::shared_ptr<ImeData>> GetAllReadyImeData(ImeType type);

    bool WaitForCurrentImeStop();
    void NotifyImeStopFinished();
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
    void HandleBindImeChanged(InputClientInfo &newClientInfo, const std::shared_ptr<ClientGroup> &clientGroup);
    int32_t NotifyCallingDisplayChanged(uint64_t displayId);
    bool GetCallingWindowInfo(const InputClientInfo &clientInfo, Rosen::CallingWindowInfo &callingWindowInfo);
    int32_t SendPrivateData(const std::unordered_map<std::string, PrivateDataValue> &privateCommand);
    void ClearRequestKeyboardReason(std::shared_ptr<InputClientInfo> &clientInfo);
    std::pair<std::string, std::string> GetImeUsedBeforeScreenLocked();
    void SetImeUsedBeforeScreenLocked(const std::pair<std::string, std::string> &ime);
    std::shared_ptr<ImeNativeCfg> GetRealCurrentIme(bool needMinGuarantee);
    int32_t NotifyImeChangedToClients();
    int32_t NotifySubTypeChangedToIme(const std::string &bundleName, const std::string &subName);
    bool CompareExchange(const int32_t value);
    bool IsLargeMemoryStateNeed();
    bool IsAttachFinished();
    uint32_t GetScbStartCount();
    void ResetRestartTasks();
    int32_t SendAllReadyImeToClient(
        std::shared_ptr<ImeData> data, const std::shared_ptr<InputClientInfo> &clientInfo);
    void SetImeConnection(const sptr<AAFwk::IAbilityConnection> &connection);
    sptr<AAFwk::IAbilityConnection> GetImeConnection();
    void ClearImeConnection(const sptr<AAFwk::IAbilityConnection> &connection);
    int32_t IsRequestOverLimit(TimeLimitType timeLimit, int32_t resetTimeOut, int32_t restartNum);
    int32_t PrepareImeInfos(ImeType type, std::vector<sptr<IRemoteObject>> &agents,
        std::vector<BindImeInfo> &imeInfos);

    std::mutex imeStartLock_;

    BlockData<bool> isImeStarted_{ MAX_IME_START_TIME, false };
    std::mutex imeDataLock_;
    std::unordered_map<ImeType, std::vector<std::shared_ptr<ImeData>>> imeData_;
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
    std::mutex imeUsedLock_;
    std::pair<std::string, std::string> imeUsedBeforeScreenLocked_;
    std::mutex virtualDisplayLock_{};
    std::unordered_set<uint64_t> virtualScreenDisplayId_;
    std::atomic<uint64_t> agentDisplayId_{ DEFAULT_DISPLAY_ID };
    std::mutex largeMemoryStateMutex_{};
    int32_t largeMemoryState_ = LargeMemoryState::LARGE_MEMORY_NOT_NEED;
    std::mutex clientGroupLock_{};
    std::unordered_map<uint64_t, std::shared_ptr<ClientGroup>> clientGroupMap_;
    std::mutex isNotifyFinishedLock_{};
    BlockData<bool> isNotifyFinished_{ MAX_NOTIFY_TIME, false };
    std::mutex attachCountMtx_{};
    uint32_t attachingCount_ { 0 };
    std::mutex scbStartCountMtx_{};
    uint32_t scbStartCount_ { 0 };
    std::mutex connectionLock_{};
    sptr<AAFwk::IAbilityConnection> connection_ = nullptr;
    std::atomic<bool> isBlockStartedByLowMem_ = false;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_PERUSER_SESSION_H