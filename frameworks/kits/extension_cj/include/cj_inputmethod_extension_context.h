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

#ifndef CJ_INPUTMETHOD_EXTENSION_CONTEXT_H
#define CJ_INPUTMETHOD_EXTENSION_CONTEXT_H

#include "cj_context.h"
#include "cj_extension_context.h"
#include "inputmethod_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {

using WantHandle = void *;

class CjInputMethodExtensionContext : public CJExtensionContext {
public:
    explicit CjInputMethodExtensionContext(const std::shared_ptr<InputMethodExtensionContext> &context);

    virtual ~CjInputMethodExtensionContext() = default;

    std::shared_ptr<InputMethodExtensionContext> GetContext();

    ErrCode StartAbility(const AAFwk::Want &want);

    ErrCode TerminateAbility();

private:
    std::weak_ptr<InputMethodExtensionContext> context_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // CJ_INPUTMETHOD_EXTENSION_CONTEXT_H
