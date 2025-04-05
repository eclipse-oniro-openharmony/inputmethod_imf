/*
* Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "input_data_channel_service_impl.h"

#include "global.h"
#include "input_method_controller.h"
#include "ipc_object_stub.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "itypes_util.h"
#include "message.h"
#include "input_method_tools.h"
namespace OHOS {
namespace MiscServices {
InputDataChannelServiceImpl::InputDataChannelServiceImpl() {}

InputDataChannelServiceImpl::~InputDataChannelServiceImpl() {}

ErrCode InputDataChannelServiceImpl::InsertText(const std::string &text)
{
    return InputMethodController::GetInstance()->InsertText(Str8ToStr16(text));
}

ErrCode InputDataChannelServiceImpl::DeleteForward(int32_t length)
{
    return InputMethodController::GetInstance()->DeleteForward(length);
}

ErrCode InputDataChannelServiceImpl::DeleteBackward(int32_t length)
{
    return InputMethodController::GetInstance()->DeleteBackward(length);
}

ErrCode InputDataChannelServiceImpl::GetTextBeforeCursor(int32_t number, std::string &text)
{
    std::u16string textu16 = Str8ToStr16(text);
    auto ret = InputMethodController::GetInstance()->GetLeft(number, textu16);
    text = Str16ToStr8(textu16);
    return ret;
}

ErrCode InputDataChannelServiceImpl::GetTextAfterCursor(int32_t number, std::string &text)
{
    std::u16string textu16 = Str8ToStr16(text);

    auto ret =  InputMethodController::GetInstance()->GetRight(number, textu16);
    text = Str16ToStr8(textu16);
    return ret;
}

ErrCode InputDataChannelServiceImpl::GetTextIndexAtCursor(int32_t &index)
{
    return InputMethodController::GetInstance()->GetTextIndexAtCursor(index);
}

ErrCode InputDataChannelServiceImpl::GetEnterKeyType(int32_t &keyType)
{
    return InputMethodController::GetInstance()->GetEnterKeyType(keyType);
}

ErrCode InputDataChannelServiceImpl::GetInputPattern(int32_t &inputPattern)
{
    return InputMethodController::GetInstance()->GetInputPattern(inputPattern);
}

ErrCode InputDataChannelServiceImpl::GetTextConfig(TextTotalConfigInner &textConfigInner)
{
    TextTotalConfig textConfig = {};
    textConfig = InputMethodTools::GetInstance().InnerToTextTotalConfig(textConfigInner);
    
    auto ret =  InputMethodController::GetInstance()->GetTextConfig(textConfig);
    textConfigInner = InputMethodTools::GetInstance().TextTotalConfigToInner(textConfig);
    return ret;
}

ErrCode InputDataChannelServiceImpl::SendKeyboardStatus(int32_t status)
{
    InputMethodController::GetInstance()->SendKeyboardStatus(static_cast<KeyboardStatus>(status));
    return ERR_OK;
}

ErrCode InputDataChannelServiceImpl::SendFunctionKey(int32_t funcKey)
{
    return InputMethodController::GetInstance()->SendFunctionKey(funcKey);
}

ErrCode InputDataChannelServiceImpl::MoveCursor(int32_t keyCode)
{
    return InputMethodController::GetInstance()->MoveCursor(static_cast<Direction>(keyCode));
}

ErrCode InputDataChannelServiceImpl::SelectByRange(int32_t start, int32_t end)
{
    InputMethodController::GetInstance()->SelectByRange(start, end);
    return ERR_OK;
}

ErrCode InputDataChannelServiceImpl::SelectByMovement(int32_t direction, int32_t cursorMoveSkip)
{
    InputMethodController::GetInstance()->SelectByMovement(direction, cursorMoveSkip);
    return ERR_OK;
}

ErrCode InputDataChannelServiceImpl::HandleExtendAction(int32_t action)
{
    return InputMethodController::GetInstance()->HandleExtendAction(action);
}

ErrCode InputDataChannelServiceImpl::NotifyPanelStatusInfo(const PanelStatusInfo &info)
{
    InputMethodController::GetInstance()->NotifyPanelStatusInfo(info);
    return ERR_OK;
}

ErrCode InputDataChannelServiceImpl::NotifyKeyboardHeight(uint32_t height)
{
    InputMethodController::GetInstance()->NotifyKeyboardHeight(height);
    return ERR_OK;
}

ErrCode InputDataChannelServiceImpl::SendPrivateCommand(
    const Value &value)
{
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    privateCommand = value.valueMap;
    return InputMethodController::GetInstance()->ReceivePrivateCommand(privateCommand);
}

ErrCode InputDataChannelServiceImpl::SetPreviewText(const std::string &text, const RangeInner &rangeInner)
{
    Range range = InputMethodTools::GetInstance().InnerToRange(rangeInner);
    return InputMethodController::GetInstance()->SetPreviewText(text, range);
}

ErrCode InputDataChannelServiceImpl::FinishTextPreview()
{
    return InputMethodController::GetInstance()->FinishTextPreview();
}

ErrCode InputDataChannelServiceImpl::FinishTextPreviewAsync()
{
    return InputMethodController::GetInstance()->FinishTextPreview();
}

ErrCode InputDataChannelServiceImpl::SendMessage(const ArrayBuffer &arraybuffer)
{
    return InputMethodController::GetInstance()->RecvMessage(arraybuffer);
}
} // namespace MiscServices
} // namespace OHOS