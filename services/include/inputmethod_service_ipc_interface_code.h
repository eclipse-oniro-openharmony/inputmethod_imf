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
    IMS_CMD_BEGIN = FIRST_CALL_TRANSACTION,
    START_INPUT = IMS_CMD_BEGIN,
    SHOW_CURRENT_INPUT,
    HIDE_CURRENT_INPUT,
    SHOW_INPUT,
    HIDE_INPUT,
    STOP_INPUT_SESSION,
    RELEASE_INPUT,
    REQUEST_SHOW_INPUT,
    REQUEST_HIDE_INPUT,
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
    PANEL_STATUS_CHANGE,
    UPDATE_LISTEN_EVENT_FLAG,
    IS_CURRENT_IME,
    UNREGISTERED_PROXY_IME,
    IS_INPUT_TYPE_SUPPORTED,
    START_INPUT_TYPE,
    EXIT_CURRENT_INPUT_TYPE,
    GET_DEFAULT_INPUT_METHOD,
    GET_INPUT_METHOD_SETTINGS,
    IS_PANEL_SHOWN,
    GET_SECURITY_MODE,
    IS_DEFAULT_IME,
    CONNECT_SYSTEM_CMD,
    IS_CURRENT_IME_BY_PID,
    INIT_CONNECT,
    IS_DEFAULT_IME_SET,
    ENABLE_IME,
    SET_CALLING_WINDOW,
    GET_INPUT_START_INFO,
    GET_IME_STATE,
    IS_SYSTEM_APP,
    
    IMS_CMD_END,
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUTMETHOD_IMF_INPUTMETHOD_SERVICE_IPC_INTERFACE_CODE_H
