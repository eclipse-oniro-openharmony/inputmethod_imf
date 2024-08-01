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
    FFI_EXPORT int32_t CJ_GetDefaultInputMethod(CInputMethodProperty &props);
    FFI_EXPORT int32_t CJ_GetCurrentInputMethod(CInputMethodProperty &props);
    FFI_EXPORT int32_t CJ_SwitchInputMethod(bool &result, CInputMethodProperty props);
    FFI_EXPORT int32_t CJ_SwitchCurrentInputMethodSubtype(bool &result, CInputMethodSubtype target);
    FFI_EXPORT int32_t CJ_GetCurrentInputMethodSubtype(CInputMethodSubtype &props);
    FFI_EXPORT int32_t CJ_SwitchCurrentInputMethodAndSubtype(bool &result, CInputMethodProperty target, CInputMethodSubtype subtype);
    FFI_EXPORT int32_t CJ_GetSystemInputMethodConfigAbility(CElementName &elem);
    FFI_EXPORT RetInputMethodSubtype CJ_ListInputMethodSubtype(CInputMethodProperty props);
    FFI_EXPORT RetInputMethodSubtype CJ_ListCurrentInputMethodSubtype();
    FFI_EXPORT RetInputMethodProperty CJ_GetInputMethods(bool enable);
    FFI_EXPORT RetInputMethodProperty CJ_GetAllInputMethods();
    FFI_EXPORT int32_t CJ_InputMethodSettingOn(uint32_t type, void (*func)(CInputMethodProperty, CInputMethodSubtype));
    FFI_EXPORT int32_t CJ_InputMethodSettingOff(uint32_t type);
    FFI_EXPORT int32_t CJ_ShowOptionalInputMethods(bool& result);
    FFI_EXPORT int32_t CJ_InputMethodControllerOn(char* type, int64_t id);
    FFI_EXPORT int32_t CJ_InputMethodControllerOff(char* type);

    FFI_EXPORT int32_t OHOSFfiAttach(bool showKeyboard, CTextConfig txtCfg);
    FFI_EXPORT int32_t OHOSFfiDetach();
    FFI_EXPORT int32_t OHOSFfiShowTextInput();
    FFI_EXPORT int32_t OHOSFfiHideTextInput();
    FFI_EXPORT int32_t OHOSFfiSetCallingWindow(uint32_t windowId);
    FFI_EXPORT int32_t OHOSFfiUpdateCursor(CCursorInfo cursor);
    FFI_EXPORT int32_t OHOSFfiChangeSelection(char *text, int32_t start, int32_t end);
    FFI_EXPORT int32_t OHOSFfiUpdateAttribute(CInputAttribute inputAttribute);
    FFI_EXPORT int32_t OHOSFfiShowSoftKeyboard();
    FFI_EXPORT int32_t OHOSFfiHideSoftKeyboard();
    FFI_EXPORT int32_t OHOSFfiStopInputSession();
}

#endif // INPUT_METHOD_FFI_H