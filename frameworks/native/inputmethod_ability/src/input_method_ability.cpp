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
#include "input_method_agent_proxy.h"
#include "input_method_core_proxy.h"
#include "input_method_utils.h"
#include "inputmethod_sysevent.h"
#include "iservice_registry.h"
#include "itypes_util.h"
#include "message_parcel.h"
#include "string_ex.h"
#include "sys/prctl.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
class MessageHandler;
using namespace MessageID;
sptr<InputMethodAbility> InputMethodAbility::instance_;
std::mutex InputMethodAbility::instanceLock_;
constexpr double INVALID_CURSOR_VALUE = -1.0;
constexpr int32_t INVALID_SELECTION_VALUE = -1;
constexpr uint32_t FIND_PANEL_RETRY_INTERVAL = 10;
constexpr uint32_t MAX_RETRY_TIMES = 100;
InputMethodAbility::InputMethodAbility() : msgHandler_(nullptr), stop_(false)
{
}

InputMethodAbility::~InputMethodAbility()
{
    IMSA_HILOGI("InputMethodAbility::~InputMethodAbility");
    QuitWorkThread();
    if (msgHandler_ != nullptr) {
        delete msgHandler_;
        msgHandler_ = nullptr;
    }
}

sptr<InputMethodAbility> InputMethodAbility::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            IMSA_HILOGI("InputMethodAbility::GetInstance need new IMA");
            instance_ = new (std::nothrow) InputMethodAbility();
            if (instance_ == nullptr) {
                IMSA_HILOGI("instance is nullptr.");
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
    IMSA_HILOGD("InputMethodAbility get imsa proxy");
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGE("systemAbilityManager is nullptr");
        return nullptr;
    }
    auto systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
    if (systemAbility == nullptr) {
        IMSA_HILOGE("systemAbility is nullptr");
        return nullptr;
    }
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new (std::nothrow) InputDeathRecipient();
        if (deathRecipient_ == nullptr) {
            IMSA_HILOGE("failed to new death recipient");
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

int32_t InputMethodAbility::SetCoreAndAgent()
{
    IMSA_HILOGD("InputMethodAbility, run in");
    if (isBound_.load()) {
        IMSA_HILOGD("already bound");
        return ErrorCode::NO_ERROR;
    }
    auto proxy = GetImsaProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("imsa proxy is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    int32_t ret = proxy->SetCoreAndAgent(coreStub_, agentStub_);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("set failed, ret: %{public}d", ret);
        return ret;
    }
    isBound_.store(true);
    IMSA_HILOGD("set successfully");
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::UnRegisteredProxyIme(UnRegisteredType type)
{
    isBound_.store(false);
    auto proxy = GetImsaProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("imsa proxy is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return proxy->UnRegisteredProxyIme(type, coreStub_);
}

void InputMethodAbility::Initialize()
{
    IMSA_HILOGI("InputMethodAbility::Initialize");
    coreStub_ = new (std::nothrow) InputMethodCoreStub();
    if (coreStub_ == nullptr) {
        IMSA_HILOGE("failed to create core");
        return;
    }
    agentStub_ = new (std::nothrow) InputMethodAgentStub();
    if (agentStub_ == nullptr) {
        IMSA_HILOGE("failed to create agent");
        return;
    }
    msgHandler_ = new (std::nothrow) MessageHandler();
    if (msgHandler_ == nullptr) {
        IMSA_HILOGE("failed to create message handler");
        return;
    }
    coreStub_->SetMessageHandler(msgHandler_);
    agentStub_->SetMessageHandler(msgHandler_);
    workThreadHandler = std::thread([this] { WorkThread(); });
}

void InputMethodAbility::SetImeListener(std::shared_ptr<InputMethodEngineListener> imeListener)
{
    IMSA_HILOGI("InputMethodAbility.");
    if (imeListener_ == nullptr) {
        imeListener_ = std::move(imeListener);
    }
}

void InputMethodAbility::SetKdListener(std::shared_ptr<KeyboardListener> kdListener)
{
    IMSA_HILOGI("InputMethodAbility.");
    if (kdListener_ == nullptr) {
        kdListener_ = std::move(kdListener);
    }
}

void InputMethodAbility::WorkThread()
{
    prctl(PR_SET_NAME, "IMAWorkThread");
    while (!stop_) {
        Message *msg = msgHandler_->GetMessage();
        switch (msg->msgId_) {
            case MSG_ID_INIT_INPUT_CONTROL_CHANNEL: {
                OnInitInputControlChannel(msg);
                break;
            }
            case MSG_ID_ON_CURSOR_UPDATE: {
                OnCursorUpdate(msg);
                break;
            }
            case MSG_ID_ON_SELECTION_CHANGE: {
                OnSelectionChange(msg);
                break;
            }
            case MSG_ID_ON_CONFIGURATION_CHANGE: {
                OnConfigurationChange(msg);
                break;
            }
            case MSG_ID_STOP_INPUT_SERVICE: {
                if (imeListener_ != nullptr) {
                    imeListener_->OnInputStop();
                }
                isBound_.store(false);
                break;
            }
            case MSG_ID_SET_SUBTYPE: {
                OnSetSubtype(msg);
                break;
            }
            default: {
                IMSA_HILOGD("the message is %{public}d.", msg->msgId_);
                break;
            }
        }
        delete msg;
        msg = nullptr;
    }
}

void InputMethodAbility::OnInitInputControlChannel(Message *msg)
{
    IMSA_HILOGI("InputMethodAbility::OnInitInputControlChannel");
    MessageParcel *data = msg->msgContent_;
    sptr<IRemoteObject> channelObject = data->ReadRemoteObject();
    if (channelObject == nullptr) {
        IMSA_HILOGI("InputMethodAbility::OnInitInputControlChannel channelObject is nullptr");
        return;
    }
    SetInputControlChannel(channelObject);
}

int32_t InputMethodAbility::StartInput(InputClientInfo &clientInfo, bool isBindFromClient)
{
    IMSA_HILOGI("InputMethodAbility::isShowKeyboard: %{public}d", clientInfo.isShowKeyboard);
    if (clientInfo.channel->AsObject() == nullptr) {
        IMSA_HILOGE("InputMethodAbility::channelObject is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    SetInputDataChannel(clientInfo.channel->AsObject());
    isBindFromClient ? OnTextConfigChange(clientInfo.config) : NotifyAllTextConfig();
    if (imeListener_ == nullptr) {
        IMSA_HILOGE("InputMethodAbility, imeListener is nullptr");
        return ErrorCode::ERROR_IME;
    }
    imeListener_->OnInputStart();
    return clientInfo.isShowKeyboard ? ShowKeyboard() : ErrorCode::NO_ERROR;
}

void InputMethodAbility::OnSetSubtype(Message *msg)
{
    IMSA_HILOGI("InputMethodAbility::OnSetSubtype");
    auto data = msg->msgContent_;
    SubProperty subProperty;
    if (!ITypesUtil::Unmarshal(*data, subProperty)) {
        IMSA_HILOGE("read message parcel failed");
        return;
    }
    if (imeListener_ == nullptr) {
        IMSA_HILOGI("InputMethodAbility::OnSetSubtype imeListener_ is nullptr");
        return;
    }
    imeListener_->OnSetSubtype(subProperty);
}

void InputMethodAbility::ClearDataChannel(const sptr<IRemoteObject> &channel)
{
    IMSA_HILOGI("run in");
    std::lock_guard<std::mutex> lock(dataChannelLock_);
    if (dataChannelObject_ == nullptr || channel == nullptr) {
        return;
    }
    if (dataChannelObject_.GetRefPtr() == channel.GetRefPtr()) {
        IMSA_HILOGI("clear data channel");
        dataChannelObject_ = nullptr;
        dataChannelProxy_ = nullptr;
    }
}

int32_t InputMethodAbility::StopInput(const sptr<IRemoteObject> &channelObject)
{
    IMSA_HILOGI("run in");
    HideKeyboard();
    ClearDataChannel(channelObject);
    if (imeListener_ != nullptr) {
        imeListener_->OnInputFinish();
    }
    return ErrorCode::NO_ERROR;
}

bool InputMethodAbility::DispatchKeyEvent(const std::shared_ptr<MMI::KeyEvent> &keyEvent)
{
    IMSA_HILOGD("InputMethodAbility, run in");
    if (keyEvent == nullptr) {
        IMSA_HILOGE("keyEvent is nullptr");
        return false;
    }
    if (kdListener_ == nullptr) {
        IMSA_HILOGI("kdListener_ is nullptr");
        return false;
    }
    bool isFullKeyEventConsumed = kdListener_->OnKeyEvent(keyEvent);
    bool isKeyEventConsumed = kdListener_->OnKeyEvent(keyEvent->GetKeyCode(), keyEvent->GetKeyAction());
    return isFullKeyEventConsumed || isKeyEventConsumed;
}

void InputMethodAbility::SetCallingWindow(uint32_t windowId)
{
    IMSA_HILOGI("InputMethodAbility::SetCallingWindow");

    if (imeListener_ == nullptr) {
        IMSA_HILOGI("InputMethodAbility::SetCallingWindow imeListener_ is nullptr");
        return;
    }
    panels_.ForEach([windowId](const PanelType &panelType, const std::shared_ptr<InputMethodPanel> &panel) {
        panel->SetCallingWindow(windowId);
        return false;
    });
    imeListener_->OnSetCallingWindow(windowId);
}

void InputMethodAbility::OnCursorUpdate(Message *msg)
{
    IMSA_HILOGD("InputMethodAbility::OnCursorUpdate");
    MessageParcel *data = msg->msgContent_;
    int32_t positionX = data->ReadInt32();
    int32_t positionY = data->ReadInt32();
    int32_t height = data->ReadInt32();
    if (kdListener_ == nullptr) {
        IMSA_HILOGE("InputMethodAbility::OnCursorUpdate kdListener_ is nullptr");
        return;
    }
    kdListener_->OnCursorUpdate(positionX, positionY, height);
}

void InputMethodAbility::OnSelectionChange(Message *msg)
{
    IMSA_HILOGD("InputMethodAbility::OnSelectionChange");
    MessageParcel *data = msg->msgContent_;
    std::string text = Str16ToStr8(data->ReadString16());
    int32_t oldBegin = data->ReadInt32();
    int32_t oldEnd = data->ReadInt32();
    int32_t newBegin = data->ReadInt32();
    int32_t newEnd = data->ReadInt32();

    if (kdListener_ == nullptr) {
        IMSA_HILOGE("InputMethodAbility::OnSelectionChange kdListener_ is nullptr");
        return;
    }
    kdListener_->OnTextChange(text);

    kdListener_->OnSelectionChange(oldBegin, oldEnd, newBegin, newEnd);
}

void InputMethodAbility::OnConfigurationChange(Message *msg)
{
    IMSA_HILOGD("InputMethodAbility in.");
    if (kdListener_ == nullptr) {
        IMSA_HILOGE("InputMethodAbility in, kdListener_ is nullptr");
        return;
    }
    MessageParcel *data = msg->msgContent_;
    InputAttribute attribute;
    attribute.enterKeyType = data->ReadInt32();
    attribute.inputPattern = data->ReadInt32();
    kdListener_->OnEditorAttributeChange(attribute);
}

int32_t InputMethodAbility::ShowKeyboard()
{
    IMSA_HILOGI("run in");
    if (imeListener_ == nullptr) {
        IMSA_HILOGE("InputMethodAbility, imeListener is nullptr");
        return ErrorCode::ERROR_IME;
    }
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    if (isPanelKeyboard_.load()) {
        auto panel = GetSoftKeyboardPanel();
        if (panel == nullptr) {
            return ErrorCode::ERROR_IME;
        }
        auto flag = panel->GetPanelFlag();
        imeListener_->OnKeyboardStatus(true);
        if (flag == FLG_CANDIDATE_COLUMN) {
            IMSA_HILOGD("panel flag is candidate, no need to show.");
            return ErrorCode::NO_ERROR;
        }
        return ShowPanel(panel, Trigger::IMF);
    }

    channel->SendKeyboardStatus(KeyboardStatus::SHOW);
    imeListener_->OnKeyboardStatus(true);
    return ErrorCode::NO_ERROR;
}

void InputMethodAbility::NotifyPanelStatusInfo(const PanelStatusInfo &info)
{
    // only notify the status info of soft keyboard(not contain candidate column) at present
    if (info.panelInfo.panelType != PanelType::SOFT_KEYBOARD
        || info.panelInfo.panelFlag == PanelFlag::FLG_CANDIDATE_COLUMN) {
        return;
    }
    auto channel = GetInputDataChannelProxy();
    if (channel != nullptr) {
        info.visible ? channel->SendKeyboardStatus(KeyboardStatus::SHOW)
                     : channel->SendKeyboardStatus(KeyboardStatus::HIDE);
        channel->NotifyPanelStatusInfo(info);
    }

    auto controlChannel = GetInputControlChannel();
    if (controlChannel != nullptr && info.trigger == Trigger::IME_APP && !info.visible) {
        controlChannel->HideKeyboardSelf();
    }
}

void InputMethodAbility::NotifyAllTextConfig()
{
    TextTotalConfig textConfig = {};
    int32_t ret = GetTextConfig(textConfig);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("InputMethodAbility, get text config failed, ret is %{public}d", ret);
        return;
    }
    OnTextConfigChange(textConfig);
}

void InputMethodAbility::OnTextConfigChange(const TextTotalConfig &textConfig)
{
    IMSA_HILOGI("InputMethodAbility run in.");
    if (kdListener_ == nullptr) {
        IMSA_HILOGE("kdListener_ is nullptr.");
    } else {
        IMSA_HILOGI("send on('editorAttributeChanged') callback.");
        kdListener_->OnEditorAttributeChange(textConfig.inputAttribute);
        if (textConfig.cursorInfo.left != INVALID_CURSOR_VALUE) {
            IMSA_HILOGI("send on('cursorUpdate') callback.");
            kdListener_->OnCursorUpdate(
                textConfig.cursorInfo.left, textConfig.cursorInfo.top, textConfig.cursorInfo.height);
        }
        if (textConfig.textSelection.newBegin != INVALID_SELECTION_VALUE) {
            IMSA_HILOGI("send on('selectionChange') callback.");
            kdListener_->OnSelectionChange(textConfig.textSelection.oldBegin, textConfig.textSelection.oldEnd,
                                           textConfig.textSelection.newBegin, textConfig.textSelection.newEnd);
        }
    }
    if (textConfig.windowId == INVALID_WINDOW_ID) {
        return;
    }
    panels_.ForEach([&textConfig](const PanelType &panelType, const std::shared_ptr<InputMethodPanel> &panel) {
        panel->SetCallingWindow(textConfig.windowId);
        return false;
    });
    if (imeListener_ == nullptr) {
        IMSA_HILOGE("imeListener_ is nullptr, do not need to send callback of setCallingWindow.");
        return;
    }
    imeListener_->OnSetCallingWindow(textConfig.windowId);
    IMSA_HILOGD("setCallingWindow end.");
}

int32_t InputMethodAbility::HideKeyboard()
{
    IMSA_HILOGI("InputMethodAbility::HideKeyboard");
    if (imeListener_ == nullptr) {
        IMSA_HILOGE("InputMethodAbility::HideKeyboard imeListener_ is nullptr");
        return ErrorCode::ERROR_IME;
    }
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }

    if (isPanelKeyboard_.load()) {
        auto panel = GetSoftKeyboardPanel();
        if (panel == nullptr) {
            return ErrorCode::ERROR_IME;
        }
        auto flag = panel->GetPanelFlag();
        imeListener_->OnKeyboardStatus(false);
        if (flag == FLG_CANDIDATE_COLUMN) {
            IMSA_HILOGD("panel flag is candidate, no need to hide.");
            return ErrorCode::NO_ERROR;
        }
        return HidePanel(panel, Trigger::IMF);
    }

    channel->SendKeyboardStatus(KeyboardStatus::HIDE);
    imeListener_->OnKeyboardStatus(false);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::InsertText(const std::string text)
{
    IMSA_HILOGD("InputMethodAbility, in");
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGI("InputMethodAbility::InsertText channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->InsertText(Str8ToStr16(text));
}

int32_t InputMethodAbility::DeleteForward(int32_t length)
{
    IMSA_HILOGD("InputMethodAbility, length = %{public}d", length);
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility, channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->DeleteForward(length);
}

int32_t InputMethodAbility::DeleteBackward(int32_t length)
{
    IMSA_HILOGD("InputMethodAbility, length = %{public}d", length);
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility, channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->DeleteBackward(length);
}

int32_t InputMethodAbility::SendFunctionKey(int32_t funcKey)
{
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::SendFunctionKey channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->SendFunctionKey(funcKey);
}

int32_t InputMethodAbility::HideKeyboardSelf()
{
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    if (imeListener_ == nullptr) {
        IMSA_HILOGE("imeListener_ is nullptr");
        return ErrorCode::NO_ERROR;
    }
    imeListener_->OnKeyboardStatus(false);
    channel->SendKeyboardStatus(KeyboardStatus::HIDE);
    auto controlChannel = GetInputControlChannel();
    if (controlChannel != nullptr) {
        controlChannel->HideKeyboardSelf();
    }
    InputMethodSysEvent::GetInstance().OperateSoftkeyboardBehaviour(OperateIMEInfoCode::IME_HIDE_SELF);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::SendExtendAction(int32_t action)
{
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGI("InputMethodAbility::SendExtendAction channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->HandleExtendAction(action);
}

int32_t InputMethodAbility::GetTextBeforeCursor(int32_t number, std::u16string &text)
{
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::GetTextBeforeCursor channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetTextBeforeCursor(number, text);
}

int32_t InputMethodAbility::GetTextAfterCursor(int32_t number, std::u16string &text)
{
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::GetTextAfterCursor channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetTextAfterCursor(number, text);
}

int32_t InputMethodAbility::MoveCursor(int32_t keyCode)
{
    IMSA_HILOGD("InputMethodAbility, keyCode = %{public}d", keyCode);
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::MoveCursor channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }

    return channel->MoveCursor(keyCode);
}

int32_t InputMethodAbility::SelectByRange(int32_t start, int32_t end)
{
    IMSA_HILOGD("InputMethodAbility, start = %{public}d, end = %{public}d", start, end);
    if (start < 0 || end < 0) {
        IMSA_HILOGE("check parameter failed, start: %{public}d, end: %{public}d", start, end);
        return ErrorCode::ERROR_PARAMETER_CHECK_FAILED;
    }
    auto dataChannel = GetInputDataChannelProxy();
    if (dataChannel == nullptr) {
        IMSA_HILOGE("datachannel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return dataChannel->SelectByRange(start, end);
}

int32_t InputMethodAbility::SelectByMovement(int32_t direction)
{
    IMSA_HILOGD("InputMethodAbility, direction = %{public}d", direction);
    auto dataChannel = GetInputDataChannelProxy();
    if (dataChannel == nullptr) {
        IMSA_HILOGE("datachannel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return dataChannel->SelectByMovement(direction, 0);
}

int32_t InputMethodAbility::GetEnterKeyType(int32_t &keyType)
{
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::GetEnterKeyType channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetEnterKeyType(keyType);
}

int32_t InputMethodAbility::GetInputPattern(int32_t &inputPattern)
{
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::GetInputPattern channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetInputPattern(inputPattern);
}

int32_t InputMethodAbility::GetTextIndexAtCursor(int32_t &index)
{
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetTextIndexAtCursor(index);
}

int32_t InputMethodAbility::GetTextConfig(TextTotalConfig &textConfig)
{
    IMSA_HILOGD("InputMethodAbility, run in.");
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetTextConfig(textConfig);
}

void InputMethodAbility::SetInputDataChannel(const sptr<IRemoteObject> &object)
{
    IMSA_HILOGD("run in SetInputDataChannel");
    std::lock_guard<std::mutex> lock(dataChannelLock_);
    auto channelProxy = std::make_shared<InputDataChannelProxy>(object);
    if (channelProxy == nullptr) {
        IMSA_HILOGE("failed to new data channel proxy");
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
    IMSA_HILOGI("run in SetInputControlChannel");
    std::lock_guard<std::mutex> lock(controlChannelLock_);
    std::shared_ptr<InputControlChannelProxy> channelProxy = std::make_shared<InputControlChannelProxy>(object);
    if (channelProxy == nullptr) {
        IMSA_HILOGI("InputMethodAbility::SetInputControlChannel inputDataChannel is nullptr");
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
    IMSA_HILOGI("input method service died");
    isBound_.store(false);
    ClearInputControlChannel();
    {
        std::lock_guard<std::mutex> lock(abilityLock_);
        abilityManager_ = nullptr;
    }
    if (imeListener_ != nullptr) {
        imeListener_->OnInputStop();
    }
}

void InputMethodAbility::QuitWorkThread()
{
    stop_ = true;
    Message *msg = new Message(MessageID::MSG_ID_QUIT_WORKER_THREAD, nullptr);
    msgHandler_->SendMessage(msg);
    if (workThreadHandler.joinable()) {
        workThreadHandler.join();
    }
}

int32_t InputMethodAbility::CreatePanel(const std::shared_ptr<AbilityRuntime::Context> &context,
    const PanelInfo &panelInfo, std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    IMSA_HILOGI("InputMethodAbility::CreatePanel start.");
    bool isSoftKeyboard = panelInfo.panelType == PanelType::SOFT_KEYBOARD;
    if (isSoftKeyboard) {
        isPanelKeyboard_.store(true);
    }
    auto flag = panels_.ComputeIfAbsent(panelInfo.panelType,
        [&panelInfo, &context, &inputMethodPanel](const PanelType &panelType,
            std::shared_ptr<InputMethodPanel> &panel) {
            inputMethodPanel = std::make_shared<InputMethodPanel>();
            auto ret = inputMethodPanel->CreatePanel(context, panelInfo);
            if (ret == ErrorCode::NO_ERROR) {
                panel = inputMethodPanel;
                return true;
            }
            inputMethodPanel = nullptr;
            return false;
        });
    if (!flag) {
        isPanelKeyboard_.store(false);
        return ErrorCode::ERROR_OPERATE_PANEL;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAbility::DestroyPanel(const std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    IMSA_HILOGI("InputMethodAbility, in.");
    if (inputMethodPanel == nullptr) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    PanelType panelType = inputMethodPanel->GetPanelType();
    if (panelType == PanelType::SOFT_KEYBOARD) {
        isPanelKeyboard_.store(false);
    }
    auto ret = inputMethodPanel->DestroyPanel();
    if (ret == ErrorCode::NO_ERROR) {
        panels_.Erase(panelType);
    }
    return ret;
}

int32_t InputMethodAbility::ShowPanel(const std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    return ShowPanel(inputMethodPanel, Trigger::IME_APP);
}

int32_t InputMethodAbility::HidePanel(const std::shared_ptr<InputMethodPanel> &inputMethodPanel)
{
    return HidePanel(inputMethodPanel, Trigger::IME_APP);
}

int32_t InputMethodAbility::ShowPanel(const std::shared_ptr<InputMethodPanel> &inputMethodPanel, Trigger trigger)
{
    if (inputMethodPanel == nullptr) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto channel = GetInputDataChannelProxy();
    if (channel == nullptr) {
        IMSA_HILOGE("channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto ret = inputMethodPanel->ShowPanel();
    if (ret == ErrorCode::NO_ERROR) {
        NotifyPanelStatusInfo(
            { { inputMethodPanel->GetPanelType(), inputMethodPanel->GetPanelFlag() }, true, trigger });
    }
    return ret;
}

int32_t InputMethodAbility::HidePanel(const std::shared_ptr<InputMethodPanel> &inputMethodPanel, Trigger trigger)
{
    if (inputMethodPanel == nullptr) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto ret = inputMethodPanel->HidePanel();
    if (ret == ErrorCode::NO_ERROR) {
        NotifyPanelStatusInfo(
            { { inputMethodPanel->GetPanelType(), inputMethodPanel->GetPanelFlag() }, false, trigger });
    }
    return ret;
}

bool InputMethodAbility::IsCurrentIme()
{
    IMSA_HILOGD("InputMethodAbility, in");
    auto proxy = GetImsaProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("failed to get imsa proxy");
        return false;
    }
    return proxy->IsCurrentIme();
}

bool InputMethodAbility::IsEnable()
{
    if (imeListener_ == nullptr) {
        return false;
    }
    return imeListener_->IsEnable();
}

int32_t InputMethodAbility::ExitCurrentInputType()
{
    IMSA_HILOGD("InputMethodAbility, in");
    auto proxy = GetImsaProxy();
    if (proxy == nullptr) {
        IMSA_HILOGE("failed to get imsa proxy");
        return false;
    }
    return proxy->ExitCurrentInputType();
}

std::shared_ptr<InputMethodPanel> InputMethodAbility::GetSoftKeyboardPanel()
{
    IMSA_HILOGD("find SOFT_KEYBOARD panel.");
    if (!BlockRetry(FIND_PANEL_RETRY_INTERVAL, MAX_RETRY_TIMES,
                    [this]() -> bool { return panels_.Find(SOFT_KEYBOARD).first; })) {
        IMSA_HILOGE("SOFT_KEYBOARD panel not found");
        return nullptr;
    }
    auto result = panels_.Find(SOFT_KEYBOARD);
    if (!result.first) {
        IMSA_HILOGE("SOFT_KEYBOARD panel not found");
        return nullptr;
    }
    return result.second;
}
} // namespace MiscServices
} // namespace OHOS
