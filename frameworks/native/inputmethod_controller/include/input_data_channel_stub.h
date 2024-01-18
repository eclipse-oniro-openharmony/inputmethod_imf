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
    void SetHandler(MessageHandler *handler);
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

private:
    MessageHandler *msgHandler;
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
    using RequestHandler = int32_t (InputDataChannelStub::*)(MessageParcel &, MessageParcel &);
    static inline const std::unordered_map<int32_t, RequestHandler> HANDLERS = {
        { static_cast<uint32_t>(INSERT_TEXT), &InputDataChannelStub::InsertTextOnRemote },
        { static_cast<uint32_t>(DELETE_FORWARD), &InputDataChannelStub::DeleteForwardOnRemote },
        { static_cast<uint32_t>(DELETE_BACKWARD), &InputDataChannelStub::DeleteBackwardOnRemote },
        { static_cast<uint32_t>(GET_TEXT_BEFORE_CURSOR), &InputDataChannelStub::GetTextBeforeCursorOnRemote },
        { static_cast<uint32_t>(GET_TEXT_AFTER_CURSOR), &InputDataChannelStub::GetTextAfterCursorOnRemote },
        { static_cast<uint32_t>(GET_ENTER_KEY_TYPE), &InputDataChannelStub::GetEnterKeyTypeOnRemote },
        { static_cast<uint32_t>(GET_INPUT_PATTERN), &InputDataChannelStub::GetInputPatternOnRemote },
        { static_cast<uint32_t>(SEND_KEYBOARD_STATUS), &InputDataChannelStub::SendKeyboardStatusOnRemote },
        { static_cast<uint32_t>(SEND_FUNCTION_KEY), &InputDataChannelStub::SendFunctionKeyOnRemote },
        { static_cast<uint32_t>(MOVE_CURSOR), &InputDataChannelStub::MoveCursorOnRemote },
        { static_cast<uint32_t>(SELECT_BY_RANGE), &InputDataChannelStub::SelectByRangeOnRemote },
        { static_cast<uint32_t>(SELECT_BY_MOVEMENT), &InputDataChannelStub::SelectByMovementOnRemote },
        { static_cast<uint32_t>(HANDLE_EXTEND_ACTION), &InputDataChannelStub::HandleExtendActionOnRemote },
        { static_cast<uint32_t>(GET_TEXT_INDEX_AT_CURSOR), &InputDataChannelStub::GetTextIndexAtCursorOnRemote },
        { static_cast<uint32_t>(GET_TEXT_CONFIG), &InputDataChannelStub::GetTextConfigOnRemote },
        { static_cast<uint32_t>(NOTIFY_PANEL_STATUS_INFO), &InputDataChannelStub::NotifyPanelStatusInfoOnRemote },
        { static_cast<uint32_t>(NOTIFY_KEYBOARD_HEIGHT), &InputDataChannelStub::NotifyKeyboardHeightOnRemote },
    };
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_STUB_H
