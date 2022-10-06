/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "input_method_system_ability_stub.h"

#include <memory>
#include <utils.h>

#include "access_token.h"
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"

namespace OHOS ::MiscServices {
using namespace Security::AccessToken;
static const std::string PERMISSION_CONNECT_IME_ABILITY = "ohos.permission.CONNECT_IME_ABILITY";

int32_t InputMethodSystemAbilityStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    IMSA_HILOGE("code:%{public}u, callingPid:%{public}d", code, IPCSkeleton::GetCallingPid());
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (remoteDescriptor != IInputMethodSystemAbility::GetDescriptor()) {
        IMSA_HILOGE("%{public}s descriptor failed", __func__);
        return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
    }
    if (code >= 0 && code < INPUT_SERVICE_CMD_LAST) {
        return (this->*HANDLERS[code])(data, reply);
    } else {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t InputMethodSystemAbilityStub::PrepareInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t displayId = data.ReadInt32();
    auto clientObject = data.ReadRemoteObject();
    if (clientObject == nullptr) {
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        IMSA_HILOGE("%{public}s nullptr", __func__);
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto channelObject = data.ReadRemoteObject();
    if (channelObject == nullptr) {
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        IMSA_HILOGE("%{public}s nullptr", __func__);
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    InputAttribute attribute;
    if (!InputAttribute::Unmarshalling(attribute, data)) {
        reply.WriteInt32(ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT);
        IMSA_HILOGE("%{public}s illegal argument", __func__);
        return ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT;
    }
    int32_t ret = PrepareInput(displayId, iface_cast<IInputClient>(clientObject),
        iface_cast<IInputDataChannel>(channelObject), attribute);
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::StartInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto clientObject = data.ReadRemoteObject();
    if (clientObject == nullptr) {
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        IMSA_HILOGE("%{public}s nullptr", __func__);
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    bool isShowKeyboard = data.ReadBool();
    int32_t ret = StartInput(iface_cast<IInputClient>(clientObject), isShowKeyboard);
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::ShowCurrentInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckPermission(PERMISSION_CONNECT_IME_ABILITY)) {
        reply.WriteInt32(ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
        IMSA_HILOGE("%{public}s Permission denied", __func__);
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    int32_t ret = ShowCurrentInput();
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::HideCurrentInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckPermission(PERMISSION_CONNECT_IME_ABILITY)) {
        reply.WriteInt32(ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
        IMSA_HILOGE("%{public}s Permission denied", __func__);
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    int32_t ret = HideCurrentInput();
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::StopInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto clientObject = data.ReadRemoteObject();
    if (clientObject == nullptr) {
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        IMSA_HILOGE("%{public}s nullptr", __func__);
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    int32_t ret = StopInput(iface_cast<IInputClient>(clientObject));
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::ReleaseInputOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto clientObject = data.ReadRemoteObject();
    if (clientObject == nullptr) {
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        IMSA_HILOGE("%{public}s nullptr", __func__);
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    int32_t ret = ReleaseInput(iface_cast<IInputClient>(clientObject));
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::DisplayOptionalInputMethodOnRemote(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckPermission(PERMISSION_CONNECT_IME_ABILITY)) {
        reply.WriteInt32(ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
        IMSA_HILOGE("%{public}s Permission denied", __func__);
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    int32_t ret = DisplayOptionalInputMethod();
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::SetCoreAndAgentOnRemote(MessageParcel &data, MessageParcel &reply)
{
    if (!CheckPermission(PERMISSION_CONNECT_IME_ABILITY)) {
        reply.WriteInt32(ErrorCode::ERROR_STATUS_PERMISSION_DENIED);
        IMSA_HILOGE("%{public}s Permission denied", __func__);
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    auto coreObject = data.ReadRemoteObject();
    if (coreObject == nullptr) {
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        IMSA_HILOGE("%{public}s nullptr", __func__);
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto agentObject = data.ReadRemoteObject();
    if (agentObject == nullptr) {
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        IMSA_HILOGE("%{public}s nullptr", __func__);
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    int32_t ret = SetCoreAndAgent(iface_cast<IInputMethodCore>(coreObject), iface_cast<IInputMethodAgent>(agentObject));
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::GetKeyboardWindowHeightOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t height = 0;
    int32_t value = GetKeyboardWindowHeight(height);
    bool ret = reply.WriteInt32(value) && reply.WriteInt32(height);
    return ret ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::GetCurrentInputMethodOnRemote(MessageParcel &data, MessageParcel &reply)
{
    auto property = GetCurrentInputMethod();
    if (property == nullptr) {
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        IMSA_HILOGE("%{public}s property is nullptr", __func__);
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    if (!(reply.WriteInt32(ErrorCode::NO_ERROR) && Property::Marshalling(*property, reply))) {
        reply.WriteInt32(ErrorCode::ERROR_EX_PARCELABLE);
        IMSA_HILOGE("%{public}s parcel failed", __func__);
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbilityStub::ListInputMethodOnRemote(MessageParcel &data, MessageParcel &reply)
{
    uint32_t status = data.ReadUint32();
    std::vector<Property> properties = ListInputMethod(InputMethodStatus(status));
    if (!(reply.WriteInt32(ErrorCode::NO_ERROR) && reply.WriteUint32(properties.size()))) {
        reply.WriteInt32(ErrorCode::ERROR_EX_PARCELABLE);
        IMSA_HILOGE("%{public}s parcel failed", __func__);
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    for (auto &property : properties) {
        if (!Property::Marshalling(property, reply)) {
            reply.WriteInt32(ErrorCode::ERROR_EX_PARCELABLE);
            IMSA_HILOGE("%{public}s parcel Marshalling failed", __func__);
            return ErrorCode::ERROR_EX_PARCELABLE;
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbilityStub::SwitchInputMethodOnRemote(MessageParcel &data, MessageParcel &reply)
{
    Property target;
    if (!Property::Unmarshalling(target, data)) {
        reply.WriteInt32(ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT);
        IMSA_HILOGE("%{public}s parcel failed", __func__);
        return ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT;
    }
    int32_t ret = SwitchInputMethod(target);
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::ShowCurrentInputOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = ShowCurrentInputDeprecated();
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::HideCurrentInputOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = HideCurrentInputDeprecated();
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::DisplayInputOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = DisplayOptionalInputMethodDeprecated();
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputMethodSystemAbilityStub::SetCoreAndAgentOnRemoteDeprecated(MessageParcel &data, MessageParcel &reply)
{
    auto coreObject = data.ReadRemoteObject();
    if (coreObject == nullptr) {
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        IMSA_HILOGE("%{public}s coreObject is nullptr", __func__);
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto agentObject = data.ReadRemoteObject();
    if (agentObject == nullptr) {
        reply.WriteInt32(ErrorCode::ERROR_EX_NULL_POINTER);
        IMSA_HILOGE("%{public}s agentObject is nullptr", __func__);
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    int32_t ret = SetCoreAndAgent(iface_cast<IInputMethodCore>(coreObject), iface_cast<IInputMethodAgent>(agentObject));
    return reply.WriteInt32(ret) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

bool InputMethodSystemAbilityStub::CheckPermission(const std::string &permission)
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    TypeATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == TOKEN_INVALID) {
        IMSA_HILOGE("invalid token");
        return false;
    }
    int result = AccessTokenKit::VerifyAccessToken(tokenId, permission);
    IMSA_HILOGI("CheckPermission %{public}s", result == PERMISSION_GRANTED ? "success" : "failed");
    return result == PERMISSION_GRANTED;
}
} // namespace OHOS::MiscServices
