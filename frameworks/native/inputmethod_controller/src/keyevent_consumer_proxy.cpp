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

#include "keyevent_consumer_proxy.h"

#include "global.h"
#include "ipc_types.h"
#include "itypes_util.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
KeyEventConsumerProxy::KeyEventConsumerProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IKeyEventConsumer>(object)
{
}

int32_t KeyEventConsumerProxy::OnKeyEventResult(bool isConsumed)
{
    return SendRequest(KEY_EVENT_RESULT,
        [isConsumed](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, isConsumed); });
}

void KeyEventConsumerProxy::OnKeyEventConsumeResult(bool isConsumed)
{
    IMSA_HILOGI("result: %{public}d", isConsumed);
    keyEventConsume_ = true;
    keyEventResult_ = isConsumed;
    if (keyEventConsume_ && keyCodeConsume_) {
        OnKeyEventResult(keyCodeResult_ || keyEventResult_);
    }
}

void KeyEventConsumerProxy::OnKeyCodeConsumeResult(bool isConsumed)
{
    IMSA_HILOGI("result: %{public}d", isConsumed);
    keyCodeConsume_ = true;
    keyCodeResult_ = isConsumed;
    if (keyEventConsume_ && keyCodeConsume_) {
        OnKeyEventResult(keyCodeResult_ || keyEventResult_);
    }
}

int32_t KeyEventConsumerProxy::SendRequest(int code, ParcelHandler input, ParcelHandler output)
{
    IMSA_HILOGD("KeyEventConsumerProxy run in, code = %{public}d", code);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = MessageOption::TF_SYNC;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        IMSA_HILOGE("write interface token failed");
        return ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT;
    }
    if (input != nullptr && (!input(data))) {
        IMSA_HILOGE("write data failed");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        IMSA_HILOGE("KeyEventConsumerProxy remote is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        IMSA_HILOGE("KeyEventConsumerProxy send request failed, code: %{public}d ret %{public}d", code, ret);
        return ret;
    }
    if (option.GetFlags() == MessageOption::TF_ASYNC) {
        return ErrorCode::NO_ERROR;
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
