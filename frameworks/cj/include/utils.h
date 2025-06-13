/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CJ_INPUT_METHOD_UTILS_H
#define CJ_INPUT_METHOD_UTILS_H

#include <cstdint>
#include <memory>
#include <string>
#include "cj_ffi/cj_common_ffi.h"
#include "input_method_ffi_structs.h"
#include "input_method_property.h"
#include "global.h"
#include <map>

namespace OHOS::MiscServices {
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
    EXCEPTION_BASIC_MODE = 12800014,
    EXCEPTION_REQUEST_NOT_ACCEPT = 12800015,
    EXCEPTION_EDITABLE = 12800016,
    EXCEPTION_INVALID_PANEL_TYPE_FLAG = 12800017,
    EXCEPTION_IME_NOT_FOUND = 12800018,
    EXCEPTION_OPERATE_DEFAULTIME = 12800019,
    EXCEPTION_INVALID_IMMERSIVE_EFFECT = 12800020,
    EXCEPTION_PRECONDITION_REQUIRED = 12800021,
};

class FFI_EXPORT Utils {
public:
    static constexpr int32_t ERR_NO_MEMORY = -2;
    static const std::map<int32_t, IMFErrorCode> ERROR_CODE_MAP;
    static char* MallocCString(const std::string &origin);
    static void InputMethodProperty2C(CInputMethodProperty *props, const Property &property);
    static Property C2InputMethodProperty(CInputMethodProperty props);
    static void InputMethodSubProperty2C(CInputMethodSubtype *props, const SubProperty &property);
    static int32_t ConvertErrorCode(int32_t code);
};
}
#endif