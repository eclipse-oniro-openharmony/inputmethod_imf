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

#ifndef INPUT_METHOD_FFI_H
#define INPUT_METHOD_FFI_H

#include "input_method_ffi_structs.h"

extern "C" {
    FFI_EXPORT int32_t CJ_GetDefaultInputMethod(CInputMethodProperty* props);
    FFI_EXPORT int32_t CJ_GetCurrentInputMethod(CInputMethodProperty* props);
    FFI_EXPORT int32_t CJ_SwitchInputMethod(bool &result, CInputMethodProperty props);
    FFI_EXPORT int32_t CJ_SwitchCurrentInputMethodSubtype(bool &result, CInputMethodSubtype target);
    FFI_EXPORT int32_t CJ_GetCurrentInputMethodSubtype(CInputMethodSubtype* props);
    FFI_EXPORT int32_t CJ_SwitchCurrentInputMethodAndSubtype(bool &result, CInputMethodProperty target, CInputMethodSubtype subtype);
    FFI_EXPORT RetInputMethodSubtype CJ_ListInputMethodSubtype(CInputMethodProperty props);
    FFI_EXPORT RetInputMethodSubtype CJ_ListCurrentInputMethodSubtype();
    FFI_EXPORT RetInputMethodProperty CJ_GetInputMethods(bool enable);
    FFI_EXPORT RetInputMethodProperty CJ_GetAllInputMethods();
    FFI_EXPORT int32_t CJ_InputMethodSettingOn(uint32_t type, void (*func)(CInputMethodProperty, CInputMethodSubtype));
    FFI_EXPORT int32_t CJ_InputMethodSettingOff(uint32_t type);
    FFI_EXPORT int32_t CJ_ShowOptionalInputMethods(bool& result);
}

#endif // INPUT_METHOD_FFI_H