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

int32_t InputClientProxy::onInputReady(const sptr<IInputMethodAgent> &agent)
{
    IMSA_HILOGI("InputClientProxy::onInputReady");
    MessageParcel data, reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERROR_EX_PARCELABLE;
    }

    data.WriteRemoteObject(agent->AsObject().GetRefPtr());

    auto ret = Remote()->SendRequest(ON_INPUT_READY, data, reply, option);
    if (ret != NO_ERROR) {
        IMSA_HILOGI("InputClientProxy::onInputReady SendRequest failed");
        return ERROR_STATUS_FAILED_TRANSACTION;
    }

    return NO_ERROR;
}

int32_t InputClientProxy::onInputReleased(int32_t retValue)
{
    MessageParcel data, reply;
    MessageOption option;

    data.WriteInterfaceToken(GetDescriptor());
    data.WriteInt32(retValue);
    auto status = Remote()->SendRequest(ON_INPUT_RELEASED, data, reply, option);
    return status;
}

int32_t InputClientProxy::setDisplayMode(int32_t mode)
{
    MessageParcel data, reply;
    MessageOption option;

    data.WriteInterfaceToken(GetDescriptor());
    data.WriteInt32(mode);
    auto status = Remote()->SendRequest(SET_DISPLAY_MODE, data, reply, option);
    return status;
}

int32_t InputClientProxy::OnSwitchInput(const Property &property, const SubProperty &subProperty)
{
    IMSA_HILOGI("InputClientProxy::OnSwitchInput");
    return SendRequest(ON_SWITCH_INPUT,
        [&property, &subProperty](MessageParcel &data) { return ITypesUtil::Marshal(data, property, subProperty); });
}

int32_t InputClientProxy::SendRequest(int code, ParcelHandler input, ParcelHandler output)
{
    IMSA_HILOGI("InputClientProxy::%{public}s in", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option{ MessageOption::TF_SYNC };
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
        IMSA_HILOGE("InputClientProxy::SendRequest failed, ret %{public}d", ret);
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
