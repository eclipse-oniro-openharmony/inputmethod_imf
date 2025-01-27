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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_I_INPUT_CLIENT_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_I_INPUT_CLIENT_H

#include "global.h"
#include "input_method_property.h"
#include "input_window_info.h"
#include "iremote_broker.h"
/**
 * brief Definition of interface IInputClient
 * It defines the remote calls from input method management service to input client.
 */
namespace OHOS {
namespace MiscServices {
class IInputClient : public IRemoteBroker {
public:
    enum {
        ON_INPUT_READY = FIRST_CALL_TRANSACTION,
        ON_INPUT_STOP,
        ON_SWITCH_INPUT,
        ON_PANEL_STATUS_CHANGE,
        ON_NOTIFY_INPUT_START, // IME start to input, no bind
        ON_NOTIFY_INPUT_STOP,  // IME stop to input
        DEACTIVATE_CLIENT
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.InputClient");

    virtual int32_t OnInputReady(
        const sptr<IRemoteObject> &agent, const std::pair<int64_t, std::string> &imeInfo = {}) = 0;
    virtual int32_t OnInputStop(bool isStopInactiveClient, bool isAsync = false) = 0;
    virtual int32_t OnSwitchInput(const Property &property, const SubProperty &subProperty) = 0;
    virtual int32_t OnPanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info) = 0;
    virtual int32_t NotifyInputStart(uint32_t callingWndId, int32_t requestKeyboardReason) = 0;
    virtual int32_t NotifyInputStop() = 0;
    virtual void DeactivateClient() = 0;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_I_INPUT_CLIENT_H
