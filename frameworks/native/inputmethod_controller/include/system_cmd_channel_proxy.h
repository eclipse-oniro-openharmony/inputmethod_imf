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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_SYSTEM_CMD_CHANNEL_PROXY_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_SYSTEM_CMD_CHANNEL_PROXY_H

#include <cstdint>
#include <functional>
#include <string>

#include "i_system_cmd_channel.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
namespace MiscServices {
class SystemCmdChannelProxy : public IRemoteProxy<ISystemCmdChannel> {
public:
    explicit SystemCmdChannelProxy(const sptr<IRemoteObject> &object);
    ~SystemCmdChannelProxy() = default;
    DISALLOW_COPY_AND_MOVE(SystemCmdChannelProxy);

    int32_t SendPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;
    int32_t NotifyPanelStatus(const SysPanelStatus &sysPanelStatus) override;

private:
    static inline BrokerDelegator<SystemCmdChannelProxy> delegator_;
    using ParcelHandler = std::function<bool(MessageParcel &)>;
    int32_t SendRequest(int code, ParcelHandler input = nullptr, ParcelHandler output = nullptr);
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_SYSTEM_CMD_CHANNEL_PROXY_H