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
    FFI_EXPORT int32_t FfiInputMethodGetDefaultInputMethod(CInputMethodProperty &props);
    FFI_EXPORT int32_t FfiInputMethodGetCurrentInputMethod(CInputMethodProperty &props);
    FFI_EXPORT int32_t FfiInputMethodSwitchInputMethod(bool &result, CInputMethodProperty props);
    FFI_EXPORT int32_t FfiInputMethodSwitchCurrentInputMethodSubtype(bool &result, CInputMethodSubtype target);
    FFI_EXPORT int32_t FfiInputMethodGetCurrentInputMethodSubtype(CInputMethodSubtype &props);
    FFI_EXPORT int32_t FfiInputMethodSwitchCurrentInputMethodAndSubtype(bool &result, CInputMethodProperty target,
        CInputMethodSubtype subtype);
    FFI_EXPORT int32_t FfiInputMethodGetSystemInputMethodConfigAbility(CElementName &elem);
    FFI_EXPORT RetInputMethodSubtype FfiInputMethodSettingListInputMethodSubtype(CInputMethodProperty props);
    FFI_EXPORT RetInputMethodSubtype FfiInputMethodSettingListCurrentInputMethodSubtype();
    FFI_EXPORT RetInputMethodProperty FfiInputMethodSettingGetInputMethods(bool enable);
    FFI_EXPORT RetInputMethodProperty FfiInputMethodSettingGetAllInputMethods();
    FFI_EXPORT int32_t FfiInputMethodSettingOn(uint32_t type, void (*func)(CInputMethodProperty, CInputMethodSubtype));
    FFI_EXPORT int32_t FfiInputMethodSettingOff(uint32_t type);
    FFI_EXPORT int32_t FfiInputMethodSettingShowOptionalInputMethods(bool& result);
    FFI_EXPORT int32_t FfiInputMethodControllerOn(int8_t type, int64_t id);
    FFI_EXPORT int32_t FfiInputMethodControllerOff(int8_t type);

    FFI_EXPORT int32_t FfiInputMethodControllerAttach(bool showKeyboard, CTextConfig txtCfg);
    FFI_EXPORT int32_t FfiInputMethodControllerDetach();
    FFI_EXPORT int32_t FfiInputMethodControllerShowTextInput();
    FFI_EXPORT int32_t FfiInputMethodControllerHideTextInput();
    FFI_EXPORT int32_t FfiInputMethodControllerSetCallingWindow(uint32_t windowId);
    FFI_EXPORT int32_t FfiInputMethodControllerUpdateCursor(CCursorInfo cursor);
    FFI_EXPORT int32_t FfiInputMethodControllerChangeSelection(char *text, int32_t start, int32_t end);
    FFI_EXPORT int32_t FfiInputMethodControllerUpdateAttribute(CInputAttribute inputAttribute);
    FFI_EXPORT int32_t FfiInputMethodControllerShowSoftKeyboard();
    FFI_EXPORT int32_t FfiInputMethodControllerHideSoftKeyboard();
    FFI_EXPORT int32_t FfiInputMethodControllerStopInputSession();
}

#endif // INPUT_METHOD_FFI_H