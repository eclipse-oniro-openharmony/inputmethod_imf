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

#include "input_control_channel_proxy.h"

#include "global.h"
#include "i_input_control_channel.h"
#include "i_input_method_agent.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "message_handler.h"
#include "message_parcel.h"
#include "parcel.h"

/*!@class InputControlChannelProxy
 *
 * @brief The proxy implementation of IInputControlChannel
 *
 * This class should be implemented by input method service
 */
namespace OHOS {
namespace MiscServices {
InputControlChannelProxy::InputControlChannelProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IInputControlChannel>(impl)
{
}

InputControlChannelProxy::~InputControlChannelProxy()
{
}

int32_t InputControlChannelProxy::HideKeyboardSelf(int flags)
{
    IMSA_HILOGI("InputControlChannelProxy::HideKeyboardSelf");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        IMSA_HILOGE("InputControlChannelProxy::HideKeyboardSelf descriptor is not match");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    if (!ITypesUtil::Marshal(data, flags)) {
        IMSA_HILOGE("Marshalling failed");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = Remote()->SendRequest(HIDE_KEYBOARD_SELF, data, reply, option);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("InputControlChannelProxy::HideKeyboardSelf SendRequest failed");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return reply.ReadInt32();
}
} // namespace MiscServices
} // namespace OHOS
