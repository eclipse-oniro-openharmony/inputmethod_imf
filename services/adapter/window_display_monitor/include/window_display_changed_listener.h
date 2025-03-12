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

#ifndef IMF_WINDOW_DISPLAY_CHANGED_LISTENER_H
#define IMF_WINDOW_DISPLAY_CHANGED_LISTENER_H

#include <functional>

#ifdef SCENE_BOARD_ENABLE
#include "window_manager_lite.h"
#endif

namespace OHOS {
namespace MiscServices {
#ifdef SCENE_BOARD_ENABLE
using WindowDisplayChangeHandler = std::function<void(OHOS::Rosen::CallingWindowInfo callingWindowInfo)>;
class WindowDisplayChangeListener : public OHOS::Rosen::IKeyboardCallingWindowDisplayChangedListener {
public:
    explicit WindowDisplayChangeListener(const WindowDisplayChangeHandler &handle) : handle_(handle){};
    ~WindowDisplayChangeListener() = default;
    void OnCallingWindowDisplayChanged(const OHOS::Rosen::CallingWindowInfo &callingWindowInfo) override;
    static std::string CallingWindowInfoToString(const OHOS::Rosen::CallingWindowInfo& info);
private:
    WindowDisplayChangeHandler handle_ = nullptr;
};
#endif
} // namespace MiscServices
} // namespace OHOS
#endif // IMF_WINDOW_DISPLAY_CHANGED_LISTENER_H
