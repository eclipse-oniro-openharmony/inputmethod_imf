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

#ifndef SERVICES_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_H
#define SERVICES_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_H

#include "identity_checker_impl.h"
#include "ime_info_inquirer.h"
#include "input_method_system_ability_stub.h"
#include "inputmethod_dump.h"
#include "inputmethod_trace.h"
#include "security_mode_parser.h"
#include "system_ability.h"
#include "input_method_types.h"
#include "user_session_manager.h"
#include "input_type_manager.h"

namespace OHOS {
namespace MiscServices {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };
class InputMethodSystemAbility : public SystemAbility, public InputMethodSystemAbilityStub {
    DECLARE_SYSTEM_ABILITY(InputMethodSystemAbility);

public:
    DISALLOW_COPY_AND_MOVE(InputMethodSystemAbility);
    InputMethodSystemAbility(int32_t systemAbilityId, bool runOnCreate);
    InputMethodSystemAbility();
    ~InputMethodSystemAbility();
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    ErrCode StartInput(const InputClientInfoInner &inputClientInfoInner, sptr<IRemoteObject> &agent,
        int64_t &pid, std::string &bundleName) override;
    ErrCode ShowCurrentInput(uint32_t type = static_cast<uint32_t>(ClientType::INNER_KIT)) override;
    ErrCode HideCurrentInput() override;
    ErrCode ShowInput(const sptr<IInputClient>& client, uint32_t type = static_cast<uint32_t>(ClientType::INNER_KIT),
        int32_t requestKeyboardReason = 0) override;
    ErrCode HideInput(const sptr<IInputClient>& client) override;
    ErrCode StopInputSession() override;
    ErrCode ReleaseInput(const sptr<IInputClient>& client, uint32_t sessionId) override;
    ErrCode RequestShowInput() override;
    ErrCode RequestHideInput(bool isFocusTriggered) override;
    ErrCode GetDefaultInputMethod(Property &prop, bool isBrief) override;
    ErrCode GetInputMethodConfig(ElementName &inputMethodConfig) override;
    ErrCode GetCurrentInputMethod(Property& resultValue) override;
    ErrCode GetCurrentInputMethodSubtype(SubProperty& resultValue) override;
    ErrCode ListInputMethod(uint32_t status, std::vector<Property> &props) override;
    ErrCode ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProps) override;
    ErrCode ListInputMethodSubtype(const std::string &bundleName, std::vector<SubProperty> &subProps) override;
    ErrCode SwitchInputMethod(
        const std::string &bundleName, const std::string &subName, uint32_t trigger) override;
    ErrCode DisplayOptionalInputMethod() override;
    ErrCode SetCoreAndAgent(const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent) override;
    ErrCode InitConnect() override;
    ErrCode UnRegisteredProxyIme(int32_t type, const sptr<IInputMethodCore> &core) override;
    ErrCode PanelStatusChange(uint32_t status, const ImeWindowInfo &info) override;
    ErrCode UpdateListenEventFlag(const InputClientInfoInner &clientInfoInner, uint32_t eventFlag) override;
    ErrCode SetCallingWindow(uint32_t windowId, const sptr<IInputClient>& client) override;
    ErrCode GetInputStartInfo(bool& isInputStart, uint32_t& callingWndId, int32_t& requestKeyboardReason) override;
    ErrCode SendPrivateData(const Value &value) override;

    ErrCode IsCurrentIme(bool& resultValue) override;
    ErrCode IsInputTypeSupported(int32_t type, bool& resultValue) override;
    ErrCode IsCurrentImeByPid(int32_t pid, bool& resultValue) override;
    ErrCode StartInputType(int32_t type) override;
    ErrCode ExitCurrentInputType() override;
    ErrCode IsPanelShown(const PanelInfo &panelInfo, bool &isShown) override;
    ErrCode GetSecurityMode(int32_t &security) override;
    ErrCode ConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent) override;
    // Deprecated because of no permission check, kept for compatibility
    ErrCode HideCurrentInputDeprecated() override;
    ErrCode ShowCurrentInputDeprecated() override;
    int Dump(int fd, const std::vector<std::u16string> &args) override;
    void DumpAllMethod(int fd);
    ErrCode IsDefaultIme() override;
    ErrCode IsDefaultImeSet(bool& resultValue) override;
    ErrCode EnableIme(const std::string &bundleName, bool& resultValue) override;
    ErrCode GetInputMethodState(int32_t &status) override;
    ErrCode IsSystemApp(bool& resultValue) override;
    int32_t RegisterProxyIme(
        uint64_t displayId, const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent) override;
    int32_t UnregisterProxyIme(uint64_t displayId) override;

protected:
    void OnStart() override;
    void OnStop() override;
    int32_t OnIdle(const SystemAbilityOnDemandReason &idleReason) override;
    int32_t OnExtension(const std::string &extension, MessageParcel &data, MessageParcel &reply) override;

