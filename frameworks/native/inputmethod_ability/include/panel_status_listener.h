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

#ifndef INPUTMETHOD_IMF_PANEL_STATUS_LISTENER_H
#define INPUTMETHOD_IMF_PANEL_STATUS_LISTENER_H
#include <cstdint>

#include "panel_common.h"

namespace OHOS {
namespace MiscServices {
class PanelStatusListener {
public:
    virtual ~PanelStatusListener() {};
    virtual void OnPanelStatus(uint32_t windowId, bool isShow) = 0;
    virtual void OnSizeChange(uint32_t windowId, const WindowSize &size) = 0;
    virtual void OnSizeChange(
        uint32_t windowId, const WindowSize &size, const PanelAdjustInfo &keyboardArea, const std::string &event) = 0;
};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_PANEL_STATUS_LISTENER_H
