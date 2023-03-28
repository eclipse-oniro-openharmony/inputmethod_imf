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

#include "input_method_ability.h"

#include <unistd.h>

#include "global.h"
#include "input_method_agent_proxy.h"
#include "input_method_agent_stub.h"
#include "input_method_core_proxy.h"
#include "input_method_core_stub.h"
#include "input_method_utils.h"
#include "iservice_registry.h"
#include "itypes_util.h"
#include "message_parcel.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "sys/prctl.h"

namespace OHOS {
namespace MiscServices {
class MessageHandler;
using namespace MessageID;
sptr<InputMethodAbility> InputMethodAbility::instance_;
std::mutex InputMethodAbility::instanceLock_;
InputMethodAbility::InputMethodAbility() : stop_(false)
{
    writeInputChannel = nullptr;
    Initialize();
}

InputMethodAbility::~InputMethodAbility()
{
    IMSA_HILOGI("InputMethodAbility::~InputMethodAbility");
    QuitWorkThread();
    if (msgHandler != nullptr) {
        delete msgHandler;
        msgHandler = nullptr;
    }
}

sptr<InputMethodAbility> InputMethodAbility::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            IMSA_HILOGI("InputMethodAbility::GetInstance need new IMA");
            instance_ = new InputMethodAbility();
        }
    }
    return instance_;
}

sptr<InputMethodSystemAbilityProxy> InputMethodAbility::GetImsaProxy()
{
    IMSA_HILOGI("InputMethodAbility::GetImsaProxy");
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGI("InputMethodAbility::GetImsaProxy systemAbilityManager is nullptr");
        return nullptr;
    }

    auto systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
    if (systemAbility == nullptr) {
        IMSA_HILOGI("InputMethodAbility::GetImsaProxy systemAbility is nullptr");
        return nullptr;
    }
    if (deathRecipientPtr_ == nullptr) {
        deathRecipientPtr_ = new (std::nothrow) ServiceDeathRecipient();
        if (deathRecipientPtr_ == nullptr) {
            IMSA_HILOGE("new ServiceDeathRecipient failed");
            return nullptr;
        }
    }
    if ((systemAbility->IsProxyObject()) && (!systemAbility->AddDeathRecipient(deathRecipientPtr_))) {
        IMSA_HILOGE("failed to add death recipient.");
    }

    sptr<InputMethodSystemAbilityProxy> iface = new InputMethodSystemAbilityProxy(systemAbility);
    return iface;
}

void InputMethodAbility::SetCoreAndAgent()
{
    IMSA_HILOGI("InputMethodAbility::SetCoreAndAgent");
    mImms = GetImsaProxy();
    if (mImms == nullptr) {
        IMSA_HILOGI("InputMethodAbility::SetCoreAndAgent() mImms is nullptr");
        return;
    }
    sptr<InputMethodCoreStub> stub = new InputMethodCoreStub(0);
    stub->SetMessageHandler(msgHandler);

    sptr<InputMethodAgentStub> inputMethodAgentStub(new InputMethodAgentStub());
    inputMethodAgentStub->SetMessageHandler(msgHandler);
    sptr<IInputMethodAgent> inputMethodAgent = sptr(new InputMethodAgentProxy(inputMethodAgentStub));
    mImms->SetCoreAndAgentDeprecated(stub, inputMethodAgent);
}

void InputMethodAbility::Initialize()
{
    IMSA_HILOGI("InputMethodAbility::Initialize");
    msgHandler = new MessageHandler();
    workThreadHandler = std::thread([this] { WorkThread(); });

    SetCoreAndAgent();
}

void InputMethodAbility::SetImeListener(std::shared_ptr<InputMethodEngineListener> imeListener)
{
    IMSA_HILOGI("InputMethodAbility.");
    if (imeListener_ == nullptr) {
        imeListener_ = std::move(imeListener);
    }
    if (deathRecipientPtr_ != nullptr && deathRecipientPtr_->listener == nullptr) {
        deathRecipientPtr_->listener = imeListener_;
    }
}

void InputMethodAbility::OnImeReady()
{
    isImeReady_ = true;
    if (!notifier_.isNotify) {
        IMSA_HILOGI("InputMethodAbility::Ime Ready, don't need to notify");
        return;
    }
    IMSA_HILOGI("InputMethodAbility::Ime Ready, notify InputStart");
    ShowInputWindow(notifier_.isShowKeyboard, notifier_.subProperty);
}

void InputMethodAbility::SetKdListener(std::shared_ptr<KeyboardListener> kdListener)
{
    IMSA_HILOGI("InputMethodAbility.");
    if (kdListener_ == nullptr) {
        kdListener_ = kdListener;
    }
}

