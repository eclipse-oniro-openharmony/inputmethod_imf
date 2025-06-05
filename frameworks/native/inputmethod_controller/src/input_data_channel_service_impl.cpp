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

ErrCode InputDataChannelServiceImpl::InsertText(const std::string &text, uint64_t msgId)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    } else {
    }
    int32_t ret = instance->InsertText(Str8ToStr16(text));
    ResponseData data = std::monostate{};
    instance->ResponseDataChannel(msgId, ret, data);
    return ret;
}

ErrCode InputDataChannelServiceImpl::DeleteForward(int32_t length, uint64_t msgId)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    int32_t ret = instance->DeleteForward(length);
    ResponseData data = std::monostate{};
    instance->ResponseDataChannel(msgId, ret, data);
    return ret;
}

ErrCode InputDataChannelServiceImpl::DeleteBackward(int32_t length, uint64_t msgId)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    int32_t ret = instance->DeleteBackward(length);
    ResponseData data = std::monostate{};
    instance->ResponseDataChannel(msgId, ret, data);
    return ret;
}

ErrCode InputDataChannelServiceImpl::GetTextBeforeCursor(int32_t number, uint64_t msgId)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    std::string text;
    std::u16string textu16 = Str8ToStr16(text);
    int32_t ret = instance->GetLeft(number, textu16);
    ResponseData data = Str16ToStr8(textu16);
    instance->ResponseDataChannel(msgId, ret, data);
    return ret;
}

ErrCode InputDataChannelServiceImpl::GetTextAfterCursor(int32_t number, uint64_t msgId)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    std::string text;
    std::u16string textu16 = Str8ToStr16(text);
    auto ret =  instance->GetRight(number, textu16);
    ResponseData data = Str16ToStr8(textu16);
    instance->ResponseDataChannel(msgId, ret, data);
    return ret;
}

ErrCode InputDataChannelServiceImpl::GetTextIndexAtCursor(uint64_t msgId)
{
    int32_t index = 0;
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret = instance->GetTextIndexAtCursor(index);
    ResponseData data = index;
    instance->ResponseDataChannel(msgId, ret, data);
    return ret;
}

ErrCode InputDataChannelServiceImpl::GetEnterKeyType(int32_t &keyType)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return instance->GetEnterKeyType(keyType);
}

ErrCode InputDataChannelServiceImpl::GetInputPattern(int32_t &inputPattern)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return instance->GetInputPattern(inputPattern);
}

ErrCode InputDataChannelServiceImpl::GetTextConfig(const TextTotalConfigInner &textConfigInner, uint64_t msgId)
{
    TextTotalConfig textConfig = InputMethodTools::GetInstance().InnerToTextTotalConfig(textConfigInner);

    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret =  instance->GetTextConfig(textConfig);
    ResponseData data = textConfig;
    instance->ResponseDataChannel(msgId, ret, data);
    return ret;
}

ErrCode InputDataChannelServiceImpl::GetTextConfigSync(TextTotalConfigInner &textConfigInner)
{
    TextTotalConfig textConfig = InputMethodTools::GetInstance().InnerToTextTotalConfig(textConfigInner);
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret =  instance->GetTextConfig(textConfig);
    textConfigInner = InputMethodTools::GetInstance().TextTotalConfigToInner(textConfig);
    return ret;
}

ErrCode InputDataChannelServiceImpl::SendKeyboardStatus(int32_t status)
{
    auto instance = InputMethodController::GetInstance();
    if (instance != nullptr) {
        instance->SendKeyboardStatus(static_cast<KeyboardStatus>(status));
    } else {
        IMSA_HILOGE("failed to get InputMethodController instance!");
    }
    return ERR_OK;
}

ErrCode InputDataChannelServiceImpl::SendFunctionKey(int32_t funcKey, uint64_t msgId)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret = instance->SendFunctionKey(funcKey);
    ResponseData data = std::monostate{};
    instance->ResponseDataChannel(msgId, ret, data);
    return ret;
}

ErrCode InputDataChannelServiceImpl::MoveCursor(int32_t keyCode, uint64_t msgId)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret = instance->MoveCursor(static_cast<Direction>(keyCode));
    ResponseData data = std::monostate{};
    instance->ResponseDataChannel(msgId, ret, data);
    return ret;
}

ErrCode InputDataChannelServiceImpl::SelectByRange(int32_t start, int32_t end, uint64_t msgId)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ERR_OK;
    }

    instance->SelectByRange(start, end);
    ResponseData data = std::monostate{};
    instance->ResponseDataChannel(msgId, ERR_OK, data);
    return ERR_OK;
}

ErrCode InputDataChannelServiceImpl::SelectByMovement(int32_t direction, int32_t cursorMoveSkip, uint64_t msgId)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGW("failed to get InputMethodController instance!");
        return ERR_OK;
    }
    instance->SelectByMovement(direction, cursorMoveSkip);
    ResponseData data = std::monostate{};
    instance->ResponseDataChannel(msgId, ERR_OK, data);
    return ERR_OK;
}

ErrCode InputDataChannelServiceImpl::HandleExtendAction(int32_t action, uint64_t msgId)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret = instance->HandleExtendAction(action);
    ResponseData data = std::monostate{};
    instance->ResponseDataChannel(msgId, ret, data);
    return ret;
}

ErrCode InputDataChannelServiceImpl::NotifyPanelStatusInfo(const PanelStatusInfoInner &info)
{
    PanelStatusInfo panelStatusInfo = InputMethodTools::GetInstance().InnerToPanelStatusInfo(info);
    auto instance = InputMethodController::GetInstance();
    if (instance != nullptr) {
        instance->NotifyPanelStatusInfo(panelStatusInfo);
    } else {
        IMSA_HILOGW("failed to get InputMethodController instance!");
    }
    return ERR_OK;
}

ErrCode InputDataChannelServiceImpl::NotifyKeyboardHeight(uint32_t height)
{
    auto instance = InputMethodController::GetInstance();
    if (instance != nullptr) {
        instance->NotifyKeyboardHeight(height);
    } else {
        IMSA_HILOGW("failed to get InputMethodController instance!");
    }
    return ERR_OK;
}

ErrCode InputDataChannelServiceImpl::SendPrivateCommand(const Value &value, uint64_t msgId)
{
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    privateCommand = value.valueMap;
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret = instance->ReceivePrivateCommand(privateCommand);
    ResponseData data = std::monostate{};
    instance->ResponseDataChannel(msgId, ret, data);
    return ret;
}

ErrCode InputDataChannelServiceImpl::SetPreviewText(
    const std::string &text, const RangeInner &rangeInner, uint64_t msgId)
{
    Range range = InputMethodTools::GetInstance().InnerToRange(rangeInner);
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret = instance->SetPreviewText(text, range);
    ResponseData data = std::monostate{};
    instance->ResponseDataChannel(msgId, ret, data);
    return ret;
}

ErrCode InputDataChannelServiceImpl::FinishTextPreview(uint64_t msgId)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    auto ret = instance->FinishTextPreview();
    ResponseData data = std::monostate{};
    instance->ResponseDataChannel(msgId, ret, data);
    return ret;
}

ErrCode InputDataChannelServiceImpl::SendMessage(const ArrayBuffer &arraybuffer)
{
    auto instance = InputMethodController::GetInstance();
    if (instance == nullptr) {
        IMSA_HILOGE("failed to get InputMethodController instance!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    return instance->RecvMessage(arraybuffer);
}
} // namespace MiscServices
} // namespace OHOS