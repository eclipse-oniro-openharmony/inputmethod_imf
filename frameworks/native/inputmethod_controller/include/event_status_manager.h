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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_LISTEN_EVENT_MANAGER_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_LISTEN_EVENT_MANAGER_H

#include "i_input_client.h"
#include "i_input_data_channel.h"
#include "input_attribute.h"
#include "input_death_recipient.h"

namespace OHOS {
namespace MiscServices {
enum EventStatus : uint32_t {
    IME_CHANGE_ON = 1,
    IME_SHOW_ON = 2,
    IME_HIDE_OFF = 3,
    IME_HIDE_ON = 4,
    IME_SHOW_OFF = 5,
    IME_CHANGE_OFF = 6,
};
class EventStatusManager {
public:
    static constexpr uint32_t NO_EVENT_ON = 0;

    inline static bool IsEventOn(EventStatus status)
    {
        return status == IME_CHANGE_ON || status == IME_SHOW_ON || status == IME_HIDE_ON;
    }
    inline static bool IsEventOff(EventStatus status)
    {
        return status == IME_CHANGE_OFF || status == IME_SHOW_OFF || status == IME_HIDE_OFF;
    }

    inline static bool IsImeShowOn(uint32_t eventFlag)
    {
        return (eventFlag & IME_SHOW_ON) == IME_SHOW_ON;
    }
    inline static bool IsImeChangeOn(uint32_t eventFlag)
    {
        return (eventFlag & IME_CHANGE_ON) == IME_CHANGE_ON;
    }
    inline static bool IsImeHideOn(uint32_t eventFlag)
    {
        return (eventFlag & IME_HIDE_ON) == IME_HIDE_ON;
    }
    inline static bool IsImeHideEvent(EventStatus status)
    {
        return status == IME_HIDE_ON || status == IME_HIDE_OFF;
    }
    inline static bool IsImeShowEvent(EventStatus status)
    {
        return status == IME_SHOW_ON || status == IME_SHOW_OFF;
    }
};
} // namespace MiscServices
} // namespace OHOS

#endif //FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_LISTEN_EVENT_MANAGER_H
