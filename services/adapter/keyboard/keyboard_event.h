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

#include <key_event.h>

#include <cstdint>
#include <functional>
#include <set>

#include "global.h"

namespace OHOS {
namespace MiscServices {
struct CombineKey {
    std::set<int32_t> preKeys;
    int32_t finalKey;
};

enum CombineKeyCode : uint32_t { COMBINE_KEYCODE_CAPS = 0, COMBINE_KEYCODE_SHIFT, COMBINE_KEYCODE_CTRL_SHIFT };

using KeyHandle = std::function<void()>;

class KeyboardEvent {
public:
    static KeyboardEvent &GetInstance();
    int32_t SubscribeKeyboardEvent(const CombineKey &combine, KeyHandle handle);
    int32_t InitKeyEventMonitor();

private:
    static constexpr int32_t PRESS_KEY_DELAY_MS = 200;
    KeyboardEvent() = default;
    KeyboardEvent(const KeyboardEvent &) = delete;
    KeyboardEvent(KeyboardEvent &&) = delete;
    KeyboardEvent &operator=(const KeyboardEvent &) = delete;
    KeyboardEvent &operator=(KeyboardEvent &&) = delete;
};

struct KeyboardEventHandler {
    CombineKey combine;
    KeyHandle handle;
};

} // namespace MiscServices
} // namespace OHOS
#endif // IMF_KEYBOARD_EVENT_H
