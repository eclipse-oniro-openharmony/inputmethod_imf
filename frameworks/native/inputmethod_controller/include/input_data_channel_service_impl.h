/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_IMPL_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_IMPL_H

#include "system_ability.h"

#include "iinput_data_channel.h"
#include "input_data_channel_stub.h"
#include "iremote_object.h"
#include "inputmethod_message_handler.h"

namespace OHOS {
namespace MiscServices {
class InputDataChannelServiceImpl final : public InputDataChannelStub,
    public std::enable_shared_from_this<InputDataChannelServiceImpl> {
    DISALLOW_COPY_AND_MOVE(InputDataChannelServiceImpl);

public:
    InputDataChannelServiceImpl();
    ~InputDataChannelServiceImpl();

    ErrCode InsertText(const std::string &text) override;
    ErrCode DeleteForward(int32_t length) override;
    ErrCode DeleteBackward(int32_t length) override;
    ErrCode GetTextBeforeCursor(int32_t number, std::string &text) override;
    ErrCode GetTextAfterCursor(int32_t number, std::string &text) override;
    ErrCode GetTextConfig(TextTotalConfigInner &textConfigInner) override;
    ErrCode SendKeyboardStatus(int32_t status) override;
    ErrCode SendFunctionKey(int32_t funcKey) override;
    ErrCode MoveCursor(int32_t keyCode) override;
    ErrCode GetEnterKeyType(int32_t &keyType) override;
    ErrCode GetInputPattern(int32_t &inputPattern) override;
    ErrCode SelectByRange(int32_t start, int32_t end) override;
    ErrCode SelectByMovement(int32_t direction, int32_t cursorMoveSkip) override;
    ErrCode HandleExtendAction(int32_t action) override;
    ErrCode GetTextIndexAtCursor(int32_t &index) override;
    ErrCode NotifyPanelStatusInfo(const PanelStatusInfo &info) override;
    ErrCode NotifyKeyboardHeight(uint32_t height) override;
    ErrCode SendPrivateCommand(const Value &value) override;
    ErrCode SetPreviewText(const std::string &text, const RangeInner &rangeInner) override;
    ErrCode FinishTextPreview() override;
    ErrCode FinishTextPreviewAsync() override;
    ErrCode SendMessage(const ArrayBuffer &arraybuffer) override;
};
}  // namespace MiscServices
}  // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_IMPL_H