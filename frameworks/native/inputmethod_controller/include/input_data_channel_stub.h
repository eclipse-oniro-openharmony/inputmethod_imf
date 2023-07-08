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
    void SendKeyboardStatus(int32_t status) override;
    int32_t SendFunctionKey(int32_t funcKey) override;
    int32_t MoveCursor(int32_t keyCode) override;
    int32_t GetEnterKeyType(int32_t &keyType) override;
    int32_t GetInputPattern(int32_t &inputPattern) override;
    int32_t SelectByRange(int32_t start, int32_t end) override;
    int32_t SelectByMovement(int32_t direction, int32_t cursorMoveSkip) override;
    int32_t HandleExtendAction(int32_t action) override;
    int32_t GetTextConfig(TextTotalConfig &textConfig) override;

private:
    MessageHandler *msgHandler;
    int32_t SelectByRangeOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t SelectByMovementOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t HandleExtendActionOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t GetText(int32_t msgId, MessageParcel &data, MessageParcel &reply);
    int32_t GetTextIndexAtCursor(int32_t msgId, MessageParcel &data, MessageParcel &reply);
    using MsgConstructor = std::function<Message *(MessageParcel &parcel)>;
    int32_t SendMessage(const MsgConstructor &msgConstructor);
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_STUB_H
