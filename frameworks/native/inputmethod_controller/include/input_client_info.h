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

#include "event_status_manager.h"
#include "i_input_client.h"
#include "i_input_data_channel.h"
#include "input_attribute.h"
#include "input_death_recipient.h"

namespace OHOS {
namespace MiscServices {
struct InputClientInfo {
    pid_t pid{ 0 };                                        // process id
    pid_t uid{ 0 };                                        // uid
    int32_t userID{ 0 };                                   // user id of input client
    int32_t displayID{ 0 };                                // the display id on which the input client is showing
    uint32_t tokenID{ 0 };                                 // the token id of input client
    bool isShowKeyboard{ false };                          // soft keyboard status
    bool isValid{ false };                                 // whether client is valid
    uint32_t eventFlag{ EventStatusManager::NO_EVENT_ON }; // the flag of the all listen event
    InputAttribute attribute;                              // the input client attribute
    sptr<IInputClient> client;                // the remote object handler for service to callback input client
    sptr<IInputDataChannel> channel;          // the remote object handler for ime to callback input client
    sptr<InputDeathRecipient> deathRecipient; // death recipient of client
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUTMETHOD_IMF_INPUT_CLIENT_INFO_H