void InputMethodAbility::WorkThread()
{
    prctl(PR_SET_NAME, "IMAWorkThread");
    while (!stop_) {
        Message *msg = msgHandler->GetMessage();
        switch (msg->msgId_) {
            case MSG_ID_INIT_INPUT_CONTROL_CHANNEL: {
                OnInitInputControlChannel(msg);
                break;
            }
            case MSG_ID_SHOW_KEYBOARD: {
                OnShowKeyboard(msg);
                break;
            }
            case MSG_ID_HIDE_KEYBOARD: {
                OnHideKeyboard(msg);
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
            case MSG_ID_STOP_INPUT_SERVICE: {
                MessageParcel *data = msg->msgContent_;
                std::string imeId = Str16ToStr8(data->ReadString16());
                if (imeListener_ != nullptr) {
                    imeListener_->OnInputStop(imeId);
                }
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
    if (deathRecipientPtr_ != nullptr) {
        deathRecipientPtr_->currentIme_ = data->ReadString();
    }
    SetInputControlChannel(channelObject);
}

void InputMethodAbility::OnShowKeyboard(Message *msg)
{
    IMSA_HILOGI("InputMethodAbility::OnShowKeyboard");
    MessageParcel *data = msg->msgContent_;
    sptr<IRemoteObject> channelObject = nullptr;
    bool isShowKeyboard = false;
    SubProperty subProperty;
    if (!ITypesUtil::Unmarshal(*data, channelObject, isShowKeyboard, subProperty)) {
        IMSA_HILOGE("InputMethodAbility::OnShowKeyboard read message parcel failed");
        return;
    }
    if (channelObject == nullptr) {
        IMSA_HILOGI("InputMethodAbility::OnShowKeyboard channelObject is nullptr");
        return;
    }
    SetInputDataChannel(channelObject);
    ShowInputWindow(isShowKeyboard, subProperty);
}

void InputMethodAbility::OnHideKeyboard(Message *msg)
{
    IMSA_HILOGI("InputMethodAbility::OnHideKeyboard");
    DismissInputWindow();
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

bool InputMethodAbility::DispatchKeyEvent(int32_t keyCode, int32_t keyStatus)
{
    IMSA_HILOGD("InputMethodAbility, run in");
    if (kdListener_ == nullptr) {
        IMSA_HILOGI("InputMethodAbility::DispatchKeyEvent kdListener_ is nullptr");
        return false;
    }
    return kdListener_->OnKeyEvent(keyCode, keyStatus);
}

void InputMethodAbility::SetCallingWindow(uint32_t windowId)
{
    IMSA_HILOGI("InputMethodAbility::SetCallingWindow");

    if (imeListener_ == nullptr) {
        IMSA_HILOGI("InputMethodAbility::SetCallingWindow imeListener_ is nullptr");
        return;
    }
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

void InputMethodAbility::ShowInputWindow(bool isShowKeyboard, const SubProperty &subProperty)
{
    IMSA_HILOGI("InputMethodAbility::ShowInputWindow");
    if (!isImeReady_) {
        IMSA_HILOGE("InputMethodAbility::ime is unready, store notifier_");
        notifier_.isNotify = true;
        notifier_.isShowKeyboard = isShowKeyboard;
        notifier_.subProperty = subProperty;
        return;
    }
    if (imeListener_ == nullptr) {
        IMSA_HILOGE("InputMethodAbility::ShowInputWindow imeListener_ is nullptr");
        return;
    }
    imeListener_->OnInputStart();
    imeListener_->OnSetSubtype(subProperty);
    if (!isShowKeyboard) {
        IMSA_HILOGI("InputMethodAbility::ShowInputWindow will not show keyboard");
        return;
    }
    imeListener_->OnKeyboardStatus(true);
    std::shared_ptr<InputDataChannelProxy> channel = GetInputDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::ShowInputWindow channel is nullptr");
        return;
    }
    channel->SendKeyboardStatus(KEYBOARD_SHOW);
}

void InputMethodAbility::DismissInputWindow()
{
    IMSA_HILOGI("InputMethodAbility::DismissInputWindow");
    if (imeListener_ == nullptr) {
        IMSA_HILOGE("InputMethodAbility::DismissInputWindow imeListener_ is nullptr");
        return;
    }
    imeListener_->OnKeyboardStatus(false);
    std::shared_ptr<InputDataChannelProxy> channel = GetInputDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::DismissInputWindow channel is nullptr");
        return;
    }
    channel->SendKeyboardStatus(KEYBOARD_HIDE);
}

int32_t InputMethodAbility::InsertText(const std::string text)
{
    IMSA_HILOGD("InputMethodAbility, in");
    std::shared_ptr<InputDataChannelProxy> channel = GetInputDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGI("InputMethodAbility::InsertText channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->InsertText(Utils::ToStr16(text));
}

int32_t InputMethodAbility::DeleteForward(int32_t length)
{
    IMSA_HILOGD("InputMethodAbility, length = %{public}d", length);
    std::shared_ptr<InputDataChannelProxy> channel = GetInputDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility, channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->DeleteForward(length);
}

int32_t InputMethodAbility::DeleteBackward(int32_t length)
{
    IMSA_HILOGD("InputMethodAbility, length = %{public}d", length);
    std::shared_ptr<InputDataChannelProxy> channel = GetInputDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility, channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->DeleteBackward(length);
}

int32_t InputMethodAbility::SendFunctionKey(int32_t funcKey)
{
    std::shared_ptr<InputDataChannelProxy> channel = GetInputDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::SendFunctionKey channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->SendFunctionKey(funcKey);
}

int32_t InputMethodAbility::HideKeyboardSelf()
{
    auto controlChannel = GetInputControlChannel();
    if (controlChannel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::HideKeyboardSelf controlChannel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return controlChannel->HideKeyboardSelf(1);
}

int32_t InputMethodAbility::GetTextBeforeCursor(int32_t number, std::u16string &text)
{
    std::shared_ptr<InputDataChannelProxy> channel = GetInputDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::GetTextBeforeCursor channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetTextBeforeCursor(number, text);
}

int32_t InputMethodAbility::GetTextAfterCursor(int32_t number, std::u16string &text)
{
    std::shared_ptr<InputDataChannelProxy> channel = GetInputDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::GetTextAfterCursor channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetTextAfterCursor(number, text);
}

int32_t InputMethodAbility::MoveCursor(int32_t keyCode)
{
    IMSA_HILOGD("InputMethodAbility, keyCode = %{public}d", keyCode);
    std::shared_ptr<InputDataChannelProxy> channel = GetInputDataChannel();
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
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto dataChannel = GetInputDataChannel();
    if (dataChannel == nullptr) {
        IMSA_HILOGE("datachannel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return dataChannel->SelectByRange(start, end);
}

int32_t InputMethodAbility::SelectByMovement(int32_t direction)
{
    IMSA_HILOGD("InputMethodAbility, direction = %{public}d", direction);
    auto dataChannel = GetInputDataChannel();
    if (dataChannel == nullptr) {
        IMSA_HILOGE("datachannel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return dataChannel->SelectByMovement(direction, 0);
}

int32_t InputMethodAbility::GetEnterKeyType(int32_t &keyType)
{
    std::shared_ptr<InputDataChannelProxy> channel = GetInputDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::GetEnterKeyType channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetEnterKeyType(keyType);
}

int32_t InputMethodAbility::GetInputPattern(int32_t &inputPattern)
{
    std::shared_ptr<InputDataChannelProxy> channel = GetInputDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::GetInputPattern channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetInputPattern(inputPattern);
}

int32_t InputMethodAbility::GetTextIndexAtCursor(int32_t &index)
{
    std::shared_ptr<InputDataChannelProxy> channel = GetInputDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGE("InputMethodAbility::channel is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return channel->GetTextIndexAtCursor(index);
}

void InputMethodAbility::SetInputDataChannel(sptr<IRemoteObject> &object)
{
    IMSA_HILOGD("run in SetInputDataChannel");
    std::lock_guard<std::mutex> lock(dataChannelLock_);
    std::shared_ptr<InputDataChannelProxy> channelProxy = std::make_shared<InputDataChannelProxy>(object);
    if (channelProxy == nullptr) {
        IMSA_HILOGE("InputMethodAbility::SetInputDataChannel inputDataChannel is nullptr");
        return;
    }
    dataChannel_ = channelProxy;
}

std::shared_ptr<InputDataChannelProxy> InputMethodAbility::GetInputDataChannel()
{
    std::lock_guard<std::mutex> lock(dataChannelLock_);
    return dataChannel_;
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

std::shared_ptr<InputControlChannelProxy> InputMethodAbility::GetInputControlChannel()
{
    std::lock_guard<std::mutex> lock(controlChannelLock_);
    return controlChannel_;
}

void InputMethodAbility::ServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    IMSA_HILOGI("ServiceDeathRecipient::OnRemoteDied");
    if (listener != nullptr) {
        listener->OnInputStop(currentIme_);
    }
}

void InputMethodAbility::QuitWorkThread()
{
    stop_ = true;
    Message *msg = new Message(MessageID::MSG_ID_QUIT_WORKER_THREAD, nullptr);
    msgHandler->SendMessage(msg);
    if (workThreadHandler.joinable()) {
        workThreadHandler.join();
    }
}
} // namespace MiscServices
} // namespace OHOS
