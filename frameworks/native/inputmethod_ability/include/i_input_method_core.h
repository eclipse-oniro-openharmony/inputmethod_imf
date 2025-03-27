/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_I_INPUT_METHOD_CORE_H
#define FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_I_INPUT_METHOD_CORE_H

#include "global.h"
#include "i_input_control_channel.h"
#include "input_attribute.h"
#include "input_client_info.h"
#include "input_method_property.h"
#include "ipc_types.h"
#include "iremote_broker.h"
#include "panel_info.h"

/**
 * brief Definition of interface IInputMethodCore
 * It defines the remote calls from input method management service to input method service
 */
namespace OHOS {
namespace MiscServices {
class IInputMethodCore : public IRemoteBroker {
public:
    enum {
        CORE_CMD_BEGIN,
        SHOW_KEYBOARD = CORE_CMD_BEGIN,
        STOP_INPUT_SERVICE,
        HIDE_KEYBOARD,
        INIT_INPUT_CONTROL_CHANNEL,
        SET_SUBTYPE,
        START_INPUT,
        STOP_INPUT,
        IS_ENABLE,
        IS_PANEL_SHOWN,
        SECURITY_CHANGE,
        ON_CLIENT_INACTIVE,
        ON_CONNECT_SYSTEM_CMD,
        ON_SET_INPUT_TYPE,
        ON_CALLING_DISPLAY_CHANGE,
        CORE_CMD_END,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.IInputMethodCore");

    virtual int32_t StartInput(const InputClientInfo &clientInfo, bool isBindFromClient) = 0;
    virtual int32_t StopInput(const sptr<IRemoteObject> &channel, uint32_t sessionId) = 0;
    virtual int32_t ShowKeyboard() = 0;
    virtual int32_t HideKeyboard() = 0;
    virtual int32_t InitInputControlChannel(const sptr<IInputControlChannel> &inputControlChannel) = 0;
    virtual int32_t StopInputService(bool isTerminateIme) = 0;
    virtual int32_t SetSubtype(const SubProperty &property) = 0;
    virtual bool IsEnable() = 0;
    virtual int32_t IsPanelShown(const PanelInfo &panelInfo, bool &isShown) = 0;
    virtual int32_t OnSecurityChange(int32_t security) = 0;
    virtual int32_t OnConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent) = 0;
    virtual void OnClientInactive(const sptr<IRemoteObject> &channel) = 0;
    virtual int32_t OnSetInputType(InputType inputType) = 0;
    virtual void OnCallingDisplayIdChange(uint64_t dispalyId) = 0;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_I_INPUT_METHOD_CORE_H
