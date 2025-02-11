/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef INTERFACE_KITS_JS_UTILS_H
#define INTERFACE_KITS_JS_UTILS_H

#include <map>

#include "ability.h"
#include "global.h"
#include "input_method_panel.h"
#include "input_method_utils.h"
#include "js_callback_object.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "string_ex.h"

using Ability = OHOS::AppExecFwk::Ability;
namespace OHOS {
namespace MiscServices {
enum IMFErrorCode : int32_t {
    EXCEPTION_PERMISSION = 201,
    EXCEPTION_SYSTEM_PERMISSION = 202,
    EXCEPTION_PARAMCHECK = 401,
    EXCEPTION_UNSUPPORTED = 801,
    EXCEPTION_PACKAGEMANAGER = 12800001,
    EXCEPTION_IMENGINE = 12800002,
    EXCEPTION_IMCLIENT = 12800003,
    EXCEPTION_IME = 12800004,
    EXCEPTION_CONFPERSIST = 12800005,
    EXCEPTION_CONTROLLER = 12800006,
    EXCEPTION_SETTINGS = 12800007,
    EXCEPTION_IMMS = 12800008,
    EXCEPTION_DETACHED = 12800009,
    EXCEPTION_DEFAULTIME = 12800010,
    EXCEPTION_TEXT_PREVIEW_NOT_SUPPORTED = 12800011,
    EXCEPTION_PANEL_NOT_FOUND = 12800012,
    EXCEPTION_WINDOW_MANAGER = 12800013,
    EXCEPTION_INVALID_PANEL_TYPE_FLAG = 12800017,
};

enum TypeCode : int32_t {
    TYPE_NONE = 0,
    TYPE_UNDEFINED,
    TYPE_NULL,
    TYPE_BOOLEAN,
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_SYMBOL,
    TYPE_OBJECT,
    TYPE_FUNCTION,
    TYPE_EXTERNAL,
    TYPE_BIGINT,
};

/* check condition, return and logging if condition not true. */
#define PARAM_CHECK_RETURN(env, condition, message, typeCode, retVal)                            \
    do {                                                                                         \
        if (!(condition)) {                                                                      \
            JsUtils::ThrowException(env, IMFErrorCode::EXCEPTION_PARAMCHECK, message, typeCode); \
            return retVal;                                                                       \
        }                                                                                        \
    } while (0)

/* check condition, return and logging. */
#define CHECK_RETURN_VOID(condition, message)                      \
    do {                                                           \
        if (!(condition)) {                                        \
            IMSA_HILOGE("test (" #condition ") failed: " message); \
            return;                                                \
        }                                                          \
    } while (0)

/* check condition, return and logging. */
#define CHECK_RETURN(condition, message, retVal)                   \
    do {                                                           \
        if (!(condition)) {                                        \
            IMSA_HILOGE("test (" #condition ") failed: " message); \
            return retVal;                                         \
        }                                                          \
    } while (0)

#define RESULT_CHECK_RETURN(env, condition, errCode, message, typeCode, retVal) \
    do {                                                                       \
        if (!(condition)) {                                                    \
            JsUtils::ThrowException(env, errCode, message, typeCode);          \
            return retVal;                                                     \
        }                                                                      \
    } while (0)

class JsUtils {
public:
    static void ThrowException(napi_env env, int32_t err, const std::string &msg, TypeCode type);

    static napi_value ToError(napi_env env, int32_t code, const std::string &msg);

    static int32_t Convert(int32_t code);

    static bool Equals(napi_env env, napi_value value, napi_ref copy, std::thread::id threadId);

    static void *GetNativeSelf(napi_env env, napi_callback_info info);

    static napi_status GetValue(napi_env env, napi_value in, int32_t &out);
    static napi_status GetValue(napi_env env, napi_value in, uint32_t &out);
    static napi_status GetValue(napi_env env, napi_value in, bool &out);
    static napi_status GetValue(napi_env env, napi_value in, double &out);
    static napi_status GetValue(napi_env env, napi_value in, std::string &out);
    static napi_status GetValue(napi_env env, napi_value in, std::unordered_map<std::string, PrivateDataValue> &out);
    static napi_status GetValue(napi_env env, napi_value in, PrivateDataValue &out);
    static napi_status GetValue(napi_env env, napi_value in, const std::string &type, napi_value &out);
    static napi_status GetValue(napi_env env, napi_value in, PanelInfo &out);
    static napi_value GetValue(napi_env env, const std::vector<InputWindowInfo> &in);
    static napi_value GetValue(napi_env env, const InputWindowInfo &in);
    static napi_value GetJsPrivateCommand(napi_env env, const std::unordered_map<std::string, PrivateDataValue> &in);
    static napi_status GetValue(napi_env env, const std::string &in, napi_value &out);

private:
    static const std::string ToMessage(int32_t code);

    static const std::map<int32_t, int32_t> ERROR_CODE_MAP;

    static const std::map<int32_t, std::string> ERROR_CODE_CONVERT_MESSAGE_MAP;

    static const std::map<int32_t, std::string> PARAMETER_TYPE;

    static constexpr int32_t ERROR_CODE_QUERY_FAILED = 1;

    static constexpr uint8_t MAX_ARGMENT_COUNT = 10;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INTERFACE_KITS_JS_UTILS_H