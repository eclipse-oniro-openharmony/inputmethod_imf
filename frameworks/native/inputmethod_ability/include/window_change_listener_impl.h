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

#ifndef INPUTMETHOD_IMF_WINDOW_CHANGE_LISTENER_H
#define INPUTMETHOD_IMF_WINDOW_CHANGE_LISTENER_H
#include "panel_status_listener.h"
#include "window.h"
#include <functional>

namespace OHOS {
namespace MiscServices {
using ChangeHandler = std::function<void(WindowSize size)>;
class WindowChangeListenerImpl : public OHOS::Rosen::IWindowChangeListener {
public:
    explicit WindowChangeListenerImpl(ChangeHandler handler) : changeHandler_(std::move(handler)){};
    virtual ~WindowChangeListenerImpl(){};
    void OnSizeChange(OHOS::Rosen::Rect rect, OHOS::Rosen::WindowSizeChangeReason reason,
        const std::shared_ptr<OHOS::Rosen::RSTransaction>& rsTransaction = nullptr) override;

private:
    ChangeHandler changeHandler_ = nullptr;
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUTMETHOD_IMF_WINDOW_CHANGE_LISTENER_H
