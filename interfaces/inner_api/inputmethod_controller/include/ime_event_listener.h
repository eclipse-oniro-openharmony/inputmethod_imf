/*
 * Copyright (C) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_IMF_IME_EVENT_LISTENER_H
#define INPUTMETHOD_IMF_IME_EVENT_LISTENER_H

#include "input_method_property.h"
#include "input_window_info.h"

namespace OHOS {
namespace MiscServices {
constexpr uint32_t EVENT_IME_CHANGE_MASK = 1u;
constexpr uint32_t EVENT_IME_SHOW_MASK = 1u << 1u;
constexpr uint32_t EVENT_IME_HIDE_MASK = 1u << 2u;
constexpr uint32_t EVENT_INPUT_STATUS_CHANGED_MASK = 1u << 3u; // OnInputStart and OnInputStop
class ImeEventListener {
public:
    virtual ~ImeEventListener() = default;
    virtual void OnImeChange(const Property &property, const SubProperty &subProperty){};
    virtual void OnImeShow(const ImeWindowInfo &info){};
    virtual void OnImeHide(const ImeWindowInfo &info){};
    virtual void OnInputStart(uint32_t callingWndId, int32_t requestKeyboardReason) {};
    virtual void OnInputStop() {};
};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_IME_EVENT_LISTENER_H
