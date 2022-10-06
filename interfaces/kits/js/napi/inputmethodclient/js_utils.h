/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "string_ex.h"
#include "global.h"
#include <map>

namespace OHOS {
namespace MiscServices {
enum IMFErrorCode : int32_t {
    EXCEPTION_PERMISSION = 12800201,
    EXCEPTION_PARAMCHECK = 12800401,
    EXCEPTION_UNSUPPORTED = 12800801,
    EXCEPTION_OTHERS = 12899999,
    EXCEPTION_PACKAGEMANAGER = 12800001,
    EXCEPTION_IMENGINE,
    EXCEPTION_IMCLIENT,
    EXCEPTION_KEYEVENT,
    EXCEPTION_CONFPERSIST,
    EXCEPTION_CONTROLLER,
    EXCEPTION_SETTINGS,
    EXCEPTION_IMMS,
};

const int32_t ERROR_CODE_QUERY_FAILED = 1;
const std::map<int32_t, int32_t> IMENGINE_ERROR_CODE_MAP = {
    { ErrorCode::ERROR_STATUS_PERMISSION_DENIED, EXCEPTION_PERMISSION },
    { ErrorCode::ERROR_REMOTE_IME_DIED, EXCEPTION_IMENGINE },
    { ErrorCode::ERROR_RESTART_IME_FAILED, EXCEPTION_IMENGINE },
    { ErrorCode::ERROR_REMOTE_CLIENT_DIED, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_DUPLICATED, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NOT_FOUND, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_CLIENT_NULL_POINTER, EXCEPTION_IMCLIENT },
    { ErrorCode::ERROR_NOT_IME_PACKAGE, EXCEPTION_SETTINGS },
    { ErrorCode::ERROR_IME_PACKAGE_DUPLICATED, EXCEPTION_SETTINGS },
    { ErrorCode::ERROR_SETTING_SAME_VALUE, EXCEPTION_SETTINGS },
    { ErrorCode::ERROR_NULL_POINTER, EXCEPTION_IMMS }, 
    { ErrorCode::ERROR_BAD_PARAMETERS, EXCEPTION_IMMS },
    { ErrorCode::ERROR_SERVICE_START_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_USER_NOT_STARTED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_USER_ALREADY_STARTED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_USER_NOT_UNLOCKED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_USER_NOT_LOCKED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_NOT_AVAILABLE, EXCEPTION_IMMS },
    { ErrorCode::ERROR_SECURITY_IME_NOT_AVAILABLE, EXCEPTION_IMMS },
    { ErrorCode::ERROR_TOKEN_CREATE_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_TOKEN_DESTROY_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_BIND_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_UNBIND_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_START_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_STOP_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_KBD_SHOW_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_KBD_HIDE_FAILED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_NOT_STARTED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_KBD_IS_OCCUPIED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_KBD_IS_NOT_SHOWING, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_ALREADY_STARTED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_NO_NEXT_IME, EXCEPTION_IMMS },
    { ErrorCode::ERROR_CLIENTWINDOW_NOT_FOCUSED, EXCEPTION_IMMS },
    { ErrorCode::ERROR_CLIENT_NOT_WINDOW, EXCEPTION_IMMS },
    { ErrorCode::ERROR_IME_PROPERTY_MARSHALL, EXCEPTION_IMMS },
    { ErrorCode::ERROR_GETTING_CURRENT_IME, EXCEPTION_IMMS },
    { ErrorCode::ERROR_LIST_IME, EXCEPTION_IMMS },
};
class JsUtils {
public:
    static inline void ThrowError(napi_env env, int32_t err, const std::string &msg, const std::string &type = "")
    {
        if (type == "") {
            std::string errMsg = ToString(err) + msg;
            napi_throw_error(env, std::to_string(err).c_str(),  errMsg.c_str());
            IMSA_HILOGE("THROW_PARAMTER_ERROR message: %{public}s", errMsg.c_str());
        } else {
            std::string errMsg = ToString(err) + "The type of " + msg + " must be " + type;
            napi_throw_error((env), std::to_string(err).c_str(), errMsg.c_str());
            IMSA_HILOGE("THROW_PARAMTER_TYPE_ERROR message: %{public}s", errMsg.c_str());
        }
    }

    static napi_value GenerateError(napi_env env, int32_t err);

    static int32_t GetIMEngineErrorCode(int32_t innerErrorCode);

    static const std::string ToString(int32_t errorCode);
};
}
}
#endif // INTERFACE_KITS_JS_UTILS_H