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

    ErrCode InsertText(const std::string &text, uint64_t msgId) override;
    ErrCode DeleteForward(int32_t length, uint64_t msgId) override;
    ErrCode DeleteBackward(int32_t length, uint64_t msgId) override;
    ErrCode GetTextBeforeCursor(int32_t number, uint64_t msgId) override;
    ErrCode GetTextAfterCursor(int32_t number, uint64_t msgId) override;
    ErrCode GetTextConfig(const TextTotalConfigInner &textConfigInner, uint64_t msgId) override;
    ErrCode GetTextConfigSync(TextTotalConfigInner &textConfigInner) override;
    ErrCode SendKeyboardStatus(int32_t status, uint64_t msgId) override;
    ErrCode SendFunctionKey(int32_t funcKey, uint64_t msgId) override;
    ErrCode MoveCursor(int32_t keyCode, uint64_t msgId) override;
    ErrCode GetEnterKeyType(uint64_t msgId) override;
    ErrCode GetInputPattern(uint64_t msgId) override;
    ErrCode SelectByRange(int32_t start, int32_t end, uint64_t msgId) override;
    ErrCode SelectByMovement(int32_t direction, int32_t cursorMoveSkip, uint64_t msgId) override;
    ErrCode HandleExtendAction(int32_t action, uint64_t msgId) override;
    ErrCode GetTextIndexAtCursor(uint64_t msgId) override;
    ErrCode NotifyPanelStatusInfo(const PanelStatusInfoInner &info, uint64_t msgId) override;
    ErrCode NotifyKeyboardHeight(uint32_t height, uint64_t msgId) override;
    ErrCode SendPrivateCommand(const Value &value, uint64_t msgId) override;
    ErrCode SetPreviewText(const std::string &text, const RangeInner &rangeInner, uint64_t msgId) override;
    ErrCode FinishTextPreview(uint64_t msgId) override;
    ErrCode SendMessage(const ArrayBuffer &arraybuffer, uint64_t msgId) override;
};
}  // namespace MiscServices
}  // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_IMPL_H