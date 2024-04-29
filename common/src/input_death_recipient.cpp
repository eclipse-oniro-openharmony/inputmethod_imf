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

#include "input_death_recipient.h"

#include "global.h"

namespace OHOS {
namespace MiscServices {
void InputDeathRecipient::SetDeathRecipient(RemoteDiedHandler handler)
{
    handler_ = std::move(handler);
}

void InputDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    IMSA_HILOGI("start");
    if (handler_ != nullptr) {
        handler_(remote);
    }
}
} // namespace MiscServices
} // namespace OHOS