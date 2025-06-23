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

#include "cj_inputmethod_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {

CjInputMethodExtensionContext::CjInputMethodExtensionContext(
    const std::shared_ptr<InputMethodExtensionContext> &context)
    : CJExtensionContext(context, context->GetAbilityInfo()), context_(context)
{
}

std::shared_ptr<InputMethodExtensionContext> CjInputMethodExtensionContext::GetContext()
{
    return context_.lock();
}

ErrCode CjInputMethodExtensionContext::StartAbility(const AAFwk::Want &want)
{
    return GetContext()->StartAbility(want);
}

ErrCode CjInputMethodExtensionContext::TerminateAbility()
{
    return GetContext()->TerminateAbility();
}
} // namespace AbilityRuntime
} // namespace OHOS
