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

#ifndef FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_CORE_STUB_H
#define FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_CORE_STUB_H

#include <condition_variable>
#include <cstdint>
#include <mutex>

#include "i_input_control_channel.h"
#include "i_input_data_channel.h"
#include "i_input_method_agent.h"
#include "i_input_method_core.h"
#include "input_attribute.h"
#include "input_channel.h"
#include "iremote_broker.h"
#include "iremote_stub.h"
#include "message_handler.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
class InputMethodCoreStub : public IRemoteStub<IInputMethodCore> {
public:
    DISALLOW_COPY_AND_MOVE(InputMethodCoreStub);
    InputMethodCoreStub();
    virtual ~InputMethodCoreStub();
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    int32_t StartInput(const InputClientInfo &clientInfo, bool isBindFromClient) override;
    int32_t StopInput(const sptr<IInputDataChannel> &channel) override;
    int32_t ShowKeyboard() override;
    int32_t HideKeyboard() override;
    int32_t InitInputControlChannel(const sptr<IInputControlChannel> &inputControlChannel) override;
    void StopInputService() override;
    int32_t SetSubtype(const SubProperty &property) override;
    bool IsEnable() override;
    int32_t IsPanelShown(const PanelInfo &panelInfo, bool &isShown) override;
    int32_t OnSecurityChange(int32_t security) override;
    void OnClientInactive(const sptr<IInputDataChannel> &channel) override;
    int32_t OnTextConfigChange(const TextTotalConfig &config) override;
    void SetMessageHandler(MessageHandler *msgHandler);

private:
    MessageHandler *msgHandler_;
    int32_t StartInputOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t StopInputOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t ShowKeyboardOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t HideKeyboardOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t InitInputControlChannelOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t StopInputServiceOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t SetSubtypeOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t IsEnableOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t IsPanelShownOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t SecurityChangeOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t OnClientInactiveOnRemote(MessageParcel &data, MessageParcel &reply);
    int32_t OnTextConfigChangeOnRemote(MessageParcel &data, MessageParcel &reply);
    using ParcelHandler = std::function<bool(MessageParcel &)>;
    int32_t SendMessage(int code, ParcelHandler input = nullptr);
    using RequestHandler = int32_t (InputMethodCoreStub::*)(MessageParcel &, MessageParcel &);
    static inline const std::unordered_map<int32_t, RequestHandler> HANDLERS = {
        { static_cast<uint32_t>(SHOW_KEYBOARD), &InputMethodCoreStub::ShowKeyboardOnRemote },
        { static_cast<uint32_t>(STOP_INPUT_SERVICE), &InputMethodCoreStub::StopInputServiceOnRemote },
        { static_cast<uint32_t>(HIDE_KEYBOARD), &InputMethodCoreStub::HideKeyboardOnRemote },
        { static_cast<uint32_t>(INIT_INPUT_CONTROL_CHANNEL), &InputMethodCoreStub::InitInputControlChannelOnRemote },
        { static_cast<uint32_t>(SET_SUBTYPE), &InputMethodCoreStub::SetSubtypeOnRemote },
        { static_cast<uint32_t>(START_INPUT), &InputMethodCoreStub::StartInputOnRemote },
        { static_cast<uint32_t>(STOP_INPUT), &InputMethodCoreStub::StopInputOnRemote },
        { static_cast<uint32_t>(IS_ENABLE), &InputMethodCoreStub::IsEnableOnRemote },
        { static_cast<uint32_t>(IS_PANEL_SHOWN), &InputMethodCoreStub::IsPanelShownOnRemote },
        { static_cast<uint32_t>(SECURITY_CHANGE), &InputMethodCoreStub::SecurityChangeOnRemote },
        { static_cast<uint32_t>(ON_CLIENT_INACTIVE), &InputMethodCoreStub::OnClientInactiveOnRemote },
        { static_cast<uint32_t>(ON_TEXT_CONFIG_CHANGE), &InputMethodCoreStub::OnTextConfigChangeOnRemote },
    };
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_CORE_STUB_H
