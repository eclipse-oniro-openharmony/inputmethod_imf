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

#include "input_method_system_ability_proxy.h"

#include "global.h"
#include "itypes_util.h"
#include "message_option.h"

namespace OHOS {
namespace MiscServices {
using namespace ErrorCode;

InputMethodSystemAbilityProxy::InputMethodSystemAbilityProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IInputMethodSystemAbility>(object)
{
}

int32_t InputMethodSystemAbilityProxy::PrepareInput(
    int32_t displayId, sptr<IInputClient> client, sptr<IInputDataChannel> channel, InputAttribute &attribute)
{
    return SendRequest(PREPARE_INPUT, [displayId, client, channel, &attribute](MessageParcel &data) {
        return data.WriteInt32(displayId) && data.WriteRemoteObject(client->AsObject()) &&
               data.WriteRemoteObject(channel->AsObject()) && InputAttribute::Marshalling(attribute, data);
    });
}

int32_t InputMethodSystemAbilityProxy::StartInput(sptr<IInputClient> client, bool isShowKeyboard)
{
    IMSA_HILOGD("%{public}s in", __func__);
    return SendRequest(START_INPUT, [isShowKeyboard, client](MessageParcel &data) {
        return data.WriteRemoteObject(client->AsObject()) && data.WriteBool(isShowKeyboard);
    });
}

int32_t InputMethodSystemAbilityProxy::ShowCurrentInput()
{
    IMSA_HILOGD("%{public}s in", __func__);
    return SendRequest(SHOW_CURRENT_INPUT);
}

int32_t InputMethodSystemAbilityProxy::HideCurrentInput()
{
    IMSA_HILOGD("%{public}s in", __func__);
    return SendRequest(HIDE_CURRENT_INPUT);
}

int32_t InputMethodSystemAbilityProxy::StopInputSession()
{
    IMSA_HILOGD("%{public}s in", __func__);
    return SendRequest(STOP_INPUT_SESSION);
}

int32_t InputMethodSystemAbilityProxy::StopInput(sptr<IInputClient> client)
{
    IMSA_HILOGD("%{public}s in", __func__);
    return SendRequest(
        STOP_INPUT, [client](MessageParcel &data) { return data.WriteRemoteObject(client->AsObject()); });
}

int32_t InputMethodSystemAbilityProxy::ReleaseInput(sptr<IInputClient> client)
{
    return SendRequest(
        RELEASE_INPUT, [client](MessageParcel &data) { return data.WriteRemoteObject(client->AsObject()); });
}

int32_t InputMethodSystemAbilityProxy::DisplayOptionalInputMethod()
{
    IMSA_HILOGI("%{public}s in", __func__);
    return SendRequest(DISPLAY_OPTIONAL_INPUT_METHOD);
}

int32_t InputMethodSystemAbilityProxy::SetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent)
{
    IMSA_HILOGD("%{public}s in", __func__);
    return SendRequest(SET_CORE_AND_AGENT, [core, agent](MessageParcel &data) {
        return data.WriteRemoteObject(core->AsObject()) && data.WriteRemoteObject(agent->AsObject());
    });
}

std::shared_ptr<Property> InputMethodSystemAbilityProxy::GetCurrentInputMethod()
{
    IMSA_HILOGD("%{public}s in", __func__);
    std::shared_ptr<Property> property = nullptr;
    int32_t ret = SendRequest(GET_CURRENT_INPUT_METHOD, nullptr, [&property](MessageParcel &reply) {
        property = std::make_shared<Property>();
        if (property == nullptr) {
            IMSA_HILOGE("%{public}s make_shared nullptr", __func__);
            return false;
        }
        return ITypesUtil::Unmarshal(reply, *property);
    });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}s SendRequest failed, ret %{public}d", __func__, ret);
        return nullptr;
    }
    return property;
}

std::shared_ptr<SubProperty> InputMethodSystemAbilityProxy::GetCurrentInputMethodSubtype()
{
    IMSA_HILOGD("%{public}s in", __func__);
    std::shared_ptr<SubProperty> property = nullptr;
    int32_t ret = SendRequest(GET_CURRENT_INPUT_METHOD_SUBTYPE, nullptr, [&property](MessageParcel &reply) {
        property = std::make_shared<SubProperty>();
        if (property == nullptr) {
            IMSA_HILOGE("%{public}s make_shared nullptr", __func__);
            return false;
        }
        return ITypesUtil::Unmarshal(reply, *property);
    });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("%{public}s SendRequest failed, ret %{public}d", __func__, ret);
        return nullptr;
    }
    return property;
}

