/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_IMF_INPUTMETHOD_SERVICE_IPC_INTERFACE_CODE_H
#define INPUTMETHOD_IMF_INPUTMETHOD_SERVICE_IPC_INTERFACE_CODE_H

/* SAID: 3703*/
namespace OHOS {
namespace MiscServices {
enum class InputMethodInterfaceCode {
    PREPARE_INPUT = 0,
    START_INPUT,
    SHOW_CURRENT_INPUT,
    HIDE_CURRENT_INPUT,
    SHOW_INPUT,
    HIDE_INPUT,
    STOP_INPUT_SESSION,
    RELEASE_INPUT,
    GET_CURRENT_INPUT_METHOD,
    GET_CURRENT_INPUT_METHOD_SUBTYPE,
    LIST_INPUT_METHOD,
    LIST_INPUT_METHOD_SUBTYPE,
    LIST_CURRENT_INPUT_METHOD_SUBTYPE,
    SWITCH_INPUT_METHOD,
    DISPLAY_OPTIONAL_INPUT_METHOD,
    SET_CORE_AND_AGENT,
    SHOW_CURRENT_INPUT_DEPRECATED,
    HIDE_CURRENT_INPUT_DEPRECATED,
    DISPLAY_OPTIONAL_INPUT_DEPRECATED,
    PANEL_STATUS_CHANGE,
    UPDATE_LISTEN_EVENT_FLAG,
    IS_CURRENT_IME,
    UNREGISTERED_PROXY_IME,
    IMS_CMD_LAST
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUTMETHOD_IMF_INPUTMETHOD_SERVICE_IPC_INTERFACE_CODE_H
