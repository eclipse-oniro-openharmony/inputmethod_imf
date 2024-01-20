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

#include "input_method_core_proxy.h"

#include <string_ex.h>

#include "input_attribute.h"
#include "itypes_util.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
InputMethodCoreProxy::InputMethodCoreProxy(const OHOS::sptr<OHOS::IRemoteObject> &impl)
    : IRemoteProxy<IInputMethodCore>(impl)
{
}

InputMethodCoreProxy::~InputMethodCoreProxy() = default;

int32_t InputMethodCoreProxy::InitInputControlChannel(const sptr<IInputControlChannel> &inputControlChannel)
{
    return SendRequest(INIT_INPUT_CONTROL_CHANNEL, [&inputControlChannel](MessageParcel &data) {
        return ITypesUtil::Marshal(data, inputControlChannel->AsObject());
    });
}

int32_t InputMethodCoreProxy::StartInput(const InputClientInfo &clientInfo, bool isBindFromClient)
{
    IMSA_HILOGI("CoreProxy");
    return SendRequest(START_INPUT, [&clientInfo, isBindFromClient](MessageParcel &data) {
        return ITypesUtil::Marshal(data, isBindFromClient, clientInfo);
    });
}

int32_t InputMethodCoreProxy::OnSecurityChange(int32_t security)
{
    return SendRequest(SECURITY_CHANGE, [security](MessageParcel& data) {
        return ITypesUtil::Marshal(data, security);
    });
}

void InputMethodCoreProxy::StopInputService(bool isTerminateIme)
{
    SendRequest(STOP_INPUT_SERVICE,
        [isTerminateIme](MessageParcel &data) { return ITypesUtil::Marshal(data, isTerminateIme); });
}

int32_t InputMethodCoreProxy::ShowKeyboard()
{
    return SendRequest(SHOW_KEYBOARD);
}

int32_t InputMethodCoreProxy::HideKeyboard()
{
    return SendRequest(HIDE_KEYBOARD);
}

int32_t InputMethodCoreProxy::SetSubtype(const SubProperty &property)
{
    return SendRequest(SET_SUBTYPE, [&property](MessageParcel &data) { return ITypesUtil::Marshal(data, property); });
}

int32_t InputMethodCoreProxy::StopInput(const sptr<IInputDataChannel> &channel)
{
    return SendRequest(
        STOP_INPUT, [&channel](MessageParcel &data) { return ITypesUtil::Marshal(data, channel->AsObject()); });
}

bool InputMethodCoreProxy::IsEnable()
{
    bool isEnable = false;
    SendRequest(
        IS_ENABLE, nullptr, [&isEnable](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, isEnable); });
    return isEnable;
}

int32_t InputMethodCoreProxy::IsPanelShown(const PanelInfo &panelInfo, bool &isShown)
{
    return SendRequest(
        IS_PANEL_SHOWN, [&panelInfo](MessageParcel &data) { return ITypesUtil::Marshal(data, panelInfo); },
        [&isShown](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, isShown); });
}

void InputMethodCoreProxy::OnClientInactive(const sptr<IInputDataChannel> &channel)
{
    SendRequest(
        ON_CLIENT_INACTIVE, [&channel](MessageParcel &data) { return ITypesUtil::Marshal(data, channel->AsObject()); },
        nullptr, MessageOption::TF_ASYNC);
}

int32_t InputMethodCoreProxy::OnTextConfigChange(const InputClientInfo &clientInfo)
{
    return SendRequest(
        ON_TEXT_CONFIG_CHANGE, [&clientInfo](MessageParcel &data) { return ITypesUtil::Marshal(data, clientInfo); });
}

int32_t InputMethodCoreProxy::SendRequest(int code, ParcelHandler input, ParcelHandler output, MessageOption option)
{
    IMSA_HILOGD("InputMethodCoreProxy, run in, code = %{public}d", code);
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        IMSA_HILOGE("InputMethodCoreProxy::write interface token failed");
        return ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT;
    }
    if (input != nullptr && (!input(data))) {
        IMSA_HILOGE("InputMethodCoreProxy::write data failed");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = Remote()->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        IMSA_HILOGE("InputMethodCoreProxy send request failed, code: %{public}d, ret %{public}d", code, ret);
        return ret;
    }
    ret = reply.ReadInt32();
    if (ret != NO_ERROR) {
        IMSA_HILOGE("InputMethodCoreProxy::reply error, ret %{public}d", ret);
        return ret;
    }
    if (output != nullptr && (!output(reply))) {
        IMSA_HILOGE("InputMethodCoreProxy::reply parcel error");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ret;
}
} // namespace MiscServices
} // namespace OHOS
