/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "window_display_changed_listener.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {
void WindowDisplayChangeListener::OnCallingWindowDisplayChanged(OHOS::Rosen::CallingWindowInfo callingWindowInfo)
{
    IMSA_HILOGD("callback callingWindowInfo:%{public}s", CallingWindowInfoToString(callingWindowInfo).c_str());
    if (handle_) {
        handle_(callingWindowInfo);
    }
}

std::string WindowDisplayChangeListener::CallingWindowInfoToString(const OHOS::Rosen::CallingWindowInfo& info)
{
    std::stringstream ss;
    ss << "windowId_:" << info.windowId_
    << "callingPid_:" << info.callingPid_
    << "displayId_:" << info.displayId_
    << "userId_:" << info.userId_;
    return ss.str();
}
} // namespace MiscServices
} // namespace OHOS
