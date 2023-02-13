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

#include "global.h"
#include "input_method_controller.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "ipc_skeleton.h"
#include "itypes_util.h"
#include "message.h"
namespace OHOS {
namespace MiscServices {
constexpr int32_t WAIT_TIME_STUB = 110;
InputDataChannelStub::InputDataChannelStub() : msgHandler(nullptr)
{
}

InputDataChannelStub::~InputDataChannelStub()
{
}

int32_t InputDataChannelStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    // todo
    IMSA_HILOGI("code = %{public}u, callingPid:%{public}d, callingUid:%{public}d", code, IPCSkeleton::GetCallingPid(),
                IPCSkeleton::GetCallingUid());
    auto descriptorToken = data.ReadInterfaceToken();
    if (descriptorToken != GetDescriptor()) {
        return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
    }
    switch (code) {
        case INSERT_TEXT: {
            reply.WriteInt32(InsertText(data.ReadString16()));
            break;
        }
        case DELETE_FORWARD: {
            reply.WriteInt32(DeleteForward(data.ReadInt32()));
            break;
        }
        case DELETE_BACKWARD: {
            reply.WriteInt32(DeleteBackward(data.ReadInt32()));
            break;
        }
        case GET_TEXT_BEFORE_CURSOR: {
            int32_t index = 0;
            std::u16string text;
            reply.WriteInt32(HandleGetOperation(data.ReadInt32(), text, index, GET_TEXT_BEFORE_CURSOR));
            reply.WriteString16(text);
            break;
        }
        case GET_TEXT_AFTER_CURSOR: {
            int32_t index = 0;
            std::u16string text;
            reply.WriteInt32(HandleGetOperation(data.ReadInt32(), text, index, GET_TEXT_AFTER_CURSOR));
            reply.WriteString16(text);
            break;
        }
        case GET_TEXT_INDEX_AT_CURSOR: {
            int32_t index = 0;
            std::u16string text;
            reply.WriteInt32(HandleGetOperation(0, text, index, GET_TEXT_INDEX_AT_CURSOR));
            reply.WriteInt32(index);
            break;
        }
        case SEND_KEYBOARD_STATUS: {
            SendKeyboardStatus(data.ReadInt32());
            break;
        }
        case SEND_FUNCTION_KEY: {
            reply.WriteInt32(SendFunctionKey(data.ReadInt32()));
            break;
        }
        case MOVE_CURSOR: {
            reply.WriteInt32(MoveCursor(data.ReadInt32()));
            break;
        }
        case GET_ENTER_KEY_TYPE: {
            int32_t keyType = 0;
            reply.WriteInt32(GetEnterKeyType(keyType));
            reply.WriteInt32(keyType);
            break;
        }
        case GET_INPUT_PATTERN: {
            int32_t inputPattern = 0;
            reply.WriteInt32(GetInputPattern(inputPattern));
            reply.WriteInt32(inputPattern);
            break;
        }
        case SELECT_BY_RANGE: {
            SelectByRangeOnRemote(data, reply);
            break;
        }
        case HANDLE_EXTEND_ACTION: {
            HandleExtendActionOnRemote(data, reply);
            break;
        }
        case SELECT_BY_MOVEMENT: {
            SelectByMovementOnRemote(data, reply);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return NO_ERROR;
}

int32_t InputDataChannelStub::SelectByRangeOnRemote(MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGD("InputDataChannelStub run in");
    int32_t start = 0;
    int32_t end = 0;
    int ret = SendMessage(MessageID::MSG_ID_SELECT_BY_RANGE, [&data, &start, &end](MessageParcel &parcel) {
        return ITypesUtil::Unmarshal(data, start, end) && ITypesUtil::Marshal(parcel, start, end);
    });
    if (!ITypesUtil::Marshal(reply, ret)) {
        IMSA_HILOGE("failed to write reply");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelStub::SelectByMovementOnRemote(MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGD("InputDataChannelStub run in");
    int32_t direction = 0;
    int32_t cursorMoveSkip = 0;
    auto ret =
        SendMessage(MessageID::MSG_ID_SELECT_BY_MOVEMENT, [&data, &direction, &cursorMoveSkip](MessageParcel &parcel) {
            return ITypesUtil::Unmarshal(data, direction, cursorMoveSkip)
                   && ITypesUtil::Marshal(parcel, direction, cursorMoveSkip);
        });
    if (!ITypesUtil::Marshal(reply, ret)) {
        IMSA_HILOGE("failed to write reply");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelStub::HandleExtendActionOnRemote(MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGD("InputDataChannelStub run in");
    int32_t action;
    auto ret = SendMessage(MessageID::MSG_ID_HANDLE_EXTEND_ACTION, [&data, &action](MessageParcel &parcel) {
        return ITypesUtil::Unmarshal(data, action) && ITypesUtil::Marshal(parcel, action);
    });
    if (!ITypesUtil::Marshal(reply, ret)) {
        IMSA_HILOGE("failed to write reply");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelStub::InsertText(const std::u16string &text)
{
    IMSA_HILOGI("InputDataChannelStub::InsertText");
    if (msgHandler == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    MessageParcel *parcel = new MessageParcel;
    parcel->WriteString16(text);
    Message *msg = new Message(MessageID::MSG_ID_INSERT_CHAR, parcel);
    msgHandler->SendMessage(msg);
    IMSA_HILOGI("InputDataChannelStub::InsertText return true");
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelStub::DeleteForward(int32_t length)
{
    IMSA_HILOGI("InputDataChannelStub::DeleteForward");
    if (msgHandler == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    MessageParcel *parcel = new MessageParcel;
    parcel->WriteInt32(length);
    Message *msg = new Message(MessageID::MSG_ID_DELETE_FORWARD, parcel);
    msgHandler->SendMessage(msg);

    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelStub::DeleteBackward(int32_t length)
{
    IMSA_HILOGI("InputDataChannelStub::DeleteBackward");
    if (msgHandler == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    MessageParcel *parcel = new MessageParcel;
    parcel->WriteInt32(length);
    Message *msg = new Message(MessageID::MSG_ID_DELETE_BACKWARD, parcel);
    msgHandler->SendMessage(msg);
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelStub::GetTextBeforeCursor(int32_t number, std::u16string &text)
{
    IMSA_HILOGI("InputDataChannelStub::GetTextBeforeCursor");
    return InputMethodController::GetInstance()->GetTextBeforeCursor(number, text);
}

int32_t InputDataChannelStub::GetTextAfterCursor(int32_t number, std::u16string &text)
{
    IMSA_HILOGI("InputDataChannelStub::GetTextAfterCursor");
    return InputMethodController::GetInstance()->GetTextAfterCursor(number, text);
}

int32_t InputDataChannelStub::GetTextIndexAtCursor(int32_t &index)
{
    return 0;
}

int32_t InputDataChannelStub::GetEnterKeyType(int32_t &keyType)
{
    IMSA_HILOGI("InputDataChannelStub::GetEnterKeyType");
    return InputMethodController::GetInstance()->GetEnterKeyType(keyType);
}

int32_t InputDataChannelStub::GetInputPattern(int32_t &inputPattern)
{
    IMSA_HILOGI("InputDataChannelStub::GetInputPattern");
    return InputMethodController::GetInstance()->GetInputPattern(inputPattern);
}

int32_t InputDataChannelStub::HandleGetOperation(int32_t number, std::u16string &text, int32_t &index, int32_t msgType)
{
    IMSA_HILOGI("InputDataChannelStub::start, msgId: %{public}d, number: %{public}d", msgType, number);
    if (msgHandler == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    int32_t msgId;
    if (msgType == GET_TEXT_BEFORE_CURSOR) {
        msgId = MessageID::MSG_ID_GET_TEXT_BEFORE_CURSOR;
    } else if (msgType == GET_TEXT_AFTER_CURSOR) {
        msgId = MessageID::MSG_ID_GET_TEXT_AFTER_CURSOR;
    } else {
        msgId = MessageID::MSG_ID_GET_TEXT_INDEX_AT_CURSOR;
    }
    MessageParcel *parcel = new MessageParcel;
    Message *msg = new Message(msgId, parcel);
    msgHandler->SendMessage(msg);

    std::unique_lock<std::mutex> lock(getOperationListenerLock_);
    getOperationListenerCv_.wait_for(lock, std::chrono::milliseconds(WAIT_TIME_STUB));
    if (msgType == GET_TEXT_BEFORE_CURSOR) {
        return InputMethodController::GetInstance()->GetTextBeforeCursor(number, text);
    } else if (msgType == GET_TEXT_AFTER_CURSOR) {
        return InputMethodController::GetInstance()->GetTextAfterCursor(number, text);
    } else {
        return InputMethodController::GetInstance()->GetTextIndexAtCursor(index);
    }
}

void InputDataChannelStub::NotifyGetOperationCompletion()
{
    getOperationListenerCv_.notify_one();
}

void InputDataChannelStub::SendKeyboardStatus(int32_t status)
{
    IMSA_HILOGI("InputDataChannelStub::SendKeyboardStatus");
    if (msgHandler != nullptr) {
        MessageParcel *parcel = new MessageParcel;
        parcel->WriteInt32(status);
        Message *msg = new Message(MessageID::MSG_ID_SEND_KEYBOARD_STATUS, parcel);
        msgHandler->SendMessage(msg);
    }
}

int32_t InputDataChannelStub::SendFunctionKey(int32_t funcKey)
{
    IMSA_HILOGI("InputDataChannelStub::SendFunctionKey");
    if (msgHandler == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    MessageParcel *parcel = new MessageParcel;
    parcel->WriteInt32(funcKey);
    Message *msg = new Message(MessageID::MSG_ID_SEND_FUNCTION_KEY, parcel);
    msgHandler->SendMessage(msg);
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelStub::MoveCursor(int32_t keyCode)
{
    IMSA_HILOGI("InputDataChannelStub::MoveCursor");
    if (msgHandler == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    MessageParcel *parcel = new MessageParcel;
    parcel->WriteInt32(keyCode);
    Message *msg = new Message(MessageID::MSG_ID_MOVE_CURSOR, parcel);
    msgHandler->SendMessage(msg);
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelStub::SelectByRange(int32_t start, int32_t end)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelStub::SelectByMovement(int32_t direction, int32_t cursorMoveSkip)
{
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelStub::HandleExtendAction(int32_t action)
{
    return ErrorCode::NO_ERROR;
}

void InputDataChannelStub::SetHandler(MessageHandler *handler)
{
    msgHandler = handler;
}

int32_t InputDataChannelStub::SendMessage(int code, ParcelHandler input)
{
    IMSA_HILOGD("InputMethodCoreStub run in");
    if (msgHandler == nullptr) {
        IMSA_HILOGE("InputMethodCoreStub msgHandler_ is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto *parcel = new (std::nothrow) MessageParcel();
    if (parcel == nullptr) {
        IMSA_HILOGE("parcel is nullptr");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    if (input != nullptr && (!input(*parcel))) {
        IMSA_HILOGE("write data failed");
        delete parcel;
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    auto *msg = new (std::nothrow) Message(code, parcel);
    if (msg == nullptr) {
        IMSA_HILOGE("msg is nullptr");
        delete parcel;
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    msgHandler->SendMessage(msg);
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS
