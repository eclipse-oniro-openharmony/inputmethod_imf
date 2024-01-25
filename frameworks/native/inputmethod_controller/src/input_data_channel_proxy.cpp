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

#include "input_data_channel_proxy.h"

#include "global.h"
#include "ipc_types.h"
#include "itypes_util.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
InputDataChannelProxy::InputDataChannelProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IInputDataChannel>(object)
{
}

int32_t InputDataChannelProxy::InsertText(const std::u16string &text)
{
    return SendRequest(INSERT_TEXT, [&text](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, text); });
}

int32_t InputDataChannelProxy::DeleteForward(int32_t length)
{
    return SendRequest(DELETE_FORWARD, [length](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, length); });
}

int32_t InputDataChannelProxy::DeleteBackward(int32_t length)
{
    return SendRequest(
        DELETE_BACKWARD, [length](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, length); });
}

int32_t InputDataChannelProxy::GetTextBeforeCursor(int32_t number, std::u16string &text)
{
    return SendRequest(
        GET_TEXT_BEFORE_CURSOR, [number](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, number); },
        [&text](MessageParcel &parcel) { return ITypesUtil::Unmarshal(parcel, text); });
}

int32_t InputDataChannelProxy::GetTextAfterCursor(int32_t number, std::u16string &text)
{
    return SendRequest(
        GET_TEXT_AFTER_CURSOR, [number](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, number); },
        [&text](MessageParcel &parcel) { return ITypesUtil::Unmarshal(parcel, text); });
}

void InputDataChannelProxy::SendKeyboardStatus(KeyboardStatus status)
{
    SendRequest(SEND_KEYBOARD_STATUS,
        [status](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, static_cast<int32_t>(status)); });
}

void InputDataChannelProxy::NotifyPanelStatusInfo(const PanelStatusInfo &info)
{
    SendRequest(NOTIFY_PANEL_STATUS_INFO, [&info](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, info); });
}

int32_t InputDataChannelProxy::SendFunctionKey(int32_t funcKey)
{
    return SendRequest(
        SEND_FUNCTION_KEY, [funcKey](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, funcKey); });
}

int32_t InputDataChannelProxy::MoveCursor(int32_t keyCode)
{
    return SendRequest(MOVE_CURSOR, [keyCode](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, keyCode); });
}

int32_t InputDataChannelProxy::GetEnterKeyType(int32_t &keyType)
{
    return SendRequest(GET_ENTER_KEY_TYPE, nullptr,
        [&keyType](MessageParcel &parcel) { return ITypesUtil::Unmarshal(parcel, keyType); });
}

int32_t InputDataChannelProxy::GetInputPattern(int32_t &inputPattern)
{
    return SendRequest(GET_INPUT_PATTERN, nullptr,
        [&inputPattern](MessageParcel &parcel) { return ITypesUtil::Unmarshal(parcel, inputPattern); });
}

int32_t InputDataChannelProxy::GetTextIndexAtCursor(int32_t &index)
{
    return SendRequest(GET_TEXT_INDEX_AT_CURSOR, nullptr,
        [&index](MessageParcel &parcel) { return ITypesUtil::Unmarshal(parcel, index); });
}

int32_t InputDataChannelProxy::GetTextConfig(TextTotalConfig &textConfig)
{
    return SendRequest(GET_TEXT_CONFIG, nullptr,
        [&textConfig](MessageParcel &parcel) { return ITypesUtil::Unmarshal(parcel, textConfig); });
}

int32_t InputDataChannelProxy::SelectByRange(int32_t start, int32_t end)
{
    return SendRequest(
        SELECT_BY_RANGE, [start, end](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, start, end); });
}

int32_t InputDataChannelProxy::SelectByMovement(int32_t direction, int32_t cursorMoveSkip)
{
    return SendRequest(SELECT_BY_MOVEMENT, [direction, cursorMoveSkip](MessageParcel &parcel) {
        return ITypesUtil::Marshal(parcel, direction, cursorMoveSkip);
    });
}

int32_t InputDataChannelProxy::HandleExtendAction(int32_t action)
{
    return SendRequest(
        HANDLE_EXTEND_ACTION, [action](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, action); });
}

void InputDataChannelProxy::NotifyKeyboardHeight(uint32_t height)
{
    SendRequest(
        NOTIFY_KEYBOARD_HEIGHT, [height](MessageParcel &parcel) { return ITypesUtil::Marshal(parcel, height); });
}

void InputDataChannelProxy::GetMessageOption(int32_t code, MessageOption &option)
{
    switch (code) {
        case SEND_KEYBOARD_STATUS:
        case NOTIFY_KEYBOARD_HEIGHT:
            IMSA_HILOGD("Async IPC.");
            option.SetFlags(MessageOption::TF_ASYNC);
            break;

        default:
            option.SetFlags(MessageOption::TF_SYNC);
            break;
    }
}

int32_t InputDataChannelProxy::SendRequest(int code, ParcelHandler input, ParcelHandler output)
{
    IMSA_HILOGD("InputDataChannelProxy run in, code = %{public}d", code);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    GetMessageOption(code, option);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        IMSA_HILOGE("write interface token failed");
        return ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT;
    }
    if (input != nullptr && (!input(data))) {
        IMSA_HILOGE("write data failed");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto ret = Remote()->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        IMSA_HILOGE("InputDataChannelProxy send request failed, code: %{public}d ret %{public}d", code, ret);
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
