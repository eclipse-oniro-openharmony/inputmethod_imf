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
#include "string_ex.h"
#include "sys/prctl.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
sptr<InputMethodController> InputMethodController::instance_;
std::shared_ptr<AppExecFwk::EventHandler> InputMethodController::handler_{ nullptr };
std::mutex InputMethodController::instanceLock_;
constexpr int32_t LOOP_COUNT = 5;
constexpr int64_t DELAY_TIME = 100;
const std::unordered_map<std::string, EventType> EVENT_TYPE{ { "imeChange", IME_CHANGE }, { "imeShow", IME_SHOW },
    { "imeHide", IME_HIDE } };
InputMethodController::InputMethodController()
{
    IMSA_HILOGI("InputMethodController structure");
}

InputMethodController::~InputMethodController()
{
}

sptr<InputMethodController> InputMethodController::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            IMSA_HILOGI("InputMethodController::GetInstance instance_ is nullptr");
            instance_ = new (std::nothrow) InputMethodController();
            if (instance_ == nullptr) {
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
    IMSA_HILOGI("InputMethodController run in");
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
        IMSA_HILOGI("system ability manager is nullptr");
        return nullptr;
    }
    auto systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
    if (systemAbility == nullptr) {
        IMSA_HILOGI("system ability is nullptr");
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
    IMSA_HILOGE("InputMethodController::OnSwitchInput");
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
    IMSA_HILOGD("InputMethodController::OnPanelStatusChange");
    if (settingListener_ == nullptr) {
        IMSA_HILOGE("imeListener_ is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    settingListener_->OnPanelStatusChange(status, windowInfo);
    return ErrorCode::NO_ERROR;
}

void InputMethodController::SaveTextConfig(const TextConfig &textConfig)
{
    IMSA_HILOGI("InputMethodController in, save text config.");
    std::lock_guard<std::mutex> lock(textConfigLock_);
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
    IMSA_HILOGI("InputMethodController::Attach isShowKeyboard %{public}s", isShowKeyboard ? "true" : "false");
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
    SetTextListener(listener);
    clientInfo_.isShowKeyboard = isShowKeyboard;
    SaveTextConfig(textConfig);

    int32_t ret = PrepareInput(clientInfo_);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to prepare, ret: %{public}d", ret);
        return ret;
    }
    ret = StartInput(clientInfo_.client, isShowKeyboard);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to start input, ret:%{public}d", ret);
        return ret;
    }
    IMSA_HILOGI("bind imf successfully, enter editable state");

    if (isShowKeyboard) {
        InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_SHOW_ATTACH);
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::ShowTextInput()
{
    IMSA_HILOGI("InputMethodController::ShowTextInput");
    if (!isBound_.load()) {
        IMSA_HILOGE("not bound yet");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
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
    IMSA_HILOGI("InputMethodController::HideTextInput");
    if (!isBound_.load()) {
        IMSA_HILOGE("not bound yet");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    isEditable_.store(false);
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_UNEDITABLE);
    return HideInput(clientInfo_.client);
}

int32_t InputMethodController::HideCurrentInput()
{
    IMSA_HILOGD("InputMethodController::HideCurrentInput");
    if (!isEditable_.load()) {
        IMSA_HILOGE("not in editable state");
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
    IMSA_HILOGI("InputMethodController::ShowCurrentInput");
    if (!isEditable_.load()) {
        IMSA_HILOGE("not in editable state");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    clientInfo_.isShowKeyboard = true;
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_SHOW_NORMAL);
    return proxy->ShowCurrentInputDeprecated();
}

int32_t InputMethodController::Close()
{
    IMSA_HILOGI("InputMethodController::Close");
    bool isReportHide = clientInfo_.isShowKeyboard;
    InputMethodSyncTrace tracer("InputMethodController Close trace.");
    isReportHide ? InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_UNBIND)
                 : InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_UNBIND);
    return ReleaseInput(clientInfo_.client);
}

int32_t InputMethodController::PrepareInput(InputClientInfo &inputClientInfo)
{
    IMSA_HILOGI("InputMethodController::PrepareInput");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    return proxy->PrepareInput(inputClientInfo);
}

int32_t InputMethodController::DisplayOptionalInputMethod()
{
    IMSA_HILOGI("InputMethodController::DisplayOptionalInputMethod");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return proxy->DisplayOptionalInputMethodDeprecated();
}

bool InputMethodController::WasAttached()
{
    return isBound_.load();
}

int32_t InputMethodController::ListInputMethodCommon(InputMethodStatus status, std::vector<Property> &props)
{
    IMSA_HILOGI("InputMethodController::ListInputMethodCommon");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return proxy->ListInputMethod(status, props);
}

int32_t InputMethodController::ListInputMethod(std::vector<Property> &props)
{
    IMSA_HILOGI("InputMethodController::listInputMethod");
    return ListInputMethodCommon(ALL, props);
}

int32_t InputMethodController::ListInputMethod(bool enable, std::vector<Property> &props)
{
    IMSA_HILOGI("InputMethodController::listInputMethod enable = %{public}s", enable ? "ENABLE" : "DISABLE");
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
        IMSA_HILOGE("InputMethodController::GetCurrentInputMethod property is nullptr");
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
        IMSA_HILOGE("InputMethodController::GetCurrentInputMethodSubtype property is nullptr");
        return nullptr;
    }
    return property;
}

