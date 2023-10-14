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
#include <sys/time.h>
#include <time.h>

#include "hilog/log.h"
#include "ipc_object_stub.h"
#include "iremote_broker.h"
#include "peer_holder.h"
#include "refbase.h"

namespace OHOS {
namespace MiscServices {

using BRemoteObject = IPCObjectStub;

#define LOG_INFO(fmt, args...) \
    LogTimeStamp();            \
    printf("I %s:%d  %s - " fmt, basename(__FILE__), __LINE__, __FUNCTION__, ##args)

#define LOG_ERROR(fmt, args...) \
    LogTimeStamp();             \
    printf("E %s:%d  %s - " fmt, basename(__FILE__), __LINE__, __FUNCTION__, ##args)

#define LOG_WARNING(fmt, args...) \
    LogTimeStamp();               \
    printf("W %s:%d  %s - " fmt, basename(__FILE__), __LINE__, __FUNCTION__, ##args)

#if DEBUG
#define LOG_DEBUG(fmt, args...) \
    LogTimeStamp();             \
    printf("D %s:%d  %s - " fmt, basename(__FILE__), __LINE__, __FUNCTION__, ##args)
#else
#define LOG_DEBUG(fmt, args...)
#endif

void LogTimeStamp();

// Error Code
namespace ErrorCode {
// Error Code definition in the input method management system
enum {
    ERROR_STATUS_PERMISSION_DENIED = -EPERM,                          // permission denied
    ERROR_STATUS_UNKNOWN_TRANSACTION = -EBADMSG,                      // unknown transaction

    // binder exception error code from Status.h
    ERROR_EX_ILLEGAL_ARGUMENT = -3,      // illegal argument exception
    ERROR_EX_NULL_POINTER = -4,          // null pointer exception
    ERROR_EX_ILLEGAL_STATE = -5,         // illegal state exception
    ERROR_EX_PARCELABLE  = -6,           // parcelable exception
    ERROR_EX_UNSUPPORTED_OPERATION = -7, // unsupported operation exception
    ERROR_EX_SERVICE_SPECIFIC = -8,      // service specific exception
    // no error
    NO_ERROR = 0, // no error

    // system service error
    ERROR_NULL_POINTER = 1,          // null pointer
    ERROR_BAD_PARAMETERS = 2,        // bad parameters
    ERROR_CLIENT_NOT_FOUND = 3,
    ERROR_CLIENT_NULL_POINTER = 4,
    ERROR_SUBSCRIBE_KEYBOARD_EVENT = 5,
    ERROR_IME_NOT_STARTED = 6,
    ERROR_SERVICE_START_FAILED = 7,

    ERROR_CONTROLLER_INVOKING_FAILED = 8,
    ERROR_PERSIST_CONFIG = 9,
    ERROR_KBD_HIDE_FAILED = 10,
    ERROR_SWITCH_IME = 11,
    ERROR_PACKAGE_MANAGER = 12,
    ERROR_REMOTE_CLIENT_DIED = 13,
    ERROR_IME_START_FAILED = 14,          // failed to start IME service
    ERROR_KBD_SHOW_FAILED = 15,           // failed to show keyboard
    ERROR_CLIENT_NOT_BOUND = 16,
    ERROR_CLIENT_NOT_EDITABLE = 17,
    ERROR_CLIENT_NOT_FOCUSED = 18,
    ERROR_CLIENT_ADD_FAILED = 19,
    ERROR_OPERATE_PANEL = 20,
    ERROR_NOT_CURRENT_IME = 21,
    ERROR_NOT_IME = 22,
    ERROR_ADD_DEATH_RECIPIENT_FAILED = 23,
    ERROR_STATUS_SYSTEM_PERMISSION = 24, // not system application
    ERROR_IME = 25,
    ERROR_PARAMETER_CHECK_FAILED = 26,
    ERROR_IME_START_INPUT_FAILED = 27,
    ERROR_PARSE_CONFIG_FILE = 28,
};
}; // namespace ErrorCode

static constexpr HiviewDFX::HiLogLabel g_SMALL_SERVICES_LABEL = { LOG_CORE, 0xD001C00, "ImsaKit" };

#define IMSA_HILOGD(fmt, ...)                                                       \
    (void)OHOS::HiviewDFX::HiLog::Debug(OHOS::MiscServices::g_SMALL_SERVICES_LABEL, \
        "line: %{public}d, function: %{public}s," fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define IMSA_HILOGE(fmt, ...)                                                       \
    (void)OHOS::HiviewDFX::HiLog::Error(OHOS::MiscServices::g_SMALL_SERVICES_LABEL, \
        "line: %{public}d, function: %{public}s," fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define IMSA_HILOGF(fmt, ...)                                                       \
    (void)OHOS::HiviewDFX::HiLog::Fatal(OHOS::MiscServices::g_SMALL_SERVICES_LABEL, \
        "line: %{public}d, function: %{public}s," fmt, __LINE__FILE__, __FUNCTION__, ##__VA_ARGS__)
#define IMSA_HILOGI(fmt, ...)                                                      \
    (void)OHOS::HiviewDFX::HiLog::Info(OHOS::MiscServices::g_SMALL_SERVICES_LABEL, \
        "line: %{public}d, function: %{public}s," fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define IMSA_HILOGW(fmt, ...)                                                      \
    (void)OHOS::HiviewDFX::HiLog::Warn(OHOS::MiscServices::g_SMALL_SERVICES_LABEL, \
        "line: %{public}d, function: %{public}s," fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
using Function = std::function<bool()>;
bool BlockRetry(uint32_t interval, uint32_t maxRetryTimes, Function func);
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_GLOBAL_H
