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
            if (!msgHandler) {
                break;
            }
            MessageParcel *parcel = new MessageParcel();
            parcel->WriteRemoteObject(data.ReadRemoteObject());

            Message *msg = new Message(MessageID::MSG_ID_ON_INPUT_READY, parcel);
            msgHandler->SendMessage(msg);
            break;
        }
        case ON_SWITCH_INPUT: {
            OnSwitchInputOnRemote(data, reply);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return NO_ERROR;
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

int32_t InputClientStub::OnInputReady(const sptr<IInputMethodAgent> &agent)
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
} // namespace MiscServices
} // namespace OHOS
