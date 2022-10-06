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

#ifndef FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_ABILITY_H
#define FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_ABILITY_H

#include <thread>
#include "iremote_object.h"
#include "i_input_control_channel.h"
#include "i_input_method_core.h"
#include "i_input_data_channel.h"
#include "i_input_method_agent.h"
#include "input_method_core_stub.h"
#include "input_control_channel_proxy.h"
#include "input_data_channel_proxy.h"
#include "input_attribute.h"
#include "message_handler.h"
#include "input_channel.h"
#include "message.h"
#include "utils.h"
#include "input_method_system_ability_proxy.h"
#include "input_method_engine_listener.h"
#include "keyboard_listener.h"

namespace OHOS {
namespace MiscServices {
    class MessageHandler;
    class InputMethodAbility : public RefBase {
    public:
        InputMethodAbility();
        ~InputMethodAbility();
        static sptr<InputMethodAbility> GetInstance();
        sptr<IInputMethodCore> OnConnect();
        bool InsertText(const std::string text);
        void setImeListener(std::shared_ptr<InputMethodEngineListener> imeListener);
        void setKdListener(std::shared_ptr<KeyboardListener> kdListener);
        void DeleteForward(int32_t length);
        void DeleteBackward(int32_t length);
        void HideKeyboardSelf();
        std::u16string GetTextBeforeCursor(int32_t number);
        std::u16string GetTextAfterCursor(int32_t number);
        void SendFunctionKey(int32_t funcKey);
        void MoveCursor(int32_t keyCode);
        bool DispatchKeyEvent(int32_t keyCode, int32_t keyStatus);
        void SetCallingWindow(uint32_t windowId);
        int32_t GetEnterKeyType();
        int32_t GetInputPattern();
        void StopInput();

    private:
        std::thread workThreadHandler;
        MessageHandler *msgHandler;
        bool mSupportPhysicalKbd = false;
        InputAttribute editorAttribute;
        int32_t displyId = 0;
        sptr<IRemoteObject> startInputToken;
        InputChannel *writeInputChannel;
        bool stop_;
        int32_t KEYBOARD_HIDE = 1;
        int32_t KEYBOARD_SHOW = 2;
        bool isBindClient = false;

        std::mutex controlChannelLock_;
        std::shared_ptr<InputControlChannelProxy> controlChannel_ = nullptr;
        void SetCoreAndAgent();

        std::mutex dataChannelLock_;
        std::shared_ptr<InputDataChannelProxy> dataChannel_ = nullptr;
        std::shared_ptr<InputMethodEngineListener> imeListener_;
        std::shared_ptr<KeyboardListener> kdListener_;
        static std::mutex instanceLock_;

        static sptr<InputMethodAbility> instance_;
        sptr<InputMethodSystemAbilityProxy> mImms;
        struct ServiceDeathRecipient : public IRemoteObject::DeathRecipient {
            std::shared_ptr<InputMethodEngineListener> listener{ nullptr };
            void OnRemoteDied(const wptr<IRemoteObject> &object) override;
        };
        sptr<ServiceDeathRecipient> deathRecipientPtr_{ nullptr };
        sptr<InputMethodSystemAbilityProxy> GetImsaProxy();

        void SetInputDataChannel(sptr<IRemoteObject> &object);
        std::shared_ptr<InputDataChannelProxy> GetInputDataChannel();
        void SetInputControlChannel(sptr<IRemoteObject> &object);
        std::shared_ptr<InputControlChannelProxy> GetInputControlChannel();

        void Initialize();
        void WorkThread();

        void OnInitialInput(Message *msg);
        void OnStartInput(Message *msg);
        void OnStopInput(Message *msg);
        void OnShowKeyboard(Message *msg);
        void OnHideKeyboard(Message *msg);
        void OnInitInputControlChannel(Message *msg);

        void OnCursorUpdate(Message *msg);
        void OnSelectionChange(Message *msg);

        void InitialInputWindow();
        void ShowInputWindow(bool isShowKeyboard);
        void DissmissInputWindow();

        void BindServiceAndClient();
    };
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_ABILITY_H
