/*
 * Copyright (C) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_MESSAGE_HANDLER_H
#define INPUTMETHOD_MESSAGE_HANDLER_H

#include <condition_variable>
#include <mutex>
#include <queue>

#include "global.h"
#include "message.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
namespace MessageID {
enum {
    MSG_ID_USER_START = 0,  //  a user started
    MSG_ID_USER_REMOVED,    // a user removed
    MSG_ID_PACKAGE_REMOVED, // a package is removed
    MSG_ID_BUNDLE_SCAN_FINISHED, // bundle scan finished
    MSG_ID_DATA_SHARE_READY, // datashare ready, ready to create data share helper
    MSG_ID_PACKAGE_ADDED,
    MSG_ID_PACKAGE_CHANGED,
    MSG_ID_SYS_LANGUAGE_CHANGED,
    MSG_ID_OS_ACCOUNT_STARTED,
    MSG_ID_BOOT_COMPLETED,
    MSG_ID_SCREEN_UNLOCK,
    MSG_ID_SELECT_BY_RANGE,
    MSG_ID_SELECT_BY_MOVEMENT,
    MSG_ID_HANDLE_EXTEND_ACTION,
    MSG_ID_USER_STOP,
    MSG_ID_REGULAR_UPDATE_IME_INFO,

    MSG_ID_HIDE_KEYBOARD_SELF, // hide the current keyboard

    // the request from IMSA to IMC
    MSG_ID_INSERT_CHAR,
    MSG_ID_DELETE_FORWARD,
    MSG_ID_DELETE_BACKWARD,
    MSG_ID_ON_INPUT_STOP,
    MSG_ID_SEND_KEYBOARD_STATUS,
    MSG_ID_SEND_FUNCTION_KEY,
    MSG_ID_MOVE_CURSOR,
    MSG_ID_ON_SWITCH_INPUT,
    MSG_ID_GET_TEXT_INDEX_AT_CURSOR,
    MSG_ID_GET_TEXT_BEFORE_CURSOR,
    MSG_ID_GET_TEXT_AFTER_CURSOR,
    MSG_ID_ON_PANEL_STATUS_CHANGE,

    // the request from IMSA to IMA
    MSG_ID_INIT_INPUT_CONTROL_CHANNEL,
    MSG_ID_SET_SUBTYPE,

    // the request from IMC to IMA
    MSG_ID_ON_CURSOR_UPDATE,
    MSG_ID_ON_SELECTION_CHANGE,
    MSG_ID_ON_CONFIGURATION_CHANGE,
    MSG_ID_ON_ATTRIBUTE_CHANGE,
    MSG_ID_QUIT_WORKER_THREAD,
    MSG_ID_SET_COREANDANGENT,
};
}

class MessageHandler {
public:
    MessageHandler();
    ~MessageHandler();
    void SendMessage(Message *msg);
    Message *GetMessage();
    static MessageHandler *Instance();
    static std::mutex handlerMutex_;

private:
    std::mutex mMutex;            // a mutex to guard message queue
    std::condition_variable mCV;  // condition variable to work with mMutex
    std::queue<Message *> mQueue; // Message queue, guarded by mMutex;

    MessageHandler(const MessageHandler &);
    MessageHandler &operator=(const MessageHandler &);
    MessageHandler(const MessageHandler &&);
    MessageHandler &operator=(const MessageHandler &&);
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_MESSAGE_HANDLER_H
