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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_I_SYSTEM_CMD_CHANNEL_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_I_SYSTEM_CMD_CHANNEL_H

#include <unordered_map>

#include "input_method_utils.h"
#include "iremote_broker.h"

/**
 * brief Definition of interface ISystemCmdChannel
 * It defines the remote calls from input method service to input client
 */
namespace OHOS {
namespace MiscServices {
class ISystemCmdChannel : public IRemoteBroker {
public:
    enum {
        SEND_PRIVATE_COMMAND = FIRST_CALL_TRANSACTION,
        SHOULD_SYSTEM_PANEL_SHOW,
        SYSTEM_CMD_LAST,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.ISystemCmdChannel");

    virtual int32_t SendPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) = 0;
    virtual int32_t ShowSysPanel(bool shouldSysPanelShow) = 0;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_I_SYSTEM_CMD_CHANNEL_H