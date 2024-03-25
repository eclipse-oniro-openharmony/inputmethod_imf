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

int32_t InputClientProxy::OnInputReady(const sptr<IInputMethodAgent> &agent)
{
    return SendRequest(
        ON_INPUT_READY, [agent](MessageParcel &data) { return ITypesUtil::Marshal(data, agent->AsObject()); });
}

int32_t InputClientProxy::OnInputStop()
{
    return SendRequest(ON_INPUT_STOP);
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
        IMSA_HILOGE("InputClientProxy::write interface token failed");
        return ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT;
    }
    if (input != nullptr && (!input(data))) {
        IMSA_HILOGE("InputClientProxy::write data failed");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = Remote()->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        IMSA_HILOGE("InputClientProxy send request failed, code: %{public}d, ret: %{public}d", code, ret);
        return ret;
    }
    ret = reply.ReadInt32();
    if (ret != NO_ERROR) {
        IMSA_HILOGE("InputClientProxy::reply error, ret %{public}d", ret);
        return ret;
    }
    if (output != nullptr && (!output(reply))) {
        IMSA_HILOGE("InputClientProxy::reply parcel error");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ret;
}
} // namespace MiscServices
} // namespace OHOS
