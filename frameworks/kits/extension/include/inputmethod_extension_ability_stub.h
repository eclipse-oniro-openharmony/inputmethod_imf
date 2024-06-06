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

#ifndef INPUTMETHOD_IMF_INPUTMETHOD_EXTENSION_ABILITY_STUB_H
#define INPUTMETHOD_IMF_INPUTMETHOD_EXTENSION_ABILITY_STUB_H
#include "i_inputmethod_extension_ability.h"
#include "iremote_stub.h"

namespace OHOS {
namespace MiscServices {
class InputMethodExtensionAbilityStub : public IRemoteStub<IInputMethodExtensionAbility> {};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_INPUTMETHOD_EXTENSION_ABILITY_STUB_H