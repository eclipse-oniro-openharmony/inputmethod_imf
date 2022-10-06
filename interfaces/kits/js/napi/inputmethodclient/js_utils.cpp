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

#include "js_utils.h"

namespace OHOS {
namespace MiscServices {
    napi_value JsUtils::GenerateError(napi_env env, int32_t err)
    {
        IMSA_HILOGE("GenerateError start");
        napi_value errorObj;
        NAPI_CALL(env, napi_create_object(env, &errorObj));
        napi_value errorCode = nullptr;
        NAPI_CALL(env, napi_create_int32(env, err, &errorCode));
        napi_value errorMessage = nullptr;
        NAPI_CALL(env, napi_create_string_utf8(env, ToString(err).c_str(), NAPI_AUTO_LENGTH, &errorMessage));
        NAPI_CALL(env, napi_set_named_property(env, errorObj, "code", errorCode));
        NAPI_CALL(env, napi_set_named_property(env, errorObj, "message", errorMessage));
        IMSA_HILOGE("GenerateError end");
        return errorObj;
    }

    int32_t JsUtils::GetIMEngineErrorCode(int32_t innerErrorCode)
    {
        IMSA_HILOGE("GetIMEngineErrorCode");
        auto iter = IMENGINE_ERROR_CODE_MAP.find(innerErrorCode);
        if (iter != IMENGINE_ERROR_CODE_MAP.end()) {
            IMSA_HILOGE("ErrorCode: %{public}d", iter->second);
            return iter->second;
        }
        IMSA_HILOGE("GetIMEngineErrorCode end");
        return ERROR_CODE_QUERY_FAILED;
    }

    const std::string JsUtils::ToString(int32_t errorCode)
    {
        switch (errorCode) {
            case EXCEPTION_PERMISSION: {
                return "the permissions check fails.";
            }
            case EXCEPTION_PARAMCHECK: {
                return "the parameters check fails.";
            }
            case EXCEPTION_UNSUPPORTED: {
                return "call unsupported api.";
            }
            case EXCEPTION_PACKAGEMANAGER: {
                return "package manager error.";
            }
            case EXCEPTION_IMENGINE: {
                return "input method engine error.";
            }
            case EXCEPTION_IMCLIENT: {
                return "input method client error.";
            }
            case EXCEPTION_KEYEVENT: {
                return "key event processing error.";
            }
            case EXCEPTION_CONFPERSIST: {
                return "configuration persisting error.";
            }
            case EXCEPTION_CONTROLLER: {
                return "input method controller error.";
            }
            case EXCEPTION_SETTINGS: {
                return "input method settings extension error.";
            }
            case EXCEPTION_IMMS: {
                return "input method manager service error.";
            }
            case EXCEPTION_OTHERS: {
                return "others error.";
            }
            default: {
                return "error is out of definition.";
            }
        }
        return "error is out of definition.";
    }
}
}