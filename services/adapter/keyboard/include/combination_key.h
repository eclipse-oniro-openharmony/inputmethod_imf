/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_IMF_COMBINATION_KEY_H
#define INPUTMETHOD_IMF_COMBINATION_KEY_H

#include <cstdint>

namespace OHOS {
namespace MiscServices {
enum class CombinationKeyFunction { SWITCH_LANGUAGE = 0, SWITCH_MODE, SWITCH_IME };

class CombinationKey {
public:
    static bool IsMatch(CombinationKeyFunction combinationKey, uint32_t state);
};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_COMBINATION_KEY_H
