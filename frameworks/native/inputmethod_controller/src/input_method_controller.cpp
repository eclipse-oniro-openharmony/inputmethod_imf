/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

#include "input_method_controller.h"

#include <algorithm>
#include <cinttypes>
#include "securec.h"

#include "block_data.h"
#include "global.h"
#include "imc_hisysevent_reporter.h"
#include "ime_event_monitor_manager_impl.h"
#include "input_client_service_impl.h"
#include "input_data_channel_service_impl.h"
#include "input_method_agent_proxy.h"
#include "input_method_property.h"
#include "input_method_status.h"
#include "input_method_system_ability_proxy.h"
#include "inputmethod_sysevent.h"
#include "inputmethod_trace.h"
#include "iservice_registry.h"
#include "keyevent_consumer_service_impl.h"
#include "on_demand_start_stop_sa.h"
#include "string_ex.h"
#include "string_utils.h"
#include "sys/prctl.h"
#include "system_ability_definition.h"
#include "system_cmd_channel_stub.h"
#include "input_method_tools.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
using namespace std::chrono;
sptr<InputMethodController> InputMethodController::instance_;
std::shared_ptr<AppExecFwk::EventHandler> InputMethodController::handler_ { nullptr };
std::mutex InputMethodController::instanceLock_;
std::mutex InputMethodController::logLock_;
int InputMethodController::keyEventCountInPeriod_ = 0;
std::chrono::system_clock::time_point InputMethodController::startLogTime_ = system_clock::now();
constexpr int32_t LOOP_COUNT = 5;
constexpr int32_t LOG_MAX_TIME = 20;
constexpr int64_t DELAY_TIME = 100;
constexpr int32_t ACE_DEAL_TIME_OUT = 200;
constexpr int32_t MAX_PLACEHOLDER_SIZE = 255; // 256 utf16 char
constexpr int32_t MAX_ABILITY_NAME_SIZE = 127; // 127 utf16 char
InputMethodController::InputMethodController()
{
    IMSA_HILOGD("IMC structure.");
}

InputMethodController::~InputMethodController() { }

sptr<InputMethodController> InputMethodController::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            IMSA_HILOGD("instance_ is nullptr.");
            instance_ = new (std::nothrow) InputMethodController();
            if (instance_ == nullptr) {
                IMSA_HILOGE("failed to create InputMethodController!");
                return instance_;
            }
            int32_t ret = instance_->Initialize();
            if (ret != ErrorCode::NO_ERROR) {
                InputMethodSysEvent::GetInstance().InputmethodFaultReporter(ret, "", "IMC initialize failed!");
            }
        }
    }
    return instance_;
}

int32_t InputMethodController::RestoreListenEventFlag()
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    // 0 represent no need to check permission
    InputClientInfoInner infoInner = InputMethodTools::GetInstance().InputClientInfoToInner(clientInfo_);
    return proxy->UpdateListenEventFlag(infoInner, 0);
}

int32_t InputMethodController::UpdateListenEventFlag(uint32_t finalEventFlag, uint32_t eventFlag, bool isOn)
{
    auto oldEventFlag = clientInfo_.eventFlag;
    clientInfo_.eventFlag = finalEventFlag;
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        if (isOn) {
            clientInfo_.eventFlag = oldEventFlag;
        }
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    InputClientInfoInner infoInner = InputMethodTools::GetInstance().InputClientInfoToInner(clientInfo_);
    auto ret = proxy->UpdateListenEventFlag(infoInner, eventFlag);
    if (ret != ErrorCode::NO_ERROR && isOn) {
        clientInfo_.eventFlag = oldEventFlag;
    }
    return ret;
}

void InputMethodController::SetControllerListener(std::shared_ptr<ControllerListener> controllerListener)
{
    IMSA_HILOGD("InputMethodController run in");
    controllerListener_ = std::move(controllerListener);
}

