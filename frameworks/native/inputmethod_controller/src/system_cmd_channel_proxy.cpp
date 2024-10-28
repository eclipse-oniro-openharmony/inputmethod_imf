/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "system_cmd_channel_proxy.h"

#include "global.h"
#include "ipc_types.h"
#include "itypes_util.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
SystemCmdChannelProxy::SystemCmdChannelProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<ISystemCmdChannel>(object)
{
}
int32_t SystemCmdChannelProxy::SendPrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    return SendRequest(SEND_PRIVATE_COMMAND,
        [&privateCommand](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, privateCommand); });
}

int32_t SystemCmdChannelProxy::NotifyPanelStatus(const SysPanelStatus &sysPanelStatus)
{
    return SendRequest(
        SHOULD_SYSTEM_PANEL_SHOW, [sysPanelStatus](MessageParcel &parcel) {
            return ITypesUtil::Marshal(parcel, sysPanelStatus);
        });
}

int32_t SystemCmdChannelProxy::SendRequest(int code, ParcelHandler input, ParcelHandler output)
{
    IMSA_HILOGD("SystemCmdChannelProxy run in, code = %{public}d.", code);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option{ MessageOption::TF_ASYNC };
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
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("send request failed, code: %{public}d, ret: %{public}d!", code, ret);
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