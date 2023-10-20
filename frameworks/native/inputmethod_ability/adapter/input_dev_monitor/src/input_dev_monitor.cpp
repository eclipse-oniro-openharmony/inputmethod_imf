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

#include "input_dev_monitor.h"

#include "global.h"
#include "input_device.h"

namespace OHOS {
namespace MiscServices {
using namespace MMI;
InputDevMonitor &InputDevMonitor::GetInstance()
{
    static InputDevMonitor monitor;
    return monitor;
}

int32_t InputDevMonitor::RegisterDevListener(const FocusHandle &handler)
{
    return MMI::InputManager::GetInstance()->RegisterDevListener(
        "change", std::make_shared<InputDeviceListenerImpl>(handler));
}

void InputDevMonitor::InputDeviceListenerImpl::OnDeviceAdded(int32_t deviceId, const std::string &type)
{
    IMSA_HILOGD("deviceId: %{public}d, type: %{public}s.", deviceId, type.c_str());
    if (type != "add") {
        return;
    }
    int32_t kbType = -1;
    auto ret = MMI::InputManager::GetInstance()->GetKeyboardType(
        deviceId, [&kbType](int32_t keyboardType) { kbType = keyboardType; });
    IMSA_HILOGD("ret: %{public}d.", ret);
    if (ret == 0 && kbType == KeyboardType::KEYBOARD_TYPE_ALPHABETICKEYBOARD) {
        handler_(true);
    }
}

void InputDevMonitor::InputDeviceListenerImpl::OnDeviceRemoved(int32_t deviceId, const std::string &type)
{
    IMSA_HILOGD("deviceId: %{public}d, type: %{public}s.", deviceId, type.c_str());
    if (type != "remove") {
        return;
    }
    int32_t kbType = -1;
    auto ret = MMI::InputManager::GetInstance()->GetKeyboardType(
        deviceId, [&kbType](int32_t keyboardType) { kbType = keyboardType; });
    IMSA_HILOGD("ret: %{public}d.", ret);
    if (ret == 0 && kbType == KeyboardType::KEYBOARD_TYPE_ALPHABETICKEYBOARD) {
        handler_(false);
    }
}
} // namespace MiscServices
} // namespace OHOS