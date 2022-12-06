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

#include "message_handler.h"
#include "input_control_channel_stub.h"
#include "i_input_control_channel.h"
#include "input_channel.h"
#include "input_method_agent_proxy.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
    /*! Constructor
    \param userId the id of the user to whom the object is linking
    */
    InputControlChannelStub::InputControlChannelStub(int userId)
    {
        userId_ = userId;
    }

    /*! Destructor
    */
    InputControlChannelStub::~InputControlChannelStub()
    {
    }

    /*! Handle the transaction from the remote binder
    \n Run in binder thread
    \param code transaction code number
    \param data the params from remote binder
    \param[out] reply the result of the transaction replied to the remote binder
    \param flags the flags of handling transaction
    \return int32_t
    */
    int32_t InputControlChannelStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
                                                     MessageParcel& reply, MessageOption& option)
    {
        IMSA_HILOGI("InputControlChannelStub::OnRemoteRequest code = %{public}u", code);
        auto descriptorToken = data.ReadInterfaceToken();
        if (descriptorToken != GetDescriptor()) {
            return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
        }
        switch (code) {
            case HIDE_KEYBOARD_SELF: {
                int flag = data.ReadInt32();
                reply.WriteInt32(HideKeyboardSelf(flag));
                break;
            }
            default: {
                return IRemoteStub::OnRemoteRequest(code, data, reply, option);
            }
        }
        return NO_ERROR;
    }


    /*! Send HideKeyboardSelf command to work thread.
    \n This call is running in binder thread,
        but the handling of HideKeyboardSelf is in the work thread of PerUserSession.
    \see PerUserSession::OnHideKeyboardSelf
    \param flags the flag value of hiding keyboard
    */
    int32_t InputControlChannelStub::HideKeyboardSelf(int flags)
    {
        IMSA_HILOGI("InputControlChannelStub::HideKeyboardSelf flags = %{public}d", flags);
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId_);
        parcel->WriteInt32(flags);

        Message *msg = new Message(MessageID::MSG_ID_HIDE_KEYBOARD_SELF, parcel);
        MessageHandler::Instance()->SendMessage(msg);
        return ErrorCode::NO_ERROR;
    }
} // namespace MiscServices
} // namespace OHOS
