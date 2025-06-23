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

#include "cj_inputmethod_extension.h"
#include "cj_inputmethod_extension_context.h"
#include "cj_inputmethod_extension_object.h"
#include "global.h"

namespace OHOS {
namespace AbilityRuntime {

extern "C" {
CJ_EXPORT int32_t FfiInputMethodExtensionContextGetContext(InputMethodExtesionAbilityHandle extAbility, int64_t *id)
{
    auto ability = static_cast<CjInputMethodExtension *>(extAbility);
    if (ability == nullptr) {
        IMSA_HILOGE("FfiInputMethodExtensionContextGetContext failed, extAbility is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    if (id == nullptr) {
        IMSA_HILOGE("FfiInputMethodExtensionContextGetContext failed, param id is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto context = ability->GetContext();
    if (context == nullptr) {
        IMSA_HILOGE("FfiInputMethodExtensionContextGetContext failed, context is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto cjContext = OHOS::FFI::FFIData::Create<CjInputMethodExtensionContext>(context);
    if (cjContext == nullptr) {
        IMSA_HILOGE("FfiInputMethodExtensionContextGetContext failed, extAbilityContext is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }
    ability->SetCjContext(cjContext);
    *id = cjContext->GetID();
    return SUCCESS_CODE;
}

CJ_EXPORT int32_t FfiInputMethodExtensionContextDestroy(int64_t id)
{
    auto cjContext = FFI::FFIData::GetData<CjInputMethodExtensionContext>(id);
    if (cjContext == nullptr) {
        IMSA_HILOGE("invalid instance id");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return cjContext->TerminateAbility();
}

CJ_EXPORT int32_t FfiInputMethodExtensionContextStartAbility(int64_t id, WantHandle want)
{
    auto cjContext = FFI::FFIData::GetData<CjInputMethodExtensionContext>(id);
    if (cjContext == nullptr) {
        IMSA_HILOGE("invalid instance id");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto actualWant = reinterpret_cast<AAFwk::Want *>(want);
    return cjContext->StartAbility(*actualWant);
}
}
} // namespace AbilityRuntime
} // namespace OHOS
