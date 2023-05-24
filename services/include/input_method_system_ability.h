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
#include "application_info.h"
#include "bundle_mgr_proxy.h"
#include "event_handler.h"
#include "ime_info_inquirer.h"
#include "input_method_status.h"
#include "input_method_system_ability_stub.h"
#include "inputmethod_dump.h"
#include "inputmethod_trace.h"
#include "peruser_session.h"
#include "system_ability.h"

namespace OHOS {
namespace MiscServices {
using AbilityType = AppExecFwk::ExtensionAbilityType;
using namespace AppExecFwk;
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

struct SwitchInfo {
    std::chrono::system_clock::time_point timestamp{};
    std::string bundleName;
    std::string subName;
    bool operator==(const SwitchInfo &info) const
    {
        return (timestamp == info.timestamp && bundleName == info.bundleName && subName == info.subName);
    }
};

class InputMethodSystemAbility : public SystemAbility, public InputMethodSystemAbilityStub {
    DECLARE_SYSTEM_ABILITY(InputMethodSystemAbility);

public:
    DISALLOW_COPY_AND_MOVE(InputMethodSystemAbility);
    InputMethodSystemAbility(int32_t systemAbilityId, bool runOnCreate);
    InputMethodSystemAbility();
    ~InputMethodSystemAbility();

    int32_t PrepareInput(InputClientInfo &clientInfo) override;
    int32_t StartInput(sptr<IInputClient> client, bool isShowKeyboard) override;
    int32_t ShowCurrentInput() override;
    int32_t HideCurrentInput() override;
    int32_t StopInput(sptr<IInputClient> client) override;
    int32_t StopInputSession() override;
    int32_t ReleaseInput(sptr<IInputClient> client) override;
    std::shared_ptr<Property> GetCurrentInputMethod() override;
    std::shared_ptr<SubProperty> GetCurrentInputMethodSubtype() override;
    int32_t ListInputMethod(InputMethodStatus status, std::vector<Property> &props) override;
    int32_t ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProps) override;
    int32_t ListInputMethodSubtype(const std::string &bundleName, std::vector<SubProperty> &subProps) override;
    int32_t SwitchInputMethod(const std::string &bundleName, const std::string &subName) override;
    int32_t DisplayOptionalInputMethod() override;
    int32_t SetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent) override;
    int32_t PanelStatusChange(const InputWindowStatus &status, const InputWindowInfo &windowInfo) override;
    int32_t UpdateListenEventFlag(InputClientInfo &clientInfo, EventType eventType) override;

    // Deprecated because of no permission check, kept for compatibility
    int32_t HideCurrentInputDeprecated() override;
    int32_t ShowCurrentInputDeprecated() override;
    int32_t DisplayOptionalInputMethodDeprecated() override;
    int Dump(int fd, const std::vector<std::u16string> &args) override;
    void DumpAllMethod(int fd);

protected:
    void OnStart() override;
    void OnStop() override;

private:
    int32_t Init();
    void Initialize();

    std::thread workThreadHandler; /*!< thread handler of the WorkThread */
    std::shared_ptr<PerUserSession> userSession_ = nullptr;
    void WorkThread();
    bool StartInputService(const std::string &imeId);
    void StopInputService(const std::string &imeId);
    int32_t OnUserStarted(const Message *msg);
    int32_t OnUserRemoved(const Message *msg);
    int32_t OnPackageRemoved(const Message *msg);
    int32_t OnDisplayOptionalInputMethod();
    static sptr<AAFwk::IAbilityManager> GetAbilityManagerService();
    void StartUserIdListener();
    bool IsNeedSwitch(const std::string &bundleName, const std::string &subName);
    int32_t OnSwitchInputMethod(const SwitchInfo &switchInfo);
    int32_t Switch(const std::string &bundleName, const ImeInfo &info);
    int32_t SwitchExtension(const ImeInfo &info);
    int32_t SwitchSubType(const ImeInfo &info);
    ServiceRunningState state_;
    void InitServiceHandler();
    static std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    int32_t userId_;
    static constexpr const char *SELECT_DIALOG_ACTION = "action.system.inputmethodchoose";
    static constexpr const char *SELECT_DIALOG_HAP = "cn.openharmony.inputmethodchoosedialog";
    static constexpr const char *SELECT_DIALOG_ABILITY = "InputMethod";

    std::mutex switchMutex_;
    std::condition_variable switchCV_;
    std::mutex switchQueueMutex_;
    std::queue<SwitchInfo> switchQueue_;
    void PopSwitchQueue();
    void PushToSwitchQueue(const SwitchInfo &info);
    bool CheckReadyToSwitch(const SwitchInfo &info);

    int32_t InitKeyEventMonitor();
    bool InitFocusChangeMonitor();
    int32_t SwitchByCombinationKey(uint32_t state);
    int32_t SwitchMode();
    int32_t SwitchLanguage();
    int32_t SwitchType();
    int32_t GenerateClientInfo(InputClientInfo &clientInfo);
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_INPUT_METHOD_SYSTEM_ABILITY_H
