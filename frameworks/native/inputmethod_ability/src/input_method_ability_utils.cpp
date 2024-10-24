/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "input_method_ability_utils.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
sptr<IInputMethodSystemAbility> ImaUtils::abilityManager_{ nullptr }; // for tdd test
sptr<IInputMethodSystemAbility> ImaUtils::GetImsaProxy()
{
    IMSA_HILOGD("ImaUtils::GetImsaProxy start.");
    // for tdd test begin
    if (abilityManager_ != nullptr) {
        return abilityManager_;
    }
    // for tdd test end
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGE("ImaUtils systemAbilityManager is nullptr");
        return nullptr;
    }

    auto systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
    if (systemAbility == nullptr) {
        IMSA_HILOGE("ImaUtils systemAbility is nullptr");
        return nullptr;
    }
    return iface_cast<IInputMethodSystemAbility>(systemAbility);
}
} // namespace MiscServices
} // namespace OHOS
