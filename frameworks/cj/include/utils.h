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

namespace OHOS::MiscServices {
class FFI_EXPORT Utils {
public:
    static char* MallocCString(const std::string &origin);
    static void InputMethodProperty2C(CInputMethodProperty &props, const Property &property);
    static Property C2InputMethodProperty(CInputMethodProperty props);
    static void InputMethodSubProperty2C(CInputMethodSubtype &props, const SubProperty &property);
};
}
#endif