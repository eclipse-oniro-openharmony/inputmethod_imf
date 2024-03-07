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

#include "block_data.h"
#include "global.h"
#include "input_client_stub.h"
#include "input_data_channel_stub.h"
#include "input_method_agent_proxy.h"
#include "input_method_property.h"
#include "input_method_status.h"
#include "input_method_system_ability_proxy.h"
#include "inputmethod_sysevent.h"
#include "inputmethod_trace.h"
#include "iservice_registry.h"
#include "keyevent_consumer_stub.h"
#include "string_ex.h"
#include "sys/prctl.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
using namespace std::chrono;
sptr<InputMethodController> InputMethodController::instance_;
std::shared_ptr<AppExecFwk::EventHandler> InputMethodController::handler_{ nullptr };
std::mutex InputMethodController::instanceLock_;
constexpr int32_t LOOP_COUNT = 5;
constexpr int64_t DELAY_TIME = 100;
constexpr int32_t ACE_DEAL_TIME_OUT = 200;
const std::unordered_map<std::string, EventType> EVENT_TYPE{ { "imeChange", IME_CHANGE }, { "imeShow", IME_SHOW },
    { "imeHide", IME_HIDE } };
InputMethodController::InputMethodController()
{
    IMSA_HILOGD("IMC structure");
}

InputMethodController::~InputMethodController()
{
}

sptr<InputMethodController> InputMethodController::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            IMSA_HILOGD("IMC instance_ is nullptr");
            instance_ = new (std::nothrow) InputMethodController();
            if (instance_ == nullptr) {
                IMSA_HILOGE("failed to create InputMethodController");
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

void InputMethodController::SetSettingListener(std::shared_ptr<InputMethodSettingListener> listener)
{
    settingListener_ = std::move(listener);
}

int32_t InputMethodController::RestoreListenEventFlag()
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    return proxy->UpdateListenEventFlag(clientInfo_, IME_NONE);
}

int32_t InputMethodController::UpdateListenEventFlag(const std::string &type, bool isOn)
{
    auto it = EVENT_TYPE.find(type);
    if (it == EVENT_TYPE.end()) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto eventType = it->second;
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
    auto oldEventFlag = clientInfo_.eventFlag;
    UpdateNativeEventFlag(eventType, isOn);
    auto ret = proxy->UpdateListenEventFlag(clientInfo_, eventType);
    if (ret != ErrorCode::NO_ERROR && isOn) {
        clientInfo_.eventFlag = oldEventFlag;
    }
    return ret;
}

void InputMethodController::UpdateNativeEventFlag(EventType eventType, bool isOn)
{
    uint32_t currentEvent = isOn ? 1u << eventType : ~(1u << eventType);
    clientInfo_.eventFlag = isOn ? clientInfo_.eventFlag | currentEvent : clientInfo_.eventFlag & currentEvent;
}

void InputMethodController::SetControllerListener(std::shared_ptr<ControllerListener> controllerListener)
{
    IMSA_HILOGD("InputMethodController run in");
    controllerListener_ = std::move(controllerListener);
}

int32_t InputMethodController::Initialize()
{
    auto client = new (std::nothrow) InputClientStub();
    if (client == nullptr) {
        IMSA_HILOGE("failed to new client");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto channel = new (std::nothrow) InputDataChannelStub();
    if (channel == nullptr) {
        delete client;
        IMSA_HILOGE("failed to new channel");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    InputAttribute attribute = { .inputPattern = InputAttribute::PATTERN_TEXT };
    clientInfo_ = { .attribute = attribute, .client = client, .channel = channel };

    // make AppExecFwk::EventHandler handler
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    return ErrorCode::NO_ERROR;
}

sptr<IInputMethodSystemAbility> InputMethodController::GetSystemAbilityProxy()
{
    std::lock_guard<std::mutex> lock(abilityLock_);
    if (abilityManager_ != nullptr) {
        return abilityManager_;
    }
    IMSA_HILOGI("get input method service proxy");
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGE("system ability manager is nullptr");
        return nullptr;
    }
    auto systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
    if (systemAbility == nullptr) {
        IMSA_HILOGE("system ability is nullptr");
        return nullptr;
    }
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new (std::nothrow) InputDeathRecipient();
        if (deathRecipient_ == nullptr) {
            IMSA_HILOGE("new death recipient failed");
            return nullptr;
        }
    }
    deathRecipient_->SetDeathRecipient([this](const wptr<IRemoteObject> &remote) { OnRemoteSaDied(remote); });
    if ((systemAbility->IsProxyObject()) && (!systemAbility->AddDeathRecipient(deathRecipient_))) {
        IMSA_HILOGE("failed to add death recipient.");
        return nullptr;
    }
    abilityManager_ = iface_cast<IInputMethodSystemAbility>(systemAbility);
    return abilityManager_;
}

