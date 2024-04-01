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

#include "input_method_agent_stub.h"

#include "global.h"
#include "input_method_ability.h"
#include "ipc_skeleton.h"
#include "message.h"
#include "message_handler.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;

InputMethodAgentStub::InputMethodAgentStub()
{
    msgHandler_ = nullptr;
}

InputMethodAgentStub::~InputMethodAgentStub()
{
}

int32_t InputMethodAgentStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    IMSA_HILOGD("InputMethodAgentStub, code = %{public}u, callingPid: %{public}d, callingUid: %{public}d", code,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
    auto descriptorToken = data.ReadInterfaceToken();
    if (descriptorToken != GetDescriptor()) {
        return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
    }

    switch (code) {
        case DISPATCH_KEY_EVENT: {
            return DispatchKeyEventOnRemote(data, reply);
        }
        case SET_CALLING_WINDOW_ID: {
            SetCallingWindow(data.ReadUint32());
            break;
        }
        case ON_CURSOR_UPDATE: {
            int32_t positionX = data.ReadInt32();
            int32_t positionY = data.ReadInt32();
            int32_t height = data.ReadInt32();
            OnCursorUpdate(positionX, positionY, height);
            reply.WriteNoException();
            return ErrorCode::NO_ERROR;
        }
        case ON_SELECTION_CHANGE: {
            std::u16string text = data.ReadString16();
            int32_t oldBegin = data.ReadInt32();
            int32_t oldEnd = data.ReadInt32();
            int32_t newBegin = data.ReadInt32();
            int32_t newEnd = data.ReadInt32();
            OnSelectionChange(text, oldBegin, oldEnd, newBegin, newEnd);
            reply.WriteNoException();
            return ErrorCode::NO_ERROR;
        }
        case ON_CONFIGURATION_CHANGE: {
            Configuration configuration;
            configuration.SetEnterKeyType(EnterKeyType(data.ReadInt32()));
            configuration.SetTextInputType(TextInputType(data.ReadInt32()));
            OnConfigurationChange(configuration);
            reply.WriteNoException();
            return ErrorCode::NO_ERROR;
        }
        case SEND_PRIVATE_COMMAND: {
            return SendPrivateCommandOnRemote(data, reply);
        }
        default: {
            return IRemoteStub::OnRemoteRequest(code, data, reply, option);
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAgentStub::DispatchKeyEventOnRemote(MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGI("callingPid/Uid: %{public}d/%{public}d", IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    if (!keyEvent->ReadFromParcel(data)) {
        IMSA_HILOGE("failed to read key event from parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto consumerObject = data.ReadRemoteObject();
    if (consumerObject == nullptr) {
        IMSA_HILOGE("consumerObject is nullptr");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    sptr<KeyEventConsumerProxy> consumer = new (std::nothrow) KeyEventConsumerProxy(consumerObject);
    auto ret = InputMethodAbility::GetInstance()->DispatchKeyEvent(keyEvent, consumer);
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodAgentStub::SendPrivateCommandOnRemote(MessageParcel &data, MessageParcel &reply)
{
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    if (!ITypesUtil::Unmarshal(data, privateCommand)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = InputMethodAbility::GetInstance()->ReceivePrivateCommand(privateCommand);
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodAgentStub::DispatchKeyEvent(
    const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<IKeyEventConsumer> &consumer)
{
    return false;
}

void InputMethodAgentStub::SetCallingWindow(uint32_t windowId)
{
    InputMethodAbility::GetInstance()->SetCallingWindow(windowId);
}

void InputMethodAgentStub::OnCursorUpdate(int32_t positionX, int32_t positionY, int height)
{
    if (msgHandler_ == nullptr) {
        return;
    }
    MessageParcel *data = new MessageParcel();
    data->WriteInt32(positionX);
    data->WriteInt32(positionY);
    data->WriteInt32(height);
    Message *message = new Message(MessageID::MSG_ID_ON_CURSOR_UPDATE, data);
    msgHandler_->SendMessage(message);
}

void InputMethodAgentStub::OnSelectionChange(
    std::u16string text, int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd)
{
    if (msgHandler_ == nullptr) {
        return;
    }
    MessageParcel *data = new MessageParcel();
    data->WriteString16(text);
    data->WriteInt32(oldBegin);
    data->WriteInt32(oldEnd);
    data->WriteInt32(newBegin);
    data->WriteInt32(newEnd);
    Message *message = new Message(MessageID::MSG_ID_ON_SELECTION_CHANGE, data);
    msgHandler_->SendMessage(message);
}

void InputMethodAgentStub::OnConfigurationChange(const Configuration &config)
{
    if (msgHandler_ == nullptr) {
        return;
    }
    MessageParcel *data = new MessageParcel();
    data->WriteInt32(static_cast<int32_t>(config.GetEnterKeyType()));
    data->WriteInt32(static_cast<int32_t>(config.GetTextInputType()));
    Message *message = new Message(MessageID::MSG_ID_ON_CONFIGURATION_CHANGE, data);
    msgHandler_->SendMessage(message);
}

int32_t InputMethodAgentStub::SendPrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    return ErrorCode::NO_ERROR;
}

void InputMethodAgentStub::SetMessageHandler(MessageHandler *msgHandler)
{
    msgHandler_ = msgHandler;
}
} // namespace MiscServices
} // namespace OHOS
