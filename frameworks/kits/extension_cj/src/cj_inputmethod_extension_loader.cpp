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
constexpr char CJ_INPUTMETHOD_EXT_LIB_NAME[] = "libcj_inputmethod_extension_ffi.z.so";

using CreateFunc = InputMethodExtension *(*)();
static constexpr char CJ_INPUTMETHOD_EXT_CREATE_FUNC[] = "OHOS_ABILITY_CjInputMethodExtension";

} // namespace

InputMethodExtension *CreateCjInputMethodExtension()
{
    void *handle = dlopen(CJ_INPUTMETHOD_EXT_LIB_NAME, RTLD_LAZY);
    if (handle == nullptr) {
        IMSA_HILOGE("open cj_inputmethod_extension library %{public}s failed, reason: %{public}s",
            CJ_INPUTMETHOD_EXT_LIB_NAME, dlerror());
        return new (std::nothrow) InputMethodExtension();
    }

    auto entry = reinterpret_cast<CreateFunc>(dlsym(handle, CJ_INPUTMETHOD_EXT_CREATE_FUNC));
    if (entry == nullptr) {
        dlclose(handle);
        IMSA_HILOGE("get cj_inputmethod_extension symbol %{public}s in %{public}s failed",
            CJ_INPUTMETHOD_EXT_CREATE_FUNC, CJ_INPUTMETHOD_EXT_LIB_NAME);
        return new (std::nothrow) InputMethodExtension();
    }

    auto instance = entry();
    if (instance == nullptr) {
        dlclose(handle);
        IMSA_HILOGE("get cj_inputmethod_extension instance in %{public}s failed", CJ_INPUTMETHOD_EXT_LIB_NAME);
        return new (std::nothrow) InputMethodExtension();
    }

    return instance;
}
} // namespace AbilityRuntime
} // namespace OHOS