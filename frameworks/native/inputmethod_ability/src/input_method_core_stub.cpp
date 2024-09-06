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

#include <string_ex.h>

#include <cstdint>

#include "i_input_data_channel.h"
#include "input_control_channel_proxy.h"
#include "system_cmd_channel_proxy.h"
#include "input_method_ability.h"
#include "ipc_skeleton.h"
#include "itypes_util.h"
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
    IMSA_HILOGD("InputMethodCoreStub, code: %{public}u, callingPid: %{public}d, callingUid: %{public}d", code,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
    auto descriptorToken = data.ReadInterfaceToken();
    if (descriptorToken != IInputMethodCore::GetDescriptor()) {
        IMSA_HILOGE("InputMethodCoreStub descriptor error");
        return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
    }
    if (code >= FIRST_CALL_TRANSACTION && code < static_cast<uint32_t>(CORE_CMD_LAST)) {
        return (this->*HANDLERS[code])(data, reply);
    } else {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t InputMethodCoreStub::InitInputControlChannel(const sptr<IInputControlChannel> &inputControlChannel)
{
    return SendMessage(MessageID::MSG_ID_INIT_INPUT_CONTROL_CHANNEL, [inputControlChannel](MessageParcel &data) {
        return ITypesUtil::Marshal(data, inputControlChannel->AsObject());
    });
}

int32_t InputMethodCoreStub::ShowKeyboard()
{
    return InputMethodAbility::GetInstance()->ShowKeyboard();
}

int32_t InputMethodCoreStub::HideKeyboard(bool isForce)
{
    return InputMethodAbility::GetInstance()->HideKeyboard(isForce);
}

void InputMethodCoreStub::StopInputService(bool isTerminateIme)
{
    SendMessage(MessageID::MSG_ID_STOP_INPUT_SERVICE,
        [isTerminateIme](MessageParcel &data) { return ITypesUtil::Marshal(data, isTerminateIme); });
}

void InputMethodCoreStub::SetMessageHandler(MessageHandler *msgHandler)
{
    msgHandler_ = msgHandler;
}

int32_t InputMethodCoreStub::InitInputControlChannelOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> channelObject = data.ReadRemoteObject();
    if (channelObject == nullptr) {
        IMSA_HILOGE("channelObject is nullptr");
        return reply.WriteInt32(ErrorCode::ERROR_EX_PARCELABLE) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
    }
    sptr<IInputControlChannel> inputControlChannel = new (std::nothrow) InputControlChannelProxy(channelObject);
    if (inputControlChannel == nullptr) {
        IMSA_HILOGE("failed to new inputControlChannel");
        return reply.WriteInt32(ErrorCode::ERROR_NULL_POINTER) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = InitInputControlChannel(inputControlChannel);
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::StartInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGI(
        "CoreStub, callingPid/Uid: %{public}d/%{public}d", IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
    bool isBindFromClient = false;
    InputClientInfo clientInfo = {};
    sptr<IRemoteObject> channel = nullptr;
    if (!ITypesUtil::Unmarshal(data, isBindFromClient, clientInfo, clientInfo.channel)) {
        IMSA_HILOGE("Unmarshal failed.");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = StartInput(clientInfo, isBindFromClient);
    return ITypesUtil::Marshal(reply, ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::SecurityChangeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t security;
    if (!ITypesUtil::Unmarshal(data, security)) {
        IMSA_HILOGE("Unmarshal failed.");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = InputMethodAbility::GetInstance()->OnSecurityChange(security);
    return ITypesUtil::Marshal(reply, ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::OnConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::OnConnectSystemCmdOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> channelObject = nullptr;
    if (!ITypesUtil::Unmarshal(data, channelObject)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    sptr<IRemoteObject> agent = nullptr;
    auto ret = InputMethodAbility::GetInstance()->OnConnectSystemCmd(channelObject, agent);
    return reply.WriteInt32(ret) && reply.WriteRemoteObject(agent) ? ErrorCode::NO_ERROR
                                                                   : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::SetSubtypeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    SubProperty property;
    int32_t ret = SendMessage(MessageID::MSG_ID_SET_SUBTYPE, [&data, &property](MessageParcel &parcel) {
        return ITypesUtil::Unmarshal(data, property) && ITypesUtil::Marshal(parcel, property);
    });
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::StopInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> channel = nullptr;
    if (!ITypesUtil::Unmarshal(data, channel)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = InputMethodAbility::GetInstance()->StopInput(channel);
    return ITypesUtil::Marshal(reply, ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::IsEnableOnRemote(MessageParcel &data, MessageParcel &reply)
{
    bool isEnable = IsEnable();
    return ITypesUtil::Marshal(reply, ErrorCode::NO_ERROR, isEnable) ? ErrorCode::NO_ERROR
                                                                     : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::ShowKeyboardOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto ret = ShowKeyboard();
    return ITypesUtil::Marshal(reply, ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::HideKeyboardOnRemote(MessageParcel &data, MessageParcel &reply)
{
    bool isForce = false;
    if (!ITypesUtil::Unmarshal(data, isForce)) {
        IMSA_HILOGE("unmarshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = HideKeyboard(isForce);
    return ITypesUtil::Marshal(reply, ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::StopInputServiceOnRemote(MessageParcel &data, MessageParcel &reply)
{
    bool isTerminateIme = false;
    if (!ITypesUtil::Unmarshal(data, isTerminateIme)) {
        IMSA_HILOGE("unmarshal failed");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    StopInputService(isTerminateIme);
    return ITypesUtil::Marshal(reply, ErrorCode::NO_ERROR) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::IsPanelShownOnRemote(MessageParcel &data, MessageParcel &reply)
{
    PanelInfo info;
    if (!ITypesUtil::Unmarshal(data, info)) {
        IMSA_HILOGE("unmarshal failed");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    bool isShown = false;
    int32_t ret = IsPanelShown(info, isShown);
    return ITypesUtil::Marshal(reply, ret, isShown) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::OnClientInactiveOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> channel = nullptr;
    if (!ITypesUtil::Unmarshal(data, channel)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    InputMethodAbility::GetInstance()->OnClientInactive(channel);
    return ITypesUtil::Marshal(reply, ErrorCode::NO_ERROR) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::StartInput(const InputClientInfo &clientInfo, bool isBindFromClient)
{
    return InputMethodAbility::GetInstance()->StartInput(clientInfo, isBindFromClient);
}

int32_t InputMethodCoreStub::SetSubtype(const SubProperty &property)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::OnSecurityChange(int32_t security)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::StopInput(const sptr<IRemoteObject> &channel)
{
    return ErrorCode::NO_ERROR;
}

bool InputMethodCoreStub::IsEnable()
{
    return InputMethodAbility::GetInstance()->IsEnable();
}

int32_t InputMethodCoreStub::IsPanelShown(const PanelInfo &panelInfo, bool &isShown)
{
    return InputMethodAbility::GetInstance()->IsPanelShown(panelInfo, isShown);
}

void InputMethodCoreStub::OnClientInactive(const sptr<IRemoteObject> &channel)
{
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
