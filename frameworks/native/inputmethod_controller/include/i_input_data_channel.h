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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_I_INPUT_DATA_CHANNEL_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_I_INPUT_DATA_CHANNEL_H
#include <errors.h>

#include <unordered_map>

#include "global.h"
#include "input_method_utils.h"
#include "iremote_broker.h"

/**
 * brief Definition of interface IInputDataChannel
 * It defines the remote calls from input method service to input client
 */
namespace OHOS {
namespace MiscServices {
class IInputDataChannel : public IRemoteBroker {
public:
    enum {
        INSERT_TEXT = FIRST_CALL_TRANSACTION,
        DELETE_FORWARD,
        DELETE_BACKWARD,
        GET_TEXT_BEFORE_CURSOR,
        GET_TEXT_AFTER_CURSOR,
        GET_ENTER_KEY_TYPE,
        GET_INPUT_PATTERN,
        SEND_KEYBOARD_STATUS,
        SEND_FUNCTION_KEY,
        MOVE_CURSOR,
        SELECT_BY_RANGE,
        SELECT_BY_MOVEMENT,
        HANDLE_EXTEND_ACTION,
        GET_TEXT_INDEX_AT_CURSOR,
        GET_TEXT_CONFIG,
        NOTIFY_PANEL_STATUS_INFO,
        NOTIFY_KEYBOARD_HEIGHT,
        SEND_PRIVATE_COMMAND,
        DATA_CHANNEL_CMD_LAST
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.IInputDataChannel");

    virtual int32_t InsertText(const std::u16string &text) = 0;
    virtual int32_t DeleteForward(int32_t length) = 0;
    virtual int32_t DeleteBackward(int32_t length) = 0;
    virtual int32_t GetTextBeforeCursor(int32_t number, std::u16string &text) = 0;
    virtual int32_t GetTextAfterCursor(int32_t number, std::u16string &text) = 0;
    virtual int32_t GetTextConfig(TextTotalConfig &textConfig) = 0;
    virtual void SendKeyboardStatus(KeyboardStatus status) = 0;
    virtual int32_t SendFunctionKey(int32_t funcKey) = 0;
    virtual int32_t MoveCursor(int32_t keyCode) = 0;
    virtual int32_t GetEnterKeyType(int32_t &keyType) = 0;
    virtual int32_t GetInputPattern(int32_t &inputPattern) = 0;
    virtual int32_t SelectByRange(int32_t start, int32_t end) = 0;
    virtual int32_t SelectByMovement(int32_t direction, int32_t cursorMoveSkip) = 0;
    virtual int32_t HandleExtendAction(int32_t action) = 0;
    virtual int32_t GetTextIndexAtCursor(int32_t &index) = 0;
    virtual void NotifyPanelStatusInfo(const PanelStatusInfo &info) = 0;
    virtual void NotifyKeyboardHeight(uint32_t height) = 0;
    virtual int32_t SendPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) = 0;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_I_INPUT_DATA_CHANNEL_H
