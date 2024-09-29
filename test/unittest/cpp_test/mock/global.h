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

#include <sys/time.h>

#include <cerrno>
#include <ctime>

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
constexpr int32_t NO_ERROR = 0;

// system service error
constexpr int32_t ERROR_NULL_POINTER = 1;
constexpr int32_t ERROR_PACKAGE_MANAGER = 12;
constexpr int32_t ERROR_KEYWORD_NOT_FOUND = 26;
constexpr int32_t ERROR_ENABLE_IME = 27;

constexpr int32_t ERROR_ENABLE_SECURITY_MODE = 32;
}; // namespace ErrorCode

constexpr HiviewDFX::HiLogLabel g_SMALL_SERVICES_LABEL = { LOG_CORE, 0xD001C10, "ImsaKit" };

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
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_GLOBAL_H
