/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "input_method_ability_interface.h"

#include "global.h"
#include "input_method_ability.h"
namespace OHOS {
namespace MiscServices {
InputMethodAbilityInterface &InputMethodAbilityInterface::GetInstance()
{
    static InputMethodAbilityInterface interface;
    return interface;
}

int32_t InputMethodAbilityInterface::RegisteredProxy()
{
    return InputMethodAbility::GetInstance()->SetCoreAndAgent();
}

int32_t InputMethodAbilityInterface::UnRegisteredProxy(UnRegisteredType type)
{
    return InputMethodAbility::GetInstance()->UnRegisteredProxyIme(type);
}

int32_t InputMethodAbilityInterface::InsertText(const std::string &text)
{
    return InputMethodAbility::GetInstance()->InsertText(text);
}

int32_t InputMethodAbilityInterface::DeleteForward(int32_t length)
{
    return InputMethodAbility::GetInstance()->DeleteForward(length);
}

int32_t InputMethodAbilityInterface::DeleteBackward(int32_t length)
{
    return InputMethodAbility::GetInstance()->DeleteBackward(length);
}

int32_t InputMethodAbilityInterface::MoveCursor(int32_t keyCode)
{
    return InputMethodAbility::GetInstance()->MoveCursor(keyCode);
}

void InputMethodAbilityInterface::SetImeListener(std::shared_ptr<InputMethodEngineListener> imeListener)
{
    InputMethodAbility::GetInstance()->SetImeListener(imeListener);
}

void InputMethodAbilityInterface::SetKdListener(std::shared_ptr<KeyboardListener> kdListener)
{
    InputMethodAbility::GetInstance()->SetKdListener(kdListener);
}
} // namespace MiscServices
} // namespace OHOS
