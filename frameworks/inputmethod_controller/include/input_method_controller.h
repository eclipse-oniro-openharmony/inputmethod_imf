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

namespace OHOS {
namespace MiscServices {
class OnTextChangedListener : public virtual RefBase {
public:
    virtual void InsertText(const std::u16string &text) = 0;
    virtual void DeleteForward(int32_t length) = 0;
    virtual void DeleteBackward(int32_t length) = 0;
    virtual void SendKeyEventFromInputMethod(const KeyEvent &event) = 0;
    virtual void SendKeyboardInfo(const KeyboardInfo &info) = 0;
    virtual void SetKeyboardStatus(bool status) = 0;
    virtual void MoveCursor(const Direction direction) = 0;
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
    int32_t GetTextBeforeCursor(int32_t number, std::u16string &text);
    int32_t GetTextAfterCursor(int32_t number, std::u16string &text);
    void ShowTextInput();
    void HideTextInput();
    void Close();
    void OnRemoteSaDied(const wptr<IRemoteObject> &object);
    void OnCursorUpdate(CursorInfo cursorInfo);
    void OnSelectionChange(std::u16string text, int start, int end);
    void OnConfigurationChange(Configuration info);
    void setImeListener(std::shared_ptr<InputMethodSettingListener> imeListener);
    bool dispatchKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent);
    int32_t ListInputMethod(std::vector<Property> &props);
    int32_t ListInputMethod(bool enable, std::vector<Property> &props);
    int32_t ListInputMethodSubtype(const Property &property, std::vector<SubProperty> &subProperties);
    int32_t ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProperties);
    int32_t GetEnterKeyType(int32_t &keyType);
    int32_t GetInputPattern(int32_t &inputPattern);
    std::shared_ptr<Property> GetCurrentInputMethod();
    std::shared_ptr<SubProperty> GetCurrentInputMethodSubtype();
    void SetCallingWindow(uint32_t windowId);
    int32_t SwitchInputMethod(const std::string &name, const std::string &subName = "");
    int32_t ShowSoftKeyboard();
    int32_t HideSoftKeyboard();
    int32_t StopInputSession();
    int32_t ShowOptionalInputMethod();

    // Deprecated innerkits with no permission check, kept for compatibility
    int32_t ShowCurrentInput();
    int32_t HideCurrentInput();
    int32_t DisplayOptionalInputMethod();

private:
    InputMethodController();
    ~InputMethodController();

    bool Initialize();
    sptr<IInputMethodSystemAbility> GetSystemAbilityProxy();
    void PrepareInput(
        int32_t displayId, sptr<IInputClient> &client, sptr<IInputDataChannel> &channel, InputAttribute &attribute);
    void StartInput(sptr<IInputClient> &client, bool isShowKeyboard);
    void StopInput(sptr<IInputClient> &client);
    void ReleaseInput(sptr<IInputClient> &client);
    void SetInputMethodAgent(sptr<IRemoteObject> &object);
    void OnSwitchInput(const Property &property, const SubProperty &subProperty);
    std::shared_ptr<IInputMethodAgent> GetInputMethodAgent();
    void WorkThread();
    int32_t ListInputMethodCommon(InputMethodStatus status, std::vector<Property> &props);

    sptr<IInputDataChannel> mInputDataChannel;
    std::shared_ptr<InputMethodSettingListener> imeListener_;
    sptr<IInputClient> mClient;
    std::mutex abilityLock_;
    sptr<IInputMethodSystemAbility> abilityManager_ = nullptr;
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

    bool isStopInput{ false };
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_CONTROLLER_H
