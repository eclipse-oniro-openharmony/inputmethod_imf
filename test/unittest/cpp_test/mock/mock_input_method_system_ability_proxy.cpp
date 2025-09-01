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
#include "mock_input_method_system_ability_proxy.h"

namespace OHOS {
namespace MiscServices {
using namespace testing::ext;
using namespace testing;
MockInputMethodSystemAbilityProxy::MockInputMethodSystemAbilityProxy() : InputMethodSystemAbilityProxy(nullptr)
{
}
void MockInputMethodSystemAbilityProxy::SetImsaProxyForTest(
    sptr<InputMethodController> controller, sptr<IInputMethodSystemAbility> proxy)
{
    if (controller == nullptr) {
        IMSA_HILOGE("MockInputMethodSystemAbilityProxy::SetImsaProxyForTest: controller is nullptr");
        return;
    }
    std::lock_guard<std::mutex> autoLock(controller->abilityLock_);
    controller->abilityManager_ = proxy;
}
}
}