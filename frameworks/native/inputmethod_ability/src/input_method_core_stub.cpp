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
#include "input_control_channel_proxy.h"
#include "input_method_ability.h"
#include "ipc_skeleton.h"
#include "itypes_util.h"
#include "message_parcel.h"
#include "system_cmd_channel_proxy.h"
#include "task_manager.h"
#include "tasks/task_imsa.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
InputMethodCoreStub::InputMethodCoreStub() { }

InputMethodCoreStub::~InputMethodCoreStub() { }

int32_t InputMethodCoreStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    IMSA_HILOGD("InputMethodCoreStub, code: %{public}u, callingPid: %{public}d, callingUid: %{public}d.", code,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
    auto descriptorToken = data.ReadInterfaceToken();
    if (descriptorToken != IInputMethodCore::GetDescriptor()) {
        IMSA_HILOGE("InputMethodCoreStub descriptor error!");
        return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
    }
    if (code < CORE_CMD_BEGIN || code >= CORE_CMD_END) {
        IMSA_HILOGE("code error, code = %{public}u, callingPid: %{public}d, callingUid: %{public}d.", code,
            IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
        return IRemoteStub::OnRemoteRequest(code, data, reply, option);
    }
    return (this->*HANDLERS[code])(data, reply);
}

int32_t InputMethodCoreStub::InitInputControlChannel(const sptr<IInputControlChannel> &inputControlChannel)
{
    auto task = std::make_shared<TaskImsaInitInputCtrlChannel>(inputControlChannel->AsObject());
    TaskManager::GetInstance().PostTask(task);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::ShowKeyboard()
{
    auto task = std::make_shared<TaskImsaShowKeyboard>();
    TaskManager::GetInstance().PostTask(task);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::HideKeyboard()
{
    auto task = std::make_shared<TaskImsaHideKeyboard>();
    TaskManager::GetInstance().PostTask(task);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::StopInputService(bool isTerminateIme)
{
    auto task = std::make_shared<TaskImsaStopInputService>(isTerminateIme);
    TaskManager::GetInstance().PostTask(task);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::InitInputControlChannelOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> channelObject = data.ReadRemoteObject();
    if (channelObject == nullptr) {
        IMSA_HILOGE("channelObject is nullptr!");
        return reply.WriteInt32(ErrorCode::ERROR_EX_PARCELABLE) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
    }
    sptr<IInputControlChannel> inputControlChannel = new (std::nothrow) InputControlChannelProxy(channelObject);
    if (inputControlChannel == nullptr) {
        IMSA_HILOGE("failed to new inputControlChannel!");
        return reply.WriteInt32(ErrorCode::ERROR_NULL_POINTER) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = InitInputControlChannel(inputControlChannel);
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::StartInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGI(
        "CoreStub, callingPid/Uid: %{public}d/%{public}d.", IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
    bool isBindFromClient = false;
    InputClientInfo clientInfo = {};
    sptr<IRemoteObject> channel = nullptr;
    if (!ITypesUtil::Unmarshal(data, isBindFromClient, clientInfo, clientInfo.channel)) {
        IMSA_HILOGE("Unmarshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = StartInput(clientInfo, isBindFromClient);
    return ITypesUtil::Marshal(reply, ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::SecurityChangeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t security;
    if (!ITypesUtil::Unmarshal(data, security)) {
        IMSA_HILOGE("Unmarshal failed!");
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
        IMSA_HILOGE("failed to read message parcel!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    sptr<IRemoteObject> agent = nullptr;
    auto ret = InputMethodAbility::GetInstance()->OnConnectSystemCmd(channelObject, agent);
    return reply.WriteInt32(ret) && reply.WriteRemoteObject(agent) ? ErrorCode::NO_ERROR :
                                                                     ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::SetSubtypeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    SubProperty property;
    if (!ITypesUtil::Unmarshal(data, property)) {
        IMSA_HILOGE("failed to read message parcel!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }

    auto task = std::make_shared<TaskImsaOnSetSubProperty>(property);
    TaskManager::GetInstance().PostTask(task);
    return reply.WriteInt32(ErrorCode::NO_ERROR) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::OnSetInputTypeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    InputType inputType;
    if (!ITypesUtil::Unmarshal(data, inputType)) {
        IMSA_HILOGE("failed to read inputType parcel!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    InputMethodAbility::GetInstance()->OnSetInputType(inputType);
    return reply.WriteInt32(ErrorCode::NO_ERROR) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::StopInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> channel = nullptr;
    if (!ITypesUtil::Unmarshal(data, channel)) {
        IMSA_HILOGE("failed to read message parcel!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    uint32_t sessionId = 0;
    if (!ITypesUtil::Unmarshal(data, sessionId)) {
        IMSA_HILOGE("failed to read sessionId parcel!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = StopInput(channel, sessionId);
    return ITypesUtil::Marshal(reply, ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::IsEnableOnRemote(MessageParcel &data, MessageParcel &reply)
{
    bool isEnable = IsEnable();
    return ITypesUtil::Marshal(reply, ErrorCode::NO_ERROR, isEnable) ? ErrorCode::NO_ERROR :
                                                                       ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::ShowKeyboardOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto ret = ShowKeyboard();
    return ITypesUtil::Marshal(reply, ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::HideKeyboardOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto ret = HideKeyboard();
    return ITypesUtil::Marshal(reply, ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::StopInputServiceOnRemote(MessageParcel &data, MessageParcel &reply)
{
    bool isTerminateIme = false;
    if (!ITypesUtil::Unmarshal(data, isTerminateIme)) {
        IMSA_HILOGE("unmarshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = StopInputService(isTerminateIme);
    return ITypesUtil::Marshal(reply, ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::IsPanelShownOnRemote(MessageParcel &data, MessageParcel &reply)
{
    PanelInfo info;
    if (!ITypesUtil::Unmarshal(data, info)) {
        IMSA_HILOGE("unmarshal failed!");
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
        IMSA_HILOGE("failed to read message parcel!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }

    auto task = std::make_shared<TaskImsaOnClientInactive>(channel);
    TaskManager::GetInstance().PostTask(task);
    return ITypesUtil::Marshal(reply, ErrorCode::NO_ERROR) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::StartInput(const InputClientInfo &clientInfo, bool isBindFromClient)
{
    auto task = std::make_shared<TaskImsaStartInput>(clientInfo, isBindFromClient);
    TaskManager::GetInstance().PostTask(task);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::SetSubtype(const SubProperty &property)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::OnSecurityChange(int32_t security)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::OnSetInputType(InputType inputType)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodCoreStub::StopInput(const sptr<IRemoteObject> &channel, uint32_t sessionId)
{
    auto task = std::make_shared<TaskImsaStopInput>(channel, sessionId);
    TaskManager::GetInstance().PostTask(task);
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

void InputMethodCoreStub::OnClientInactive(const sptr<IRemoteObject> &channel) { }

void InputMethodCoreStub::OnCallingDisplayIdChanged(uint64_t dispalyId) { }

int32_t InputMethodCoreStub::OnCallingDisplayChangeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    uint64_t displayId = 0;
    if (!ITypesUtil::Unmarshal(data, displayId)) {
        IMSA_HILOGE("unmarshal failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    int32_t ret = InputMethodAbility::GetInstance()->OnCallingDisplayIdChanged(displayId);
    return ITypesUtil::Marshal(reply, ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodCoreStub::OnSendPrivateData(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    return InputMethodAbility::GetInstance()->OnSendPrivateData(privateCommand);
}

int32_t InputMethodCoreStub::OnSendPrivateDataOnRemote(MessageParcel &data, MessageParcel &reply)
{
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    if (!ITypesUtil::Unmarshal(data, privateCommand)) {
        IMSA_HILOGE("failed to read message parcel!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = OnSendPrivateData(privateCommand);
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}
} // namespace MiscServices
} // namespace OHOS