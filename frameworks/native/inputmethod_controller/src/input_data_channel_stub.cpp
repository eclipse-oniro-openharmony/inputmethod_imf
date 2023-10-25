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
constexpr uint32_t MAX_TIMEOUT = 2500;
InputDataChannelStub::InputDataChannelStub()
{
}

InputDataChannelStub::~InputDataChannelStub()
{
}

int32_t InputDataChannelStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    IMSA_HILOGI("InputDataChannelStub, code: %{public}u, callingPid: %{public}d, callingUid: %{public}d", code,
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
    auto descriptorToken = data.ReadInterfaceToken();
    if (descriptorToken != IInputDataChannel::GetDescriptor()) {
        IMSA_HILOGE("InputDataChannelStub descriptor error");
        return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
    }
    if (code >= 0 && code < static_cast<uint32_t>(DATA_CHANNEL_CMD_LAST)) {
        return (this->*HANDLERS.at(code))(data, reply);
    } else {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t InputDataChannelStub::InsertTextOnRemote(MessageParcel &data, MessageParcel &reply)
{
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
    int32_t status = 0;
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
    auto result = std::make_shared<BlockData<int32_t>>(MAX_TIMEOUT);
    auto blockTask = [text, result]() {
        auto ret = InputMethodController::GetInstance()->InsertText(text);
        result->SetValue(ret);
    };
    ffrt::submit(blockTask);
    int32_t ret = 0;
    if (!result->GetValue(ret)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    return ret;
}

int32_t InputDataChannelStub::DeleteForward(int32_t length)
{
    auto result = std::make_shared<BlockData<int32_t>>(MAX_TIMEOUT);
    auto blockTask = [length, result]() {
        auto ret = InputMethodController::GetInstance()->DeleteForward(length);
        result->SetValue(ret);
    };
    ffrt::submit(blockTask);
    int32_t ret = 0;
    if (!result->GetValue(ret)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    return ret;
}

int32_t InputDataChannelStub::DeleteBackward(int32_t length)
{
    auto result = std::make_shared<BlockData<int32_t>>(MAX_TIMEOUT);
    auto blockTask = [length, result]() {
        auto ret = InputMethodController::GetInstance()->DeleteBackward(length);
        result->SetValue(ret);
    };
    ffrt::submit(blockTask);
    int32_t ret = 0;
    if (!result->GetValue(ret)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    return ret;
}

int32_t InputDataChannelStub::GetTextBeforeCursor(int32_t number, std::u16string &text)
{
    auto ret = std::make_shared<BlockData<ResultInfo<std::u16string>>>(MAX_TIMEOUT);
    auto blockTask = [number, ret]() {
        ResultInfo<std::u16string> info;
        info.errCode = InputMethodController::GetInstance()->GetLeft(number, info.data);
        ret->SetValue(info);
    };
    ffrt::submit(blockTask);
    ResultInfo<std::u16string> result;
    if (!ret->GetValue(result)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    text = result.data;
    return result.errCode;
}

int32_t InputDataChannelStub::GetTextAfterCursor(int32_t number, std::u16string &text)
{
    auto ret = std::make_shared<BlockData<ResultInfo<std::u16string>>>(MAX_TIMEOUT);
    auto blockTask = [number, ret]() {
        ResultInfo<std::u16string> info;
        info.errCode = InputMethodController::GetInstance()->GetRight(number, info.data);
        ret->SetValue(info);
    };
    ffrt::submit(blockTask);
    ResultInfo<std::u16string> result;
    if (!ret->GetValue(result)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    text = result.data;
    return result.errCode;
}

int32_t InputDataChannelStub::GetTextIndexAtCursor(int32_t &index)
{
    auto ret = std::make_shared<BlockData<ResultInfo<int32_t>>>(MAX_TIMEOUT);
    auto blockTask = [ret]() {
        ResultInfo<int32_t> info;
        info.errCode = InputMethodController::GetInstance()->GetTextIndexAtCursor(info.data);
        ret->SetValue(info);
    };
    ffrt::submit(blockTask);
    ResultInfo<int32_t> result;
    if (!ret->GetValue(result)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    index = result.data;
    return result.errCode;
}

int32_t InputDataChannelStub::GetEnterKeyType(int32_t &keyType)
{
    auto ret = std::make_shared<BlockData<ResultInfo<int32_t>>>(MAX_TIMEOUT);
    auto blockTask = [ret]() {
        ResultInfo<int32_t> info;
        info.errCode = InputMethodController::GetInstance()->GetEnterKeyType(info.data);
        ret->SetValue(info);
    };
    ffrt::submit(blockTask);
    ResultInfo<int32_t> result;
    if (!ret->GetValue(result)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    keyType = result.data;
    return result.errCode;
}

int32_t InputDataChannelStub::GetInputPattern(int32_t &inputPattern)
{
    auto ret = std::make_shared<BlockData<ResultInfo<int32_t>>>(MAX_TIMEOUT);
    auto blockTask = [ret]() {
        ResultInfo<int32_t> info;
        info.errCode = InputMethodController::GetInstance()->GetInputPattern(info.data);
        ret->SetValue(info);
    };
    ffrt::submit(blockTask);
    ResultInfo<int32_t> result;
    if (!ret->GetValue(result)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    inputPattern = result.data;
    return result.errCode;
}

int32_t InputDataChannelStub::GetTextConfig(TextTotalConfig &textConfig)
{
    auto ret = std::make_shared<BlockData<ResultInfo<TextTotalConfig>>>(MAX_TIMEOUT);
    auto blockTask = [ret]() {
        ResultInfo<TextTotalConfig> info;
        info.errCode = InputMethodController::GetInstance()->GetTextConfig(info.data);
        ret->SetValue(info);
    };
    ffrt::submit(blockTask);
    ResultInfo<TextTotalConfig> result;
    if (!ret->GetValue(result)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    textConfig = result.data;
    return result.errCode;
}

void InputDataChannelStub::SendKeyboardStatus(KeyboardStatus status)
{
    auto result = std::make_shared<BlockData<bool>>(MAX_TIMEOUT, false);
    auto blockTask = [status, result]() {
        InputMethodController::GetInstance()->SendKeyboardStatus(status);
        bool ret = true;
        result->SetValue(ret);
    };
    ffrt::submit(blockTask);
    if (!result->GetValue()) {
        IMSA_HILOGE("failed due to timeout");
    }
}

int32_t InputDataChannelStub::SendFunctionKey(int32_t funcKey)
{
    auto result = std::make_shared<BlockData<int32_t>>(MAX_TIMEOUT);
    auto blockTask = [funcKey, result]() {
        auto ret = InputMethodController::GetInstance()->SendFunctionKey(funcKey);
        result->SetValue(ret);
    };
    ffrt::submit(blockTask);
    int32_t ret = 0;
    if (!result->GetValue(ret)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    return ret;
}

int32_t InputDataChannelStub::MoveCursor(int32_t keyCode)
{
    auto result = std::make_shared<BlockData<int32_t>>(MAX_TIMEOUT);
    auto blockTask = [keyCode, result]() {
        auto ret = InputMethodController::GetInstance()->MoveCursor(static_cast<Direction>(keyCode));
        result->SetValue(ret);
    };
    ffrt::submit(blockTask);
    int32_t ret = 0;
    if (!result->GetValue(ret)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    return ret;
}

int32_t InputDataChannelStub::SelectByRange(int32_t start, int32_t end)
{
    auto result = std::make_shared<BlockData<int32_t>>(MAX_TIMEOUT);
    auto blockTask = [start, end, result]() {
        InputMethodController::GetInstance()->SelectByRange(start, end);
        int32_t ret = ErrorCode::NO_ERROR;
        result->SetValue(ret);
    };
    ffrt::submit(blockTask);
    int32_t ret = 0;
    if (!result->GetValue(ret)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    return ret;
}

int32_t InputDataChannelStub::SelectByMovement(int32_t direction, int32_t cursorMoveSkip)
{
    auto result = std::make_shared<BlockData<int32_t>>(MAX_TIMEOUT);
    auto blockTask = [direction, cursorMoveSkip, result]() {
        InputMethodController::GetInstance()->SelectByMovement(direction, cursorMoveSkip);
        int32_t ret = ErrorCode::NO_ERROR;
        result->SetValue(ret);
    };
    ffrt::submit(blockTask);
    int32_t ret = 0;
    if (!result->GetValue(ret)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    return ret;
}

int32_t InputDataChannelStub::HandleExtendAction(int32_t action)
{
    auto result = std::make_shared<BlockData<int32_t>>(MAX_TIMEOUT);
    auto blockTask = [action, result]() {
        InputMethodController::GetInstance()->HandleExtendAction(action);
        int32_t ret = ErrorCode::NO_ERROR;
        result->SetValue(ret);
    };
    ffrt::submit(blockTask);
    int32_t ret = 0;
    if (!result->GetValue(ret)) {
        IMSA_HILOGE("failed due to timeout");
        return ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED;
    }
    return ret;
}

void InputDataChannelStub::NotifyPanelStatusInfo(const PanelStatusInfo &info)
{
    auto result = std::make_shared<BlockData<bool>>(MAX_TIMEOUT, false);
    auto blockTask = [info, result]() {
        InputMethodController::GetInstance()->NotifyPanelStatusInfo(info);
        bool ret = true;
        result->SetValue(ret);
    };
    ffrt::submit(blockTask);
    if (!result->GetValue()) {
        IMSA_HILOGE("failed due to timeout");
    }
}
} // namespace MiscServices
} // namespace OHOS