int32_t InputMethodSystemAbilityProxy::ListInputMethod(InputMethodStatus status, std::vector<Property> &props)
{
    IMSA_HILOGD("%{public}s in", __func__);
    int32_t ret = SendRequest(
        LIST_INPUT_METHOD, [status](MessageParcel &data) { return ITypesUtil::Marshal(data, uint32_t(status)); },
        [&props](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, props); });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("InputMethodSystemAbilityProxy::SendRequest failed, ret %{public}d", ret);
    }
    return ret;
}

int32_t InputMethodSystemAbilityProxy::ShowCurrentInputDeprecated()
{
    IMSA_HILOGD("%{public}s in", __func__);
    return SendRequest(SHOW_CURRENT_INPUT_DEPRECATED);
}

int32_t InputMethodSystemAbilityProxy::HideCurrentInputDeprecated()
{
    IMSA_HILOGD("%{public}s in", __func__);
    return SendRequest(HIDE_CURRENT_INPUT_DEPRECATED);
}

int32_t InputMethodSystemAbilityProxy::DisplayOptionalInputMethodDeprecated()
{
    IMSA_HILOGD("%{public}s in", __func__);
    return SendRequest(DISPLAY_OPTIONAL_INPUT_DEPRECATED);
}

int32_t InputMethodSystemAbilityProxy::SetCoreAndAgentDeprecated(
    sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent)
{
    IMSA_HILOGD("%{public}s in", __func__);
    return SendRequest(SET_CORE_AND_AGENT_DEPRECATED, [core, agent](MessageParcel &data) {
        return data.WriteRemoteObject(core->AsObject()) && data.WriteRemoteObject(agent->AsObject());
    });
}

int32_t InputMethodSystemAbilityProxy::ListInputMethodSubtype(
    const std::string &name, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("InputMethodSystemAbilityProxy::ListInputMethodSubtype");
    int32_t ret = SendRequest(
        LIST_INPUT_METHOD_SUBTYPE, [&name](MessageParcel &data) { return ITypesUtil::Marshal(data, name); },
        [&subProps](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, subProps); });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("InputMethodSystemAbilityProxy::SendRequest failed, ret %{public}d", ret);
    }
    return ret;
}

int32_t InputMethodSystemAbilityProxy::ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("InputMethodSystemAbilityProxy::ListCurrentInputMethodSubtype");
    int32_t ret = SendRequest(LIST_CURRENT_INPUT_METHOD_SUBTYPE, nullptr,
        [&subProps](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, subProps); });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("InputMethodSystemAbilityProxy::SendRequest failed, ret %{public}d", ret);
    }
    return ret;
}

int32_t InputMethodSystemAbilityProxy::SwitchInputMethod(const std::string &name, const std::string &subName)
{
    IMSA_HILOGD("InputMethodSystemAbilityProxy::SwitchInputMethod");
    return SendRequest(SWITCH_INPUT_METHOD,
        [&name, &subName](MessageParcel &data) { return ITypesUtil::Marshal(data, name, subName); });
}

int32_t InputMethodSystemAbilityProxy::SendRequest(int code, ParcelHandler input, ParcelHandler output)
{
    IMSA_HILOGD("%{public}s in", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option{ MessageOption::TF_SYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        IMSA_HILOGE("write interface token failed");
        return ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT;
    }
    if (input != nullptr && (!input(data))) {
        IMSA_HILOGE("write data failed");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = Remote()->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        IMSA_HILOGE("SendRequest failed, ret %{public}d", ret);
        return ret;
    }
    ret = reply.ReadInt32();
    if (ret != NO_ERROR) {
        IMSA_HILOGE("reply error, ret %{public}d", ret);
        return ret;
    }
    if (output != nullptr && (!output(reply))) {
        IMSA_HILOGE("reply parcel error");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ret;
}
} // namespace MiscServices
} // namespace OHOS
