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

#ifndef SERVICES_INCLUDE_GLOBAL_H
#define SERVICES_INCLUDE_GLOBAL_H

#include <errno.h>
#include <functional>
#include <sys/time.h>
#include <time.h>

#include "hilog/log.h"

namespace OHOS {
namespace MiscServices {
// Error Code
namespace ErrorCode {
// Error Code definition in the input method management system
enum {
    ERROR_STATUS_UNKNOWN_TRANSACTION = -EBADMSG, // unknown transaction

    // binder exception error code from Status.h
    ERROR_EX_ILLEGAL_ARGUMENT = -3,      // illegal argument exception
    ERROR_EX_NULL_POINTER = -4,          // null pointer exception
    ERROR_EX_ILLEGAL_STATE = -5,         // illegal state exception
    ERROR_EX_PARCELABLE = -6,            // parcelable exception
    ERROR_EX_UNSUPPORTED_OPERATION = -7, // unsupported operation exception
    ERROR_EX_SERVICE_SPECIFIC = -8,      // service specific exception
    // no error
    NO_ERROR = 0, // no error

    ERROR_NULL_POINTER,   // null pointer
    ERROR_BAD_PARAMETERS, // bad parameters
    ERROR_SUBSCRIBE_KEYBOARD_EVENT,

    ERROR_CONTROLLER_INVOKING_FAILED,
    ERROR_PERSIST_CONFIG,
    ERROR_KBD_HIDE_FAILED,
    ERROR_PACKAGE_MANAGER,
    ERROR_REMOTE_CLIENT_DIED,

    ERROR_NOT_CURRENT_IME,
    ERROR_NOT_IME,
    ERROR_NOT_AI_APP_IME,
    ERROR_ADD_DEATH_RECIPIENT_FAILED,
    ERROR_STATUS_SYSTEM_PERMISSION, // not system application
    ERROR_PARAMETER_CHECK_FAILED,
    ERROR_KEYWORD_NOT_FOUND,
    ERROR_ENABLE_IME,
    ERROR_NOT_DEFAULT_IME,
    ERROR_ENABLE_SECURITY_MODE,
    ERROR_DISPATCH_KEY_EVENT,
    ERROR_INVALID_PRIVATE_COMMAND_SIZE,
    ERROR_PANEL_NOT_FOUND,
    ERROR_WINDOW_MANAGER,
    ERROR_GET_TEXT_CONFIG,
    ERROR_SYSTEM_CMD_CHANNEL_ERROR,
    ERROR_INVALID_PRIVATE_COMMAND,
    ERROR_OS_ACCOUNT,
    ERROR_TASK_MANAGER_PEND_FAILED,
    ERROR_INVALID_PANEL_TYPE,
    ERROR_INVALID_PANEL_FLAG,
    ERROR_MSG_HANDLER_NOT_REGIST,
    ERROR_SECURITY_MODE_OFF,
    ERROR_MESSAGE_HANDLER,
    ERROR_INVALID_ARRAY_BUFFER_SIZE,
    ERROR_SERVICE_START_FAILED,
    ERROR_JS_CB_NOT_REGISTER,       // only for hiSysEvent
    ERROR_DEAL_TIMEOUT,              // only for hiSysEvent
    ERROR_IPC_REMOTE_NULLPTR,

    ERROR_IMA_BEGIN,
    ERROR_IME,
    ERROR_OPERATE_PANEL,
    ERROR_IMA_CHANNEL_NULLPTR,
    ERROR_IMA_NULLPTR,
    ERROR_IMA_END,

    ERROR_IMC_BEGIN,
    ERROR_CLIENT_NOT_EDITABLE,
    ERROR_TEXT_PREVIEW_NOT_SUPPORTED,
    ERROR_TEXT_LISTENER_ERROR,
    ERROR_INVALID_RANGE,
    ERROR_CLIENT_NOT_BOUND,
    ERROR_IMC_NULLPTR,
    ERROR_IMC_END,

