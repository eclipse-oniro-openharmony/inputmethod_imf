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
#include "input_method_core_stub.h"

#include <cstdint>
#include <string_ex.h>

#include "i_input_data_channel.h"
#include "input_channel.h"
#include "input_control_channel_proxy.h"
#include "input_method_ability.h"
#include "ipc_skeleton.h"
#include "message_handler.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
InputMethodCoreStub::InputMethodCoreStub()
{
    msgHandler_ = nullptr;
}

InputMethodCoreStub::~InputMethodCoreStub()
{
}

int32_t InputMethodCoreStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    IMSA_HILOGI("InputMethodCoreStub, code = %{public}u, callingPid: %{public}d, callingUid: %{public}d", code,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
    auto descriptorToken = data.ReadInterfaceToken();
    if (descriptorToken != GetDescriptor()) {
        IMSA_HILOGI("InputMethodCoreStub::OnRemoteRequest descriptorToken is invalid");
        return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
    }
    switch (code) {
        case INIT_INPUT_CONTROL_CHANNEL: {
            InitInputControlChannelOnRemote(data, reply);
            break;
        }
        case START_INPUT: {
            StartInputOnRemote(data, reply);
            break;
        }
        case HIDE_KEYBOARD: {
            reply.WriteInt32(HideKeyboard());
            break;
        }
        case STOP_INPUT_SERVICE: {
            StopInputService(Str16ToStr8(data.ReadString16()));
            reply.WriteInt32(ErrorCode::NO_ERROR);
            break;
        }
        case SET_SUBTYPE: {
            SetSubtypeOnRemote(data, reply);
            break;
        }
        case STOP_INPUT: {
            reply.WriteInt32(StopInputOnRemote(data));
            break;
        }
        case SHOW_KEYBOARD: {
            reply.WriteInt32(ShowKeyboard());
            break;
        }
        default: {
            return IRemoteStub::OnRemoteRequest(code, data, reply, option);
        }
    }
    return NO_ERROR;
}

int32_t InputMethodCoreStub::InitInputControlChannel(
    sptr<IInputControlChannel> &inputControlChannel, const std::string &imeId)
{
    IMSA_HILOGD("InputMethodCoreStub::InitInputControlChannel");
    if (msgHandler_ == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }

    MessageParcel *data = new MessageParcel();
    if (inputControlChannel != nullptr) {
        IMSA_HILOGD("InputMethodCoreStub, inputControlChannel is not nullptr");
        data->WriteRemoteObject(inputControlChannel->AsObject());
    }
    data->WriteString(imeId);
    Message *msg = new Message(MessageID::MSG_ID_INIT_INPUT_CONTROL_CHANNEL, data);
    msgHandler_->SendMessage(msg);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::ShowKeyboard()
{
    IMSA_HILOGD("InputMethodCoreStub::ShowKeyboard");
    return InputMethodAbility::GetInstance()->ShowKeyboard();
}

int32_t InputMethodCoreStub::HideKeyboard()
{
    IMSA_HILOGD("InputMethodCoreStub::hideKeyboard");
    return InputMethodAbility::GetInstance()->HideKeyboard();
}

void InputMethodCoreStub::StopInputService(std::string imeId)
{
    IMSA_HILOGD("InputMethodCoreStub::StopInputService");
    if (!msgHandler_) {
        return;
    }
    MessageParcel *data = new MessageParcel();
    data->WriteString16(Str8ToStr16(imeId));

    Message *msg = new Message(MessageID::MSG_ID_STOP_INPUT_SERVICE, data);
    msgHandler_->SendMessage(msg);
}

void InputMethodCoreStub::SetMessageHandler(MessageHandler *msgHandler)
{
    msgHandler_ = msgHandler;
}

void InputMethodCoreStub::InitInputControlChannelOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> channelObject = data.ReadRemoteObject();
    if (channelObject == nullptr) {
        IMSA_HILOGE("channelObject is nullptr");
        reply.WriteInt32(ErrorCode::ERROR_EX_PARCELABLE);
        return;
    }
    sptr<IInputControlChannel> inputControlChannel = new InputControlChannelProxy(channelObject);
    if (inputControlChannel == nullptr) {
        IMSA_HILOGE("failed to new inputControlChannel");
        reply.WriteInt32(ErrorCode::ERROR_NULL_POINTER);
        return;
    }
    auto ret = InitInputControlChannel(inputControlChannel, data.ReadString());
    reply.WriteInt32(ret);
}

void InputMethodCoreStub::StartInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGD("InputMethodCoreStub::ShowKeyboardOnRemote");
    sptr<IRemoteObject> channel;
    bool isShowKeyboard = false;
    if (!ITypesUtil::Unmarshal(data, channel, isShowKeyboard)) {
        IMSA_HILOGE("Unmarshal failed.");
        return;
    }
    auto ret = InputMethodAbility::GetInstance()->StartInput(channel, isShowKeyboard);
    reply.WriteInt32(ret);
}

void InputMethodCoreStub::SetSubtypeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGD("InputMethodCoreStub::SetSubtypeOnRemote");
    SubProperty property;
    int32_t ret = SendMessage(MessageID::MSG_ID_SET_SUBTYPE, [&data, &property](MessageParcel &parcel) {
        return ITypesUtil::Unmarshal(data, property) && ITypesUtil::Marshal(parcel, property);
    });
    reply.WriteInt32(ret);
}

int32_t InputMethodCoreStub::StopInputOnRemote(MessageParcel &data)
{
    sptr<IRemoteObject> channel = nullptr;
    if (!ITypesUtil::Unmarshal(data, channel)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    InputMethodAbility::GetInstance()->StopInput(channel);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::StartInput(const sptr<IInputDataChannel> &inputDataChannel, bool isShowKeyboard)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::SetSubtype(const SubProperty &property)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::StopInput(const sptr<IInputDataChannel> &channel)
{
    return ErrorCode::NO_ERROR;
}

bool InputMethodCoreStub::IsEnable()
{
    return true;
}

int32_t InputMethodCoreStub::SendMessage(int code, ParcelHandler input)
{
    IMSA_HILOGD("InputMethodCoreStub::SendMessage");
    if (msgHandler_ == nullptr) {
        IMSA_HILOGE("InputMethodCoreStub::msgHandler_ is nullptr");
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
    msgHandler_->SendMessage(msg);
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS
