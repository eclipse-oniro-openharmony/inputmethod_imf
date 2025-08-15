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
#ifndef MOCK_INPUT_METHOD_SYSTEM_ABILITY_PROXY_H
#define MOCK_INPUT_METHOD_SYSTEM_ABILITY_PROXY_H
#include <gmock/gmock.h>

#include "input_method_system_ability_proxy.h"
#include "mock_iremote_object.h"
namespace OHOS {
namespace MiscServices {
class MockInputMethodSystemAbilityProxy : public InputMethodSystemAbilityProxy {
public:
    MockInputMethodSystemAbilityProxy() : InputMethodSystemAbilityProxy(nullptr) { };
    MOCK_METHOD(int32_t, StartInput,
        (const InputClientInfoInner &inputClientInfoInner, std::vector<sptr<IRemoteObject>> &agents,
            std::vector<BindImeInfo> &imeInfos),
        (override));
    MOCK_METHOD(int32_t, ReleaseInput, (const sptr<IInputClient> &client, uint32_t sessionId), (override));
};
} // namespace MiscServices
} // namespace OHOS
#endif // MOCK_INPUT_METHOD_SYSTEM_ABILITY_PROXY_H