private:
    int32_t Init();
    void Initialize();

    std::thread workThreadHandler; /*!< thread handler of the WorkThread */
    void RestartSessionIme(std::shared_ptr<PerUserSession> &session);
    std::shared_ptr<PerUserSession> GetSessionFromMsg(const Message *msg);
    int32_t PrepareForOperateKeyboard(std::shared_ptr<PerUserSession> &session);
    int32_t SwitchByCondition(const Condition &condition,
        const std::shared_ptr<ImeInfo> &info);
    int32_t GetUserId(int32_t uid);
    int32_t GetCallingUserId();
    uint64_t GetCallingDisplayId();
    std::shared_ptr<IdentityChecker> identityChecker_ = nullptr;
    int32_t PrepareInput(int32_t userId, InputClientInfo &clientInfo);
    void WorkThread();
    int32_t OnUserStarted(const Message *msg);
    int32_t OnUserRemoved(const Message *msg);
    int32_t OnUserStop(const Message *msg);
    int32_t OnHideKeyboardSelf(const Message *msg);
    bool IsNeedSwitch(int32_t userId, const std::string &bundleName, const std::string &subName);
    int32_t CheckEnableAndSwitchPermission();
    std::string SetSettingValues(const std::string &settingValue, const std::string &bundleName);
    int32_t CheckSwitchPermission(int32_t userId, const SwitchInfo &switchInfo, SwitchTrigger trigger);
    bool IsStartInputTypePermitted(int32_t userId);
    int32_t OnSwitchInputMethod(int32_t userId, const SwitchInfo &switchInfo, SwitchTrigger trigger);
    int32_t OnStartInputType(int32_t userId, const SwitchInfo &switchInfo, bool isCheckPermission);
    int32_t HandlePackageEvent(const Message *msg);
    int32_t OnPackageRemoved(int32_t userId, const std::string &packageName);
    void OnScreenUnlock(const Message *msg);
    int32_t OnDisplayOptionalInputMethod();
    void SubscribeCommonEvent();
    int32_t Switch(int32_t userId, const std::string &bundleName, const std::shared_ptr<ImeInfo> &info);
    int32_t SwitchExtension(int32_t userId, const std::shared_ptr<ImeInfo> &info);
    int32_t SwitchSubType(int32_t userId, const std::shared_ptr<ImeInfo> &info);
    int32_t SwitchInputType(int32_t userId, const SwitchInfo &switchInfo);
    void GetValidSubtype(const std::string &subName, const std::shared_ptr<ImeInfo> &info);
    ServiceRunningState state_;
    void InitServiceHandler();
    void UpdateUserInfo(int32_t userId);
    void HandleWmsConnected(int32_t userId, int32_t screenId);
    void HandleWmsDisconnected(int32_t userId, int32_t screenId);
    void HandleScbStarted(int32_t userId, int32_t screenId);
    void HandleUserSwitched(int32_t userId);
    void HandleWmsStarted();
    void HandleMemStarted();
    void HandleDataShareReady();
    void HandleOsAccountStarted();
    void HandleFocusChanged(bool isFocused, uint64_t displayId, int32_t pid, int32_t uid);
    void HandleImeCfgCapsState();
    void StopImeInBackground();
    int32_t InitAccountMonitor();
    static std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    int32_t userId_;
    bool stop_ = false;
    void InitMonitors();
    int32_t InitKeyEventMonitor();
    bool InitWmsMonitor();
    void InitSystemLanguageMonitor();
    bool InitMemMgrMonitor();
    void InitWmsConnectionMonitor();
    void InitFocusChangedMonitor();
    void InitWindowDisplayChangedMonitor();
    int32_t SwitchByCombinationKey(uint32_t state);
    int32_t SwitchMode();
    int32_t SwitchLanguage();
    int32_t SwitchType();
    int32_t GenerateClientInfo(int32_t userId, InputClientInfo &clientInfo);
    void RegisterEnableImeObserver();
    void RegisterSecurityModeObserver();
    int32_t CheckInputTypeOption(int32_t userId, InputClientInfo &inputClientInfo);
    int32_t IsDefaultImeFromTokenId(int32_t userId, uint32_t tokenId);
    void DealSwitchRequest();
    void DealSecurityChange();
    void OnSecurityModeChange();
    bool IsCurrentIme(int32_t userId);
    int32_t StartInputType(int32_t userId, InputType type);
    // if switch input type need to switch ime, then no need to hide panel first.
    void NeedHideWhenSwitchInputType(int32_t userId, bool &needHide);
    bool GetDeviceFunctionKeyState(int32_t functionKey, bool &isEnable);
    bool ModifyImeCfgWithWrongCaps();
    void HandleBundleScanFinished();
    int32_t GetInputMethodState(int32_t userId, const std::string &bundleName, EnabledStatus &status);
    int32_t StartInputInner(
        InputClientInfo &inputClientInfo, sptr<IRemoteObject> &agent, std::pair<int64_t, std::string> &imeInfo);
    int32_t ShowInputInner(sptr<IInputClient> client, int32_t requestKeyboardReason = 0);
    int32_t ShowCurrentInputInner();
    std::pair<int64_t, std::string> GetCurrentImeInfoForHiSysEvent(int32_t userId);
    int32_t GetScreenLockIme(std::string &ime);
    int32_t GetAlternativeIme(std::string &ime);
#ifdef IMF_ON_DEMAND_START_STOP_SA_ENABLE
    int64_t GetTickCount();
    void ResetDelayUnloadTask(uint32_t code = 0);
    bool IsImeInUse();
#endif
    std::mutex checkMutex_;
    void DatashareCallback(const std::string &key);
    bool IsValidBundleName(const std::string &bundleName);
    std::string GetRestoreBundleName(MessageParcel &data);
    int32_t RestoreInputmethod(std::string &bundleName);

    std::atomic<bool> enableImeOn_ = false;
    std::atomic<bool> enableSecurityMode_ = false;
    std::atomic<bool> isBundleScanFinished_ = false;
    std::atomic<bool> isScbEnable_ = false;
    std::mutex switchImeMutex_;
    std::atomic<bool> switchTaskExecuting_ = false;
    std::atomic<uint32_t> targetSwitchCount_ = 0;

    std::mutex modeChangeMutex_;
    bool isChangeHandling_ = false;
    bool hasPendingChanges_ = false;
    void ChangeToDefaultImeForHiCar(int32_t userId, InputClientInfo &inputClientInfo);
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_H
