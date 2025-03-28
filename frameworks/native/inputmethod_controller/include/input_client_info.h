/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_IMF_INPUT__CLIENT_INFO_H
#define INPUTMETHOD_IMF_INPUT__CLIENT_INFO_H

#include "i_input_client.h"
#include "i_input_data_channel.h"
#include "input_attribute.h"
#include "input_death_recipient.h"

namespace OHOS {
namespace MiscServices {
enum class UpdateFlag : uint32_t {
    EVENTFLAG = 0,
    ISSHOWKEYBOARD,
    BINDIMETYPE,
    STATE,
    TEXT_CONFIG,
    UIEXTENSION_TOKENID,
    CLIENT_TYPE,
};
enum class ImeType : int32_t {
    IME = 0,
    PROXY_IME,
    PROXY_AGENT_IME,
    NONE
};
enum class ClientState : uint32_t {
    INACTIVE = 0,
    ACTIVE
};

enum ClientType : uint32_t {
    CAPI,
    JS,
    CJ,
    INNER_KIT,
    CLIENT_TYPE_END,
};

constexpr uint32_t NO_EVENT_ON = 0;
constexpr uint32_t IMF_INVALID_TOKENID = 0;
constexpr uint64_t DEFAULT_DISPLAY_ID = 0;
constexpr int64_t INVALID_PID = -1;
struct InputClientInfo {
    pid_t pid { -1 };                        // process id
    pid_t uid { -1 };                        // uid
    int32_t userID { 0 };                    // user id of input client
    uint64_t displayId { DEFAULT_DISPLAY_ID };
    bool isShowKeyboard { false };           // soft keyboard status
    ImeType bindImeType { ImeType::NONE };   // type of the ime client bind
    TextTotalConfig config = {};             // text config
    uint32_t eventFlag { NO_EVENT_ON };      // the flag of the all listen event
    InputAttribute attribute;                // the input client attribute
    sptr<IInputClient> client { nullptr };   // the remote object handler for service to callback input client
    sptr<IRemoteObject> channel { nullptr }; // the remote object handler for ime to callback input client
    sptr<InputDeathRecipient> deathRecipient { nullptr }; // death recipient of client
    ClientState state { ClientState::INACTIVE };          // the state of input client
    bool isNotifyInputStart { true };
    bool needHide { false }; // panel needs to be hidden first, when input pattern is switched between pwd and normal
    uint32_t uiExtensionTokenId { IMF_INVALID_TOKENID }; // the value is valid only in curClient and only UIExtension
    RequestKeyboardReason requestKeyboardReason { RequestKeyboardReason::NONE }; // show keyboard reason
    ClientType type{ INNER_KIT };                                               // for hiSysEvent
    std::string name; // for hiSysEvent, client name:SA/processName app/bundleName
};

struct HiSysEventClientInfo {
    int64_t pid{ -1 };
    ClientType type{ INNER_KIT };
    std::string name;
};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_INPUT_CLIENT_INFO_H
