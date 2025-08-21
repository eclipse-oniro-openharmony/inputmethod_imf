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

#include "display_adapter.h"

#include <cinttypes>

#include "display_info.h"
#include "display_manager_lite.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::Rosen;
std::string DisplayAdapter::GetDisplayName(uint64_t displayId)
{
    sptr<DisplayLite> display = DisplayManagerLite::GetInstance().GetDisplayById(displayId);
    if (display == nullptr) {
        IMSA_HILOGE("display is null!");
        return "";
    }
    sptr<DisplayInfo> displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        IMSA_HILOGE("displayInfo is null!");
        return "";
    }
    return displayInfo->GetName();
}

uint64_t DisplayAdapter::GetDefaultDisplayId()
{
    return DisplayManagerLite::GetInstance().GetDefaultDisplayId();
}
} // namespace MiscServices
} // namespace OHOS