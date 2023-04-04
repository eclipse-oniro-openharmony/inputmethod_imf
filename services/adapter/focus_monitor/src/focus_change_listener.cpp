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

#include "focus_change_listener.h"

#include "global.h"

namespace OHOS {
namespace MiscServices {
void FocusChangedListener::OnFocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo)
{
}

void FocusChangedListener::OnUnfocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo)
{
    if (focusChangeInfo == nullptr || focusHandle_ == nullptr) {
        IMSA_HILOGE("error nullptr");
        return;
    }
    focusHandle_(focusChangeInfo->pid_, focusChangeInfo->pid_);
}
} // namespace MiscServices
} // namespace OHOS