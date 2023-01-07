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

#include "combination_key.h"

#include <list>
#include <map>

#include "keyboard_event.h"

namespace OHOS {
namespace MiscServices {
const std::map<CombinationKeyFunction, std::list<uint8_t>> COMBINATION_KEY_MAP{
    { CombinationKeyFunction::SWITCH_LANGUAGE, { KeyboardEvent::SHIFT_LEFT_MASK | KeyboardEvent::SHIFT_RIGHT_MASK } },
    { CombinationKeyFunction::SWITCH_IME, { KeyboardEvent::SHIFT_LEFT_MASK | KeyboardEvent::SHIFT_RIGHT_MASK,
                                              KeyboardEvent::CTRL_LEFT_MASK | KeyboardEvent::CTRL_RIGHT_MASK } },
    { CombinationKeyFunction::SWITCH_MODE, { KeyboardEvent::CAPS_MASK } },
};

bool CombinationKey::IsMatch(CombinationKeyFunction combinationKey, uint32_t state, uint8_t lastPressedKey)
{
    auto keys = COMBINATION_KEY_MAP.find(combinationKey);
    bool isLastKeyMatch = false;
    for (const auto &key : keys->second) {
        if ((state & key) == 0) {
            return false;
        }
        if ((lastPressedKey & key) != 0) {
            isLastKeyMatch = true;
        }
    }
    return isLastKeyMatch;
}
} // namespace MiscServices
} // namespace OHOS