int32_t InputMethodController::OnSwitchInput(const Property &property, const SubProperty &subProperty)
{
    if (settingListener_ == nullptr) {
        IMSA_HILOGE("imeListener_ is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    settingListener_->OnImeChange(property, subProperty);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::OnPanelStatusChange(
    const InputWindowStatus &status, const std::vector<InputWindowInfo> &windowInfo)
{
    if (settingListener_ == nullptr) {
        IMSA_HILOGE("imeListener_ is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    settingListener_->OnPanelStatusChange(status, windowInfo);
    return ErrorCode::NO_ERROR;
}

void InputMethodController::DeactivateClient()
{
    {
        std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
        clientInfo_.state = ClientState::INACTIVE;
    }
    {
        std::lock_guard<std::mutex> autoLock(agentLock_);
        agent_ = nullptr;
        agentObject_ = nullptr;
    }
    auto listener = GetTextListener();
    if (listener != nullptr) {
        IMSA_HILOGD("textListener_ is not nullptr");
        listener->SendKeyboardStatus(KeyboardStatus::NONE);
    }
}

void InputMethodController::SaveTextConfig(const TextConfig &textConfig)
{
    std::lock_guard<std::mutex> lock(textConfigLock_);
    IMSA_HILOGD("inputPattern: %{public}d, enterKeyType: %{public}d, windowId: %{public}d",
        textConfig.inputAttribute.inputPattern, textConfig.inputAttribute.enterKeyType, textConfig.windowId);
    textConfig_ = textConfig;
}

int32_t InputMethodController::Attach(sptr<OnTextChangedListener> &listener)
{
    return Attach(listener, true);
}

int32_t InputMethodController::Attach(sptr<OnTextChangedListener> &listener, bool isShowKeyboard)
{
    InputAttribute attribute;
    attribute.inputPattern = InputAttribute::PATTERN_TEXT;
    return Attach(listener, isShowKeyboard, attribute);
}

int32_t InputMethodController::Attach(
    sptr<OnTextChangedListener> &listener, bool isShowKeyboard, const InputAttribute &attribute)
{
    InputMethodSyncTrace tracer("InputMethodController Attach trace.");
    TextConfig textConfig;
    textConfig.inputAttribute = attribute;
    return Attach(listener, isShowKeyboard, textConfig);
}

int32_t InputMethodController::Attach(
    sptr<OnTextChangedListener> &listener, bool isShowKeyboard, const TextConfig &textConfig)
{
    IMSA_HILOGI("isShowKeyboard %{public}d", isShowKeyboard);
    ClearEditorCache();
    InputMethodSyncTrace tracer("InputMethodController Attach with textConfig trace.");
    clientInfo_.isNotifyInputStart = GetTextListener() != listener;
    SetTextListener(listener);
    clientInfo_.isShowKeyboard = isShowKeyboard;
    SaveTextConfig(textConfig);
    GetTextConfig(clientInfo_.config);

    sptr<IRemoteObject> agent = nullptr;
    int32_t ret = StartInput(clientInfo_, agent);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to start input, ret:%{public}d", ret);
        return ret;
    }
    clientInfo_.state = ClientState::ACTIVE;
    OnInputReady(agent);
    if (isShowKeyboard) {
        InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_SHOW_ATTACH);
    }
    IMSA_HILOGI("bind imf successfully");
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::ShowTextInput()
{
    if (!IsBound()) {
        IMSA_HILOGE("not bound");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    IMSA_HILOGI("run in");
    clientInfo_.isShowKeyboard = true;
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_SHOW_ENEDITABLE);
    int32_t ret = ShowInput(clientInfo_.client);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to start input, ret: %{public}d", ret);
        return ret;
    }
    isEditable_.store(true);
    IMSA_HILOGI("enter editable state");
    return ret;
}

int32_t InputMethodController::HideTextInput()
{
    if (!IsBound()) {
        IMSA_HILOGE("not bound");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    IMSA_HILOGI("run in");
    isEditable_.store(false);
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_UNEDITABLE);
    return HideInput(clientInfo_.client);
}

int32_t InputMethodController::HideCurrentInput()
{
    IMSA_HILOGD("InputMethodController::HideCurrentInput");
    if (!IsEditable()) {
        IMSA_HILOGD("not editable");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    clientInfo_.isShowKeyboard = false;
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_NORMAL);
    return proxy->HideCurrentInputDeprecated();
}

int32_t InputMethodController::ShowCurrentInput()
{
    if (!IsEditable()) {
        IMSA_HILOGD("not editable");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGI("run in");
    clientInfo_.isShowKeyboard = true;
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_SHOW_NORMAL);
    return proxy->ShowCurrentInputDeprecated();
}

int32_t InputMethodController::Close()
{
    IMSA_HILOGI("run in");
    bool isReportHide = clientInfo_.isShowKeyboard;
    InputMethodSyncTrace tracer("InputMethodController Close trace.");
    isReportHide ? InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_UNBIND)
                 : InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_UNBIND);
    return ReleaseInput(clientInfo_.client);
}

