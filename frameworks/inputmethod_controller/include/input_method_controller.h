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


#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_CONTROLLER_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_CONTROLLER_H

#include <mutex>
#include <thread>

#include "global.h"
#include "i_input_method_agent.h"
#include "input_method_utils.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "key_event.h"
#include "message_handler.h"
#include "i_input_client.h"
#include "input_method_property.h"
#include "i_input_data_channel.h"
#include "i_input_method_system_ability.h"

namespace OHOS {
namespace MiscServices {
    class OnTextChangedListener : public virtual RefBase {
    public:
        virtual void InsertText(const std::u16string& text) = 0;
        virtual void DeleteForward(int32_t length) = 0;
        virtual void DeleteBackward(int32_t length) = 0;
        virtual void SendKeyEventFromInputMethod(const KeyEvent& event) = 0;
        virtual void SendKeyboardInfo(const KeyboardInfo& info) = 0;
        virtual void SetKeyboardStatus(bool status) = 0;
        virtual void MoveCursor(const Direction direction) = 0;
    };

    struct Property {
        std::string packageName;
        std::string abilityName;
    };

    class ImsaDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit ImsaDeathRecipient();
        ~ImsaDeathRecipient() = default;

        void OnRemoteDied(const wptr<IRemoteObject> &object) override;
    };

    class InputMethodController : public RefBase {
    public:
        static sptr<InputMethodController> GetInstance();
        void Attach(sptr<OnTextChangedListener> &listener);
        void Attach(sptr<OnTextChangedListener> &listener, bool isShowKeyboard);
        std::u16string GetTextBeforeCursor(int32_t number);
        std::u16string GetTextAfterCursor(int32_t number);
        void ShowTextInput();
        void HideTextInput();
        void Close();
        void OnRemoteSaDied(const wptr<IRemoteObject> &object);
        void OnCursorUpdate(CursorInfo cursorInfo);
        void OnSelectionChange(std::u16string text, int start, int end);
        void OnConfigurationChange(Configuration info);
        bool dispatchKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent);
        int32_t DisplayOptionalInputMethod();
        std::vector<Property> ListInputMethodCommon(InputMethodStatus status);
        std::vector<Property> ListInputMethod();
        std::vector<Property> ListInputMethod(bool enable);
        int32_t GetEnterKeyType();
        int32_t GetInputPattern();
        std::shared_ptr<Property> GetCurrentInputMethod();
        int32_t HideCurrentInput();
        int32_t ShowCurrentInput();
        void SetCallingWindow(uint32_t windowId);
        int32_t SwitchInputMethod(const Property &target);

        int32_t ShowSoftKeyboard();
        int32_t HideSoftKeyboard();
        int32_t ShowOptionalInputMethod();

    private:
        InputMethodController();
        ~InputMethodController();

        bool Initialize();
        sptr<IInputMethodSystemAbility> GetImsaProxy();
        void PrepareInput(int32_t displayId, sptr<IInputClient> &client, sptr<IInputDataChannel> &channel,
                          InputAttribute &attribute);
        void StartInput(sptr<IInputClient> &client, bool isShowKeyboard);
        void StopInput(sptr<IInputClient> &client);
        void ReleaseInput(sptr<IInputClient> &client);
        void SetInputMethodAgent(sptr<IRemoteObject> &object);
        std::shared_ptr<IInputMethodAgent> GetInputMethodAgent();
        void WorkThread();

        sptr<IInputDataChannel> mInputDataChannel;
        sptr<IInputClient> mClient;
        sptr<IInputMethodSystemAbility> mImms;
        sptr<ImsaDeathRecipient> deathRecipient_;
        std::mutex agentLock_;
        std::shared_ptr<IInputMethodAgent> mAgent = nullptr;
        sptr<OnTextChangedListener> textListener;
        InputAttribute mAttribute;
        std::u16string mTextString;
        int mSelectOldBegin = 0;
        int mSelectOldEnd = 0;
        int mSelectNewBegin = 0;
        int mSelectNewEnd = 0;
        CursorInfo cursorInfo_;

        static std::mutex instanceLock_;
        static sptr<InputMethodController> instance_;
        std::thread workThreadHandler;
        MessageHandler *msgHandler;
        bool stop_;
        int32_t enterKeyType_ = 0;
        int32_t inputPattern_ = 0;
        
        bool isStopInput {false};
    };
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_CONTROLLER_H