    ERROR_IMSA_BEGIN,
    ERROR_PARSE_CONFIG_FILE,
    ERROR_IME_START_INPUT_FAILED,
    ERROR_STATUS_PERMISSION_DENIED,
    ERROR_CLIENT_NOT_FOCUSED,
    ERROR_CLIENT_NULL_POINTER,
    ERROR_CLIENT_ADD_FAILED,
    ERROR_CLIENT_NOT_FOUND,
    ERROR_IME_NOT_STARTED,
    ERROR_KBD_SHOW_FAILED,  // failed to show keyboard
    ERROR_IMSA_INPUT_TYPE_NOT_FOUND,
    ERROR_IMSA_DEFAULT_IME_NOT_FOUND,
    ERROR_IMSA_CLIENT_INPUT_READY_FAILED,
    ERROR_IMSA_MALLOC_FAILED,
    ERROR_IMSA_NULLPTR,
    ERROR_IMSA_USER_SESSION_NOT_FOUND,
    ERROR_IMSA_GET_IME_INFO_FAILED,
    ERROR_IMSA_IME_TO_START_NULLPTR,
    ERROR_IMSA_REBOOT_OLD_IME_NOT_STOP,
    ERROR_IMSA_IME_EVENT_CONVERT_FAILED,
    ERROR_IMSA_IME_CONNECT_FAILED,
    ERROR_IMSA_IME_DISCONNECT_FAILED,
    ERROR_IMSA_IME_START_TIMEOUT,
    ERROR_IMSA_IME_START_MORE_THAN_EIGHT_SECOND,
    ERROR_IMSA_FORCE_STOP_IME_TIMEOUT,
    ERROR_DEVICE_UNSUPPORTED,
    ERROR_SCENE_UNSUPPORTED,
    ERROR_PRIVATE_COMMAND_IS_EMPTY,
    ERROR_IME_NOT_FOUND,
    ERROR_DISABLE_SYSTEM_IME,
    ERROR_IMSA_END,
};
}; // namespace ErrorCode

static constexpr HiviewDFX::HiLogLabel g_SMALL_SERVICES_LABEL = { LOG_CORE, 0xD001C10, "ImsaKit" };

#define IMSA_HILOGD(fmt, ...)                                                                                    \
    (void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, OHOS::MiscServices::g_SMALL_SERVICES_LABEL.domain,                     \
        OHOS::MiscServices::g_SMALL_SERVICES_LABEL.tag, "line: %{public}d, function: %{public}s," fmt, __LINE__, \
        __FUNCTION__, ##__VA_ARGS__)
#define IMSA_HILOGE(fmt, ...)                                                                                    \
    (void)HILOG_IMPL(LOG_CORE, LOG_ERROR, OHOS::MiscServices::g_SMALL_SERVICES_LABEL.domain,                     \
        OHOS::MiscServices::g_SMALL_SERVICES_LABEL.tag, "line: %{public}d, function: %{public}s," fmt, __LINE__, \
        __FUNCTION__, ##__VA_ARGS__)
#define IMSA_HILOGF(fmt, ...)                                                                                    \
    (void)HILOG_IMPL(LOG_CORE, LOG_FATAL, OHOS::MiscServices::g_SMALL_SERVICES_LABEL.domain,                     \
        OHOS::MiscServices::g_SMALL_SERVICES_LABEL.tag, "line: %{public}d, function: %{public}s," fmt, __LINE__, \
        __FUNCTION__, ##__VA_ARGS__)
#define IMSA_HILOGI(fmt, ...)                                                                                    \
    (void)HILOG_IMPL(LOG_CORE, LOG_INFO, OHOS::MiscServices::g_SMALL_SERVICES_LABEL.domain,                      \
        OHOS::MiscServices::g_SMALL_SERVICES_LABEL.tag, "line: %{public}d, function: %{public}s," fmt, __LINE__, \
        __FUNCTION__, ##__VA_ARGS__)
#define IMSA_HILOGW(fmt, ...)                                                                                    \
    (void)HILOG_IMPL(LOG_CORE, LOG_WARN, OHOS::MiscServices::g_SMALL_SERVICES_LABEL.domain,                      \
        OHOS::MiscServices::g_SMALL_SERVICES_LABEL.tag, "line: %{public}d, function: %{public}s," fmt, __LINE__, \
        __FUNCTION__, ##__VA_ARGS__)
using Function = std::function<bool()>;
bool BlockRetry(uint32_t interval, uint32_t maxRetryTimes, Function func);
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_GLOBAL_H