int32_t InputMethodController::RequestShowInput()
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGI("InputMethodController, run in");
    return proxy->RequestShowInput();
}

int32_t InputMethodController::RequestHideInput()
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGI("InputMethodController, run in");
    return proxy->RequestHideInput();
}

int32_t InputMethodController::DisplayOptionalInputMethod()
{
    IMSA_HILOGD("InputMethodController::DisplayOptionalInputMethod");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return proxy->DisplayOptionalInputMethod();
}

bool InputMethodController::WasAttached()
{
    return isBound_.load();
}

int32_t InputMethodController::ListInputMethodCommon(InputMethodStatus status, std::vector<Property> &props)
{
    IMSA_HILOGD("InputMethodController::ListInputMethodCommon");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return proxy->ListInputMethod(status, props);
}

int32_t InputMethodController::ListInputMethod(std::vector<Property> &props)
{
    IMSA_HILOGD("InputMethodController::listInputMethod");
    return ListInputMethodCommon(ALL, props);
}

int32_t InputMethodController::ListInputMethod(bool enable, std::vector<Property> &props)
{
    IMSA_HILOGI("enable = %{public}s", enable ? "ENABLE" : "DISABLE");
    return ListInputMethodCommon(enable ? ENABLE : DISABLE, props);
}

int32_t InputMethodController::GetDefaultInputMethod(std::shared_ptr<Property> &property)
{
    IMSA_HILOGD("InputMethodController::GetDefaultInputMethod");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    return proxy->GetDefaultInputMethod(property);
}

int32_t InputMethodController::GetInputMethodConfig(OHOS::AppExecFwk::ElementName &inputMethodConfig)
{
    IMSA_HILOGD("InputMethodController::inputMethodConfig");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    return proxy->GetInputMethodConfig(inputMethodConfig);
}

std::shared_ptr<Property> InputMethodController::GetCurrentInputMethod()
{
    IMSA_HILOGD("InputMethodController::GetCurrentInputMethod");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return nullptr;
    }
    auto property = proxy->GetCurrentInputMethod();
    if (property == nullptr) {
        IMSA_HILOGE("property is nullptr");
        return nullptr;
    }
    return property;
}

std::shared_ptr<SubProperty> InputMethodController::GetCurrentInputMethodSubtype()
{
    IMSA_HILOGD("InputMethodController::GetCurrentInputMethod");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return nullptr;
    }
    auto property = proxy->GetCurrentInputMethodSubtype();
    if (property == nullptr) {
        IMSA_HILOGE("property is nullptr");
        return nullptr;
    }
    return property;
}

int32_t InputMethodController::StartInput(InputClientInfo &inputClientInfo, sptr<IRemoteObject> &agent)
{
    IMSA_HILOGD("InputMethodController::StartInput");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    return proxy->StartInput(inputClientInfo, agent);
}

int32_t InputMethodController::ReleaseInput(sptr<IInputClient> &client)
{
    IMSA_HILOGD("InputMethodController::ReleaseInput");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    int32_t ret = proxy->ReleaseInput(client);
    if (ret == ErrorCode::NO_ERROR) {
        OnInputStop();
    }
    return ret;
}

