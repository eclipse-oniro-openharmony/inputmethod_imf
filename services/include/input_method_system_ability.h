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

#include <atomic>
#include <map>
#include <thread>

#include "ability_manager_interface.h"
#include "bundle_mgr_proxy.h"
#include "event_handler.h"
#include "input_method_status.h"
#include "input_method_system_ability_stub.h"
#include "inputmethod_dump.h"
#include "inputmethod_trace.h"
#include "peruser_session.h"
#include "peruser_setting.h"
#include "system_ability.h"

namespace OHOS {
namespace MiscServices {
using AbilityType = AppExecFwk::ExtensionAbilityType;
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

class InputMethodSystemAbility : public SystemAbility, public InputMethodSystemAbilityStub {
    DECLARE_SYSTEM_ABILITY(InputMethodSystemAbility);

public:
    DISALLOW_COPY_AND_MOVE(InputMethodSystemAbility);
    InputMethodSystemAbility(int32_t systemAbilityId, bool runOnCreate);
    InputMethodSystemAbility();
    ~InputMethodSystemAbility();

    int32_t GetUserState(int32_t userId);

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    int32_t PrepareInput(int32_t displayId, sptr<IInputClient> client, sptr<IInputDataChannel> channel,
        InputAttribute &attribute) override;
    int32_t StartInput(sptr<IInputClient> client, bool isShowKeyboard) override;
    int32_t ShowCurrentInput() override;
    int32_t HideCurrentInput() override;
    int32_t StopInput(sptr<IInputClient> client) override;
    int32_t ReleaseInput(sptr<IInputClient> client) override;
    int32_t GetKeyboardWindowHeight(int32_t &retHeight) override;
    std::shared_ptr<Property> GetCurrentInputMethod() override;
    std::vector<Property> ListInputMethod(InputMethodStatus status) override;
    std::vector<SubProperty> ListCurrentInputMethodSubtype() override;
    std::vector<SubProperty> ListInputMethodSubtype(const std::string &name) override;
    int32_t SwitchInputMethod(const std::string &name, const std::string &subName) override;
    int32_t DisplayOptionalInputMethod() override;
    int32_t SetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent) override;

    // Deprecated because of no permission check, kept for compatibility
    int32_t SetCoreAndAgentDeprecated(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent) override;
    int32_t HideCurrentInputDeprecated() override;
    int32_t ShowCurrentInputDeprecated() override;
    int32_t DisplayOptionalInputMethodDeprecated() override;

    std::vector<Property> ListInputMethodByUserId(int32_t userId, InputMethodStatus status);
    int Dump(int fd, const std::vector<std::u16string> &args) override;
    void DumpAllMethod(int fd);

protected:
    void OnStart() override;
    void OnStop() override;

private:
    int32_t Init();
    void Initialize();

    std::thread workThreadHandler; /*!< thread handler of the WorkThread */

    std::map<int32_t, PerUserSetting *> userSettings;

    std::map<int32_t, std::shared_ptr<PerUserSession>> userSessions;
    std::map<int32_t, MessageHandler *> msgHandlers;

    void WorkThread();
    PerUserSetting *GetUserSetting(int32_t userId);
    std::shared_ptr<PerUserSession> GetUserSession(int32_t userId);
    bool StartInputService(std::string imeId);
    void StopInputService(std::string imeId);
    int32_t OnUserStarted(const Message *msg);
    int32_t OnUserStopped(const Message *msg);
    int32_t OnUserUnlocked(const Message *msg);
    int32_t OnUserLocked(const Message *msg);
    int32_t OnHandleMessage(Message *msg);
    int32_t OnSettingChanged(const Message *msg);
    int32_t OnPackageRemoved(const Message *msg);
    int32_t OnPackageAdded(const Message *msg);
    int32_t OnDisableIms(const Message *msg);
    int32_t OnAdvanceToNext(const Message *msg);
    int32_t OnDisplayOptionalInputMethod(int32_t userId);
    static sptr<AAFwk::IAbilityManager> GetAbilityManagerService();
    OHOS::sptr<OHOS::AppExecFwk::IBundleMgr> GetBundleMgr();
    std::vector<InputMethodInfo> ListInputMethodInfo(int32_t userId);
    std::vector<Property> ListAllInputMethod(int32_t userId);
    std::vector<Property> ListEnabledInputMethod();
    std::vector<Property> ListDisabledInputMethod(int32_t userId);
    std::vector<Property> ListProperty(int32_t userId);
    std::vector<SubProperty> ListSubtypeByBundleName(int32_t userId, const std::string &name);
    void StartUserIdListener();
    int32_t SwitchInputMethodType(const std::string &name);
    int32_t SwitchInputMethodSubtype(const std::string &name, const std::string &subName);
    int32_t OnSwitchInputMethod(const std::string &bundleName, const std::string &name);
    Property FindProperty(const std::string &name);
    SubProperty FindSubProperty(const std::string &bundleName, const std::string &name);
    std::string GetInputMethodParam(const std::vector<InputMethodInfo> &properties);
    ServiceRunningState state_;
    void InitServiceHandler();
    std::atomic_flag dialogLock_ = ATOMIC_FLAG_INIT;
    static std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    int32_t userId_;
    static constexpr const char *SELECT_DIALOG_ACTION = "action.system.inputmethodchoose";
    static constexpr const char *SELECT_DIALOG_HAP = "cn.openharmony.inputmethodchoosedialog";
    static constexpr const char *SELECT_DIALOG_ABILITY = "InputMethod";
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_H
