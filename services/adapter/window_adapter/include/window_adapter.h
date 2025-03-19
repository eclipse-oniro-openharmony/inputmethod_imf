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

#ifndef INPUTMETHOD_IMF_WINDOW_ADAPTER_H
#define INPUTMETHOD_IMF_WINDOW_ADAPTER_H

#include <functional>
#include <mutex>
#include <tuple>
#include "window_display_changed_listener.h"
#include "iremote_object.h"

namespace OHOS {
namespace MiscServices {
class WindowAdapter final {
public:
    ~WindowAdapter();
    static WindowAdapter &GetInstance();
    static bool  GetCallingWindowInfo(const uint32_t windId, const int32_t userId,
        Rosen::CallingWindowInfo &callingWindowInfo);
    static void GetFoucusInfo(OHOS::Rosen::FocusChangeInfo &focusInfo);
    void RegisterCallingWindowInfoChangedListener(const WindowDisplayChangeHandler &handle);
private:
    WindowAdapter() = default;
};
} // namespace MiscServices
} // namespace OHOS
#endif //INPUTMETHOD_IMF_WINDOW_ADAPTER_H
