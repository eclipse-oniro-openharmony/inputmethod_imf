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

int32_t InputMethodCoreProxy::InitInputControlChannel(
    sptr<IInputControlChannel> &inputControlChannel, const std::string &imeId)
{
    return SendRequest(INIT_INPUT_CONTROL_CHANNEL, [&inputControlChannel, &imeId](MessageParcel &data) {
        return ITypesUtil::Marshal(data, inputControlChannel->AsObject(), imeId);
    });
}

int32_t InputMethodCoreProxy::StartInput(const sptr<IInputDataChannel> &inputDataChannel, bool isShowKeyboard)
{
    IMSA_HILOGD("InputMethodCoreProxy::showKeyboard");
    return SendRequest(START_INPUT, [&inputDataChannel, isShowKeyboard](MessageParcel &data) {
        return ITypesUtil::Marshal(data, inputDataChannel->AsObject(), isShowKeyboard);
    });
}

void InputMethodCoreProxy::StopInputService(std::string imeId)
{
    SendRequest(STOP_INPUT_SERVICE, [&imeId](MessageParcel &data) {
        return ITypesUtil::Marshal(data, Str8ToStr16(imeId));
    });
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

int32_t InputMethodCoreProxy::SendRequest(int code, ParcelHandler input, ParcelHandler output)
{
    IMSA_HILOGI("InputMethodCoreProxy, run in, code = %{public}d", code);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option{ MessageOption::TF_SYNC };
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