int32_t InputMethodController::Initialize()
{
    sptr<IInputClient> client = new (std::nothrow) InputClientServiceImpl();
    if (client == nullptr) {
        IMSA_HILOGE("failed to create client!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    sptr<IInputDataChannel> channel = new (std::nothrow) InputDataChannelServiceImpl();
    if (channel == nullptr) {
        IMSA_HILOGE("failed to new channel!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    InputAttribute attribute;
    attribute.inputPattern = InputAttribute::PATTERN_TEXT;
    clientInfo_.attribute = attribute;
    clientInfo_.client = client,
    clientInfo_.channel = channel->AsObject();

    // make AppExecFwk::EventHandler handler
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    return ErrorCode::NO_ERROR;
}

sptr<IInputMethodSystemAbility> InputMethodController::TryGetSystemAbilityProxy()
{
#ifdef IMF_ON_DEMAND_START_STOP_SA_ENABLE
    return GetSystemAbilityProxy(false);
#else
    return GetSystemAbilityProxy(true);
#endif
}

sptr<IInputMethodSystemAbility> InputMethodController::GetSystemAbilityProxy(bool ifRetry)
{
    std::lock_guard<std::mutex> lock(abilityLock_);
    if (abilityManager_ != nullptr) {
        return abilityManager_;
    }
    IMSA_HILOGI("get input method service proxy.");
    auto systemAbility = OnDemandStartStopSa::GetInputMethodSystemAbility(ifRetry);
    if (systemAbility == nullptr) {
        IMSA_HILOGE("systemAbility is nullptr!");
        return nullptr;
    }

    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new (std::nothrow) InputDeathRecipient();
        if (deathRecipient_ == nullptr) {
            IMSA_HILOGE("create death recipient failed!");
            return nullptr;
        }
    }
    deathRecipient_->SetDeathRecipient([this](const wptr<IRemoteObject> &remote) {
        OnRemoteSaDied(remote);
    });
    if ((systemAbility->IsProxyObject()) && (!systemAbility->AddDeathRecipient(deathRecipient_))) {
        IMSA_HILOGE("failed to add death recipient!");
        return nullptr;
    }
    abilityManager_ = iface_cast<IInputMethodSystemAbility>(systemAbility);
    return abilityManager_;
}

void InputMethodController::RemoveDeathRecipient()
{
    std::lock_guard<std::mutex> lock(abilityLock_);
    if (abilityManager_ != nullptr && abilityManager_->AsObject() != nullptr && deathRecipient_ != nullptr) {
        abilityManager_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    deathRecipient_ = nullptr;
    abilityManager_ = nullptr;
}

void InputMethodController::DeactivateClient()
{
    {
        std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
        clientInfo_.state = ClientState::INACTIVE;
    }
    SendKeyboardStatus(KeyboardStatus::NONE);
}

void InputMethodController::SaveTextConfig(const TextConfig &textConfig)
{
    IMSA_HILOGD("textConfig: %{public}s.", textConfig.ToString().c_str());
    int32_t x = textConfig.cursorInfo.left;
    int32_t y = textConfig.cursorInfo.top;
    uint32_t windowId = textConfig.windowId;
    GetWindowScaleCoordinate(x, y, windowId);
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_ = textConfig;
        textConfig_.cursorInfo.left = x;
        textConfig_.cursorInfo.top = y;
        StringUtils::TruncateUtf16String(textConfig_.inputAttribute.placeholder, MAX_PLACEHOLDER_SIZE);
        StringUtils::TruncateUtf16String(textConfig_.inputAttribute.abilityName, MAX_ABILITY_NAME_SIZE);
    }
    if (textConfig.range.start != INVALID_VALUE) {
        std::lock_guard<std::mutex> lock(editorContentLock_);
        selectOldBegin_ = selectNewBegin_;
        selectOldEnd_ = selectNewEnd_;
        selectNewBegin_ = textConfig.range.start;
        selectNewEnd_ = textConfig.range.end;
    }
}

int32_t InputMethodController::Attach(sptr<OnTextChangedListener> listener, ClientType type)
{
    return Attach(listener, true, type);
}

int32_t InputMethodController::Attach(sptr<OnTextChangedListener> listener, bool isShowKeyboard, ClientType type)
{
    InputAttribute attribute;
    attribute.inputPattern = InputAttribute::PATTERN_TEXT;
    return Attach(listener, isShowKeyboard, attribute, type);
}

int32_t InputMethodController::Attach(
    sptr<OnTextChangedListener> listener, bool isShowKeyboard, const InputAttribute &attribute, ClientType type)
{
    InputMethodSyncTrace tracer("InputMethodController Attach trace.");
    TextConfig textConfig;
    textConfig.inputAttribute = attribute;
    return Attach(listener, isShowKeyboard, textConfig, type);
}

int32_t InputMethodController::Attach(
    sptr<OnTextChangedListener> listener, bool isShowKeyboard, const TextConfig &textConfig, ClientType type)
{
    AttachOptions attachOptions;
    attachOptions.isShowKeyboard = isShowKeyboard;
    attachOptions.requestKeyboardReason = RequestKeyboardReason::NONE;
    return Attach(listener, attachOptions, textConfig, type);
}

int32_t InputMethodController::IsValidTextConfig(const TextConfig &textConfig)
{
    if (textConfig.inputAttribute.immersiveMode < static_cast<int32_t>(ImmersiveMode::NONE_IMMERSIVE) ||
        textConfig.inputAttribute.immersiveMode >= static_cast<int32_t>(ImmersiveMode::END)) {
        IMSA_HILOGE("invalid immersiveMode: %{public}d", textConfig.inputAttribute.immersiveMode);
        return ErrorCode::ERROR_PARAMETER_CHECK_FAILED;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::Attach(sptr<OnTextChangedListener> listener, const AttachOptions &attachOptions,
    const TextConfig &textConfig, ClientType type)
{
    IMSA_HILOGI("isShowKeyboard %{public}d.", attachOptions.isShowKeyboard);
    InputMethodSyncTrace tracer("InputMethodController Attach with textConfig trace.");
    if (IsValidTextConfig(textConfig) != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("invalid textConfig.");
        return ErrorCode::ERROR_PARAMETER_CHECK_FAILED;
    }
    auto lastListener = GetTextListener();
    clientInfo_.isNotifyInputStart = lastListener != listener;
    if (clientInfo_.isNotifyInputStart) {
        sessionId_++;
    }
    IMSA_HILOGI("sessionId_ %{public}u", sessionId_.load());
    if (clientInfo_.isNotifyInputStart && lastListener != nullptr) {
        lastListener->OnDetach();
    }
    ClearEditorCache(clientInfo_.isNotifyInputStart, lastListener);
    SetTextListener(listener);
    {
        std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
        clientInfo_.isShowKeyboard = attachOptions.isShowKeyboard;
    }
    SaveTextConfig(textConfig);
    GetTextConfig(clientInfo_.config);
    clientInfo_.requestKeyboardReason = attachOptions.requestKeyboardReason;
    clientInfo_.type = type;

    sptr<IRemoteObject> agent = nullptr;
    std::pair<int64_t, std::string> imeInfo{ 0, "" };
    int32_t ret = StartInput(clientInfo_, agent, imeInfo);
    if (ret != ErrorCode::NO_ERROR) {
        auto evenInfo = HiSysOriginalInfo::Builder()
                            .SetErrCode(ret)
                            .SetInputPattern(textConfig.inputAttribute.inputPattern)
                            .SetIsShowKeyboard(attachOptions.isShowKeyboard)
                            .SetClientType(type)
                            .Build();
        ImcHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_ATTACH, *evenInfo);
        return ret;
    }
    clientInfo_.state = ClientState::ACTIVE;
    OnInputReady(agent, imeInfo);
    if (attachOptions.isShowKeyboard) {
        InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_SHOW_ATTACH);
    }
    IMSA_HILOGI("bind imf successfully.");
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::ShowTextInputInner(const AttachOptions &attachOptions, ClientType type)
{
    InputMethodSyncTrace tracer("IMC_ShowTextInput");
    if (!IsBound()) {
        IMSA_HILOGE("not bound!");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    IMSA_HILOGI("start.");
    {
        std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
        clientInfo_.isShowKeyboard = true;
    }
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_SHOW_ENEDITABLE);
    int32_t ret = ShowInput(clientInfo_.client, type, static_cast<int32_t>(attachOptions.requestKeyboardReason));
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to start input: %{public}d", ret);
        return ret;
    }
    isEditable_.store(true);
    IMSA_HILOGI("enter editable state.");
    return ret;
}

int32_t InputMethodController::HideTextInput()
{
    InputMethodSyncTrace tracer("IMC_HideTextInput");
    if (!IsBound()) {
        IMSA_HILOGE("not bound!");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    IMSA_HILOGI("start.");
    isEditable_.store(false);
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_UNEDITABLE);
    return HideInput(clientInfo_.client);
}

int32_t InputMethodController::HideCurrentInput()
{
    InputMethodSyncTrace tracer("IMC_HideCurrentInput");
    IMSA_HILOGD("InputMethodController::HideCurrentInput");
    if (!IsEditable()) {
        IMSA_HILOGD("not editable.");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
        clientInfo_.isShowKeyboard = false;
    }
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_NORMAL);
    return proxy->HideCurrentInputDeprecated();
}

int32_t InputMethodController::ShowCurrentInput()
{
    InputMethodSyncTrace tracer("IMC_ShowCurrentInput");
    if (!IsEditable()) {
        IMSA_HILOGD("not editable.");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGI("start.");
    {
        std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
        clientInfo_.isShowKeyboard = true;
    }
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_SHOW_NORMAL);
    return proxy->ShowCurrentInputDeprecated();
}

int32_t InputMethodController::Close()
{
    if (IsBound()) {
        IMSA_HILOGI("start.");
    }

    auto listener = GetTextListener();
    if (listener != nullptr) {
        listener->OnDetach();
    }
    OperateIMEInfoCode infoCode = OperateIMEInfoCode::IME_UNBIND;
    {
        std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
        if (clientInfo_.isShowKeyboard) {
            infoCode = OperateIMEInfoCode::IME_HIDE_UNBIND;
        }
    }
    InputMethodSyncTrace tracer("InputMethodController Close trace.");
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(infoCode);
    return ReleaseInput(clientInfo_.client);
}

void InputMethodController::Reset()
{
    Close();
    RemoveDeathRecipient();
}

int32_t InputMethodController::RequestShowInput()
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_SERVICE_START_FAILED; // ERROR_EX_NULL_POINTER
    }
    IMSA_HILOGI("InputMethodController start.");
    return proxy->RequestShowInput();
}

