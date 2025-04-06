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

#ifndef IMF_SA_STUB_FUZZ_UTIL_H
#define IMF_SA_STUB_FUZZ_UTIL_H

#define private public
#define protected public
#include "input_method_system_ability.h"
#include "input_method_system_ability_proxy.h"
#undef private

#include <cstddef>
#include <cstdint>

namespace OHOS {
namespace MiscServices {
const std::u16string SYSTEMABILITY_INTERFACE_TOKEN = u"ohos.miscservices.inputmethod.IInputMethodSystemAbility";
constexpr const int32_t USER_ID = 100;
class ImfSaStubFuzzUtil {
public:
    static bool FuzzInputMethodSystemAbility(const uint8_t *rawData, size_t size, IInputMethodSystemAbilityIpcCode code);

private:
    static void InitKeyboardDelegate();
    static void Initialize();
    static void GrantNativePermission();
    static bool isInitialize_;
    static std::mutex initMutex_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // IMF_SA_STUB_FUZZ_UTIL_H