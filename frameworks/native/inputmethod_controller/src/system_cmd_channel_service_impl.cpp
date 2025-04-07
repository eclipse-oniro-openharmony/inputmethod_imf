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

#include "system_cmd_channel_service_impl.h"

#include "global.h"
#include "ime_system_channel.h"
#include "ipc_object_stub.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "itypes_util.h"
#include "message.h"

namespace OHOS {
namespace MiscServices {
SystemCmdChannelServiceImpl::SystemCmdChannelServiceImpl() {}

SystemCmdChannelServiceImpl::~SystemCmdChannelServiceImpl() {}

ErrCode SystemCmdChannelServiceImpl::SendPrivateCommand(const Value &value)
{
    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    privateCommand = value.valueMap;
    return ImeSystemCmdChannel::GetInstance()->ReceivePrivateCommand(privateCommand);
}

ErrCode SystemCmdChannelServiceImpl::NotifyPanelStatus(const SysPanelStatus &sysPanelStatus)
{
    return ImeSystemCmdChannel::GetInstance()->NotifyPanelStatus(sysPanelStatus);
}
} // namespace MiscServices
} // namespace OHOS