int32_t InputMethodController::RequestHideInput(bool isFocusTriggered)
{
    auto proxy = TryGetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGD("InputMethodController start.");
    return proxy->RequestHideInput(isFocusTriggered);
}

int32_t InputMethodController::DisplayOptionalInputMethod()
{
    IMSA_HILOGD("InputMethodController::DisplayOptionalInputMethod start.");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return proxy->DisplayOptionalInputMethod();
}

bool InputMethodController::WasAttached()
{
    return isBound_.load();
}

int32_t InputMethodController::GetInputStartInfo(bool &isInputStart,
    uint32_t &callingWndId, int32_t &requestKeyboardReason)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return false;
    }
    return proxy->GetInputStartInfo(isInputStart, callingWndId, requestKeyboardReason);
}

int32_t InputMethodController::ListInputMethodCommon(InputMethodStatus status, std::vector<Property> &props)
{
    IMSA_HILOGD("InputMethodController::ListInputMethodCommon start.");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return proxy->ListInputMethod(status, props);
}

int32_t InputMethodController::ListInputMethod(std::vector<Property> &props)
{
    IMSA_HILOGD("InputMethodController::listInputMethod start.");
    return ListInputMethodCommon(ALL, props);
}

int32_t InputMethodController::ListInputMethod(bool enable, std::vector<Property> &props)
{
    IMSA_HILOGI("enable: %{public}s.", enable ? "ENABLE" : "DISABLE");
    return ListInputMethodCommon(enable ? ENABLE : DISABLE, props);
}

int32_t InputMethodController::GetDefaultInputMethod(std::shared_ptr<Property> &property)
{
    InputMethodSyncTrace tracer("IMC_GetDefaultInputMethod");
    IMSA_HILOGD("InputMethodController::GetDefaultInputMethod start.");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    Property prop;
    auto ret = proxy->GetDefaultInputMethod(prop, false);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    property = std::make_shared<Property>(prop);
    return ret;
}

int32_t InputMethodController::GetInputMethodConfig(OHOS::AppExecFwk::ElementName &inputMethodConfig)
{
    InputMethodSyncTrace tracer("IMC_GetInputMethodConfig");
    IMSA_HILOGD("InputMethodController::inputMethodConfig start.");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    return proxy->GetInputMethodConfig(inputMethodConfig);
}

std::shared_ptr<Property> InputMethodController::GetCurrentInputMethod()
{
    InputMethodSyncTrace tracer("IMC_GetCurrentInputMethod");
    IMSA_HILOGD("InputMethodController::GetCurrentInputMethod start.");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return nullptr;
    }
    Property propertyData;
    proxy->GetCurrentInputMethod(propertyData);
    auto property = std::make_shared<Property>(propertyData);
    return property;
}

std::shared_ptr<SubProperty> InputMethodController::GetCurrentInputMethodSubtype()
{
    InputMethodSyncTrace tracer("IMC_GetCurrentInputMethodSubtype");
    IMSA_HILOGD("InputMethodController::GetCurrentInputMethodSubtype start.");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return nullptr;
    }
    SubProperty subPropertyData;
    proxy->GetCurrentInputMethodSubtype(subPropertyData);
    auto subProperty = std::make_shared<SubProperty>(subPropertyData);
    return subProperty;
}

bool InputMethodController::IsDefaultImeSet()
{
    IMSA_HILOGI("enter.");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return false;
    }
    bool ret = false;
    proxy->IsDefaultImeSet(ret);
    return ret;
}

int32_t InputMethodController::EnableIme(
    const std::string &bundleName, const std::string &extensionName, EnabledStatus status)
{
    IMSA_HILOGI("enter.");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return false;
    }
    return proxy->EnableIme(bundleName, extensionName, static_cast<int32_t>(status));
}

int32_t InputMethodController::StartInput(
    InputClientInfo &inputClientInfo, sptr<IRemoteObject> &agent, std::pair<int64_t, std::string> &imeInfo)
{
    IMSA_HILOGD("InputMethodController::StartInput start.");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    InputClientInfoInner inner = InputMethodTools::GetInstance().InputClientInfoToInner(inputClientInfo);
    int32_t ret = proxy->StartInput(inner, agent, imeInfo.first, imeInfo.second);
    return ret;
}

int32_t InputMethodController::ReleaseInput(sptr<IInputClient> &client)
{
    IMSA_HILOGD("InputMethodController::ReleaseInput start.");
    auto proxy = TryGetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    int32_t ret = proxy->ReleaseInput(client, sessionId_.load());
    if (ret == ErrorCode::NO_ERROR) {
        OnInputStop();
    }
    SetTextListener(nullptr);
    return ret;
}

int32_t InputMethodController::ShowInput(sptr<IInputClient> &client, ClientType type, int32_t requestKeyboardReason)
{
    IMSA_HILOGD("InputMethodController::ShowInput start.");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    return proxy->ShowInput(client, type, requestKeyboardReason);
}

int32_t InputMethodController::HideInput(sptr<IInputClient> &client)
{
    IMSA_HILOGD("InputMethodController::HideInput start.");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    return proxy->HideInput(client);
}

void InputMethodController::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
{
    IMSA_HILOGI("input method service death.");
    // imf sa died, current client callback inputStop
    ImeEventMonitorManagerImpl::GetInstance().OnInputStop();
    auto textListener = GetTextListener();
    if (textListener != nullptr && textConfig_.inputAttribute.isTextPreviewSupported) {
        IMSA_HILOGD("finish text preview.");
        textListener->FinishTextPreview();
    }
    {
        std::lock_guard<std::mutex> lock(abilityLock_);
        abilityManager_ = nullptr;
    }
    if (handler_ == nullptr) {
        IMSA_HILOGE("handler_ is nullptr!");
        return;
    }
    RestoreClientInfoInSaDied();
}

