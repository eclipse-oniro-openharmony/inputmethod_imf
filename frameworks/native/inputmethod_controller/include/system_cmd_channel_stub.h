/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_SYSTEM_CMD_CHANNEL_STUB_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_SYSTEM_CMD_CHANNEL_STUB_H

#include <cstdint>
#include <string>

#include "i_system_cmd_channel.h"
#include "iremote_stub.h"
#include "inputmethod_message_handler.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
namespace MiscServices {
class SystemCmdChannelStub : public IRemoteStub<ISystemCmdChannel> {
public:
    DISALLOW_COPY_AND_MOVE(SystemCmdChannelStub);
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    SystemCmdChannelStub();
    ~SystemCmdChannelStub();
    int32_t SendPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;
    int32_t NotifyPanelStatus(const SysPanelStatus &sysPanelStatus) override;

private:
    int32_t NotifyPanelStatusOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t SendPrivateCommandOnRemote(MessageParcel &data, MessageParcel &reply);
    using RequestHandler = int32_t (SystemCmdChannelStub::*)(MessageParcel &, MessageParcel &);
    static constexpr RequestHandler HANDLERS[SYSTEM_CMD_END] = {
        [SEND_PRIVATE_COMMAND] = &SystemCmdChannelStub::SendPrivateCommandOnRemote,
        [SHOULD_SYSTEM_PANEL_SHOW] = &SystemCmdChannelStub::NotifyPanelStatusOnRemote,
    };
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_SYSTEM_CMD_CHANNEL_STUB_H