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
#include "i_input_client.h"
#include "i_input_data_channel.h"
#include "i_input_method_agent.h"
#include "i_input_method_system_ability.h"
#include "input_method_property.h"
#include "input_method_setting_listener.h"
#include "input_method_status.h"
#include "input_method_utils.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "key_event.h"
#include "message_handler.h"
#include "visibility.h"

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
        virtual void HandleSetSelection(int32_t start, int32_t end) = 0;
        virtual void HandleExtendAction(int32_t action) = 0;
        virtual void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip) = 0;
    };

    class ImsaDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit ImsaDeathRecipient();
        ~ImsaDeathRecipient() = default;

        void OnRemoteDied(const wptr<IRemoteObject> &object) override;
    };

    class InputMethodController : public RefBase {
    public:
        IMF_API static sptr<InputMethodController> GetInstance();
        IMF_API void Attach(sptr<OnTextChangedListener> &listener);
        IMF_API void Attach(sptr<OnTextChangedListener> &listener, bool isShowKeyboard);
        IMF_API void Attach(sptr<OnTextChangedListener> &listener, bool isShowKeyboard, InputAttribute &attribute);
        IMF_API int32_t GetTextBeforeCursor(int32_t number, std::u16string &text);
        IMF_API int32_t GetTextAfterCursor(int32_t number, std::u16string &text);
        IMF_API void ShowTextInput();
        IMF_API void HideTextInput();
        IMF_API void Close();
        void OnRemoteSaDied(const wptr<IRemoteObject> &object);
        IMF_API void OnCursorUpdate(CursorInfo cursorInfo);
        IMF_API void OnSelectionChange(std::u16string text, int start, int end);
        IMF_API void OnConfigurationChange(Configuration info);
        IMF_API void setImeListener(std::shared_ptr<InputMethodSettingListener> imeListener);
        IMF_API bool dispatchKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent);
        IMF_API int32_t ListInputMethod(std::vector<Property> &props);
        IMF_API int32_t ListInputMethod(bool enable, std::vector<Property> &props);
        IMF_API int32_t ListInputMethodSubtype(const Property &property, std::vector<SubProperty> &subProperties);
        IMF_API int32_t ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProperties);
        IMF_API int32_t GetEnterKeyType(int32_t &keyType);
        IMF_API int32_t GetInputPattern(int32_t &inputPattern);
        IMF_API std::shared_ptr<Property> GetCurrentInputMethod();
        IMF_API std::shared_ptr<SubProperty> GetCurrentInputMethodSubtype();
        IMF_API void SetCallingWindow(uint32_t windowId);
        IMF_API int32_t SwitchInputMethod(const std::string &name, const std::string &subName = "");
        IMF_API int32_t ShowSoftKeyboard();
        IMF_API int32_t HideSoftKeyboard();
        IMF_API int32_t StopInputSession();
        IMF_API int32_t ShowOptionalInputMethod();

        // Deprecated innerkits with no permission check, kept for compatibility
        IMF_API int32_t ShowCurrentInput();
        IMF_API int32_t HideCurrentInput();
        IMF_API int32_t DisplayOptionalInputMethod();

    private:
        InputMethodController();
        ~InputMethodController();

        bool Initialize();
        sptr<IInputMethodSystemAbility> GetSystemAbilityProxy();
        void PrepareInput(int32_t displayId, sptr<IInputClient> &client, sptr<IInputDataChannel> &channel,
                          InputAttribute &attribute);
        void StartInput(sptr<IInputClient> &client, bool isShowKeyboard);
        void StopInput(sptr<IInputClient> &client);
        void ReleaseInput(sptr<IInputClient> &client);
        void SetInputMethodAgent(sptr<IRemoteObject> &object);
        void OnSwitchInput(const Property &property, const SubProperty &subProperty);
        std::shared_ptr<IInputMethodAgent> GetInputMethodAgent();
        void WorkThread();
        void QuitWorkThread();
        int32_t ListInputMethodCommon(InputMethodStatus status, std::vector<Property> &props);

        sptr<IInputDataChannel> mInputDataChannel;
        std::shared_ptr<InputMethodSettingListener> imeListener_;
        sptr<IInputClient> mClient;
        std::mutex abilityLock_;
        sptr<IInputMethodSystemAbility> abilityManager_ = nullptr;
        sptr<ImsaDeathRecipient> deathRecipient_;
        std::mutex agentLock_;
        std::shared_ptr<IInputMethodAgent> mAgent = nullptr;
        std::mutex textListenerLock_;
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
        
        bool isStopInput {true};
    };
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_CONTROLLER_H