void InputMethodController::RestoreListenInfoInSaDied()
{
    {
        std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
        if (clientInfo_.eventFlag == NO_EVENT_ON) {
            return;
        }
    }
    isDiedRestoreListen_.store(false);
    auto restoreListenTask = [=]() {
        if (isDiedRestoreListen_.load()) {
            return;
        }
        auto ret = RestoreListenEventFlag();
        if (ret == ErrorCode::NO_ERROR) {
            isDiedRestoreListen_.store(true);
            IMSA_HILOGI("try to RestoreListen success.");
        }
    };
    for (int i = 0; i < LOOP_COUNT; i++) {
        handler_->PostTask(restoreListenTask, "OnRemoteSaDied", DELAY_TIME * (i + 1),
            AppExecFwk::EventQueue::Priority::VIP);
    }
}

void InputMethodController::RestoreClientInfoInSaDied()
{
    if (!IsEditable()) {
        IMSA_HILOGD("not editable.");
        RestoreListenInfoInSaDied();
        return;
    }
    auto attach = [=]() -> bool {
        TextConfig tempConfig {};
        {
            std::lock_guard<std::mutex> lock(textConfigLock_);
            tempConfig = textConfig_;
            tempConfig.cursorInfo = cursorInfo_;
            tempConfig.range.start = selectNewBegin_;
            tempConfig.range.end = selectNewEnd_;
        }
        auto listener = GetTextListener();
        bool isShowKeyboard = false;
        {
            std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
            isShowKeyboard = clientInfo_.isShowKeyboard;
        }
        auto errCode = Attach(listener, isShowKeyboard, tempConfig);
        IMSA_HILOGI("attach end, errCode: %{public}d", errCode);
        return errCode == ErrorCode::NO_ERROR;
    };
    if (attach()) {
        return;
    }
    isDiedAttached_.store(false);
    auto attachTask = [this, attach]() {
        if (isDiedAttached_.load()) {
            return;
        }
        attach();
    };
    for (int i = 0; i < LOOP_COUNT; i++) {
        handler_->PostTask(attachTask, "OnRemoteSaDied", DELAY_TIME * (i + 1), AppExecFwk::EventQueue::Priority::VIP);
    }
}

