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

#ifndef FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_CORE_SERVICE_IMPL_H
#define FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_CORE_SERVICE_IMPL_H

#include "system_ability.h"

#include "iinput_method_core.h"
#include "input_method_core_stub.h"
#include "iremote_object.h"
#include "inputmethod_message_handler.h"

namespace OHOS {
namespace MiscServices {
class InputMethodCoreServiceImpl final : public InputMethodCoreStub,
    public std::enable_shared_from_this<InputMethodCoreServiceImpl> {
    DISALLOW_COPY_AND_MOVE(InputMethodCoreServiceImpl);

public:
    InputMethodCoreServiceImpl();
    ~InputMethodCoreServiceImpl();
    ErrCode StartInput(const InputClientInfoInner &clientInfoInner, bool isBindFromClient) override;
    ErrCode StopInput(const sptr<IRemoteObject> &channel, uint32_t sessionId) override;
    ErrCode ShowKeyboard() override;
    ErrCode HideKeyboard() override;
    ErrCode InitInputControlChannel(const sptr<IInputControlChannel> &inputControlChannel) override;
    ErrCode StopInputService(bool isTerminateIme) override;
    ErrCode SetSubtype(const SubProperty &property) override;
    ErrCode IsEnable(bool &resultValue) override;
    ErrCode IsPanelShown(const PanelInfo &panelInfo, bool &isShown) override;
    ErrCode OnSecurityChange(int32_t security) override;
    ErrCode OnConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent) override;
    ErrCode OnClientInactive(const sptr<IRemoteObject> &channel) override;
    ErrCode OnSetInputType(int32_t inputType) override;
    ErrCode OnCallingDisplayIdChanged(uint64_t dispalyId) override;
    ErrCode OnSendPrivateData(const Value &Value) override;
};
}  // namespace MiscServices
}  // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_CORE_SERVICE_IMPL_H