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

#include "input_control_channel_stub.h"

#include "i_input_control_channel.h"
#include "input_channel.h"
#include "input_method_agent_proxy.h"
#include "ipc_skeleton.h"
#include "message_handler.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
InputControlChannelStub::InputControlChannelStub(int32_t userId)
{
    userId_ = userId;
}

InputControlChannelStub::~InputControlChannelStub()
{
}

int32_t InputControlChannelStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    IMSA_HILOGD("InputControlChannelStub, code = %{public}u, callingPid: %{public}d, callingUid: %{public}d", code,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
    auto descriptorToken = data.ReadInterfaceToken();
    if (descriptorToken != GetDescriptor()) {
        return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
    }
    switch (code) {
        case HIDE_KEYBOARD_SELF: {
            reply.WriteInt32(HideKeyboardSelf());
            break;
        }
        default: {
            return IRemoteStub::OnRemoteRequest(code, data, reply, option);
        }
    }
    return NO_ERROR;
}

int32_t InputControlChannelStub::HideKeyboardSelf()
{
    auto msg = new (std::nothrow) Message(MessageID::MSG_ID_HIDE_KEYBOARD_SELF, nullptr);
    if (msg == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    MessageHandler::Instance()->SendMessage(msg);
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS
