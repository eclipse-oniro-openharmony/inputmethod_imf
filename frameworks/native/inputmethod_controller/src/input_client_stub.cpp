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

#include "input_client_stub.h"

#include "global.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "ipc_skeleton.h"
#include "itypes_util.h"
#include "message.h"

namespace OHOS {
namespace MiscServices {
InputClientStub::InputClientStub()
{
}

InputClientStub::~InputClientStub()
{
}

int32_t InputClientStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    IMSA_HILOGI("code = %{public}u, callingPid:%{public}d, callingUid:%{public}d", code, IPCSkeleton::GetCallingPid(),
                IPCSkeleton::GetCallingUid());
    auto descriptorToken = data.ReadInterfaceToken();
    if (descriptorToken != GetDescriptor()) {
        return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
    }
    switch (code) {
        case ON_INPUT_READY: {
            OnInputReadyOnRemote(data, reply);
            break;
        }
        case ON_INPUT_STOP: {
            OnInputStopOnRemote(data, reply);
            break;
        }
        case ON_SWITCH_INPUT: {
            OnSwitchInputOnRemote(data, reply);
            break;
        }
        case ON_PANEL_STATUS_CHANGE: {
            OnPanelStatusChangeOnRemote(data, reply);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return NO_ERROR;
}

void InputClientStub::OnInputReadyOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> agentObject;
    int32_t ret = SendMessage(MessageID::MSG_ID_ON_INPUT_READY, [&data, &agentObject](MessageParcel &parcel) {
        return ITypesUtil::Unmarshal(data, agentObject) && ITypesUtil::Marshal(parcel, agentObject);
    });
    if (!ITypesUtil::Marshal(reply, ret)) {
        IMSA_HILOGE("failed to write reply");
    }
}

void InputClientStub::OnInputStopOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = SendMessage(MessageID::MSG_ID_ON_INPUT_STOP);
    if (!ITypesUtil::Marshal(reply, ret)) {
        IMSA_HILOGE("failed to write reply");
    }
}

void InputClientStub::OnSwitchInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGI("InputClientStub::OnSwitchInputOnRemote");
    if (msgHandler == nullptr) {
        IMSA_HILOGE("InputClientStub::msgHandler is nullptr");
        return;
    }
    auto *parcel = new (std::nothrow) MessageParcel();
    if (parcel == nullptr) {
        IMSA_HILOGE("parcel is nullptr");
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        return;
    }
    Property property;
    SubProperty subProperty;
    if (!ITypesUtil::Unmarshal(data, property, subProperty)) {
        IMSA_HILOGE("read message parcel failed");
        reply.WriteInt32(ErrorCode::ERROR_EX_PARCELABLE);
        delete parcel;
        return;
    }
    if (!ITypesUtil::Marshal(*parcel, property, subProperty)) {
        IMSA_HILOGE("write message parcel failed");
        reply.WriteInt32(ErrorCode::ERROR_EX_PARCELABLE);
        delete parcel;
        return;
    }
    auto *msg = new (std::nothrow) Message(MessageID::MSG_ID_ON_SWITCH_INPUT, parcel);
    if (msg == nullptr) {
        IMSA_HILOGE("msg is nullptr");
        delete parcel;
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        return;
    }
    msgHandler->SendMessage(msg);
    reply.WriteInt32(ErrorCode::NO_ERROR);
}

void InputClientStub::OnPanelStatusChangeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGD("InputClientStub::OnPanelStatusChangeOnRemote");
    if (msgHandler == nullptr) {
        IMSA_HILOGE("InputClientStub::msgHandler is nullptr");
        return;
    }
    auto *parcel = new (std::nothrow) MessageParcel();
    if (parcel == nullptr) {
        IMSA_HILOGE("parcel is nullptr");
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        return;
    }
    uint32_t status;
    std::vector<InputWindowInfo> windowInfo;
    if (!ITypesUtil::Unmarshal(data, status, windowInfo)) {
        IMSA_HILOGE("read message parcel failed");
        reply.WriteInt32(ErrorCode::ERROR_EX_PARCELABLE);
        delete parcel;
        return;
    }
    if (!ITypesUtil::Marshal(*parcel, status, windowInfo)) {
        IMSA_HILOGE("write message parcel failed");
        reply.WriteInt32(ErrorCode::ERROR_EX_PARCELABLE);
        delete parcel;
        return;
    }
    auto *msg = new (std::nothrow) Message(MessageID::MSG_ID_ON_PANEL_STATUS_CHANGE, parcel);
    if (msg == nullptr) {
        IMSA_HILOGE("msg is nullptr");
        delete parcel;
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        return;
    }
    msgHandler->SendMessage(msg);
    reply.WriteInt32(ErrorCode::NO_ERROR);
}

int32_t InputClientStub::OnInputReady(const sptr<IInputMethodAgent> &agent)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputClientStub::OnInputStop()
{
    return ErrorCode::NO_ERROR;
}

void InputClientStub::SetHandler(MessageHandler *handler)
{
    msgHandler = handler;
}

int32_t InputClientStub::OnSwitchInput(const Property &property, const SubProperty &subProperty)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputClientStub::OnPanelStatusChange(
    const InputWindowStatus &status, const std::vector<InputWindowInfo> &windowInfo)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputClientStub::SendMessage(int code, ParcelHandler input)
{
    IMSA_HILOGD("InputClientStub run in");
    if (msgHandler == nullptr) {
        IMSA_HILOGE("msgHandler_ is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto *parcel = new (std::nothrow) MessageParcel();
    if (parcel == nullptr) {
        IMSA_HILOGE("parcel is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    if (input != nullptr && (!input(*parcel))) {
        IMSA_HILOGE("write data failed");
        delete parcel;
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto *msg = new (std::nothrow) Message(code, parcel);
    if (msg == nullptr) {
        IMSA_HILOGE("msg is nullptr");
        delete parcel;
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    msgHandler->SendMessage(msg);
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS
