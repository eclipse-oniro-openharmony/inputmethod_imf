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

#include "input_method_agent_proxy.h"

#include "global.h"
#include "itypes_util.h"
#include "message_option.h"

namespace OHOS {
namespace MiscServices {
using namespace ErrorCode;
InputMethodAgentProxy::InputMethodAgentProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IInputMethodAgent>(object)
{
}

int32_t InputMethodAgentProxy::DispatchKeyEvent(
    const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<IKeyEventConsumer> &consumer)
{
    int32_t res = -1;
    int32_t ret = SendRequest(
        DISPATCH_KEY_EVENT,
        [&keyEvent, &consumer](MessageParcel &data) {
            return keyEvent->WriteToParcel(data) && data.WriteRemoteObject(consumer->AsObject());
        },
        [&res](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, res); });
    return ret == ErrorCode::NO_ERROR ? res : ret;
}

void InputMethodAgentProxy::OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height)
{
    auto ret = SendRequest(ON_CURSOR_UPDATE, [positionX, positionY, height](MessageParcel &data) {
        return ITypesUtil::Marshal(data, positionX, positionY, height);
    });
    IMSA_HILOGD("InputMethodAgentProxy::OnCursorUpdate ret = %{public}d", ret);
}

void InputMethodAgentProxy::OnSelectionChange(
    std::u16string text, int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEnd)
{
    auto ret = SendRequest(ON_SELECTION_CHANGE, [&text, oldBegin, oldEnd, newBegin, newEnd](MessageParcel &data) {
        return ITypesUtil::Marshal(data, text, oldBegin, oldEnd, newBegin, newEnd);
    });
    IMSA_HILOGD("InputMethodAgentProxy::OnSelectionChange ret = %{public}d", ret);
}

void InputMethodAgentProxy::SetCallingWindow(uint32_t windowId)
{
    auto ret = SendRequest(
        SET_CALLING_WINDOW_ID, [windowId](MessageParcel &data) { return ITypesUtil::Marshal(data, windowId); });
    IMSA_HILOGD("InputMethodAgentProxy::SetCallingWindow ret = %{public}d", ret);
}

void InputMethodAgentProxy::OnConfigurationChange(const Configuration &config)
{
    auto ret = SendRequest(ON_CONFIGURATION_CHANGE, [&config](MessageParcel &data) {
        return data.WriteInt32(static_cast<int32_t>(config.GetEnterKeyType()))
               && data.WriteInt32(static_cast<int32_t>(config.GetTextInputType()));
    });
    IMSA_HILOGD("InputMethodAgentProxy, ret = %{public}d", ret);
}

int32_t InputMethodAgentProxy::SendPrivateCommand(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    int32_t res = -1;
    int32_t ret = SendRequest(
        SEND_PRIVATE_COMMAND,
        [&privateCommand](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, privateCommand); },
        [&res](MessageParcel &reply) { return ITypesUtil::Unmarshal(reply, res); });
    return ret == ErrorCode::NO_ERROR ? res : ret;
}

int32_t InputMethodAgentProxy::SendRequest(int code, ParcelHandler input, ParcelHandler output)
{
    IMSA_HILOGD("InputMethodAgentProxy run in, code = %{public}d", code);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option{ MessageOption::TF_SYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        IMSA_HILOGE("InputMethodAgentProxy::write interface token failed");
        return ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT;
    }
    if (input != nullptr && (!input(data))) {
        IMSA_HILOGE("InputMethodAgentProxy::write data failed");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        IMSA_HILOGE("InputMethodAgentProxy::SendRequest remote is nullptr.");
        return ERROR_EX_NULL_POINTER;
    }
    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        IMSA_HILOGE("InputMethodCoreProxy send request failed, code: %{public}d, ret: %{public}d", code, ret);
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
