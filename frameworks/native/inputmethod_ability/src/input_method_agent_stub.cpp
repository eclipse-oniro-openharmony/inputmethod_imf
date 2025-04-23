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
#include "itypes_util.h"
#include "task_manager.h"
#include "tasks/task_imsa.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;

InputMethodAgentStub::InputMethodAgentStub() { }

InputMethodAgentStub::~InputMethodAgentStub() { }

int32_t InputMethodAgentStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    IMSA_HILOGD("InputMethodAgentStub, code = %{public}u, callingPid: %{public}d, callingUid: %{public}d.", code,
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
        case SEND_PRIVATE_COMMAND: {
            return SendPrivateCommandOnRemote(data, reply);
        }
        case ON_ATTRIBUTE_CHANGE: {
            return OnAttributeChangeOnRemote(data, reply);
        }
        case SEND_MESSAGE: {
            return RecvMessageOnRemote(data, reply);
        }
        default: {
            return IRemoteStub::OnRemoteRequest(code, data, reply, option);
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAgentStub::DispatchKeyEventOnRemote(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    if (keyEvent == nullptr) {
        IMSA_HILOGE("keyEvent is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (!keyEvent->ReadFromParcel(data)) {
        IMSA_HILOGE("failed to read key event from parcel!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto consumerObject = data.ReadRemoteObject();
    if (consumerObject == nullptr) {
        IMSA_HILOGE("consumerObject is nullptr!");
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
        IMSA_HILOGE("failed to read message parcel!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    if (!InputMethodAbility::GetInstance()->IsDefaultIme()) {
        IMSA_HILOGE("current is not default ime!");
        return ErrorCode::ERROR_NOT_DEFAULT_IME;
    }
    auto task = std::make_shared<TaskImsaSendPrivateCommand>(privateCommand);
    TaskManager::GetInstance().PostTask(task);

    return reply.WriteInt32(ErrorCode::NO_ERROR) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodAgentStub::OnAttributeChangeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    InputAttribute attribute;
    if (!ITypesUtil::Unmarshal(data, attribute)) {
        IMSA_HILOGE("failed to read attribute from parcel!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    OnAttributeChange(attribute);
    reply.WriteNoException();
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodAgentStub::DispatchKeyEvent(const std::shared_ptr<MMI::KeyEvent> &keyEvent,
    sptr<IKeyEventConsumer> &consumer)
{
    return false;
}

void InputMethodAgentStub::SetCallingWindow(uint32_t windowId)
{
    InputMethodAbility::GetInstance()->SetCallingWindow(windowId);
}

void InputMethodAgentStub::OnCursorUpdate(int32_t positionX, int32_t positionY, int height)
{
    auto task = std::make_shared<TaskImsaOnCursorUpdate>(positionX, positionY, height);
    TaskManager::GetInstance().PostTask(task);
}

void InputMethodAgentStub::OnSelectionChange(std::u16string text, int32_t oldBegin, int32_t oldEnd, int32_t newBegin,
    int32_t newEnd)
{
    auto task = std::make_shared<TaskImsaOnSelectionChange>(text, oldBegin, oldEnd, newBegin, newEnd);
    TaskManager::GetInstance().PostTask(task);
}

int32_t InputMethodAgentStub::SendPrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    return ErrorCode::NO_ERROR;
}

void InputMethodAgentStub::OnAttributeChange(const InputAttribute &attribute)
{
    auto task = std::make_shared<TaskImsaAttributeChange>(attribute);
    TaskManager::GetInstance().PostTask(task);
}

int32_t InputMethodAgentStub::RecvMessageOnRemote(MessageParcel &data, MessageParcel &reply)
{
    ArrayBuffer arrayBuffer;
    if (!ITypesUtil::Unmarshal(data, arrayBuffer)) {
        IMSA_HILOGE("Failed to read arrayBuffer parcel!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = InputMethodAbility::GetInstance()->RecvMessage(arrayBuffer);
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodAgentStub::SendMessage(const ArrayBuffer &arraybuffer)
{
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS