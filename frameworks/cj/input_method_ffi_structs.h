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

#ifndef INTPU_METHOD_FFI_STRUCTS_H
#define INTPU_METHOD_FFI_STRUCTS_H

#include <cstdint>

#include "cj_ffi/cj_common_ffi.h"

extern "C" {
    struct CInputMethodProperty {
        char* name;
        char* id;
        char* label;
        int32_t labelId;
        char* icon;
        int32_t iconId;
    };

    struct CInputMethodSubtype {
        char* name;
        char* id;
        char* locale;
        char* language;
        char* label;
        int32_t labelId;
        char* icon;
        int32_t iconId;
        char* mode;
    };

    struct RetInputMethodSubtype {
        int32_t code;
        int64_t size;
        CInputMethodSubtype *head;
    };

    struct RetInputMethodProperty {
        int32_t code;
        int64_t size;
        CInputMethodProperty *head;
    };
}


#endif // INTPU_METHOD_FFI_STRUCTS_H