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

#ifndef SERVICE_INCLUDE_INPUT_CONTROL_CHANNEL_SERVICE_IMPL_H
#define SERVICE_INCLUDE_INPUT_CONTROL_CHANNEL_SERVICE_IMPL_H

#include "system_ability.h"

#include "iinput_control_channel.h"
#include "input_control_channel_stub.h"
#include "iremote_object.h"
#include "inputmethod_message_handler.h"

namespace OHOS {
namespace MiscServices {
class InputControlChannelServiceImpl final : public SystemAbility, public InputControlChannelStub,
    public std::enable_shared_from_this<InputControlChannelServiceImpl> {
    DISALLOW_COPY_AND_MOVE(InputControlChannelServiceImpl);
    DECLARE_SYSTEM_ABILITY(InputControlChannelServiceImpl);

public:
    explicit InputControlChannelServiceImpl(int32_t userId);
    virtual ~InputControlChannelServiceImpl();
    ErrCode HideKeyboardSelf() override;

private:
    int32_t userId_;
};
}  // namespace MiscServices
}  // namespace OHOS
#endif // SERVICE_INCLUDE_INPUT_CONTROL_CHANNEL_SERVICE_IMPL_H