/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef IMF_KEYBOARD_EVENT_H
#define IMF_KEYBOARD_EVENT_H

#include <cstdint>
#include <functional>
#include <set>

#include "global.h"
#include "key_event.h"

namespace OHOS {
namespace MiscServices {
enum CombinationKey : uint32_t {
    UNKNOWN = 0,
    CAPS,
    SHIFT,
    CTRL_SHIFT
};

using KeyHandle = std::function<int32_t(const CombinationKey &)>;

class KeyboardEvent {
public:
    static KeyboardEvent &GetInstance();
    static int32_t AddKeyEventMonitor(KeyHandle handle);

private:
    static constexpr int32_t PRESS_KEY_DELAY_MS = 200;
    KeyboardEvent() = default;
    KeyboardEvent(const KeyboardEvent &) = delete;
    KeyboardEvent(KeyboardEvent &&) = delete;
    KeyboardEvent &operator=(const KeyboardEvent &) = delete;
    KeyboardEvent &operator=(KeyboardEvent &&) = delete;
};
} // namespace MiscServices
} // namespace OHOS
#endif // IMF_KEYBOARD_EVENT_H