int32_t InputMethodController::DiscardTypingText()
{
    if (!IsBound()) {
        IMSA_HILOGE("not bound.");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return agent->DiscardTypingText();
}

int32_t InputMethodController::OnCursorUpdate(CursorInfo cursorInfo)
{
    if (!IsBound()) {
        IMSA_HILOGD("not bound.");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    if (!IsEditable()) {
        IMSA_HILOGD("not editable.");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    int32_t x = cursorInfo.left;
    int32_t y = cursorInfo.top;
    uint32_t windowId = 0;
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        windowId = textConfig_.windowId;
    }
    GetWindowScaleCoordinate(x, y, windowId);
    cursorInfo.left = x;
    cursorInfo.top = y;
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_.cursorInfo = cursorInfo;
    }
    {
        std::lock_guard<std::mutex> lk(cursorInfoMutex_);
        if (cursorInfo_ == cursorInfo) {
            IMSA_HILOGD("same to last update.");
            return ErrorCode::NO_ERROR;
        }
        cursorInfo_ = cursorInfo;
    }
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent is nullptr!");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    IMSA_HILOGI("left: %{public}d, top: %{public}d, height: %{public}d.", static_cast<int32_t>(cursorInfo.left),
        static_cast<int32_t>(cursorInfo.top), static_cast<int32_t>(cursorInfo.height));
    agent->OnCursorUpdate(cursorInfo.left, cursorInfo.top, cursorInfo.height);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::OnSelectionChange(std::u16string text, int start, int end)
{
    if (!IsBound()) {
        IMSA_HILOGD("not bound.");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    if (!IsEditable()) {
        IMSA_HILOGD("not editable.");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_.range.start = start;
        textConfig_.range.end = end;
    }
    if (isTextNotified_.exchange(true) && textString_ == text && selectNewBegin_ == start && selectNewEnd_ == end) {
        IMSA_HILOGD("same to last update.");
        return ErrorCode::NO_ERROR;
    }
    textString_ = text;
    selectOldBegin_ = selectNewBegin_;
    selectOldEnd_ = selectNewEnd_;
    selectNewBegin_ = start;
    selectNewEnd_ = end;
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent is nullptr!");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    IMSA_HILOGI("IMC size: %{public}zu, range: %{public}d/%{public}d/%{public}d/%{public}d.", text.size(),
        selectOldBegin_, selectOldEnd_, start, end);
    std::string testString = Str16ToStr8(textString_);
    agent->OnSelectionChange(testString, selectOldBegin_, selectOldEnd_, selectNewBegin_, selectNewEnd_);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::OnConfigurationChange(Configuration info)
{
    if (!IsBound()) {
        IMSA_HILOGD("not bound.");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    bool oldSecurityFlag = textConfig_.inputAttribute.GetSecurityFlag();
    InputAttribute attribute;
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_.inputAttribute.enterKeyType = static_cast<int32_t>(info.GetEnterKeyType());
        textConfig_.inputAttribute.inputPattern = static_cast<int32_t>(info.GetTextInputType());
        attribute = textConfig_.inputAttribute;
    }
    if (!IsEditable()) {
        IMSA_HILOGD("not editable.");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    IMSA_HILOGI(
        "IMC enterKeyType: %{public}d, textInputType: %{public}d.", attribute.enterKeyType, attribute.inputPattern);
    if (oldSecurityFlag != attribute.GetSecurityFlag()) {
        GetTextConfig(clientInfo_.config);
        sptr<IRemoteObject> agent = nullptr;
        std::pair<int64_t, std::string> imeInfo{ 0, "" };
        int32_t ret = StartInput(clientInfo_, agent, imeInfo);
        if (ret != ErrorCode::NO_ERROR) {
            return ret;
        }
        OnInputReady(agent, imeInfo);
    }
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent is nullptr!");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    InputAttributeInner inner = InputMethodTools::GetInstance().AttributeToInner(attribute);
    agent->OnAttributeChange(inner);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::GetLeft(int32_t length, std::u16string &text)
{
    InputMethodSyncTrace tracer("IMC_GetForward");
    IMSA_HILOGD("start, length: %{public}d", length);
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or listener is nullptr!");
        ReportBaseTextOperation(static_cast<int32_t>(IInputDataChannelIpcCode::COMMAND_GET_TEXT_BEFORE_CURSOR),
            ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    {
        InputMethodSyncTrace aceTracer("ACE_GetForward");
        text = listener->GetLeftTextOfCursor(length);
    }
    PrintLogIfAceTimeout(start);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::GetRight(int32_t length, std::u16string &text)
{
    IMSA_HILOGD("start, length: %{public}d", length);
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener_ is nullptr!");
        ReportBaseTextOperation(static_cast<int32_t>(IInputDataChannelIpcCode::COMMAND_GET_TEXT_AFTER_CURSOR),
            ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    text = listener->GetRightTextOfCursor(length);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::GetTextIndexAtCursor(int32_t &index)
{
    IMSA_HILOGD("start.");
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener_ is nullptr!");
        ReportBaseTextOperation(static_cast<int32_t>(IInputDataChannelIpcCode::COMMAND_GET_TEXT_INDEX_AT_CURSOR),
            ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    index = listener->GetTextIndexAtCursor();
    return ErrorCode::NO_ERROR;
}

void InputMethodController::PrintKeyEventLog()
{
    std::lock_guard<std::mutex> lock(logLock_);
    auto now = system_clock::now();
    if (keyEventCountInPeriod_ == 0) {
        startLogTime_ = now;
    }
    keyEventCountInPeriod_++;
    if (std::chrono::duration_cast<seconds>(now - startLogTime_).count() >= LOG_MAX_TIME) {
        auto start = std::chrono::duration_cast<seconds>(startLogTime_.time_since_epoch()).count();
        auto end = std::chrono::duration_cast<seconds>(now.time_since_epoch()).count();
        IMSA_HILOGI("KeyEventCountInPeriod: %{public}d, startTime: %{public}lld, endTime: %{public}lld",
            keyEventCountInPeriod_, start, end);
        keyEventCountInPeriod_ = 0;
    }
}

int32_t InputMethodController::DispatchKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent, KeyEventCallback callback)
{
    PrintKeyEventLog();
    KeyEventInfo keyEventInfo = { std::chrono::system_clock::now(), keyEvent };
    keyEventQueue_.Push(keyEventInfo);
    InputMethodSyncTrace tracer("DispatchKeyEvent trace");
    keyEventQueue_.Wait(keyEventInfo);
    if (!IsEditable()) {
        IMSA_HILOGD("not editable.");
        keyEventQueue_.Pop();
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    if (keyEvent == nullptr) {
        IMSA_HILOGE("keyEvent is nullptr!");
        keyEventQueue_.Pop();
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent is nullptr!");
        keyEventQueue_.Pop();
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    IMSA_HILOGD("start.");
    sptr<IKeyEventConsumer> consumer = new (std::nothrow) KeyEventConsumerServiceImpl(callback, keyEvent);
    if (consumer == nullptr) {
        IMSA_HILOGE("consumer is nullptr!");
        keyEventQueue_.Pop();
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    KeyEventValue keyEventValue;
    keyEventValue.event = keyEvent;
    auto ret = agent->DispatchKeyEvent(keyEventValue, consumer);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to DispatchKeyEvent: %{public}d", ret);
    }
    keyEventQueue_.Pop();
    return ret;
}

int32_t InputMethodController::GetEnterKeyType(int32_t &keyType)
{
    IMSA_HILOGD("InputMethodController::GetEnterKeyType start.");
    std::lock_guard<std::mutex> lock(textConfigLock_);
    keyType = textConfig_.inputAttribute.enterKeyType;
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::GetInputPattern(int32_t &inputpattern)
{
    IMSA_HILOGD("InputMethodController::GetInputPattern start.");
    std::lock_guard<std::mutex> lock(textConfigLock_);
    inputpattern = textConfig_.inputAttribute.inputPattern;
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::GetTextConfig(TextTotalConfig &config)
{
    std::lock_guard<std::mutex> lock(textConfigLock_);
    config.inputAttribute = textConfig_.inputAttribute;
    config.cursorInfo = textConfig_.cursorInfo;
    config.windowId = textConfig_.windowId;
    config.positionY = textConfig_.positionY;
    config.height = textConfig_.height;
    config.privateCommand = textConfig_.privateCommand;
    config.abilityToken = textConfig_.abilityToken;
    if (textConfig_.range.start == INVALID_VALUE) {
        IMSA_HILOGD("SelectionRange is invalid.");
    } else {
        {
            std::lock_guard<std::mutex> editorLock(editorContentLock_);
            config.textSelection.oldBegin = selectOldBegin_;
            config.textSelection.oldEnd = selectOldEnd_;
        }
        config.textSelection.newBegin = textConfig_.range.start;
        config.textSelection.newEnd = textConfig_.range.end;
    }
    IMSA_HILOGD("textConfig: %{public}s.", config.ToString().c_str());
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::SetCallingWindow(uint32_t windowId)
{
    if (!IsBound()) {
        IMSA_HILOGD("not bound.");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    if (!IsEditable()) {
        IMSA_HILOGD("not editable.");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_.windowId = windowId;
    }
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent is nullptr!");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    IMSA_HILOGI("windowId: %{public}d.", windowId);
    agent->SetCallingWindow(windowId);
    auto proxy = GetSystemAbilityProxy();
    if (proxy != nullptr) {
        proxy->SetCallingWindow(windowId, clientInfo_.client);
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::ShowSoftKeyboardInner(ClientType type)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    IMSA_HILOGI("start.");
    {
        std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
        clientInfo_.isShowKeyboard = true;
    }
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_SHOW_NORMAL);
    return proxy->ShowCurrentInput(type);
}

int32_t InputMethodController::HideSoftKeyboard()
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGI("start.");
    {
        std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
        clientInfo_.isShowKeyboard = false;
    }
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_NORMAL);
    return proxy->HideCurrentInput();
}

int32_t InputMethodController::StopInputSession()
{
    IMSA_HILOGI("start.");
    isEditable_.store(false);
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return proxy->StopInputSession();
}

int32_t InputMethodController::ShowOptionalInputMethod()
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGI("IMC start.");
    return proxy->DisplayOptionalInputMethod();
}

int32_t InputMethodController::ListInputMethodSubtype(const Property &property, std::vector<SubProperty> &subProps)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGD("ime bundleName: %{public}s.", property.name.c_str());
    return proxy->ListInputMethodSubtype(property.name, subProps);
}

int32_t InputMethodController::ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProps)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGD("start.");
    return proxy->ListCurrentInputMethodSubtype(subProps);
}

int32_t InputMethodController::SwitchInputMethod(
    SwitchTrigger trigger, const std::string &name, const std::string &subName)
{
    InputMethodSyncTrace tracer("IMC_SwitchInputMethod");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGI("name: %{public}s, subName: %{public}s, trigger: %{public}d.", name.c_str(), subName.c_str(),
        static_cast<uint32_t>(trigger));
        return proxy->SwitchInputMethod(name, subName, static_cast<uint32_t>(trigger));
}

void InputMethodController::OnInputReady(
    sptr<IRemoteObject> agentObject, const std::pair<int64_t, std::string> &imeInfo)
{
    IMSA_HILOGI("InputMethodController start.");
    SetBindImeInfo(imeInfo);
    isBound_.store(true);
    isEditable_.store(true);
    if (agentObject == nullptr) {
        IMSA_HILOGE("agentObject is nullptr!");
        return;
    }
    SetAgent(agentObject);
}

void InputMethodController::OnInputStop(bool isStopInactiveClient)
{
    {
        std::lock_guard<std::mutex> autoLock(agentLock_);
        agent_ = nullptr;
        agentObject_ = nullptr;
    }
    auto listener = GetTextListener();
    if (listener != nullptr) {
        IMSA_HILOGD("listener is not nullptr!");
        if (textConfig_.inputAttribute.isTextPreviewSupported) {
            IMSA_HILOGD("finish text preview.");
            listener->FinishTextPreview();
        }
        if (!isStopInactiveClient || !listener->IsFromTs()) {
            listener->SendKeyboardStatus(KeyboardStatus::HIDE);
        }
    }
    isBound_.store(false);
    isEditable_.store(false);
    isTextNotified_.store(false);
    textString_ = Str8ToStr16("");
    selectOldBegin_ = INVALID_VALUE;
    selectOldEnd_ = INVALID_VALUE;
    selectNewBegin_ = INVALID_VALUE;
    selectNewEnd_ = INVALID_VALUE;
}

void InputMethodController::ClearEditorCache(bool isNewEditor, sptr<OnTextChangedListener> lastListener)
{
    IMSA_HILOGD("isNewEditor: %{public}d.", isNewEditor);
    if (isNewEditor && isBound_.load() && lastListener != nullptr &&
        textConfig_.inputAttribute.isTextPreviewSupported) {
        IMSA_HILOGD("last editor FinishTextPreview");
        lastListener->FinishTextPreview();
    }
    {
        std::lock_guard<std::mutex> lock(editorContentLock_);
        // reset old range when editor changes or first attach
        if (isNewEditor || !isBound_.load()) {
            isTextNotified_.store(false);
            textString_ = Str8ToStr16("");
            selectOldBegin_ = INVALID_VALUE;
            selectOldEnd_ = INVALID_VALUE;
            selectNewBegin_ = INVALID_VALUE;
            selectNewEnd_ = INVALID_VALUE;
        }
    }
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_ = {};
    }
    {
        std::lock_guard<std::mutex> lock(cursorInfoMutex_);
        cursorInfo_ = {};
    }
    clientInfo_.config = {};
}

void InputMethodController::SelectByRange(int32_t start, int32_t end)
{
    IMSA_HILOGD("InputMethodController start: %{public}d, end: %{public}d.", start, end);
    auto listener = GetTextListener();
    if (IsEditable() && listener != nullptr) {
        listener->HandleSetSelection(start, end);
    } else {
        IMSA_HILOGE("not editable or textListener_ is nullptr!");
    }

    if (controllerListener_ != nullptr) {
        controllerListener_->OnSelectByRange(start, end);
    } else {
        IMSA_HILOGE("controllerListener_ is nullptr!");
    }
}

void InputMethodController::SelectByMovement(int32_t direction, int32_t cursorMoveSkip)
{
    IMSA_HILOGD(
        "InputMethodController start, direction: %{public}d, cursorMoveSkip: %{public}d", direction, cursorMoveSkip);
    auto listener = GetTextListener();
    if (IsEditable() && listener != nullptr) {
        listener->HandleSelect(CURSOR_DIRECTION_BASE_VALUE + direction, cursorMoveSkip);
    } else {
        IMSA_HILOGE("not editable or textListener_ is nullptr!");
    }

    if (controllerListener_ != nullptr) {
        controllerListener_->OnSelectByMovement(direction);
    } else {
        IMSA_HILOGE("controllerListener_ is nullptr!");
    }
}

int32_t InputMethodController::HandleExtendAction(int32_t action)
{
    IMSA_HILOGD("InputMethodController start, action: %{public}d.", action);
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    listener->HandleExtendAction(action);
    return ErrorCode::NO_ERROR;
}

sptr<OnTextChangedListener> InputMethodController::GetTextListener()
{
    std::lock_guard<std::mutex> lock(textListenerLock_);
    return textListener_;
}

void InputMethodController::SetTextListener(sptr<OnTextChangedListener> listener)
{
    std::lock_guard<std::mutex> lock(textListenerLock_);
    textListener_ = listener;
}

bool InputMethodController::IsEditable()
{
    std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
    if (clientInfo_.state != ClientState::ACTIVE) {
        IMSA_HILOGD("client is not active.");
        return false;
    }
    if (!isEditable_.load()) {
        IMSA_HILOGD("not in editable state.");
        return false;
    }
    return true;
}

bool InputMethodController::IsBound()
{
    std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
    if (clientInfo_.state != ClientState::ACTIVE) {
        IMSA_HILOGD("client is not active.");
        return false;
    }
    if (!isBound_.load()) {
        IMSA_HILOGD("not bound.");
        return false;
    }
    return true;
}

int32_t InputMethodController::InsertText(const std::u16string &text)
{
    InputMethodSyncTrace tracer("IMC_InsertText");
    IMSA_HILOGD("start.");
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener is nullptr!");
        ReportBaseTextOperation(static_cast<int32_t>(IInputDataChannelIpcCode::COMMAND_INSERT_TEXT),
            ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    {
        InputMethodSyncTrace aceTracer("ACE_InsertText");
        IMSA_HILOGD("ACE InsertText.");
        listener->InsertText(text);
    }

    PrintLogIfAceTimeout(start);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::DeleteForward(int32_t length)
{
    InputMethodSyncTrace tracer("IMC_DeleteForward");
    IMSA_HILOGD("start, length: %{public}d.", length);
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener is nullptr!");
        ReportBaseTextOperation(static_cast<int32_t>(IInputDataChannelIpcCode::COMMAND_DELETE_FORWARD),
            ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    {
        InputMethodSyncTrace aceTracer("ACE_DeleteForward");
        // reverse for compatibility
        listener->DeleteBackward(length);
    }
    PrintLogIfAceTimeout(start);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::DeleteBackward(int32_t length)
{
    IMSA_HILOGD("InputMethodController start, length: %{public}d.", length);
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener is nullptr!");
        ReportBaseTextOperation(static_cast<int32_t>(IInputDataChannelIpcCode::COMMAND_DELETE_BACKWARD),
            ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    // reverse for compatibility
    listener->DeleteForward(length);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::MoveCursor(Direction direction)
{
    IMSA_HILOGD("InputMethodController start, direction: %{public}d.", static_cast<int32_t>(direction));
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener_ is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    listener->MoveCursor(direction);
    return ErrorCode::NO_ERROR;
}

void InputMethodController::SendKeyboardStatus(KeyboardStatus status)
{
    IMSA_HILOGD("InputMethodController status: %{public}d.", static_cast<int32_t>(status));
    auto listener = GetTextListener();
    if (listener == nullptr) {
        IMSA_HILOGE("listener is nullptr!");
        return;
    }
    listener->SendKeyboardStatus(status);
    if (status == KeyboardStatus::HIDE) {
        std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
        clientInfo_.isShowKeyboard = false;
    }
}

void InputMethodController::NotifyPanelStatusInfo(const PanelStatusInfo &info)
{
    IMSA_HILOGD("InputMethodController start, type: %{public}d, flag: %{public}d, visible: %{public}d, trigger: "
                "%{public}d, sessionId: %{public}u.",
        static_cast<PanelType>(info.panelInfo.panelType), static_cast<PanelFlag>(info.panelInfo.panelFlag),
        info.visible, static_cast<Trigger>(info.trigger), info.sessionId);
    auto listener = GetTextListener();
    if (listener == nullptr) {
        IMSA_HILOGE("listener is nullptr!");
        return;
    }
    if (info.panelInfo.panelType == PanelType::SOFT_KEYBOARD) {
        info.visible ? SendKeyboardStatus(KeyboardStatus::SHOW) : SendKeyboardStatus(KeyboardStatus::HIDE);
    }

    if (info.visible || info.sessionId == 0 || info.sessionId == sessionId_) {
        listener->NotifyPanelStatusInfo(info);
    }
    if (info.panelInfo.panelType == PanelType::SOFT_KEYBOARD &&
        info.panelInfo.panelFlag != PanelFlag::FLG_CANDIDATE_COLUMN && !info.visible) {
        std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
        clientInfo_.isShowKeyboard = false;
    }
}

void InputMethodController::NotifyKeyboardHeight(uint32_t height)
{
    IMSA_HILOGD("InputMethodController start, height: %{public}u.", height);
    auto listener = GetTextListener();
    if (listener == nullptr) {
        IMSA_HILOGE("listener is nullptr!");
        return;
    }
    listener->NotifyKeyboardHeight(height);
}

int32_t InputMethodController::SendFunctionKey(int32_t functionKey)
{
    IMSA_HILOGD("InputMethodController start, functionKey: %{public}d", static_cast<int32_t>(functionKey));
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or listener is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    FunctionKey funcKey;
    funcKey.SetEnterKeyType(static_cast<EnterKeyType>(functionKey));
    listener->SendFunctionKey(funcKey);
    return ErrorCode::NO_ERROR;
}

bool InputMethodController::IsInputTypeSupported(InputType type)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    IMSA_HILOGI("type: %{public}d.", static_cast<int32_t>(type));
    bool ret = false;
    proxy->IsInputTypeSupported(static_cast<int32_t>(type), ret);
    return ret;
}

bool InputMethodController::IsCurrentImeByPid(int32_t pid)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return false;
    }
    bool ret = false;
    proxy->IsCurrentImeByPid(pid, ret);
    return ret;
}

int32_t InputMethodController::StartInputType(InputType type)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    IMSA_HILOGI("type: %{public}d.", static_cast<int32_t>(type));
    return proxy->StartInputType(static_cast<int32_t>(type));
}

int32_t InputMethodController::IsPanelShown(const PanelInfo &panelInfo, bool &isShown)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    IMSA_HILOGD("type: %{public}d, flag: %{public}d.", static_cast<int32_t>(panelInfo.panelType),
        static_cast<int32_t>(panelInfo.panelFlag));
    return proxy->IsPanelShown(panelInfo, isShown);
}

void InputMethodController::SetAgent(sptr<IRemoteObject> &agentObject)
{
    std::lock_guard<std::mutex> autoLock(agentLock_);
    if (agent_ != nullptr && agentObject_.GetRefPtr() == agentObject.GetRefPtr()) {
        IMSA_HILOGD("agent has already been set.");
        return;
    }
    agent_ = std::make_shared<InputMethodAgentProxy>(agentObject);
    agentObject_ = agentObject;
}

std::shared_ptr<IInputMethodAgent> InputMethodController::GetAgent()
{
    std::lock_guard<std::mutex> autoLock(agentLock_);
    return agent_;
}

void InputMethodController::PrintLogIfAceTimeout(int64_t start)
{
    int64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    if (end - start > ACE_DEAL_TIME_OUT) {
        IMSA_HILOGW("timeout: [%{public}" PRId64 ", %{public}" PRId64 "].", start, end);
    }
}

int32_t InputMethodController::ReceivePrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    auto listener = GetTextListener();
    if (listener == nullptr) {
        IMSA_HILOGE("listener is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGD("IMC in.");
    auto ret = listener->ReceivePrivateCommand(privateCommand);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("ReceivePrivateCommand err, ret: %{public}d!", ret);
        return ErrorCode::ERROR_TEXT_LISTENER_ERROR;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::SendPrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    if (!IsBound()) {
        IMSA_HILOGD("not bound.");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    if (!IsEditable()) {
        IMSA_HILOGD("not editable.");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    if (!TextConfig::IsPrivateCommandValid(privateCommand)) {
        IMSA_HILOGE("invalid private command size!");
        return ErrorCode::ERROR_INVALID_PRIVATE_COMMAND_SIZE;
    }
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent is nullptr!");
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    IMSA_HILOGD("IMC start.");
    Value value(privateCommand);
    return agent->SendPrivateCommand(value);
}

void InputMethodController::SetBindImeInfo(const std::pair<int64_t, std::string> &imeInfo)
{
    std::lock_guard<std::mutex> lock(bindImeInfoLock_);
    bindImeInfo_ = imeInfo;
}

std::pair<int64_t, std::string> InputMethodController::GetBindImeInfo()
{
    std::lock_guard<std::mutex> lock(bindImeInfoLock_);
    return bindImeInfo_;
}

int32_t InputMethodController::SetPreviewTextInner(const std::string &text, const Range &range)
{
    InputMethodSyncTrace tracer("IMC_SetPreviewText");
    IMSA_HILOGD("IMC start.");
    if (!textConfig_.inputAttribute.isTextPreviewSupported) {
        IMSA_HILOGE("text preview do not supported!");
        return ErrorCode::ERROR_TEXT_PREVIEW_NOT_SUPPORTED;
    }
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or listener is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    int32_t ret = 0;
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    {
        InputMethodSyncTrace aceTracer("ACE_SetPreviewText");
        ret = listener->SetPreviewText(Str8ToStr16(text), range);
    }
    PrintLogIfAceTimeout(start);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to SetPreviewText: %{public}d!", ret);
        return ret == -1 ? ErrorCode::ERROR_INVALID_RANGE : ErrorCode::ERROR_TEXT_LISTENER_ERROR;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::FinishTextPreview()
{
    InputMethodSyncTrace tracer("IMC_FinishTextPreview");
    IMSA_HILOGD("IMC start.");
    if (!textConfig_.inputAttribute.isTextPreviewSupported) {
        IMSA_HILOGD("text preview do not supported!");
        ReportBaseTextOperation(static_cast<int32_t>(IInputDataChannelIpcCode::COMMAND_FINISH_TEXT_PREVIEW),
            ErrorCode::ERROR_TEXT_PREVIEW_NOT_SUPPORTED);
        return ErrorCode::ERROR_TEXT_PREVIEW_NOT_SUPPORTED;
    }
    auto listener = GetTextListener();
    if (!isBound_.load() || listener == nullptr) {
        IMSA_HILOGW("not bound or listener is nullptr!");
        ReportBaseTextOperation(static_cast<int32_t>(IInputDataChannelIpcCode::COMMAND_FINISH_TEXT_PREVIEW),
            ErrorCode::ERROR_CLIENT_NOT_EDITABLE);
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    {
        InputMethodSyncTrace aceTracer("ACE_FinishTextPreview");
        listener->FinishTextPreview();
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::SendMessage(const ArrayBuffer &arrayBuffer)
{
    if (!IsBound()) {
        IMSA_HILOGE("not bound.");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    if (!IsEditable()) {
        IMSA_HILOGE("not editable.");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    if (!ArrayBuffer::IsSizeValid(arrayBuffer)) {
        IMSA_HILOGE("arrayBuffer size is invalid!");
        return ErrorCode::ERROR_INVALID_ARRAY_BUFFER_SIZE;
    }
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return agent->SendMessage(arrayBuffer);
}

int32_t InputMethodController::RecvMessage(const ArrayBuffer &arrayBuffer)
{
    if (!IsBound()) {
        IMSA_HILOGE("not bound.");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    if (!IsEditable()) {
        IMSA_HILOGE("not editable.");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    auto msgHandlerCallback = GetMsgHandlerCallback();
    if (msgHandlerCallback == nullptr) {
        IMSA_HILOGW("Message handler was not regist!");
        return ErrorCode::ERROR_MSG_HANDLER_NOT_REGIST;
    }
    return msgHandlerCallback->OnMessage(arrayBuffer);
}

int32_t InputMethodController::RegisterMsgHandler(const std::shared_ptr<MsgHandlerCallbackInterface> &msgHandler)
{
    IMSA_HILOGI("isRegist: %{public}d", msgHandler != nullptr);
    std::shared_ptr<MsgHandlerCallbackInterface> exMsgHandler = nullptr;
    {
        std::lock_guard<decltype(msgHandlerMutex_)> lock(msgHandlerMutex_);
        exMsgHandler = msgHandler_;
        msgHandler_ = msgHandler;
    }
    if (exMsgHandler != nullptr) {
        IMSA_HILOGI("Trigger exMessageHandler OnTerminated.");
        exMsgHandler->OnTerminated();
    }
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<MsgHandlerCallbackInterface> InputMethodController::GetMsgHandlerCallback()
{
    std::lock_guard<decltype(msgHandlerMutex_)> lock(msgHandlerMutex_);
    return msgHandler_;
}

int32_t InputMethodController::GetInputMethodState(EnabledStatus &state)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    int32_t stateData = 0;
    int32_t ret = proxy->GetInputMethodState(stateData);
    state = static_cast<EnabledStatus>(stateData);
    return ret;
}

int32_t InputMethodController::SetPreviewText(const std::string &text, const Range &range)
{
    auto ret = SetPreviewTextInner(text, range);
    ReportBaseTextOperation(static_cast<int32_t>(IInputDataChannelIpcCode::COMMAND_SET_PREVIEW_TEXT), ret);
    return ret;
}

int32_t InputMethodController::ShowTextInput(ClientType type)
{
    AttachOptions attachOptions;
    return ShowTextInput(attachOptions, type);
}

int32_t InputMethodController::ShowTextInput(const AttachOptions &attachOptions, ClientType type)
{
    auto ret = ShowTextInputInner(attachOptions, type);
    ReportClientShow(static_cast<int32_t>(IInputMethodSystemAbilityIpcCode::COMMAND_SHOW_INPUT), ret, type);
    return ret;
}

int32_t InputMethodController::ShowSoftKeyboard(ClientType type)
{
    auto ret = ShowSoftKeyboardInner(type);
    ReportClientShow(static_cast<int32_t>(IInputMethodSystemAbilityIpcCode::COMMAND_SHOW_CURRENT_INPUT), ret, type);
    return ret;
}

void InputMethodController::ReportClientShow(int32_t eventCode, int32_t errCode, ClientType type)
{
    auto evenInfo =
        HiSysOriginalInfo::Builder().SetClientType(type).SetEventCode(eventCode).SetErrCode(errCode).Build();
    ImcHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_SHOW, *evenInfo);
}

void InputMethodController::ReportBaseTextOperation(int32_t eventCode, int32_t errCode)
{
    auto imeInfo = GetBindImeInfo();
    auto evenInfo = HiSysOriginalInfo::Builder()
                        .SetEventCode(eventCode)
                        .SetErrCode(errCode)
                        .SetPeerName(imeInfo.second)
                        .SetPeerPid(imeInfo.first)
                        .SetClientType(clientInfo_.type)
                        .Build();
    ImcHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::BASE_TEXT_OPERATOR, *evenInfo);
}

void InputMethodController::UpdateTextPreviewState(bool isSupport)
{
    if (textConfig_.inputAttribute.isTextPreviewSupported == isSupport) {
        return;
    }
    textConfig_.inputAttribute.isTextPreviewSupported = isSupport;
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent is nullptr!");
        return;
    }
    InputAttributeInner inner = InputMethodTools::GetInstance().AttributeToInner(textConfig_.inputAttribute);
    agent->OnAttributeChange(inner);
}

int32_t InputMethodController::SendPrivateData(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    Value value(privateCommand);
    return proxy->SendPrivateData(value);
}

int32_t InputMethodController::RegisterWindowScaleCallbackHandler(WindowScaleCallback&& callback)
{
    IMSA_HILOGD("isRegister: %{public}d", callback != nullptr);
    std::lock_guard<std::mutex> lock(windowScaleCallbackMutex_);
    windowScaleCallback_ = std::move(callback);
    return static_cast<int32_t>(ErrorCode::NO_ERROR);
}

void InputMethodController::GetWindowScaleCoordinate(int32_t& x, int32_t& y, uint32_t windowId)
{
    WindowScaleCallback handler = nullptr;
    {
        std::lock_guard<std::mutex> lock(windowScaleCallbackMutex_);
        handler = windowScaleCallback_;
    }
    if (handler == nullptr) {
        IMSA_HILOGD("handler is nullptr");
        return;
    }
    handler(x, y, windowId);
}
} // namespace MiscServices
} // namespace OHOS
