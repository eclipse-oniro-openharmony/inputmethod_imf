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

#include "input_method_ability.h"

#include <unistd.h>
#include <utility>

#include "global.h"
#include "ima_hisysevent_reporter.h"
#include "input_method_agent_stub.h"
#include "input_method_core_stub.h"
#include "input_method_system_ability_proxy.h"
#include "input_method_utils.h"
#include "inputmethod_sysevent.h"
#include "inputmethod_trace.h"
#include "iservice_registry.h"
#include "itypes_util.h"
#include "message_parcel.h"
#include "on_demand_start_stop_sa.h"
#include "string_ex.h"
#include "sys/prctl.h"
#include "system_ability_definition.h"
#include "task_manager.h"
#include "tasks/task.h"
#include "tasks/task_imsa.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
using namespace std::chrono;
sptr<InputMethodAbility> InputMethodAbility::instance_;
std::mutex InputMethodAbility::instanceLock_;
constexpr double INVALID_CURSOR_VALUE = -1.0;
constexpr int32_t INVALID_SELECTION_VALUE = -1;
constexpr uint32_t FIND_PANEL_RETRY_INTERVAL = 10;
constexpr uint32_t MAX_RETRY_TIMES = 100;
constexpr uint32_t START_INPUT_CALLBACK_TIMEOUT_MS = 1000;
constexpr uint32_t INVALID_SECURITY_MODE = -1;
constexpr uint32_t BASE_TEXT_OPERATION_TIMEOUT = 200;

InputMethodAbility::InputMethodAbility() { }

InputMethodAbility::~InputMethodAbility()
{
    IMSA_HILOGI("InputMethodAbility::~InputMethodAbility.");
}

sptr<InputMethodAbility> InputMethodAbility::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            IMSA_HILOGI("InputMethodAbility need new IMA.");
            instance_ = new (std::nothrow) InputMethodAbility();
            if (instance_ == nullptr) {
                IMSA_HILOGE("instance is nullptr!");
                return instance_;
            }
            instance_->Initialize();
        }
    }
    return instance_;
}

