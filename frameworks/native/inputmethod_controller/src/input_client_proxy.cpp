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

#include "input_client_proxy.h"

#include "global.h"
#include "itypes_util.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
using namespace ErrorCode;
InputClientProxy::InputClientProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<IInputClient>(object)
{
}

int32_t InputClientProxy::OnInputReady(const sptr<IRemoteObject> &agent)
{
    return SendRequest(ON_INPUT_READY, [agent](MessageParcel &data) { return ITypesUtil::Marshal(data, agent); });
}

int32_t InputClientProxy::OnInputStop(bool isStopInactiveClient)
{
    return SendRequest(ON_INPUT_STOP,
        [isStopInactiveClient](MessageParcel &data) { return ITypesUtil::Marshal(data, isStopInactiveClient); });
}

int32_t InputClientProxy::OnSwitchInput(const Property &property, const SubProperty &subProperty)
{
    return SendRequest(ON_SWITCH_INPUT,
        [&property, &subProperty](MessageParcel &data) { return ITypesUtil::Marshal(data, property, subProperty); });
}

int32_t InputClientProxy::OnPanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info)
{
    return SendRequest(ON_PANEL_STATUS_CHANGE, [&status, &info](MessageParcel &data) {
        return ITypesUtil::Marshal(data, static_cast<uint32_t>(status), info);
    });
}

int32_t InputClientProxy::NotifyInputStart(uint32_t callingWndId, int32_t requestKeyboardReason)
{
    IMSA_HILOGD("InputClientProxy::NotifyInputStart");
    return SendRequest(ON_NOTIFY_INPUT_START, [callingWndId, requestKeyboardReason](MessageParcel &data) {
        return ITypesUtil::Marshal(data, callingWndId, requestKeyboardReason);
    });
}

int32_t InputClientProxy::NotifyInputStop()
{
    IMSA_HILOGD("InputClientProxy::NotifyInputStop");
    return SendRequest(ON_NOTIFY_INPUT_STOP, nullptr);
}
void InputClientProxy::DeactivateClient()
{
    SendRequest(DEACTIVATE_CLIENT, nullptr, nullptr, MessageOption::TF_ASYNC);
}

int32_t InputClientProxy::SendRequest(int code, ParcelHandler input, ParcelHandler output, MessageOption option)
{
    IMSA_HILOGD("InputClientProxy run in, code = %{public}d", code);
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        IMSA_HILOGE("write interface token failed!");
        return ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT;
    }
    if (input != nullptr && (!input(data))) {
        IMSA_HILOGE("write data failed!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        IMSA_HILOGE("remote is nullptr!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        IMSA_HILOGE("send request failed, code: %{public}d, ret: %{public}d!", code, ret);
        return ret;
    }
    ret = reply.ReadInt32();
    if (ret != NO_ERROR) {
        IMSA_HILOGE("reply error, ret: %{public}d", ret);
        return ret;
    }
    if (output != nullptr && (!output(reply))) {
        IMSA_HILOGE("reply parcel error!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ret;
}
} // namespace MiscServices
} // namespace OHOS