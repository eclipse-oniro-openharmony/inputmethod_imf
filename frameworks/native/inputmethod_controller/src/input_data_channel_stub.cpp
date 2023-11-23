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

#include "input_data_channel_stub.h"

#include "ffrt_inner.h"
#include "global.h"
#include "input_method_controller.h"
#include "ipc_object_stub.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "itypes_util.h"
#include "message.h"
namespace OHOS {
namespace MiscServices {
InputDataChannelStub::InputDataChannelStub()
{
}

InputDataChannelStub::~InputDataChannelStub()
{
}

int32_t InputDataChannelStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    IMSA_HILOGD("InputDataChannelStub, code: %{public}u, callingPid: %{public}d, callingUid: %{public}d", code,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
    auto descriptorToken = data.ReadInterfaceToken();
    if (descriptorToken != IInputDataChannel::GetDescriptor()) {
        IMSA_HILOGE("InputDataChannelStub descriptor error");
        return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
    }
    if (code >= FIRST_CALL_TRANSACTION && code < static_cast<uint32_t>(DATA_CHANNEL_CMD_LAST)) {
        return (this->*HANDLERS.at(code))(data, reply);
    } else {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t InputDataChannelStub::InsertTextOnRemote(MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGI("InputDataChannelStub, callingPid: %{public}d, callingUid: %{public}d", IPCSkeleton::GetCallingPid(),
        IPCSkeleton::GetCallingUid());
    std::u16string text;
    if (!ITypesUtil::Unmarshal(data, text)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return reply.WriteInt32(InsertText(text)) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::DeleteForwardOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t length = 0;
    if (!ITypesUtil::Unmarshal(data, length)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return reply.WriteInt32(DeleteForward(length)) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::DeleteBackwardOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t length = 0;
    if (!ITypesUtil::Unmarshal(data, length)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return reply.WriteInt32(DeleteBackward(length)) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::GetTextBeforeCursorOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t length = 0;
    if (!ITypesUtil::Unmarshal(data, length)) {
        IMSA_HILOGE("failed to unmarshal");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    std::u16string text;
    return ITypesUtil::Marshal(reply, GetTextBeforeCursor(length, text), text) ? ErrorCode::NO_ERROR
                                                                               : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::GetTextAfterCursorOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t length = 0;
    if (!ITypesUtil::Unmarshal(data, length)) {
        IMSA_HILOGE("failed to unmarshal");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    std::u16string text;
    return ITypesUtil::Marshal(reply, GetTextAfterCursor(length, text), text) ? ErrorCode::NO_ERROR
                                                                              : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::GetTextConfigOnRemote(MessageParcel &data, MessageParcel &reply)
{
    TextTotalConfig config;
    return ITypesUtil::Marshal(reply, GetTextConfig(config), config) ? ErrorCode::NO_ERROR
                                                                     : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::SendKeyboardStatusOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t status = -1;
    if (!ITypesUtil::Unmarshal(data, status)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    SendKeyboardStatus(static_cast<KeyboardStatus>(status));
    return reply.WriteInt32(ErrorCode::NO_ERROR) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::SendFunctionKeyOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t functionKey = 0;
    if (!ITypesUtil::Unmarshal(data, functionKey)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return reply.WriteInt32(SendFunctionKey(functionKey)) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::MoveCursorOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t direction = 0;
    if (!ITypesUtil::Unmarshal(data, direction)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return reply.WriteInt32(MoveCursor(direction)) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::GetEnterKeyTypeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t type = 0;
    return ITypesUtil::Marshal(reply, GetEnterKeyType(type), type) ? ErrorCode::NO_ERROR
                                                                   : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::GetInputPatternOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t pattern = 0;
    return ITypesUtil::Marshal(reply, GetInputPattern(pattern), pattern) ? ErrorCode::NO_ERROR
                                                                         : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::SelectByRangeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t start = 0;
    int32_t end = 0;
    if (!ITypesUtil::Unmarshal(data, start, end)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return reply.WriteInt32(SelectByRange(start, end)) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::SelectByMovementOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t direction = 0;
    int32_t skip = 0;
    if (!ITypesUtil::Unmarshal(data, direction, skip)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return reply.WriteInt32(SelectByMovement(direction, skip)) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::HandleExtendActionOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t action = 0;
    if (!ITypesUtil::Unmarshal(data, action)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return reply.WriteInt32(HandleExtendAction(action)) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::GetTextIndexAtCursorOnRemote(MessageParcel &data, MessageParcel &reply)
{
    int32_t index = -1;
    return ITypesUtil::Marshal(reply, GetTextIndexAtCursor(index), index) ? ErrorCode::NO_ERROR
                                                                          : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::NotifyPanelStatusInfoOnRemote(MessageParcel &data, MessageParcel &reply)
{
    PanelStatusInfo info{};
    if (!ITypesUtil::Unmarshal(data, info)) {
        IMSA_HILOGE("failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    NotifyPanelStatusInfo(info);
    return reply.WriteInt32(ErrorCode::NO_ERROR) ? ErrorCode::NO_ERROR : ErrorCode::ERROR_EX_PARCELABLE;
}

int32_t InputDataChannelStub::InsertText(const std::u16string &text)
{
    return InputMethodController::GetInstance()->InsertText(text);
}

int32_t InputDataChannelStub::DeleteForward(int32_t length)
{
    return InputMethodController::GetInstance()->DeleteForward(length);
}

int32_t InputDataChannelStub::DeleteBackward(int32_t length)
{
    return InputMethodController::GetInstance()->DeleteBackward(length);
}

int32_t InputDataChannelStub::GetTextBeforeCursor(int32_t number, std::u16string &text)
{
    return InputMethodController::GetInstance()->GetLeft(number, text);
}

int32_t InputDataChannelStub::GetTextAfterCursor(int32_t number, std::u16string &text)
{
    return InputMethodController::GetInstance()->GetRight(number, text);
}

int32_t InputDataChannelStub::GetTextIndexAtCursor(int32_t &index)
{
    return InputMethodController::GetInstance()->GetTextIndexAtCursor(index);
}

int32_t InputDataChannelStub::GetEnterKeyType(int32_t &keyType)
{
    return InputMethodController::GetInstance()->GetEnterKeyType(keyType);
}

int32_t InputDataChannelStub::GetInputPattern(int32_t &inputPattern)
{
    return InputMethodController::GetInstance()->GetInputPattern(inputPattern);
}

int32_t InputDataChannelStub::GetTextConfig(TextTotalConfig &textConfig)
{
    return InputMethodController::GetInstance()->GetTextConfig(textConfig);
}

void InputDataChannelStub::SendKeyboardStatus(KeyboardStatus status)
{
    InputMethodController::GetInstance()->SendKeyboardStatus(status);
}

int32_t InputDataChannelStub::SendFunctionKey(int32_t funcKey)
{
    return InputMethodController::GetInstance()->SendFunctionKey(funcKey);
}

int32_t InputDataChannelStub::MoveCursor(int32_t keyCode)
{
    return InputMethodController::GetInstance()->MoveCursor(static_cast<Direction>(keyCode));
}

int32_t InputDataChannelStub::SelectByRange(int32_t start, int32_t end)
{
    InputMethodController::GetInstance()->SelectByRange(start, end);
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelStub::SelectByMovement(int32_t direction, int32_t cursorMoveSkip)
{
    InputMethodController::GetInstance()->SelectByMovement(direction, cursorMoveSkip);
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelStub::HandleExtendAction(int32_t action)
{
    return InputMethodController::GetInstance()->HandleExtendAction(action);
}

void InputDataChannelStub::NotifyPanelStatusInfo(const PanelStatusInfo &info)
{
    InputMethodController::GetInstance()->NotifyPanelStatusInfo(info);
}
} // namespace MiscServices
} // namespace OHOS