sptr<IInputMethodSystemAbility> InputMethodAbility::GetImsaProxy()
{
    std::lock_guard<std::mutex> lock(abilityLock_);
    if (abilityManager_ != nullptr) {
        return abilityManager_;
    }
    IMSA_HILOGI("InputMethodAbility get imsa proxy.");
    auto systemAbility = OnDemandStartStopSa::GetInputMethodSystemAbility();
    if (systemAbility == nullptr) {
        IMSA_HILOGE("systemAbility is nullptr!");
        return nullptr;
    }
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new (std::nothrow) InputDeathRecipient();
        if (deathRecipient_ == nullptr) {
            IMSA_HILOGE("failed to new death recipient!");
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

int32_t InputMethodAbility::SetCoreAndAgent()
{
    IMSA_HILOGD("InputMethodAbility, start.");
    TaskManager::GetInstance().SetInited(true);

    if (isBound_.load()) {
        IMSA_HILOGD("already bound.");
        return ErrorCode::NO_ERROR;
    }
    auto proxy = GetImsaProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("imsa proxy is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    int32_t ret = proxy->SetCoreAndAgent(coreStub_, agentStub_->AsObject());
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("set failed, ret: %{public}d!", ret);
        return ret;
    }
    isBound_.store(true);
    IMSA_HILOGD("set successfully.");
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::InitConnect()
{
    IMSA_HILOGD("InputMethodAbility, init connect.");
    auto proxy = GetImsaProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("imsa proxy is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    int32_t ret = proxy->InitConnect();
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("set failed, ret: %{public}d!", ret);
        return ret;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::UnRegisteredProxyIme(UnRegisteredType type)
{
    isBound_.store(false);
    auto proxy = GetImsaProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("imsa proxy is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return proxy->UnRegisteredProxyIme(type, coreStub_);
}

void InputMethodAbility::Initialize()
{
    IMSA_HILOGD("IMA init.");
    sptr<InputMethodCoreStub> coreStub = new (std::nothrow) InputMethodCoreStub();
    if (coreStub == nullptr) {
        IMSA_HILOGE("failed to create core!");
        return;
    }
    sptr<InputMethodAgentStub> agentStub = new (std::nothrow) InputMethodAgentStub();
    if (agentStub == nullptr) {
        IMSA_HILOGE("failed to create agent!");
        return;
    }
    agentStub_ = agentStub;
    coreStub_ = coreStub;
}

void InputMethodAbility::SetImeListener(std::shared_ptr<InputMethodEngineListener> imeListener)
{
    IMSA_HILOGD("InputMethodAbility start.");
    if (imeListener_ == nullptr) {
        imeListener_ = std::move(imeListener);
    }
}

std::shared_ptr<InputMethodEngineListener> InputMethodAbility::GetImeListener()
{
    return imeListener_;
}

void InputMethodAbility::SetKdListener(std::shared_ptr<KeyboardListener> kdListener)
{
    IMSA_HILOGD("InputMethodAbility start.");
    if (kdListener_ == nullptr) {
        kdListener_ = std::move(kdListener);
    }
}

void InputMethodAbility::OnInitInputControlChannel(sptr<IRemoteObject> channelObj)
{
    IMSA_HILOGD("InputMethodAbility::OnInitInputControlChannel start.");
    SetInputControlChannel(channelObj);
}

int32_t InputMethodAbility::StartInputInner(const InputClientInfo &clientInfo, bool isBindFromClient)
{
    SetBindClientInfo(clientInfo);
    if (clientInfo.channel == nullptr) {
        IMSA_HILOGE("channelObject is nullptr!");
        return ErrorCode::ERROR_IMA_CHANNEL_NULLPTR;
    }
    IMSA_HILOGI("IMA showKeyboard:%{public}d,bindFromClient:%{public}d.", clientInfo.isShowKeyboard, isBindFromClient);
    SetInputDataChannel(clientInfo.channel);
    if (clientInfo.needHide) {
        IMSA_HILOGD("pwd or normal input pattern changed, need hide panel first.");
        auto panel = GetSoftKeyboardPanel();
        if (panel != nullptr) {
            panel->HidePanel();
        }
    }
    {
        std::lock_guard<std::mutex> lock(inputAttrLock_);
        inputAttribute_.bundleName = clientInfo.config.inputAttribute.bundleName;
        inputAttribute_.windowId = clientInfo.config.windowId;
        inputAttribute_.callingDisplayId = clientInfo.config.inputAttribute.callingDisplayId;
    }
    int32_t ret = isBindFromClient ? InvokeStartInputCallback(clientInfo.config, clientInfo.isNotifyInputStart) :
                                     InvokeStartInputCallback(clientInfo.isNotifyInputStart);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to invoke callback, ret: %{public}d!", ret);
        return ret;
    }
    auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    auto showPanel = [&, needShow = clientInfo.isShowKeyboard, startTime = time] {
        auto endTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        int32_t ret = ErrorCode::NO_ERROR;
        if (needShow) {
            ret = ShowKeyboardImplWithoutLock(cmdId_);
        }
        ReportImeStartInput(IInputMethodCore::START_INPUT, ret, needShow, endTime - startTime);
        isImeTerminating.store(false);
    };
    if (!imeListener_) {
        showPanel();
        return ErrorCode::NO_ERROR;
    }
    uint64_t seqId = Task::GetNextSeqId();
    imeListener_->PostTaskToEventHandler(
        [seqId] {
            TaskManager::GetInstance().Complete(seqId);
        },
        "task_manager_complete");
    TaskManager::GetInstance().WaitExec(seqId, START_INPUT_CALLBACK_TIMEOUT_MS, showPanel);
    return ErrorCode::NO_ERROR;
}

void InputMethodAbility::OnSetSubtype(SubProperty subProperty)
{
    if (imeListener_ != nullptr) {
        imeListener_->OnSetSubtype(subProperty);
    }
}

void InputMethodAbility::OnSetInputType(InputType inputType)
{
    inputType_ = inputType;
    IMSA_HILOGD("OnSetInputType, inputType = %{public}d", static_cast<int32_t>(inputType));
    auto panel = GetSoftKeyboardPanel();
    if (panel != nullptr) {
        auto keyboardSize = panel->GetKeyboardSize();
        SysPanelStatus sysPanelStatus = { inputType_, panel->GetPanelFlag(), keyboardSize.width, keyboardSize.height };
        NotifyPanelStatus(panel->GetPanelType(), sysPanelStatus);
    }
}

void InputMethodAbility::ClearDataChannel(const sptr<IRemoteObject> &channel)
{
    std::lock_guard<std::mutex> lock(dataChannelLock_);
    if (dataChannelObject_ == nullptr || channel == nullptr) {
        IMSA_HILOGD("dataChannelObject_ already nullptr.");
        return;
    }
    if (dataChannelObject_.GetRefPtr() == channel.GetRefPtr()) {
        dataChannelObject_ = nullptr;
        dataChannelProxy_ = nullptr;
        IMSA_HILOGD("end.");
    }
}

int32_t InputMethodAbility::StopInput(sptr<IRemoteObject> channelObject, uint32_t sessionId)
{
    std::lock_guard<std::recursive_mutex> lock(keyboardCmdLock_);
    int32_t cmdCount = ++cmdId_;
    IMSA_HILOGI("IMA");
    HideKeyboardImplWithoutLock(cmdCount, sessionId);
    ClearDataChannel(channelObject);
    ClearInputAttribute();
    ClearInputType();
    if (imeListener_ != nullptr) {
        imeListener_->OnInputFinish();
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::DispatchKeyEvent(
    const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<KeyEventConsumerProxy> &consumer)
{
    if (keyEvent == nullptr) {
        IMSA_HILOGE("keyEvent is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    if (kdListener_ == nullptr) {
        IMSA_HILOGE("kdListener_ is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    IMSA_HILOGD("InputMethodAbility, start.");

    if (!kdListener_->OnDealKeyEvent(keyEvent, consumer)) {
        IMSA_HILOGE("keyEvent not deal!");
        return ErrorCode::ERROR_DISPATCH_KEY_EVENT;
    }
    return ErrorCode::NO_ERROR;
}

void InputMethodAbility::SetCallingWindow(uint32_t windowId)
{
    IMSA_HILOGD("InputMethodAbility windowId: %{public}d.", windowId);
    {
        std::lock_guard<std::mutex> lock(inputAttrLock_);
        inputAttribute_.windowId = windowId;
    }
    panels_.ForEach([windowId](const PanelType &panelType, const std::shared_ptr<InputMethodPanel> &panel) {
        panel->SetCallingWindow(windowId);
        return false;
    });
    if (imeListener_ == nullptr) {
        IMSA_HILOGD("imeListener_ is nullptr!");
        return;
    }
    imeListener_->OnSetCallingWindow(windowId);
}

void InputMethodAbility::OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height)
{
    if (kdListener_ == nullptr) {
        IMSA_HILOGE("kdListener_ is nullptr!");
        return;
    }
    IMSA_HILOGD("x: %{public}d, y: %{public}d, height: %{public}d.", positionX, positionY, height);
    kdListener_->OnCursorUpdate(positionX, positionY, height);
}

void InputMethodAbility::OnSelectionChange(
    std::u16string text, int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd)
{
    if (kdListener_ == nullptr) {
        IMSA_HILOGE("kdListener_ is nullptr!");
        return;
    }
    kdListener_->OnTextChange(Str16ToStr8(text));
    kdListener_->OnSelectionChange(oldBegin, oldEnd, newBegin, newEnd);
}

void InputMethodAbility::OnAttributeChange(InputAttribute attribute)
{
    if (kdListener_ == nullptr) {
        IMSA_HILOGE("kdListener_ is nullptr!");
        return;
    }
    IMSA_HILOGD("enterKeyType: %{public}d, inputPattern: %{public}d.", attribute.enterKeyType, attribute.inputPattern);
    attribute.bundleName = GetInputAttribute().bundleName;
    attribute.windowId = GetInputAttribute().windowId;
    attribute.callingDisplayId = GetInputAttribute().callingDisplayId;
    SetInputAttribute(attribute);
    // add for mod inputPattern when panel show
    auto panel = GetSoftKeyboardPanel();
    if (panel != nullptr) {
        auto keyboardSize = panel->GetKeyboardSize();
        SysPanelStatus sysPanelStatus = { inputType_, panel->GetPanelFlag(), keyboardSize.width, keyboardSize.height };
        NotifyPanelStatus(panel->GetPanelType(), sysPanelStatus);
    }
    kdListener_->OnEditorAttributeChange(attribute);
}

int32_t InputMethodAbility::OnStopInputService(bool isTerminateIme)
{
    IMSA_HILOGI("isTerminateIme: %{public}d.", isTerminateIme);
    isBound_.store(false);
    auto imeListener = GetImeListener();
    if (imeListener == nullptr) {
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }
    if (isTerminateIme) {
        isImeTerminating.store(true);
        return imeListener->OnInputStop();
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::HideKeyboard()
{
    std::lock_guard<std::recursive_mutex> lock(keyboardCmdLock_);
    int32_t cmdCount = ++cmdId_;
    return HideKeyboardImplWithoutLock(cmdCount, 0);
}

int32_t InputMethodAbility::HideKeyboardImplWithoutLock(int32_t cmdId, uint32_t sessionId)
{
    if (cmdId != cmdId_) {
        IMSA_HILOGE("current is not last cmd cur: %{public}d, cmdId_: %{public}d!", cmdId, cmdId_);
        return ErrorCode::NO_ERROR;
    }
    return HideKeyboard(Trigger::IMF, sessionId);
}

int32_t InputMethodAbility::ShowKeyboard()
{
    std::lock_guard<std::recursive_mutex> lock(keyboardCmdLock_);
    int32_t cmdCount = ++cmdId_;
    return ShowKeyboardImplWithoutLock(cmdCount);
}

int32_t InputMethodAbility::ShowKeyboardImplWithLock(int32_t cmdId)
{
    std::lock_guard<std::recursive_mutex> lock(keyboardCmdLock_);
    return ShowKeyboardImplWithoutLock(cmdId);
}

int32_t InputMethodAbility::ShowKeyboardImplWithoutLock(int32_t cmdId)
{
    if (cmdId != cmdId_) {
        IMSA_HILOGE("current is not last cmd cur: %{public}d, cmdId_: %{public}d!", cmdId, cmdId_);
        return ErrorCode::NO_ERROR;
    }
    if (imeListener_ == nullptr) {
        IMSA_HILOGE("imeListener is nullptr!");
        return ErrorCode::ERROR_IME;
    }
    IMSA_HILOGI("IMA start.");
    if (panels_.Contains(SOFT_KEYBOARD)) {
        auto panel = GetSoftKeyboardPanel();
        if (panel == nullptr) {
            IMSA_HILOGE("panel is nullptr!");
            return ErrorCode::ERROR_IME;
        }
        auto flag = panel->GetPanelFlag();
        imeListener_->OnKeyboardStatus(true);
        if (flag == FLG_CANDIDATE_COLUMN) {
            IMSA_HILOGI("panel flag is candidate, no need to show.");
            NotifyKeyboardHeight(0, flag);
            return ErrorCode::NO_ERROR;
        }
        return ShowPanel(panel, flag, Trigger::IMF);
    }
    isShowAfterCreate_.store(true);
    IMSA_HILOGI("panel not create.");
    auto channel = GetInputDataChannelProxy();
    if (channel != nullptr) {
        channel->SendKeyboardStatus(KeyboardStatus::SHOW);
    }
    imeListener_->OnKeyboardStatus(true);
    return ErrorCode::NO_ERROR;
}

void InputMethodAbility::NotifyPanelStatusInfo(const PanelStatusInfo &info)
{
    // CANDIDATE_COLUMN not notify
    auto channel = GetInputDataChannelProxy();
    NotifyPanelStatusInfo(info, channel);
}

int32_t InputMethodAbility::InvokeStartInputCallback(bool isNotifyInputStart)
{
    TextTotalConfig textConfig = {};
    int32_t ret = GetTextConfig(textConfig);
    if (ret == ErrorCode::NO_ERROR) {
        textConfig.inputAttribute.bundleName = GetInputAttribute().bundleName;
        return InvokeStartInputCallback(textConfig, isNotifyInputStart);
    }
    IMSA_HILOGW("failed to get text config, ret: %{public}d.", ret);
    if (imeListener_ == nullptr) {
        IMSA_HILOGE("imeListener_ is nullptr!");
        return ErrorCode::ERROR_IME;
    }
    if (isNotifyInputStart) {
        imeListener_->OnInputStart();
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::InvokeStartInputCallback(const TextTotalConfig &textConfig, bool isNotifyInputStart)
{
    if (imeListener_ == nullptr) {
        IMSA_HILOGE("imeListener_ is nullptr!");
        return ErrorCode::ERROR_IME;
    }
    positionY_ = textConfig.positionY;
    height_ = textConfig.height;
    auto task = [this, textConfig]() {
        panels_.ForEach([&textConfig](const PanelType &type, const std::shared_ptr<InputMethodPanel> &panel) {
            if (panel != nullptr) {
                panel->SetCallingWindow(textConfig.windowId, true);
            }
            return false;
        });
    };
    imeListener_->PostTaskToEventHandler(task, "SetCallingWindow");
    SetInputAttribute(textConfig.inputAttribute);
    IMSA_HILOGD("attribute info:%{public}s", textConfig.inputAttribute.ToString().c_str());
    if (kdListener_ != nullptr) {
        kdListener_->OnEditorAttributeChange(textConfig.inputAttribute);
    }
    if (isNotifyInputStart) {
        imeListener_->OnInputStart();
    }
    if (TextConfig::IsPrivateCommandValid(textConfig.privateCommand) && IsDefaultIme()) {
        IMSA_HILOGI("notify privateCommand.");
        imeListener_->ReceivePrivateCommand(textConfig.privateCommand);
    }
    if (kdListener_ != nullptr) {
        if (textConfig.cursorInfo.left != INVALID_CURSOR_VALUE) {
            kdListener_->OnCursorUpdate(
                textConfig.cursorInfo.left, textConfig.cursorInfo.top, textConfig.cursorInfo.height);
        }
        if (textConfig.textSelection.newBegin == INVALID_SELECTION_VALUE ||
            (textConfig.textSelection.newBegin == textConfig.textSelection.oldBegin &&
                textConfig.textSelection.newEnd == textConfig.textSelection.oldEnd)) {
            IMSA_HILOGD("invalid selection or no selection change");
        } else {
            kdListener_->OnSelectionChange(textConfig.textSelection.oldBegin, textConfig.textSelection.oldEnd,
                textConfig.textSelection.newBegin, textConfig.textSelection.newEnd);
        }
    }
    if (textConfig.windowId != INVALID_WINDOW_ID) {
        imeListener_->OnSetCallingWindow(textConfig.windowId);
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::InsertTextInner(const std::string &text)
{
    InputMethodSyncTrace tracer("IMA_InsertText");
    IMSA_HILOGD("InputMethodAbility start.");
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_IMA_CHANNEL_NULLPTR;
    }
    return channel->InsertText(Str8ToStr16(text));
}

int32_t InputMethodAbility::DeleteForwardInner(int32_t length)
{
    InputMethodSyncTrace tracer("IMA_DeleteForward");
    IMSA_HILOGD("InputMethodAbility start, length: %{public}d.", length);
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_IMA_CHANNEL_NULLPTR;
    }
    return channel->DeleteForward(length);
}

int32_t InputMethodAbility::DeleteBackwardInner(int32_t length)
{
    IMSA_HILOGD("InputMethodAbility start, length: %{public}d.", length);
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_IMA_CHANNEL_NULLPTR;
    }
    return channel->DeleteBackward(length);
}

int32_t InputMethodAbility::SendFunctionKey(int32_t funcKey)
{
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->SendFunctionKey(funcKey);
}

int32_t InputMethodAbility::HideKeyboardSelf()
{
    // Current Ime is exiting, hide softkeyboard will cause the TextFiled to lose focus.
    if (isImeTerminating.load()) {
        IMSA_HILOGI("Current Ime is terminating, no need to hide keyboard.");
        return ErrorCode::NO_ERROR;
    }
    InputMethodSyncTrace tracer("IMA_HideKeyboardSelf start.");
    auto ret = HideKeyboard(Trigger::IME_APP, 0);
    if (ret == ErrorCode::NO_ERROR) {
        InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_SELF);
    }
    return ret == ErrorCode::ERROR_CLIENT_NULL_POINTER ? ret : ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::SendExtendAction(int32_t action)
{
    IMSA_HILOGD("InputMethodAbility, action: %{public}d.", action);
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->HandleExtendAction(action);
}

int32_t InputMethodAbility::GetTextBeforeCursorInner(int32_t number, std::u16string &text)
{
    InputMethodSyncTrace tracer("IMA_GetForward");
    IMSA_HILOGD("InputMethodAbility, number: %{public}d.", number);
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetTextBeforeCursor(number, text);
}

int32_t InputMethodAbility::GetTextAfterCursorInner(int32_t number, std::u16string &text)
{
    InputMethodSyncTrace tracer("IMA_GetTextAfterCursor");
    IMSA_HILOGD("InputMethodAbility, number: %{public}d.", number);
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetTextAfterCursor(number, text);
}

int32_t InputMethodAbility::MoveCursor(int32_t keyCode)
{
    IMSA_HILOGD("InputMethodAbility, keyCode: %{public}d.", keyCode);
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->MoveCursor(keyCode);
}

int32_t InputMethodAbility::SelectByRange(int32_t start, int32_t end)
{
    IMSA_HILOGD("InputMethodAbility, start: %{public}d, end: %{public}d", start, end);
    if (start < 0 || end < 0) {
        IMSA_HILOGE("check parameter failed, start: %{public}d, end: %{public}d!", start, end);
        return ErrorCode::ERROR_PARAMETER_CHECK_FAILED;
    }
    auto dataChannel = GetInputDataChannelProxy();
    if (dataChannel == nullptr) {
        IMSA_HILOGE("datachannel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return dataChannel->SelectByRange(start, end);
}

int32_t InputMethodAbility::SelectByMovement(int32_t direction)
{
    IMSA_HILOGD("InputMethodAbility, direction: %{public}d.", direction);
    auto dataChannel = GetInputDataChannelProxy();
    if (dataChannel == nullptr) {
        IMSA_HILOGE("datachannel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return dataChannel->SelectByMovement(direction, 0);
}

int32_t InputMethodAbility::GetEnterKeyType(int32_t &keyType)
{
    IMSA_HILOGD("InputMethodAbility start.");
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetEnterKeyType(keyType);
}

int32_t InputMethodAbility::GetInputPattern(int32_t &inputPattern)
{
    IMSA_HILOGD("InputMethodAbility start.");
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetInputPattern(inputPattern);
}

int32_t InputMethodAbility::GetTextIndexAtCursorInner(int32_t &index)
{
    IMSA_HILOGD("InputMethodAbility start.");
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetTextIndexAtCursor(index);
}

int32_t InputMethodAbility::GetTextConfig(TextTotalConfig &textConfig)
{
    IMSA_HILOGD("InputMethodAbility start.");
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto ret = channel->GetTextConfig(textConfig);
    if (ret == ErrorCode::NO_ERROR) {
        textConfig.inputAttribute.bundleName = GetInputAttribute().bundleName;
        textConfig.inputAttribute.callingDisplayId = GetInputAttribute().callingDisplayId;
        textConfig.inputAttribute.windowId = textConfig.windowId;
    }
    return ret;
}

void InputMethodAbility::SetInputDataChannel(const sptr<IRemoteObject> &object)
{
    IMSA_HILOGD("SetInputDataChannel start.");
    std::lock_guard<std::mutex> lock(dataChannelLock_);
    auto channelProxy = std::make_shared<InputDataChannelProxy>(object);
    if (channelProxy == nullptr) {
        IMSA_HILOGE("failed to create channel proxy!");
        return;
    }
    dataChannelObject_ = object;
    dataChannelProxy_ = channelProxy;
}

std::shared_ptr<InputDataChannelProxy> InputMethodAbility::GetInputDataChannelProxy()
{
    std::lock_guard<std::mutex> lock(dataChannelLock_);
    return dataChannelProxy_;
}

void InputMethodAbility::SetInputControlChannel(sptr<IRemoteObject> &object)
{
    IMSA_HILOGD("SetInputControlChannel start.");
    std::lock_guard<std::mutex> lock(controlChannelLock_);
    std::shared_ptr<InputControlChannelProxy> channelProxy = std::make_shared<InputControlChannelProxy>(object);
    if (channelProxy == nullptr) {
        IMSA_HILOGD("channelProxy is nullptr!");
        return;
    }
    controlChannel_ = channelProxy;
}

void InputMethodAbility::ClearInputControlChannel()
{
    std::lock_guard<std::mutex> lock(controlChannelLock_);
    controlChannel_ = nullptr;
}

std::shared_ptr<InputControlChannelProxy> InputMethodAbility::GetInputControlChannel()
{
    std::lock_guard<std::mutex> lock(controlChannelLock_);
    return controlChannel_;
}

void InputMethodAbility::OnRemoteSaDied(const wptr<IRemoteObject> &object)
{
    IMSA_HILOGI("input method service died.");
    isBound_.store(false);
    ClearInputControlChannel();
    ClearSystemCmdChannel();
    {
        std::lock_guard<std::mutex> lock(abilityLock_);
        abilityManager_ = nullptr;
    }
    if (imeListener_ != nullptr) {
        imeListener_->OnInputStop();
    }
}

int32_t InputMethodAbility::GetSecurityMode(int32_t &security)
{
    IMSA_HILOGI("InputMethodAbility start.");
    int32_t securityMode = securityMode_.load();
    if (securityMode != static_cast<int32_t>(INVALID_SECURITY_MODE)) {
        IMSA_HILOGD("Get cache security mode: %{public}d.", securityMode);
        security = securityMode;
        return ErrorCode::NO_ERROR;
    }
    auto proxy = GetImsaProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("Imsa proxy is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto ret = proxy->GetSecurityMode(security);
    if (ret == ErrorCode::NO_ERROR) {
        securityMode_.store(security);
    }
    return ret;
}

void InputMethodAbility::ClearSystemCmdChannel()
{
    std::lock_guard<std::mutex> lock(systemCmdChannelLock_);
    if (systemCmdChannelProxy_ == nullptr) {
        IMSA_HILOGD("systemCmdChannelProxy_ already nullptr.");
        return;
    }
    systemCmdChannelProxy_ = nullptr;
    IMSA_HILOGD("end.");
}

sptr<SystemCmdChannelProxy> InputMethodAbility::GetSystemCmdChannelProxy()
{
    std::lock_guard<std::mutex> lock(systemCmdChannelLock_);
    return systemCmdChannelProxy_;
}

int32_t InputMethodAbility::OnConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent)
{
    IMSA_HILOGD("InputMethodAbility start.");
    {
        std::lock_guard<std::mutex> lock(systemCmdChannelLock_);
        systemCmdChannelProxy_ = new (std::nothrow) SystemCmdChannelProxy(channel);
        if (systemCmdChannelProxy_ == nullptr) {
            IMSA_HILOGE("failed to create channel proxy!");
            return ErrorCode::ERROR_CLIENT_NULL_POINTER;
        }
    }
    systemAgentStub_ = new (std::nothrow) InputMethodAgentStub();
    if (systemAgentStub_ == nullptr) {
        IMSA_HILOGE("failed to create agent!");
        systemCmdChannelProxy_ = nullptr;
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    agent = systemAgentStub_->AsObject();
    auto panel = GetSoftKeyboardPanel();
    if (panel != nullptr) {
        auto flag = panel->GetPanelFlag();
        if (panel->IsShowing() && flag != FLG_CANDIDATE_COLUMN) {
            auto keyboardSize = panel->GetKeyboardSize();
            SysPanelStatus sysPanelStatus = { inputType_, flag, keyboardSize.width, keyboardSize.height };
            NotifyPanelStatus(panel->GetPanelType(), sysPanelStatus);
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::OnSecurityChange(int32_t security)
{
    IMSA_HILOGI("InputMethodAbility start.");
    securityMode_.store(security);
    if (imeListener_ == nullptr) {
        IMSA_HILOGE("imeListener_ is nullptr!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    imeListener_->OnSecurityChange(security);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::AdjustKeyboard()
{
    if (panels_.Contains(SOFT_KEYBOARD)) {
        auto panel = GetSoftKeyboardPanel();
        if (panel == nullptr) {
            IMSA_HILOGE("panel is nullptr!");
            return ErrorCode::ERROR_IME;
        }
        auto flag = panel->GetPanelFlag();
        if (flag != FLG_FIXED) {
            IMSA_HILOGI("panel flag is not fix, no need to adjust.");
            return ErrorCode::NO_ERROR;
        }
        return panel->AdjustKeyboard();
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::CreatePanel(const std::shared_ptr<AbilityRuntime::Context> &context,
    const PanelInfo &panelInfo, std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    IMSA_HILOGI("InputMethodAbility start.");
    auto panelHeightCallback = [this](uint32_t panelHeight, PanelFlag panelFlag) {
        NotifyKeyboardHeight(panelHeight, panelFlag);
    };
    auto flag = panels_.ComputeIfAbsent(panelInfo.panelType,
        [panelHeightCallback, &panelInfo, &context, &inputMethodPanel](
            const PanelType &panelType, std::shared_ptr<InputMethodPanel> &panel) {
            inputMethodPanel = std::make_shared<InputMethodPanel>();
            inputMethodPanel->SetPanelHeightCallback(panelHeightCallback);
            auto ret = inputMethodPanel->CreatePanel(context, panelInfo);
            if (ret == ErrorCode::NO_ERROR) {
                panel = inputMethodPanel;
                return true;
            }
            inputMethodPanel = nullptr;
            return false;
        });
    if (flag && isShowAfterCreate_.load() && panelInfo.panelType == SOFT_KEYBOARD &&
        panelInfo.panelFlag != FLG_CANDIDATE_COLUMN) {
        isShowAfterCreate_.store(false);
        auto task = std::make_shared<TaskImsaShowKeyboard>();
        TaskManager::GetInstance().PostTask(task);
    }
    return flag ? ErrorCode::NO_ERROR : ErrorCode::ERROR_OPERATE_PANEL;
}

int32_t InputMethodAbility::DestroyPanel(const std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    IMSA_HILOGI("InputMethodAbility start.");
    if (inputMethodPanel == nullptr) {
        IMSA_HILOGE("panel is nullptr!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto ret = inputMethodPanel->DestroyPanel();
    if (ret == ErrorCode::NO_ERROR) {
        PanelType panelType = inputMethodPanel->GetPanelType();
        panels_.Erase(panelType);
    }
    return ret;
}

int32_t InputMethodAbility::ShowPanel(const std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    std::lock_guard<std::recursive_mutex> lock(keyboardCmdLock_);
    if (inputMethodPanel == nullptr) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    return ShowPanel(inputMethodPanel, inputMethodPanel->GetPanelFlag(), Trigger::IME_APP);
}

int32_t InputMethodAbility::HidePanel(const std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    if (inputMethodPanel == nullptr) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    // Current Ime is exiting, hide softkeyboard will cause the TextFiled to lose focus.
    if (isImeTerminating.load() && inputMethodPanel->GetPanelType() == PanelType::SOFT_KEYBOARD) {
        IMSA_HILOGI("Current Ime is terminating, no need to hide keyboard.");
        return ErrorCode::NO_ERROR;
    }
    if (isShowAfterCreate_.load() && inputMethodPanel->GetPanelType() == PanelType::SOFT_KEYBOARD &&
        inputMethodPanel->GetPanelFlag() != PanelFlag::FLG_CANDIDATE_COLUMN) {
        isShowAfterCreate_.store(false);
    }
    std::lock_guard<std::recursive_mutex> lock(keyboardCmdLock_);
    return HidePanel(inputMethodPanel, inputMethodPanel->GetPanelFlag(), Trigger::IME_APP, 0);
}

int32_t InputMethodAbility::ShowPanel(
    const std::shared_ptr<InputMethodPanel> &inputMethodPanel, PanelFlag flag, Trigger trigger)
{
    if (inputMethodPanel == nullptr) {
        return ErrorCode::ERROR_IMA_NULLPTR;
    }
    if (trigger == Trigger::IME_APP && GetInputDataChannelProxy() == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_IMA_CHANNEL_NULLPTR;
    }
    if (flag == FLG_FIXED && inputMethodPanel->GetPanelType() == SOFT_KEYBOARD) {
        auto ret = inputMethodPanel->SetTextFieldAvoidInfo(positionY_, height_);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("failed to set keyBoard, ret: %{public}d!", ret);
        }
    }
    auto keyboardSize = inputMethodPanel->GetKeyboardSize();
    SysPanelStatus sysPanelStatus = { inputType_, flag, keyboardSize.width, keyboardSize.height };
    NotifyPanelStatus(inputMethodPanel->GetPanelType(), sysPanelStatus);
    auto ret = inputMethodPanel->ShowPanel();
    if (ret == ErrorCode::NO_ERROR) {
        NotifyPanelStatusInfo({
            {inputMethodPanel->GetPanelType(), flag},
            true, trigger
        });
    }
    return ret;
}

int32_t InputMethodAbility::HidePanel(
    const std::shared_ptr<InputMethodPanel> &inputMethodPanel, PanelFlag flag, Trigger trigger, uint32_t sessionId)
{
    if (inputMethodPanel == nullptr) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto ret = inputMethodPanel->HidePanel();
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGD("failed, ret: %{public}d", ret);
        return ret;
    }
    NotifyPanelStatusInfo({
        {inputMethodPanel->GetPanelType(), flag},
        false, trigger, sessionId
    });
    if (trigger == Trigger::IMF && inputMethodPanel->GetPanelType() == PanelType::SOFT_KEYBOARD) {
        FinishTextPreview(true);
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::NotifyPanelStatus(PanelType panelType, SysPanelStatus &sysPanelStatus)
{
    if (panelType != PanelType::SOFT_KEYBOARD) {
        return ErrorCode::NO_ERROR;
    }
    // If it is not binding, do not need to notify the panel
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        return ErrorCode::NO_ERROR;
    }
    sysPanelStatus.inputType = inputType_;
    auto systemChannel = GetSystemCmdChannelProxy();
    if (systemChannel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto panel = GetSoftKeyboardPanel();
    if (panel != nullptr) {
        sysPanelStatus.isMainDisplay = panel->IsInMainDisplay();
    }
    return systemChannel->NotifyPanelStatus(sysPanelStatus);
}

void InputMethodAbility::SetInputAttribute(const InputAttribute &inputAttribute)
{
    std::lock_guard<std::mutex> lock(inputAttrLock_);
    inputAttribute_ = inputAttribute;
}

void InputMethodAbility::ClearInputAttribute()
{
    std::lock_guard<std::mutex> lock(inputAttrLock_);
    inputAttribute_ = {};
}

InputAttribute InputMethodAbility::GetInputAttribute()
{
    std::lock_guard<std::mutex> lock(inputAttrLock_);
    return inputAttribute_;
}

int32_t InputMethodAbility::HideKeyboard(Trigger trigger, uint32_t sessionId)
{
    isShowAfterCreate_.store(false);
    InputMethodSyncTrace tracer("IMA_HideKeyboard");
    if (imeListener_ == nullptr) {
        IMSA_HILOGE("imeListener_ is nullptr!");
        return ErrorCode::ERROR_IME;
    }
    IMSA_HILOGD("IMA, trigger: %{public}d.", static_cast<int32_t>(trigger));
    if (panels_.Contains(SOFT_KEYBOARD)) {
        auto panel = GetSoftKeyboardPanel();
        if (panel == nullptr) {
            IMSA_HILOGE("panel is nullptr!");
            return ErrorCode::ERROR_IME;
        }
        auto flag = panel->GetPanelFlag();
        imeListener_->OnKeyboardStatus(false);
        if (flag == FLG_CANDIDATE_COLUMN) {
            IMSA_HILOGI("panel flag is candidate, no need to hide.");
            return ErrorCode::NO_ERROR;
        }
        return HidePanel(panel, flag, trigger, sessionId);
    }
    IMSA_HILOGI("panel is not created.");
    imeListener_->OnKeyboardStatus(false);
    auto channel = GetInputDataChannelProxy();
    if (channel != nullptr) {
        channel->SendKeyboardStatus(KeyboardStatus::HIDE);
    }
    auto controlChannel = GetInputControlChannel();
    if (controlChannel != nullptr && trigger == Trigger::IME_APP) {
        controlChannel->HideKeyboardSelf();
    }
    return ErrorCode::NO_ERROR;
}

std::shared_ptr<InputMethodPanel> InputMethodAbility::GetSoftKeyboardPanel()
{
    auto result = panels_.Find(SOFT_KEYBOARD);
    if (!result.first) {
        return nullptr;
    }
    auto panel = result.second;
    if (!BlockRetry(FIND_PANEL_RETRY_INTERVAL, MAX_RETRY_TIMES, [panel]() -> bool {
            return panel != nullptr && panel->windowId_ != InputMethodPanel::INVALID_WINDOW_ID;
        })) {
        return nullptr;
    }
    return panel;
}

bool InputMethodAbility::IsCurrentIme()
{
    IMSA_HILOGD("InputMethodAbility start.");
    if (isCurrentIme_) {
        return true;
    }
    std::lock_guard<std::mutex> lock(imeCheckMutex_);
    if (isCurrentIme_) {
        return true;
    }
    auto proxy = GetImsaProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("failed to get imsa proxy!");
        return false;
    }
    if (proxy->IsCurrentIme()) {
        isCurrentIme_ = true;
        return true;
    }
    return false;
}

bool InputMethodAbility::IsDefaultIme()
{
    IMSA_HILOGD("InputMethodAbility start");
    if (isDefaultIme_) {
        return true;
    }
    std::lock_guard<std::mutex> lock(defaultImeCheckMutex_);
    if (isDefaultIme_) {
        return true;
    }
    auto proxy = GetImsaProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("failed to get imsa proxy!");
        return false;
    }
    auto ret = proxy->IsDefaultIme();
    if (ret == ErrorCode::NO_ERROR) {
        isDefaultIme_ = true;
        return true;
    }
    IMSA_HILOGE("call IsDefaultIme failed, ret: %{public}d!", ret);
    return false;
}

bool InputMethodAbility::IsEnable()
{
    if (imeListener_ == nullptr) {
        return false;
    }
    return imeListener_->IsEnable();
}

bool InputMethodAbility::IsSystemApp()
{
    IMSA_HILOGD("InputMethodAbility start");
    if (isSystemApp_) {
        return true;
    }
    std::lock_guard<std::mutex> lock(systemAppCheckMutex_);
    if (isSystemApp_) {
        return true;
    }
    auto proxy = GetImsaProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("failed to get imsa proxy!");
        return false;
    }
    if (proxy->IsSystemApp()) {
        isSystemApp_ = true;
        return true;
    }
    return false;
}

int32_t InputMethodAbility::ExitCurrentInputType()
{
    IMSA_HILOGD("InputMethodAbility start.");
    ClearInputType();
    auto panel = GetSoftKeyboardPanel();
    if (panel != nullptr) {
        auto keyboardSize = panel->GetKeyboardSize();
        SysPanelStatus sysPanelStatus = { inputType_, panel->GetPanelFlag(), keyboardSize.width, keyboardSize.height };
        NotifyPanelStatus(panel->GetPanelType(), sysPanelStatus);
    }
    auto proxy = GetImsaProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("failed to get imsa proxy!");
        return false;
    }
    return proxy->ExitCurrentInputType();
}

void InputMethodAbility::ClearInputType()
{
    std::lock_guard<std::mutex> lock(inputTypeLock_);
    if (inputType_ != InputType::SECURITY_INPUT) {
        inputType_ = InputType::NONE;
    }
}

int32_t InputMethodAbility::IsPanelShown(const PanelInfo &panelInfo, bool &isShown)
{
    isShown = false;
    auto result = panels_.Find(panelInfo.panelType);
    if (!result.first) {
        IMSA_HILOGI("panel type: %{public}d not found.", static_cast<int32_t>(panelInfo.panelType));
        return ErrorCode::NO_ERROR;
    }
    auto panel = result.second;
    if (panel->GetPanelType() == PanelType::SOFT_KEYBOARD && panel->GetPanelFlag() != panelInfo.panelFlag) {
        IMSA_HILOGI("queried flag: %{public}d, current flag: %{public}d, panel not found.",
            static_cast<int32_t>(panelInfo.panelFlag), static_cast<int32_t>(panel->GetPanelFlag()));
        return ErrorCode::NO_ERROR;
    }
    isShown = panel->IsShowing();
    IMSA_HILOGI("type: %{public}d, flag: %{public}d, result: %{public}d.", static_cast<int32_t>(panelInfo.panelType),
        static_cast<int32_t>(panelInfo.panelFlag), isShown);
    return ErrorCode::NO_ERROR;
}

void InputMethodAbility::OnClientInactive(const sptr<IRemoteObject> &channel)
{
    IMSA_HILOGI("client inactive.");
    if (imeListener_ != nullptr) {
        imeListener_->OnInputFinish();
    }
    auto channelProxy = std::make_shared<InputDataChannelProxy>(channel);
    if (channelProxy == nullptr) {
        IMSA_HILOGE("failed to create channel proxy!");
        return;
    }
    auto panel = GetSoftKeyboardPanel();
    if (imeListener_ != nullptr && panel != nullptr && panel->GetPanelFlag() != PanelFlag::FLG_FIXED) {
        imeListener_->OnKeyboardStatus(false);
    }
    panels_.ForEach([this, &channelProxy](const PanelType &panelType, const std::shared_ptr<InputMethodPanel> &panel) {
        if (panelType != PanelType::SOFT_KEYBOARD || panel->GetPanelFlag() != PanelFlag::FLG_FIXED) {
            auto ret = panel->HidePanel();
            if (ret != ErrorCode::NO_ERROR) {
                IMSA_HILOGE("failed, ret: %{public}d", ret);
                return false;
            }
            NotifyPanelStatusInfo({ { panel->GetPanelType(), panel->GetPanelFlag() }, false, Trigger::IME_APP },
                channelProxy);
            // finish previewing text when soft keyboard hides
            if (panel->GetPanelType() == PanelType::SOFT_KEYBOARD) {
                FinishTextPreview(true);
            }
        }
        return false;
    });
    ClearDataChannel(channel);
}

void InputMethodAbility::NotifyKeyboardHeight(uint32_t panelHeight, PanelFlag panelFlag)
{
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return;
    }
    IMSA_HILOGD("notify panel height: %{public}u, flag: %{public}d.", panelHeight, static_cast<int32_t>(panelFlag));
    if (panelFlag != PanelFlag::FLG_FIXED) {
        channel->NotifyKeyboardHeight(0);
        return;
    }
    channel->NotifyKeyboardHeight(panelHeight);
}

int32_t InputMethodAbility::SendPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    if (!IsDefaultIme()) {
        IMSA_HILOGE("current is not default ime!");
        return ErrorCode::ERROR_NOT_DEFAULT_IME;
    }
    if (!TextConfig::IsPrivateCommandValid(privateCommand)) {
        IMSA_HILOGE("privateCommand is limit 32KB, count limit 5!");
        return ErrorCode::ERROR_INVALID_PRIVATE_COMMAND_SIZE;
    }
    if (TextConfig::IsSystemPrivateCommand(privateCommand)) {
        auto systemChannel = GetSystemCmdChannelProxy();
        if (systemChannel == nullptr) {
            IMSA_HILOGE("channel is nullptr!");
            return ErrorCode::ERROR_SYSTEM_CMD_CHANNEL_ERROR;
        }
        return systemChannel->SendPrivateCommand(privateCommand);
    } else {
        auto channel = GetInputDataChannelProxy();
        if (channel == nullptr) {
            IMSA_HILOGE("channel is nullptr!");
            return ErrorCode::ERROR_CLIENT_NULL_POINTER;
        }
        return channel->SendPrivateCommand(privateCommand);
    }
}

int32_t InputMethodAbility::ReceivePrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    if (imeListener_ == nullptr) {
        IMSA_HILOGE("imeListener is nullptr!");
        return ErrorCode::ERROR_IME;
    }
    imeListener_->ReceivePrivateCommand(privateCommand);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::SetPreviewTextInner(const std::string &text, const Range &range)
{
    InputMethodSyncTrace tracer("IMA_SetPreviewText");
    auto dataChannel = GetInputDataChannelProxy();
    if (dataChannel == nullptr) {
        IMSA_HILOGE("dataChannel is nullptr!");
        return ErrorCode::ERROR_IMA_CHANNEL_NULLPTR;
    }
    return dataChannel->SetPreviewText(text, range);
}

int32_t InputMethodAbility::FinishTextPreviewInner(bool isAsync)
{
    InputMethodSyncTrace tracer("IMA_FinishTextPreview");
    auto dataChannel = GetInputDataChannelProxy();
    if (dataChannel == nullptr) {
        IMSA_HILOGE("dataChannel is nullptr!");
        return ErrorCode::ERROR_IMA_CHANNEL_NULLPTR;
    }
    return dataChannel->FinishTextPreview(isAsync);
}

int32_t InputMethodAbility::GetCallingWindowInfo(CallingWindowInfo &windowInfo)
{
    IMSA_HILOGD("IMA start.");
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NOT_FOUND;
    }
    auto panel = GetSoftKeyboardPanel();
    if (panel == nullptr) {
        IMSA_HILOGE("panel not found!");
        return ErrorCode::ERROR_PANEL_NOT_FOUND;
    }
    TextTotalConfig textConfig;
    int32_t ret = GetTextConfig(textConfig);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to get window id, ret: %{public}d!", ret);
        return ErrorCode::ERROR_GET_TEXT_CONFIG;
    }
    ret = panel->SetCallingWindow(textConfig.windowId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to set calling window, ret: %{public}d!", ret);
        return ret;
    }
    ret = panel->GetCallingWindowInfo(windowInfo);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to get calling window, ret: %{public}d", ret);
    }
    return ret;
}

void InputMethodAbility::NotifyPanelStatusInfo(
    const PanelStatusInfo &info, std::shared_ptr<InputDataChannelProxy> &channelProxy)
{
    // CANDIDATE_COLUMN not notify
    if (info.panelInfo.panelFlag == PanelFlag::FLG_CANDIDATE_COLUMN) {
        return;
    }
    if (channelProxy != nullptr) {
        channelProxy->NotifyPanelStatusInfo(info);
    }

    auto controlChannel = GetInputControlChannel();
    if (controlChannel != nullptr && info.trigger == Trigger::IME_APP && !info.visible) {
        controlChannel->HideKeyboardSelf();
    }
}

int32_t InputMethodAbility::SendMessage(const ArrayBuffer &arrayBuffer)
{
    int32_t securityMode = INVALID_SECURITY_MODE;
    auto ret = GetSecurityMode(securityMode);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Get security mode failed!");
        return ret;
    }
    if (!ArrayBuffer::IsSizeValid(arrayBuffer)) {
        IMSA_HILOGE("arrayBuffer size is invalid!");
        return ErrorCode::ERROR_INVALID_ARRAY_BUFFER_SIZE;
    }
    if (securityMode != static_cast<int32_t>(SecurityMode::FULL)) {
        IMSA_HILOGE("Security mode must be FULL!.");
        return ErrorCode::ERROR_SECURITY_MODE_OFF;
    }
    auto dataChannel = GetInputDataChannelProxy();
    if (dataChannel == nullptr) {
        IMSA_HILOGE("datachannel is nullptr.");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return dataChannel->SendMessage(arrayBuffer);
}

int32_t InputMethodAbility::RecvMessage(const ArrayBuffer &arrayBuffer)
{
    int32_t securityMode = -1;
    auto ret = GetSecurityMode(securityMode);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Get security mode failed!");
        return ret;
    }
    if (securityMode != static_cast<int32_t>(SecurityMode::FULL)) {
        IMSA_HILOGE("Security mode must be FULL!.");
        return ErrorCode::ERROR_SECURITY_MODE_OFF;
    }
    auto msgHandlerCallback = GetMsgHandlerCallback();
    if (msgHandlerCallback == nullptr) {
        IMSA_HILOGW("Message handler was not regist!");
        return ErrorCode::ERROR_MSG_HANDLER_NOT_REGIST;
    }
    return msgHandlerCallback->OnMessage(arrayBuffer);
}

int32_t InputMethodAbility::RegisterMsgHandler(const std::shared_ptr<MsgHandlerCallbackInterface> &msgHandler)
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

std::shared_ptr<MsgHandlerCallbackInterface> InputMethodAbility::GetMsgHandlerCallback()
{
    std::lock_guard<decltype(msgHandlerMutex_)> lock(msgHandlerMutex_);
    return msgHandler_;
}

int32_t InputMethodAbility::StartInput(const InputClientInfo &clientInfo, bool isBindFromClient)
{
    auto ret = StartInputInner(clientInfo, isBindFromClient);
    if (ret == ErrorCode::NO_ERROR) {
        return ret;
    }
    ReportImeStartInput(IInputMethodCore::START_INPUT, ret, clientInfo.isShowKeyboard);
    return ret;
}

int32_t InputMethodAbility::InsertText(const std::string text)
{
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    auto ret = InsertTextInner(text);
    int64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    ReportBaseTextOperation(IInputDataChannel::INSERT_TEXT, ret, end - start);
    return ret;
}

int32_t InputMethodAbility::DeleteForward(int32_t length)
{
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    auto ret = DeleteForwardInner(length);
    int64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    ReportBaseTextOperation(IInputDataChannel::DELETE_FORWARD, ret, end - start);
    return ret;
}

int32_t InputMethodAbility::DeleteBackward(int32_t length)
{
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    auto ret = DeleteBackwardInner(length);
    int64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    ReportBaseTextOperation(IInputDataChannel::DELETE_BACKWARD, ret, end - start);
    return ret;
}

int32_t InputMethodAbility::SetPreviewText(const std::string &text, const Range &range)
{
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    auto ret = SetPreviewTextInner(text, range);
    int64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    ReportBaseTextOperation(IInputDataChannel::SET_PREVIEW_TEXT, ret, end - start);
    return ret;
}

int32_t InputMethodAbility::FinishTextPreview(bool isSync)
{
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    auto ret = FinishTextPreviewInner(isSync);
    int64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    ReportBaseTextOperation(IInputDataChannel::FINISH_TEXT_PREVIEW, ret, end - start);
    return ret;
}

int32_t InputMethodAbility::GetTextBeforeCursor(int32_t number, std::u16string &text)
{
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    auto ret = GetTextBeforeCursorInner(number, text);
    int64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    ReportBaseTextOperation(IInputDataChannel::GET_TEXT_BEFORE_CURSOR, ret, end - start);
    return ret;
}
int32_t InputMethodAbility::GetTextAfterCursor(int32_t number, std::u16string &text)
{
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    auto ret = GetTextAfterCursorInner(number, text);
    int64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    ReportBaseTextOperation(IInputDataChannel::GET_TEXT_AFTER_CURSOR, ret, end - start);
    return ret;
}
int32_t InputMethodAbility::GetTextIndexAtCursor(int32_t &index)
{
    int64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    auto ret = GetTextIndexAtCursorInner(index);
    int64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    ReportBaseTextOperation(IInputDataChannel::GET_TEXT_INDEX_AT_CURSOR, ret, end - start);
    return ret;
}

void InputMethodAbility::SetBindClientInfo(const InputClientInfo &clientInfo)
{
    std::lock_guard<std::mutex> lock(bindClientInfoLock_);
    bindClientInfo_ = { clientInfo.pid, clientInfo.type, clientInfo.name };
}

HiSysEventClientInfo InputMethodAbility::GetBindClientInfo()
{
    std::lock_guard<std::mutex> lock(bindClientInfoLock_);
    return bindClientInfo_;
}

void InputMethodAbility::ReportImeStartInput(
    int32_t eventCode, int32_t errCode, bool isShowKeyboard, int64_t consumeTime)
{
    IMSA_HILOGD("HiSysEvent report start:[%{public}d, %{public}d]!", eventCode, errCode);
    auto clientInfo = GetBindClientInfo();
    auto evenInfo = HiSysOriginalInfo::Builder()
                        .SetPeerName(clientInfo.name)
                        .SetPeerPid(clientInfo.pid)
                        .SetIsShowKeyboard(isShowKeyboard)
                        .SetEventCode(eventCode)
                        .SetErrCode(errCode)
                        .SetImeCbTime(consumeTime)
                        .Build();
    ImaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::IME_START_INPUT, *evenInfo);
    IMSA_HILOGD("HiSysEvent report end:[%{public}d, %{public}d]!", eventCode, errCode);
}

void InputMethodAbility::ReportBaseTextOperation(int32_t eventCode, int32_t errCode, int64_t consumeTime)
{
    IMSA_HILOGD("HiSysEvent report start:[%{public}d, %{public}d]!", eventCode, errCode);
    auto clientInfo = GetBindClientInfo();
    if (errCode == ErrorCode::NO_ERROR && consumeTime > BASE_TEXT_OPERATION_TIMEOUT) {
        errCode = ErrorCode::ERROR_DEAL_TIMEOUT;
    }
    auto evenInfo = HiSysOriginalInfo::Builder()
                        .SetPeerName(clientInfo.name)
                        .SetPeerPid(clientInfo.pid)
                        .SetClientType(clientInfo.type)
                        .SetEventCode(eventCode)
                        .SetErrCode(errCode)
                        .SetBaseTextOperatorTime(consumeTime)
                        .Build();
    ImaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::BASE_TEXT_OPERATOR, *evenInfo);
    IMSA_HILOGD("HiSysEvent report end:[%{public}d, %{public}d]!", eventCode, errCode);
}

int32_t InputMethodAbility::OnCallingDisplayIdChanged(uint64_t displayId)
{
    IMSA_HILOGD("InputMethodAbility calling display: %{public}" PRIu64".", displayId);
    if (imeListener_ == nullptr) {
        IMSA_HILOGD("imeListener_ is nullptr!");
        return ErrorCode::NO_ERROR;
    }
    auto windowId = GetInputAttribute().windowId;
    auto task = [this, windowId]() {
        panels_.ForEach([windowId](const PanelType &panelType, const std::shared_ptr<InputMethodPanel> &panel) {
            if (panel != nullptr) {
                panel->SetCallingWindow(windowId, true);
            }
            return false;
        });
    };
    imeListener_->PostTaskToEventHandler(task, "SetCallingWindow");
    {
        std::lock_guard<std::mutex> lock(inputAttrLock_);
        inputAttribute_.callingDisplayId = displayId;
    }
    imeListener_->OnCallingDisplayIdChanged(displayId);
    return ErrorCode::NO_ERROR;
}

uint64_t InputMethodAbility::GetCallingWindowDisplayId()
{
    return GetInputAttribute().callingDisplayId;
}

int32_t InputMethodAbility::OnSendPrivateData(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    auto ret = ReceivePrivateCommand(privateCommand);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("OnSendPrivateData failed!");
    }
    IMSA_HILOGI("InputMethodAbility OnSendPrivateData success.");
    return ret;
}
} // namespace MiscServices
} // namespace OHOS