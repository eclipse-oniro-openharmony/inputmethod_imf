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
class JsUtils {
public:
    static inline void ThrowException(napi_env env, int32_t err, const std::string &msg, const std::string &type = "")
    {
        std::string errMsg;
        if (type == "") {
            errMsg = ToMessage(err) + msg;
            IMSA_HILOGE("THROW_PARAMTER_ERROR message: %{public}s", errMsg.c_str());
        } else {
            errMsg = ToMessage(err) + "The type of " + msg + " must be " + type;
            IMSA_HILOGE("THROW_PARAMTER_TYPE_ERROR message: %{public}s", errMsg.c_str());
        }
        napi_throw_error(env, std::to_string(err).c_str(), errMsg.c_str());
    }

    static napi_value ToError(napi_env env, int32_t err);

    static int32_t Convert(int32_t code);

private:
    static const std::string ToMessage(int32_t code);

    static const std::map<int32_t, int32_t> ERROR_CODE_MAP;

    static const std::map<int32_t, std::string> ERROR_CODE_CONVERT_MESSAGE_MAP;
};
}
}
#endif // INTERFACE_KITS_JS_UTILS_H