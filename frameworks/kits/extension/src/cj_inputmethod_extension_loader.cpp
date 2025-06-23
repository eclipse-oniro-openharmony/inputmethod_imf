/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "global.h"
#include "inputmethod_extension.h"
#include <dlfcn.h>

namespace OHOS {
namespace AbilityRuntime {
namespace {
#if defined(WINDOWS_PLATFORM)
constexpr char CJ_INPUTMETHOD_EXT_LIB_NAME[] = "libcj_inputmethod_extension_ffi.dll";
#elif defined(MAC_PLATFORM)
constexpr char CJ_INPUTMETHOD_EXT_LIB_NAME[] = "libcj_inputmethod_extension_ffi.dylib";
#else
constexpr char CJ_INPUTMETHOD_EXT_LIB_NAME[] = "libcj_inputmethod_extension_ffi.z.so";
#endif

using CreateFunc = InputMethodExtension *(*)();
static constexpr char CJ_INPUTMETHOD_EXT_CREATE_FUNC[] = "OHOS_ABILITY_CjInputMethodExtension";

#ifndef CJ_EXPORT
#ifndef __WINDOWS__
#define CJ_EXPORT __attribute__((visibility("default")))
#else
#define CJ_EXPORT __declspec(dllexport)
#endif
#endif
} // namespace

InputMethodExtension *CreateCjInputMethodExtension()
{
    void *handle = dlopen(CJ_INPUTMETHOD_EXT_LIB_NAME, RTLD_LAZY);
    if (handle == nullptr) {
        IMSA_HILOGE("open cj_inputmthod_extension library %{public}s failed, reason: %{public}sn",
            CJ_INPUTMETHOD_EXT_LIB_NAME, dlerror());
        return new InputMethodExtension();
    }

    auto entry = reinterpret_cast<CreateFunc>(dlsym(handle, CJ_INPUTMETHOD_EXT_CREATE_FUNC));
    if (entry == nullptr) {
        dlclose(handle);
        IMSA_HILOGE("get cj_inputmthod_extension symbol %{public}s in %{public}s failed",
            CJ_INPUTMETHOD_EXT_CREATE_FUNC, CJ_INPUTMETHOD_EXT_LIB_NAME);
        return new InputMethodExtension();
    }

    auto instance = entry();
    if (instance == nullptr) {
        dlclose(handle);
        IMSA_HILOGE("get cj_inputmthod_extension instance in %{public}s failed", CJ_INPUTMETHOD_EXT_LIB_NAME);
        return new InputMethodExtension();
    }

    return instance;
}
} // namespace AbilityRuntime
} // namespace OHOS