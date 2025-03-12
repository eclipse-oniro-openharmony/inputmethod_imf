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

#ifndef INPUTMETHOD_IMF_WINDOW_DISPLAY_CHANGED_MANAGER_H
#define INPUTMETHOD_IMF_WINDOW_DISPLAY_CHANGED_MANAGER_H

#include <functional>
#include <mutex>
#include <tuple>
#ifdef SCENE_BOARD_ENABLE
#include "window_display_changed_listener.h"
#endif
#include "iremote_object.h"

namespace OHOS {
namespace MiscServices {
#ifdef SCENE_BOARD_ENABLE
class WindowDisplayChangedManager final {
public:
    ~WindowDisplayChangedManager();
    static WindowDisplayChangedManager &GetInstance();
    static bool  GetCallingWindowInfo(const uint32_t windId, const int32_t userId,
        Rosen::CallingWindowInfo &callingWindowInfo);
    static void GetFoucusInfo(OHOS::Rosen::FocusChangeInfo &focusInfo);
    void RegisterCallingWindowInfoChangedListener(const WindowDisplayChangeHandler &handle);
private:
    WindowDisplayChangedManager() = default;
};
#endif // SCENE_BOARD_ENABLE
} // namespace MiscServices
} // namespace OHOS
#endif //INPUTMETHOD_IMF_WINDOW_DISPLAY_CHANGED_MANAGER_H
