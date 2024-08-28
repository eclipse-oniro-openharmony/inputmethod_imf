/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef NATIVE_INPUTMETHOD_UTILS_H
#define NATIVE_INPUTMETHOD_UTILS_H
#include <stdint.h>

#include "inputmethod_controller_capi.h"
InputMethod_ErrorCode ErrorCodeConvert(int32_t code);
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
InputMethod_ErrorCode IsValidInputMethodProxy(InputMethod_InputMethodProxy *inputMethodProxy);
void ClearInputMethodProxy(void);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // NATIVE_INPUTMETHOD_UTILS_H