int32_t InputMethodController::StartInput(sptr<IInputClient> &client, bool isShowKeyboard)
{
    IMSA_HILOGI("InputMethodController::StartInput");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    return proxy->StartInput(client, isShowKeyboard);
}

int32_t InputMethodController::ReleaseInput(sptr<IInputClient> &client)
{
    IMSA_HILOGD("InputMethodController::ReleaseInput");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    return proxy->ReleaseInput(client);
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
    IMSA_HILOGE("input method service death");
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
    if (!isEditable_.load()) {
        IMSA_HILOGE("not in editable state");
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
    if (!isBound_.load()) {
        IMSA_HILOGE("not bound yet");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    if (!isEditable_.load()) {
        IMSA_HILOGE("not in editable state");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_.cursorInfo = cursorInfo;
    }
    std::lock_guard<std::mutex> lock(agentLock_);
    if (agent_ == nullptr) {
        IMSA_HILOGI("agent is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    {
        std::lock_guard<std::mutex> lk(cursorInfoMutex_);
        if (cursorInfo_ == cursorInfo) {
            IMSA_HILOGI("same to last update");
            return ErrorCode::NO_ERROR;
        }
        cursorInfo_ = cursorInfo;
    }
    agent_->OnCursorUpdate(cursorInfo.left, cursorInfo.top, cursorInfo.height);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::OnSelectionChange(std::u16string text, int start, int end)
{
    IMSA_HILOGD("size: %{public}zu, start: %{public}d, end: %{public}d", text.size(), start, end);
    if (!isBound_.load()) {
        IMSA_HILOGE("not bound yet");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    if (!isEditable_.load()) {
        IMSA_HILOGE("not in editable state");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_.range = { start, end };
    }
    if (textString_ == text && selectNewBegin_ == start && selectNewEnd_ == end) {
        IMSA_HILOGI("same to last update");
        return ErrorCode::NO_ERROR;
    }
    textString_ = text;
    selectOldBegin_ = selectNewBegin_;
    selectOldEnd_ = selectNewEnd_;
    selectNewBegin_ = start;
    selectNewEnd_ = end;
    std::lock_guard<std::mutex> lock(agentLock_);
    if (agent_ == nullptr) {
        IMSA_HILOGE("agent is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    agent_->OnSelectionChange(textString_, selectOldBegin_, selectOldEnd_, selectNewBegin_, selectNewEnd_);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::OnConfigurationChange(Configuration info)
{
    IMSA_HILOGI("InputMethodController::OnConfigurationChange");
    if (!isBound_.load()) {
        IMSA_HILOGE("not bound yet");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_.inputAttribute.enterKeyType = static_cast<uint32_t>(info.GetEnterKeyType());
        textConfig_.inputAttribute.inputPattern = static_cast<uint32_t>(info.GetTextInputType());
    }
    std::lock_guard<std::mutex> agentLock(agentLock_);
    if (agent_ == nullptr) {
        IMSA_HILOGE("agent is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    agent_->OnConfigurationChange(info);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::GetLeft(int32_t length, std::u16string &text)
{
    IMSA_HILOGD("run in, length: %{public}d", length);
    auto listener = GetTextListener();
    if (!isEditable_.load() || listener == nullptr) {
        IMSA_HILOGE("not editable or listener is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    text = listener->GetLeftTextOfCursor(length);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::GetRight(int32_t length, std::u16string &text)
{
    IMSA_HILOGD("run in, length: %{public}d", length);
    auto listener = GetTextListener();
    if (!isEditable_.load() || listener == nullptr) {
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
    if (!isEditable_.load() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener_ is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    index = listener->GetTextIndexAtCursor();
    return ErrorCode::NO_ERROR;
}

bool InputMethodController::DispatchKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent)
{
    InputMethodSyncTrace tracer("DispatchKeyEvent trace");
    IMSA_HILOGI("InputMethodController in");
    if (!isEditable_.load()) {
        IMSA_HILOGE("not in editable state");
        return false;
    }
    if (keyEvent == nullptr) {
        IMSA_HILOGE("keyEvent is nullptr");
        return false;
    }
    std::lock_guard<std::mutex> lock(agentLock_);
    if (agent_ == nullptr) {
        IMSA_HILOGI("agent is nullptr");
        return false;
    }
    return agent_->DispatchKeyEvent(keyEvent);
}

int32_t InputMethodController::GetEnterKeyType(int32_t &keyType)
{
    IMSA_HILOGI("InputMethodController::GetEnterKeyType");
    std::lock_guard<std::mutex> lock(textConfigLock_);
    keyType = textConfig_.inputAttribute.enterKeyType;
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::GetInputPattern(int32_t &inputpattern)
{
    IMSA_HILOGI("InputMethodController::GetInputPattern");
    std::lock_guard<std::mutex> lock(textConfigLock_);
    inputpattern = textConfig_.inputAttribute.inputPattern;
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::GetTextConfig(TextTotalConfig &config)
{
    IMSA_HILOGI("InputMethodController run in.");
    std::lock_guard<std::mutex> lock(textConfigLock_);
    config.inputAttribute = textConfig_.inputAttribute;
    config.cursorInfo = textConfig_.cursorInfo;
    config.windowId = textConfig_.windowId;

    if (textConfig_.range.start == INVALID_VALUE) {
        IMSA_HILOGI("no valid SelectionRange param.");
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
    if (!isBound_.load()) {
        IMSA_HILOGE("not bound yet");
        return ErrorCode::ERROR_CLIENT_NOT_BOUND;
    }
    if (!isEditable_.load()) {
        IMSA_HILOGE("not in editable state");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    {
        std::lock_guard<std::mutex> lock(textConfigLock_);
        textConfig_.windowId = windowId;
    }
    IMSA_HILOGI("InputMethodController::SetCallingWindow windowId = %{public}d", windowId);
    std::lock_guard<std::mutex> lock(agentLock_);
    if (agent_ == nullptr) {
        IMSA_HILOGE("InputMethodController::SetCallingWindow mAgent is nullptr");
        return ErrorCode::ERROR_SERVICE_START_FAILED;
    }
    agent_->SetCallingWindow(windowId);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::ShowSoftKeyboard()
{
    IMSA_HILOGI("InputMethodController ShowSoftKeyboard");
    if (!isEditable_.load()) {
        IMSA_HILOGE("not in editable state");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    clientInfo_.isShowKeyboard = true;
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_SHOW_NORMAL);
    return proxy->ShowCurrentInput();
}

int32_t InputMethodController::HideSoftKeyboard()
{
    IMSA_HILOGI("InputMethodController HideSoftKeyboard");
    if (!isEditable_.load()) {
        IMSA_HILOGE("not in editable state");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    clientInfo_.isShowKeyboard = false;
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_NORMAL);
    return proxy->HideCurrentInput();
}

int32_t InputMethodController::StopInputSession()
{
    IMSA_HILOGI("InputMethodController StopInputSession");
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
    IMSA_HILOGI("InputMethodController::ShowOptionalInputMethod");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return proxy->DisplayOptionalInputMethod();
}

int32_t InputMethodController::ListInputMethodSubtype(const Property &property, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGI("InputMethodController::ListInputMethodSubtype");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return proxy->ListInputMethodSubtype(property.name, subProps);
}

int32_t InputMethodController::ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProps)
{
    IMSA_HILOGI("InputMethodController::ListCurrentInputMethodSubtype");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return proxy->ListCurrentInputMethodSubtype(subProps);
}

int32_t InputMethodController::SwitchInputMethod(const std::string &name, const std::string &subName)
{
    IMSA_HILOGI("InputMethodController::SwitchInputMethod");
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return proxy->SwitchInputMethod(name, subName);
}

void InputMethodController::OnInputReady(sptr<IRemoteObject> agentObject)
{
    IMSA_HILOGI("InputMethodController run in");
    isBound_.store(true);
    isEditable_.store(true);
    std::lock_guard<std::mutex> lk(agentLock_);
    if (agentObject == nullptr) {
        IMSA_HILOGE("agentObject is nullptr");
        return;
    }
    if (agentObject_ != nullptr && agentObject_.GetRefPtr() == agentObject.GetRefPtr()) {
        IMSA_HILOGI("agent has already been set");
        return;
    }
    std::shared_ptr<IInputMethodAgent> agent = std::make_shared<InputMethodAgentProxy>(agentObject);
    if (agent == nullptr) {
        IMSA_HILOGE("failed to new agent proxy");
    }
    agentObject_ = agentObject;
    agent_ = agent;
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
    IMSA_HILOGI("InputMethodController run in");
    auto listener = GetTextListener();
    if (isEditable_.load() && listener != nullptr) {
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
    IMSA_HILOGI("InputMethodController run in");
    auto listener = GetTextListener();
    if (isEditable_.load() && listener != nullptr) {
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
    IMSA_HILOGI("InputMethodController run in");
    auto listener = GetTextListener();
    if (!isEditable_.load() || listener == nullptr) {
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

int32_t InputMethodController::InsertText(const std::u16string &text)
{
    IMSA_HILOGD("in");
    auto listener = GetTextListener();
    if (!isEditable_.load() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    listener->InsertText(text);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::DeleteForward(int32_t length)
{
    IMSA_HILOGD("run in, length: %{public}d", length);
    auto listener = GetTextListener();
    if (!isEditable_.load() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    // reverse for compatibility
    listener->DeleteBackward(length);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodController::DeleteBackward(int32_t length)
{
    IMSA_HILOGD("run in, length: %{public}d", length);
    auto listener = GetTextListener();
    if (!isEditable_.load() || listener == nullptr) {
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
    if (!isEditable_.load() || listener == nullptr) {
        IMSA_HILOGE("not editable or textListener_ is nullptr");
        return ErrorCode::ERROR_CLIENT_NOT_EDITABLE;
    }
    listener->MoveCursor(direction);
    return ErrorCode::NO_ERROR;
}

void InputMethodController::SendKeyboardStatus(int32_t status)
{
    IMSA_HILOGD("run in, status: %{public}d", status);
    auto listener = GetTextListener();
    if (listener == nullptr) {
        IMSA_HILOGE("textListener_ is nullptr");
        return;
    }
    auto keyboardStatus = static_cast<KeyboardStatus>(status);
    listener->SendKeyboardStatus(keyboardStatus);
    if (keyboardStatus == KeyboardStatus::HIDE) {
        clientInfo_.isShowKeyboard = false;
    }
}

int32_t InputMethodController::SendFunctionKey(int32_t functionKey)
{
    auto listener = GetTextListener();
    if (!isEditable_.load() || listener == nullptr) {
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
    IMSA_HILOGI("InputMethodController, type: %{public}d", static_cast<int32_t>(type));
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return proxy->IsInputTypeSupported(type);
}

int32_t InputMethodController::StartInputType(InputType type)
{
    IMSA_HILOGI("InputMethodController, type: %{public}d", static_cast<int32_t>(type));
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return proxy->StartInputType(type);
}

int32_t InputMethodController::IsPanelShown(const PanelInfo &panelInfo, bool &isShown)
{
    auto proxy = GetSystemAbilityProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return proxy->IsPanelShown(panelInfo, isShown);
}
} // namespace MiscServices
} // namespace OHOS