int32_t InputMethodController::ShowInput(sptr<IInputClient> &client)
{
    IMSA_HILOGD("InputMethodController::ShowInput");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    return proxy->ShowInput(client);
}

int32_t InputMethodController::HideInput(sptr<IInputClient> &client)
{
    IMSA_HILOGD("InputMethodController::HideInput");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    return proxy->HideInput(client);
}

void InputMethodController::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
{
    IMSA_HILOGI("input method service death");
    {
        std::lock_guard<std::mutex> lock(abilityLock_);
        abilityManager_ = nullptr;
    }
    if (handler_ == nullptr) {
        IMSA_HILOGE("handler_ is nullptr");
        return;
    }
    RestoreListenInfoInSaDied();
    RestoreAttachInfoInSaDied();
}

void InputMethodController::RestoreListenInfoInSaDied()
{
    {
        std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
        if (clientInfo_.eventFlag == EventStatusManager::NO_EVENT_ON) {
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
            IMSA_HILOGI("Try to RestoreListen success.");
        }
    };
    for (int i = 0; i < LOOP_COUNT; i++) {
        handler_->PostTask(restoreListenTask, "OnRemoteSaDied", DELAY_TIME * (i + 1));
    }
}

void InputMethodController::RestoreAttachInfoInSaDied()
{
    if (!IsEditable()) {
        IMSA_HILOGD("not editable");
        return;
    }
    auto attach = [=]() -> bool {
        TextConfig tempConfig{};
        {
            std::lock_guard<std::mutex> lock(textConfigLock_);
            tempConfig = textConfig_;
            tempConfig.cursorInfo = cursorInfo_;
            tempConfig.range.start = selectNewBegin_;
            tempConfig.range.end = selectNewEnd_;
        }
        auto listener = GetTextListener();
        auto errCode = Attach(listener, clientInfo_.isShowKeyboard, tempConfig);
        IMSA_HILOGI("attach end, errCode = %{public}d", errCode);
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
        handler_->PostTask(attachTask, "OnRemoteSaDied", DELAY_TIME * (i + 1));
    }
}

