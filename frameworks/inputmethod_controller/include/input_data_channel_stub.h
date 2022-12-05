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
        int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                MessageOption &option) override;
        InputDataChannelStub();
        ~InputDataChannelStub();
        void SetHandler(MessageHandler *handler);

        int32_t InsertText(const std::u16string& text) override;
        int32_t DeleteForward(int32_t length) override;
        int32_t DeleteBackward(int32_t length) override;
        int32_t GetTextBeforeCursor(int32_t number, std::u16string &text) override;
        int32_t GetTextAfterCursor(int32_t number, std::u16string &text) override;
        void SendKeyboardStatus(int32_t status) override;
        int32_t SendFunctionKey(int32_t funcKey) override;
        int32_t MoveCursor(int32_t keyCode) override;
        int32_t GetEnterKeyType(int32_t &keyType) override;
        int32_t GetInputPattern(int32_t &inputPattern) override;

    private:
        MessageHandler *msgHandler;
    };
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_STUB_H
