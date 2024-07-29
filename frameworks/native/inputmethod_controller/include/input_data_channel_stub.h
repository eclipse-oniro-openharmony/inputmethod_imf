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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_STUB_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_STUB_H

#include <cstdint>
#include <string>

#include "i_input_data_channel.h"
#include "iremote_stub.h"
#include "message_handler.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
namespace MiscServices {
class InputMethodController;

class InputDataChannelStub : public IRemoteStub<IInputDataChannel> {
public:
    DISALLOW_COPY_AND_MOVE(InputDataChannelStub);
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    InputDataChannelStub();
    ~InputDataChannelStub();
    int32_t InsertText(const std::u16string &text) override;
    int32_t DeleteForward(int32_t length) override;
    int32_t DeleteBackward(int32_t length) override;
    int32_t GetTextBeforeCursor(int32_t number, std::u16string &text) override;
    int32_t GetTextAfterCursor(int32_t number, std::u16string &text) override;
    int32_t GetTextIndexAtCursor(int32_t &index) override;
    void SendKeyboardStatus(KeyboardStatus status) override;
    int32_t SendFunctionKey(int32_t funcKey) override;
    int32_t MoveCursor(int32_t keyCode) override;
    int32_t GetEnterKeyType(int32_t &keyType) override;
    int32_t GetInputPattern(int32_t &inputPattern) override;
    int32_t SelectByRange(int32_t start, int32_t end) override;
    int32_t SelectByMovement(int32_t direction, int32_t cursorMoveSkip) override;
    int32_t HandleExtendAction(int32_t action) override;
    int32_t GetTextConfig(TextTotalConfig &textConfig) override;
    void NotifyPanelStatusInfo(const PanelStatusInfo &info) override;
    void NotifyKeyboardHeight(uint32_t height) override;
    int32_t SendPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;
    int32_t SetPreviewText(const std::string &text, const Range &range) override;
    int32_t FinishTextPreview(bool isAsync) override;

private:
    int32_t InvalidRequest(MessageParcel &data, MessageParcel &reply)
    {
        return ERR_UNKNOWN_TRANSACTION;
    };
    int32_t InsertTextOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t DeleteForwardOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t DeleteBackwardOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetTextBeforeCursorOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetTextAfterCursorOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetTextConfigOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t SendKeyboardStatusOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t SendFunctionKeyOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t MoveCursorOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetEnterKeyTypeOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetInputPatternOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t SelectByRangeOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t SelectByMovementOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t HandleExtendActionOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetTextIndexAtCursorOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyPanelStatusInfoOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyKeyboardHeightOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t SendPrivateCommandOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t SetPreviewTextOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t FinishTextPreviewOnRemote(MessageParcel &data, MessageParcel &reply);
    using RequestHandler = int32_t (InputDataChannelStub::*)(MessageParcel &, MessageParcel &);
    const RequestHandler HANDLERS[DATA_CHANNEL_CMD_LAST] = {
        &InputDataChannelStub::InvalidRequest,
        &InputDataChannelStub::InsertTextOnRemote,
        &InputDataChannelStub::DeleteForwardOnRemote,
        &InputDataChannelStub::DeleteBackwardOnRemote,
        &InputDataChannelStub::GetTextBeforeCursorOnRemote,
        &InputDataChannelStub::GetTextAfterCursorOnRemote,
        &InputDataChannelStub::GetEnterKeyTypeOnRemote,
        &InputDataChannelStub::GetInputPatternOnRemote,
        &InputDataChannelStub::SendKeyboardStatusOnRemote,
        &InputDataChannelStub::SendFunctionKeyOnRemote,
        &InputDataChannelStub::MoveCursorOnRemote,
        &InputDataChannelStub::SelectByRangeOnRemote,
        &InputDataChannelStub::SelectByMovementOnRemote,
        &InputDataChannelStub::HandleExtendActionOnRemote,
        &InputDataChannelStub::GetTextIndexAtCursorOnRemote,
        &InputDataChannelStub::GetTextConfigOnRemote,
        &InputDataChannelStub::NotifyPanelStatusInfoOnRemote,
        &InputDataChannelStub::NotifyKeyboardHeightOnRemote,
        &InputDataChannelStub::SendPrivateCommandOnRemote,
        &InputDataChannelStub::SetPreviewTextOnRemote,
        &InputDataChannelStub::FinishTextPreviewOnRemote,
    };
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_STUB_H
