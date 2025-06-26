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

#include "cj_inputmethod_extension_object.h"
#include "global.h"
#include "securec.h"

namespace OHOS {
namespace AbilityRuntime {

struct CJInputMethodExtensionAbilityFuncs {
    int64_t (*createCjInputMethodExtensionAbility)(const char *name, InputMethodExtesionAbilityHandle extAbility);
    void (*initCjInputMethodExtensionAbility)(int64_t id, InputMethodExtesionAbilityHandle extAbility);
    void (*releaseCjInputMethodExtensionAbility)(int64_t id);
    void (*onCreateCjInputMethodExtensionAbility)(int64_t id, WantHandle want);
    void (*onDestroyCjInputMethodExtensionAbility)(int64_t id);
};
} // namespace AbilityRuntime
} // namespace OHOS

namespace {
static OHOS::AbilityRuntime::CJInputMethodExtensionAbilityFuncs g_cjFuncs {};
static const int32_t CJ_OBJECT_ERR_CODE = -1;
} // namespace

namespace OHOS {
namespace AbilityRuntime {
void CjInputMethodExtensionObject::OnCreate(const AAFwk::Want &want)
{
    IMSA_HILOGD("OnCreate");
    if (cjId_ == 0) {
        IMSA_HILOGE("invalid instance id");
        return;
    }
    if (g_cjFuncs.onCreateCjInputMethodExtensionAbility == nullptr) {
        IMSA_HILOGE("onCreateCjInputMethodExtensionAbility is not registered");
        return;
    }
    WantHandle wantHandle = const_cast<AAFwk::Want *>(&want);
    g_cjFuncs.onCreateCjInputMethodExtensionAbility(cjId_, wantHandle);
}

void CjInputMethodExtensionObject::OnDestroy()
{
    IMSA_HILOGD("OnDestroy");
    if (cjId_ == 0) {
        IMSA_HILOGE("invalid instance id");
        return;
    }
    if (g_cjFuncs.onDestroyCjInputMethodExtensionAbility == nullptr) {
        IMSA_HILOGE("onDestroyCjInputMethodExtensionAbility is not registered");
        return;
    }
    g_cjFuncs.onDestroyCjInputMethodExtensionAbility(cjId_);
}

void CjInputMethodExtensionObject::Destroy()
{
    IMSA_HILOGD("destroy CjInputMethodExtensionObject");
    if (cjId_ == 0) {
        IMSA_HILOGE("invalid instance id");
        return;
    }
    if (g_cjFuncs.releaseCjInputMethodExtensionAbility == nullptr) {
        IMSA_HILOGE("releaseCjInputMethodExtensionAbility is not registered");
        return;
    }
    g_cjFuncs.releaseCjInputMethodExtensionAbility(cjId_);
    cjId_ = 0;
}

int32_t CjInputMethodExtensionObject::Init(const std::string &abilityName, InputMethodExtesionAbilityHandle extAbility)
{
    IMSA_HILOGD("init CjInputMethodExtensionObject");
    if (g_cjFuncs.createCjInputMethodExtensionAbility == nullptr) {
        IMSA_HILOGE("Function create is not registered.");
        return CJ_OBJECT_ERR_CODE;
    }
    cjId_ = g_cjFuncs.createCjInputMethodExtensionAbility(abilityName.c_str(), extAbility);
    if (cjId_ == 0) {
        IMSA_HILOGE("Failed to init CjInputMethodExtensionAbility: %{public}s is not registered.", abilityName.c_str());
        return CJ_OBJECT_ERR_CODE;
    }
    if (g_cjFuncs.initCjInputMethodExtensionAbility == nullptr) {
        IMSA_HILOGE("Function init is not registered.");
        return CJ_OBJECT_ERR_CODE;
    }
    g_cjFuncs.initCjInputMethodExtensionAbility(cjId_, extAbility);
    return 0;
}

extern "C" {
FFI_EXPORT void FfiInputMethodExtensionAbilityRegisterFuncs(void (*registerFunc)(CJInputMethodExtensionAbilityFuncs *))
{
    IMSA_HILOGD("start register CJInputMethodExtensionAbility function");
    if (g_cjFuncs.createCjInputMethodExtensionAbility != nullptr) {
        IMSA_HILOGE("Repeated registration for createCjInputMethodExtensionAbility");
        return;
    }

    if (registerFunc == nullptr) {
        IMSA_HILOGE("register fail");
        return;
    }

    registerFunc(&g_cjFuncs);
    IMSA_HILOGD("register success");
}
} // extern "C"
} // namespace AbilityRuntime
} // namespace OHOS
