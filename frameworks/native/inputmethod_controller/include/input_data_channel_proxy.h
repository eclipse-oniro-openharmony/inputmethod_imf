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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_PROXY_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_PROXY_H

#include <cstdint>
#include <functional>
#include <string>

#include "i_input_data_channel.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
namespace MiscServices {
class InputDataChannelProxy : public IRemoteProxy<IInputDataChannel> {
public:
    explicit InputDataChannelProxy(const sptr<IRemoteObject> &object);
    ~InputDataChannelProxy() = default;
    DISALLOW_COPY_AND_MOVE(InputDataChannelProxy);

    int32_t InsertText(const std::u16string &text) override;
    int32_t DeleteForward(int32_t length) override;
    int32_t DeleteBackward(int32_t length) override;
    int32_t GetTextBeforeCursor(int32_t number, std::u16string &text) override;
    int32_t GetTextAfterCursor(int32_t number, std::u16string &text) override;
    void SendKeyboardStatus(KeyboardStatus status) override;
    int32_t SendFunctionKey(int32_t funcKey) override;
    int32_t MoveCursor(int32_t keyCode) override;
    int32_t GetEnterKeyType(int32_t &keyType) override;
    int32_t GetInputPattern(int32_t &inputPattern) override;
    int32_t SelectByRange(int32_t start, int32_t end) override;
    int32_t SelectByMovement(int32_t direction, int32_t cursorMoveSkip) override;
    int32_t HandleExtendAction(int32_t action) override;
    int32_t GetTextIndexAtCursor(int32_t &index) override;
    int32_t GetTextConfig(TextTotalConfig &textConfig) override;
    void NotifyPanelStatusInfo(const PanelStatusInfo &info) override;
    void NotifyKeyboardHeight(uint32_t height) override;
    int32_t SendPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;

private:
    static inline BrokerDelegator<InputDataChannelProxy> delegator_;
    using ParcelHandler = std::function<bool(MessageParcel &)>;
    int32_t SendRequest(int code, ParcelHandler input = nullptr, ParcelHandler output = nullptr);
    void GetMessageOption(int32_t code, MessageOption &option);
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_PROXY_H