int32_t InputMethodController::OnCursorUpdate(CursorInfo cursorInfo)
{
    if (!IsBound()) {
        IMSA_HILOGD("not bound");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    if (!IsEditable()) {
        IMSA_HILOGD("not editable");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_.cursorInfo = cursorInfo;
    }
    {
        std::lock_guard<std::mutex> lk(cursorInfoMutex_);
        if (cursorInfo_ == cursorInfo) {
            IMSA_HILOGD("same to last update");
            return ErrorCode::NO_ERROR;
        }
        cursorInfo_ = cursorInfo;
    }
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    IMSA_HILOGI("left: %{public}d, top: %{public}d, height: %{public}d", static_cast<int32_t>(cursorInfo.left),
        static_cast<int32_t>(cursorInfo.top), static_cast<int32_t>(cursorInfo.height));
    agent->OnCursorUpdate(cursorInfo.left, cursorInfo.top, cursorInfo.height);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::OnSelectionChange(std::u16string text, int start, int end)
{
    if (!IsBound()) {
        IMSA_HILOGD("not bound");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    if (!IsEditable()) {
        IMSA_HILOGD("not editable");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_.range = { start, end };
    }
    if (textString_ == text && selectNewBegin_ == start && selectNewEnd_ == end) {
        IMSA_HILOGD("same to last update");
        return ErrorCode::NO_ERROR;
    }
    textString_ = text;
    selectOldBegin_ = selectNewBegin_;
    selectOldEnd_ = selectNewEnd_;
    selectNewBegin_ = start;
    selectNewEnd_ = end;
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    IMSA_HILOGI("IMC size: %{public}zu, range: %{public}d/%{public}d", text.size(), start, end);
    agent->OnSelectionChange(textString_, selectOldBegin_, selectOldEnd_, selectNewBegin_, selectNewEnd_);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::OnConfigurationChange(Configuration info)
{
    if (!IsBound()) {
        IMSA_HILOGD("not bound");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_.inputAttribute.enterKeyType = static_cast<int32_t>(info.GetEnterKeyType());
        textConfig_.inputAttribute.inputPattern = static_cast<int32_t>(info.GetTextInputType());
    }
    if (!IsEditable()) {
        IMSA_HILOGD("not editable");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    IMSA_HILOGI("IMC enterKeyType: %{public}d, textInputType: %{public}d", textConfig_.inputAttribute.enterKeyType,
        textConfig_.inputAttribute.inputPattern);
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    agent->OnConfigurationChange(info);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::GetLeft(int32_t length, std::u16string &text)
{
    InputMethodSyncTrace tracer("IMC_GetForward");
    IMSA_HILOGD("run in, length: %{public}d", length);
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or listener is nullptr");
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
    IMSA_HILOGD("run in, length: %{public}d", length);
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener_ is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    text = listener->GetRightTextOfCursor(length);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::GetTextIndexAtCursor(int32_t &index)
{
    IMSA_HILOGD("run in");
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener_ is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    index = listener->GetTextIndexAtCursor();
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::DispatchKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent, KeyEventCallback callback)
{
    KeyEventInfo keyEventInfo = { std::chrono::system_clock::now(), keyEvent };
    keyEventQueue_.Push(keyEventInfo);
    InputMethodSyncTrace tracer("DispatchKeyEvent trace");
    keyEventQueue_.Wait(keyEventInfo);
    if (!IsEditable()) {
        IMSA_HILOGD("not editable");
        keyEventQueue_.Pop();
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    if (keyEvent == nullptr) {
        IMSA_HILOGE("keyEvent is nullptr");
        keyEventQueue_.Pop();
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent is nullptr");
        keyEventQueue_.Pop();
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    IMSA_HILOGI("start");
    sptr<IKeyEventConsumer> consumer = new (std::nothrow) KeyEventConsumerStub(callback, keyEvent);
    if (consumer == nullptr) {
        IMSA_HILOGE("keyEvent is nullptr");
        keyEventQueue_.Pop();
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret = agent->DispatchKeyEvent(keyEvent, consumer);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("DispatchKeyEvent failed");
    }
    keyEventQueue_.Pop();
    return ret;
}

int32_t InputMethodController::GetEnterKeyType(int32_t &keyType)
{
    IMSA_HILOGD("InputMethodController::GetEnterKeyType");
    std::lock_guard<std::mutex> lock(textConfigLock_);
    keyType = textConfig_.inputAttribute.enterKeyType;
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::GetInputPattern(int32_t &inputpattern)
{
    IMSA_HILOGD("InputMethodController::GetInputPattern");
    std::lock_guard<std::mutex> lock(textConfigLock_);
    inputpattern = textConfig_.inputAttribute.inputPattern;
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::GetTextConfig(TextTotalConfig &config)
{
    IMSA_HILOGD("InputMethodController run in.");
    std::lock_guard<std::mutex> lock(textConfigLock_);
    config.inputAttribute = textConfig_.inputAttribute;
    config.cursorInfo = textConfig_.cursorInfo;
    config.windowId = textConfig_.windowId;
    config.positionY = textConfig_.positionY;
    config.height = textConfig_.height;

    if (textConfig_.range.start == INVALID_VALUE) {
        IMSA_HILOGD("no valid SelectionRange param.");
        return ErrorCode::NO_ERROR;
    }
    {
        std::lock_guard<std::mutex> editorLock(editorContentLock_);
        config.textSelection.oldBegin = selectOldBegin_;
        config.textSelection.oldEnd = selectOldEnd_;
    }
    config.textSelection.newBegin = textConfig_.range.start;
    config.textSelection.newEnd = textConfig_.range.end;

    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::SetCallingWindow(uint32_t windowId)
{
    if (!IsBound()) {
        IMSA_HILOGD("not bound");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    if (!IsEditable()) {
        IMSA_HILOGD("not editable");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_.windowId = windowId;
    }
    auto agent = GetAgent();
    if (agent == nullptr) {
        IMSA_HILOGE("agent_ is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    IMSA_HILOGI("windowId = %{public}d", windowId);
    agent->SetCallingWindow(windowId);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::ShowSoftKeyboard()
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGI("start");
    clientInfo_.isShowKeyboard = true;
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_SHOW_NORMAL);
    return proxy->ShowCurrentInput();
}

int32_t InputMethodController::HideSoftKeyboard()
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGI("start");
    clientInfo_.isShowKeyboard = false;
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_NORMAL);
    return proxy->HideCurrentInput();
}

int32_t InputMethodController::StopInputSession()
{
    IMSA_HILOGI("run in");
    isEditable_.store(false);
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return proxy->StopInputSession();
}

int32_t InputMethodController::ShowOptionalInputMethod()
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGI("IMC run in");
    return proxy->DisplayOptionalInputMethod();
}

int32_t InputMethodController::ListInputMethodSubtype(const Property &property, std::vector<SubProperty> &subProps)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGD("ime bundleName: %{public}s", property.name.c_str());
    return proxy->ListInputMethodSubtype(property.name, subProps);
}

int32_t InputMethodController::ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProps)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGD("run in");
    return proxy->ListCurrentInputMethodSubtype(subProps);
}

int32_t InputMethodController::SwitchInputMethod(
    SwitchTrigger trigger, const std::string &name, const std::string &subName)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    IMSA_HILOGI("name: %{public}s, subName: %{public}s, trigger: %{public}d", name.c_str(), subName.c_str(),
        static_cast<uint32_t>(trigger));
    return proxy->SwitchInputMethod(name, subName, trigger);
}

void InputMethodController::OnInputReady(sptr<IRemoteObject> agentObject)
{
    IMSA_HILOGI("IMC");
    isBound_.store(true);
    isEditable_.store(true);
    if (agentObject == nullptr) {
        IMSA_HILOGE("agentObject is nullptr");
        return;
    }
    SetAgent(agentObject);
}

void InputMethodController::OnInputStop()
{
    {
        std::lock_guard<std::mutex> autoLock(agentLock_);
        agent_ = nullptr;
        agentObject_ = nullptr;
    }
    auto listener = GetTextListener();
    if (listener != nullptr) {
        IMSA_HILOGD("textListener_ is not nullptr");
        listener->SendKeyboardStatus(KeyboardStatus::HIDE);
    }
    isBound_.store(false);
    isEditable_.store(false);
}

void InputMethodController::ClearEditorCache()
{
    IMSA_HILOGD("clear editor content cache");
    {
        std::lock_guard<std::mutex> lock(editorContentLock_);
        textString_ = Str8ToStr16("");
        selectOldBegin_ = 0;
        selectOldEnd_ = 0;
        selectNewBegin_ = 0;
        selectNewEnd_ = 0;
    }
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_ = {};
    }
    std::lock_guard<std::mutex> lock(cursorInfoMutex_);
    cursorInfo_ = {};
}

void InputMethodController::SelectByRange(int32_t start, int32_t end)
{
    IMSA_HILOGD("InputMethodController start: %{public}d, end: %{public}d", start, end);
    auto listener = GetTextListener();
    if (IsEditable() && listener != nullptr) {
        listener->HandleSetSelection(start, end);
    } else {
        IMSA_HILOGE("not editable or textListener_ is nullptr");
    }

    if (controllerListener_ != nullptr) {
        controllerListener_->OnSelectByRange(start, end);
    } else {
        IMSA_HILOGE("controllerListener_ is nullptr");
    }
}

void InputMethodController::SelectByMovement(int32_t direction, int32_t cursorMoveSkip)
{
    IMSA_HILOGD("InputMethodController, direction: %{public}d, cursorMoveSkip: %{public}d", direction, cursorMoveSkip);
    auto listener = GetTextListener();
    if (IsEditable() && listener != nullptr) {
        listener->HandleSelect(CURSOR_DIRECTION_BASE_VALUE + direction, cursorMoveSkip);
    } else {
        IMSA_HILOGE("not editable or textListener_ is nullptr");
    }

    if (controllerListener_ != nullptr) {
        controllerListener_->OnSelectByMovement(direction);
    } else {
        IMSA_HILOGE("controllerListener_ is nullptr");
    }
}

int32_t InputMethodController::HandleExtendAction(int32_t action)
{
    IMSA_HILOGD("InputMethodController, action: %{public}d", action);
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener is nullptr");
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
        IMSA_HILOGD("client not active");
        return false;
    }
    if (!isEditable_.load()) {
        IMSA_HILOGD("not in editable state");
        return false;
    }
    return true;
}

bool InputMethodController::IsBound()
{
    std::lock_guard<std::recursive_mutex> lock(clientInfoLock_);
    if (clientInfo_.state != ClientState::ACTIVE) {
        IMSA_HILOGD("client not active");
        return false;
    }
    if (!isBound_.load()) {
        IMSA_HILOGD("not bound");
        return false;
    }
    return true;
}

int32_t InputMethodController::InsertText(const std::u16string &text)
{
    InputMethodSyncTrace tracer("IMC_InsertText");
    IMSA_HILOGD("in");
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    {
        InputMethodSyncTrace aceTracer("ACE_InsertText");
        listener->InsertText(text);
    }
    PrintLogIfAceTimeout(start);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::DeleteForward(int32_t length)
{
    InputMethodSyncTrace tracer("IMC_DeleteForward");
    IMSA_HILOGD("run in, length: %{public}d", length);
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener is nullptr");
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
    IMSA_HILOGD("run in, length: %{public}d", length);
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    // reverse for compatibility
    listener->DeleteForward(length);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::MoveCursor(Direction direction)
{
    IMSA_HILOGD("run in, direction: %{public}d", static_cast<int32_t>(direction));
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener_ is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    listener->MoveCursor(direction);
    return ErrorCode::NO_ERROR;
}

void InputMethodController::SendKeyboardStatus(KeyboardStatus status)
{
    IMSA_HILOGD("InputMethodController status: %{public}d", static_cast<int32_t>(status));
    auto listener = GetTextListener();
    if (listener == nullptr) {
        IMSA_HILOGE("textListener_ is nullptr");
        return;
    }
    listener->SendKeyboardStatus(status);
    if (status == KeyboardStatus::HIDE) {
        clientInfo_.isShowKeyboard = false;
    }
}

void InputMethodController::NotifyPanelStatusInfo(const PanelStatusInfo &info)
{
    IMSA_HILOGD("InputMethodController, type: %{public}d, flag: %{public}d, visible: %{public}d, trigger: %{public}d.",
        static_cast<PanelType>(info.panelInfo.panelType), static_cast<PanelFlag>(info.panelInfo.panelFlag),
        info.visible, static_cast<Trigger>(info.trigger));
    auto listener = GetTextListener();
    if (listener == nullptr) {
        IMSA_HILOGE("textListener_ is nullptr");
        return;
    }
    listener->NotifyPanelStatusInfo(info);
    if (info.panelInfo.panelType == PanelType::SOFT_KEYBOARD
        && info.panelInfo.panelFlag != PanelFlag::FLG_CANDIDATE_COLUMN && !info.visible) {
        clientInfo_.isShowKeyboard = false;
    }
}

void InputMethodController::NotifyKeyboardHeight(uint32_t height)
{
    IMSA_HILOGD("InputMethodController, height: %{public}u.", height);
    auto listener = GetTextListener();
    if (listener == nullptr) {
        IMSA_HILOGE("textListener_ is nullptr");
        return;
    }
    listener->NotifyKeyboardHeight(height);
}

int32_t InputMethodController::SendFunctionKey(int32_t functionKey)
{
    IMSA_HILOGD("run in, functionKey: %{public}d", static_cast<int32_t>(functionKey));
    auto listener = GetTextListener();
    if (!IsEditable() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener_ is nullptr");
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
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    IMSA_HILOGI("type: %{public}d", static_cast<int32_t>(type));
    return proxy->IsInputTypeSupported(type);
}

int32_t InputMethodController::StartInputType(InputType type)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    IMSA_HILOGI("type: %{public}d", static_cast<int32_t>(type));
    return proxy->StartInputType(type);
}

int32_t InputMethodController::IsPanelShown(const PanelInfo &panelInfo, bool &isShown)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    IMSA_HILOGI("type: %{public}d, flag: %{public}d", static_cast<int32_t>(panelInfo.panelType),
        static_cast<int32_t>(panelInfo.panelFlag));
    return proxy->IsPanelShown(panelInfo, isShown);
}

void InputMethodController::SetAgent(sptr<IRemoteObject> &agentObject)
{
    std::lock_guard<std::mutex> autoLock(agentLock_);
    if (agent_ != nullptr && agentObject_.GetRefPtr() == agentObject.GetRefPtr()) {
        IMSA_HILOGD("agent has already been set");
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
        IMSA_HILOGW("timeout:[%{public}" PRId64 ", %{public}" PRId64 "]", start, end);
    }
}
} // namespace MiscServices
} // namespace OHOS
