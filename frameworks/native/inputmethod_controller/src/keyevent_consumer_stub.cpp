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

#include "keyevent_consumer_stub.h"

#include "global.h"
#include "ipc_object_stub.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "itypes_util.h"
#include "message.h"

namespace OHOS {
namespace MiscServices {
KeyEventConsumerStub::KeyEventConsumerStub(KeyEventCallback callback, std::shared_ptr<MMI::KeyEvent> keyEvent)
    : callback_(callback), keyEvent_(keyEvent)
{
}

KeyEventConsumerStub::~KeyEventConsumerStub()
{
}

int32_t KeyEventConsumerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    IMSA_HILOGD("KeyEventConsumerStub, code: %{public}u, callingPid: %{public}d, callingUid: %{public}d.", code,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
    auto descriptorToken = data.ReadInterfaceToken();
    if (descriptorToken != IKeyEventConsumer::GetDescriptor()) {
        IMSA_HILOGE("KeyEventConsumerStub descriptor error!");
        return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
    }
    if (code >= FIRST_CALL_TRANSACTION && code < static_cast<uint32_t>(KEY_EVENT_CONSUMER_CMD_LAST)) {
        return (this->*HANDLERS.at(code))(data, reply);
    } else {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t KeyEventConsumerStub::OnKeyEventResultOnRemote(MessageParcel &data, MessageParcel &reply)
{
    bool isConsumed = false;
    if (!ITypesUtil::Unmarshal(data, isConsumed)) {
        IMSA_HILOGE("failed to read message parcel!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    if (callback_ != nullptr) {
        callback_(keyEvent_, isConsumed);
    } else {
        IMSA_HILOGE("callback is nullptr, isConsumed: %{public}d!", isConsumed);
    }
    return reply.WriteInt32(ErrorCode::NO_ERROR) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t KeyEventConsumerStub::OnKeyEventResult(bool isConsumed)
{
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS