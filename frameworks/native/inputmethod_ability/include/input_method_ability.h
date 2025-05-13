/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

#include "calling_window_info.h"
#include "concurrent_map.h"
#include "context.h"
#include "iinput_control_channel.h"
#include "iinput_data_channel.h"
#include "iinput_method_agent.h"
#include "iinput_method_core.h"
#include "iinput_method_system_ability.h"
#include "input_attribute.h"
#include "input_control_channel_proxy.h"
#include "input_data_channel_proxy.h"
#include "input_method_engine_listener.h"
#include "input_method_panel.h"
#include "input_method_types.h"
#include "input_method_utils.h"
#include "iremote_object.h"
#include "keyboard_listener.h"
#include "key_event_consumer_proxy.h"
#include "text_input_client_listener.h"
#include "msg_handler_callback_interface.h"
#include "private_command_interface.h"
#include "system_cmd_channel_proxy.h"
#include "inputmethod_message_handler.h"

namespace OHOS {
namespace MiscServices {
class InputMethodAbility : public RefBase, public PrivateCommandInterface {
public:
    static InputMethodAbility &GetInstance();
    int32_t SetCoreAndAgent();
    int32_t InitConnect();
    int32_t UnRegisteredProxyIme(UnRegisteredType type);
    int32_t RegisterProxyIme(uint64_t displayId = DEFAULT_DISPLAY_ID);
    int32_t UnregisterProxyIme(uint64_t displayId);
    int32_t InsertText(const std::string text);
    void SetImeListener(std::shared_ptr<InputMethodEngineListener> imeListener);
    std::shared_ptr<InputMethodEngineListener> GetImeListener();
    void SetKdListener(std::shared_ptr<KeyboardListener> kdListener);
    void SetTextInputClientListener(std::shared_ptr<TextInputClientListener> textInputClientListener);
    int32_t DeleteForward(int32_t length);
    int32_t DeleteBackward(int32_t length);
    int32_t HideKeyboardSelf();

    int32_t SendExtendAction(int32_t action);
    int32_t GetTextBeforeCursor(int32_t number, std::u16string &text);
    int32_t GetTextAfterCursor(int32_t number, std::u16string &text);
    int32_t SendFunctionKey(int32_t funcKey);
    int32_t MoveCursor(int32_t keyCode);
    int32_t SelectByRange(int32_t start, int32_t end);
    int32_t SelectByMovement(int32_t direction);
    int32_t DispatchKeyEvent(const std::shared_ptr<MMI::KeyEvent> &keyEvent, sptr<KeyEventConsumerProxy> &consumer);
    void SetCallingWindow(uint32_t windowId);
    int32_t GetEnterKeyType(int32_t &keyType);
    int32_t GetInputPattern(int32_t &inputPattern);
    int32_t GetTextIndexAtCursor(int32_t &index);
    int32_t GetTextConfig(TextTotalConfig &textConfig);
    int32_t AdjustKeyboard();
    int32_t CreatePanel(const std::shared_ptr<AbilityRuntime::Context> &context, const PanelInfo &panelInfo,
        std::shared_ptr<InputMethodPanel> &inputMethodPanel);
    int32_t DestroyPanel(const std::shared_ptr<InputMethodPanel> &inputMethodPanel);
    int32_t ShowPanel(const std::shared_ptr<InputMethodPanel> &inputMethodPanel);
    int32_t HidePanel(const std::shared_ptr<InputMethodPanel> &inputMethodPanel);
    bool IsCurrentIme();
    bool IsEnable();
    bool IsSystemApp();
    InputType GetInputType();
    int32_t ExitCurrentInputType();
    int32_t IsPanelShown(const PanelInfo &panelInfo, bool &isShown);
    int32_t GetSecurityMode(int32_t &security);
    int32_t OnSecurityChange(int32_t security);
    int32_t OnConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent);
    void OnClientInactive(const sptr<IRemoteObject> &channel);
    void NotifyKeyboardHeight(uint32_t panelHeight, PanelFlag panelFlag);
    int32_t SendPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;
    int32_t ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;
    bool IsDefaultIme();
    int32_t GetCallingWindowInfo(CallingWindowInfo &windowInfo);
    int32_t SetPreviewText(const std::string &text, const Range &range);
    int32_t FinishTextPreview(bool isAsync);
    int32_t NotifyPanelStatus(PanelType panelType, SysPanelStatus &sysPanelStatus);
    InputAttribute GetInputAttribute();
    RequestKeyboardReason GetRequestKeyboardReason();
    void OnSetInputType(InputType inputType);
    int32_t SendMessage(const ArrayBuffer &arrayBuffer);
    int32_t RecvMessage(const ArrayBuffer &arrayBuffer);
    int32_t RegisterMsgHandler(const std::shared_ptr<MsgHandlerCallbackInterface> &msgHandler = nullptr);
    int32_t OnCallingDisplayIdChanged(uint64_t displayId);
    int32_t OnSendPrivateData(const std::unordered_map<std::string, PrivateDataValue> &privateCommand);

public:
    /* called from TaskManager worker thread */
    int32_t StartInput(const InputClientInfo &clientInfo, bool isBindFromClient);
    int32_t StopInput(sptr<IRemoteObject> channelObj, uint32_t sessionId);
    int32_t ShowKeyboard(int32_t requestKeyboardReason);
    int32_t HideKeyboard();

    void OnInitInputControlChannel(sptr<IRemoteObject> channelObj);
    void OnSetSubtype(SubProperty subProperty);
    void OnCursorUpdate(int32_t positionX, int32_t positionY, int32_t height);
    void OnSelectionChange(std::u16string text, int32_t oldBegin, int32_t oldEnd, int32_t newBegin, int32_t newEndg);
    void OnAttributeChange(InputAttribute attribute);

    int32_t OnStopInputService(bool isTerminateIme);
    uint64_t GetCallingWindowDisplayId();
private:
    std::mutex controlChannelLock_;
    std::shared_ptr<InputControlChannelProxy> controlChannel_ = nullptr;

    std::mutex dataChannelLock_;
    sptr<IRemoteObject> dataChannelObject_ = nullptr;
    std::shared_ptr<InputDataChannelProxy> dataChannelProxy_ = nullptr;

    std::mutex systemCmdChannelLock_;
    sptr<SystemCmdChannelProxy> systemCmdChannelProxy_ = nullptr;

    std::shared_ptr<InputMethodEngineListener> imeListener_;
    std::shared_ptr<KeyboardListener> kdListener_;
    std::shared_ptr<TextInputClientListener> textInputClientListener_;

    std::mutex abilityLock_;
    sptr<IInputMethodSystemAbility> abilityManager_ { nullptr };
    sptr<InputDeathRecipient> deathRecipient_ { nullptr };
    InputMethodAbility();
    ~InputMethodAbility();
    InputMethodAbility(const InputMethodAbility &) = delete;
    InputMethodAbility &operator=(const InputMethodAbility &) = delete;
    sptr<IInputMethodSystemAbility> GetImsaProxy();
    void OnRemoteSaDied(const wptr<IRemoteObject> &object);

    sptr<SystemCmdChannelProxy> GetSystemCmdChannelProxy();
    void ClearSystemCmdChannel();

    void SetInputDataChannel(const sptr<IRemoteObject> &object);
    std::shared_ptr<InputDataChannelProxy> GetInputDataChannelProxy();
    void ClearDataChannel(const sptr<IRemoteObject> &channel);
    void SetInputControlChannel(sptr<IRemoteObject> &object);
    void ClearInputControlChannel();
    std::shared_ptr<InputControlChannelProxy> GetInputControlChannel();

    void Initialize();
    int32_t InvokeStartInputCallback(bool isNotifyInputStart);
    int32_t InvokeStartInputCallback(const TextTotalConfig &textConfig, bool isNotifyInputStart);
    bool IsInputClientAttachOptionsChanged(RequestKeyboardReason requestKeyboardReason);
    int32_t HideKeyboard(Trigger trigger, uint32_t sessionId);
    std::shared_ptr<InputMethodPanel> GetSoftKeyboardPanel();
    /* param flag: ShowPanel is async, show/hide softkeyboard in alphabet keyboard attached,
       flag will be changed before finishing show/hide */
    int32_t ShowPanel(const std::shared_ptr<InputMethodPanel> &inputMethodPanel, PanelFlag flag, Trigger trigger);
    int32_t HidePanel(
        const std::shared_ptr<InputMethodPanel> &inputMethodPanel, PanelFlag flag, Trigger trigger, uint32_t sessionId);
    void SetInputAttribute(const InputAttribute &inputAttribute);
    void ClearInputAttribute();
    void SetRequestKeyboardReason(RequestKeyboardReason requestKeyboardReason);
    void ClearRequestKeyboardReason();
    void NotifyPanelStatusInfo(const PanelStatusInfo &info);
    int32_t HideKeyboardImplWithoutLock(int32_t cmdId, uint32_t sessionId);
    int32_t ShowKeyboardImplWithLock(int32_t cmdId);
    int32_t ShowKeyboardImplWithoutLock(int32_t cmdId);
    void NotifyPanelStatusInfo(const PanelStatusInfo &info, std::shared_ptr<InputDataChannelProxy> &channelProxy);
    void ClearInputType();
    std::shared_ptr<MsgHandlerCallbackInterface> GetMsgHandlerCallback();
    int32_t StartInputInner(const InputClientInfo &clientInfo, bool isBindFromClient);
    int32_t InsertTextInner(const std::string &text);
    int32_t SetPreviewTextInner(const std::string &text, const Range &range);
    int32_t DeleteForwardInner(int32_t length);
    int32_t DeleteBackwardInner(int32_t length);
    int32_t FinishTextPreviewInner(bool isAsync);
    int32_t GetTextBeforeCursorInner(int32_t number, std::u16string &text);
    int32_t GetTextAfterCursorInner(int32_t number, std::u16string &text);
    int32_t GetTextIndexAtCursorInner(int32_t &index);
    void SetBindClientInfo(const InputClientInfo &clientInfo);
    HiSysEventClientInfo GetBindClientInfo();
    void ReportImeStartInput(int32_t eventCode, int32_t errCode, bool isShowKeyboard, int64_t consumeTime = -1);
    void ReportBaseTextOperation(int32_t eventCode, int32_t errCode, int64_t consumeTime);
    ConcurrentMap<PanelType, std::shared_ptr<InputMethodPanel>> panels_ {};
    std::atomic_bool isBound_ { false };
    std::atomic_bool isProxyIme_{ false };

    sptr<IInputMethodCore> coreStub_ { nullptr };
    sptr<IInputMethodAgent> agentStub_ { nullptr };
    sptr<IInputMethodAgent> systemAgentStub_ { nullptr };
    std::mutex imeCheckMutex_;
    bool isCurrentIme_ = false;

    double positionY_ = 0;
    double height_ = 0;

    std::mutex defaultImeCheckMutex_;
    bool isDefaultIme_ = false;
    std::mutex inputAttrLock_;
    InputAttribute inputAttribute_ {};
    std::mutex requestKeyboardReasonLock_;
    RequestKeyboardReason requestKeyboardReason_ = RequestKeyboardReason::NONE;
    std::recursive_mutex keyboardCmdLock_;
    int32_t cmdId_ = 0;

    std::mutex inputTypeLock_;
    InputType inputType_ = InputType::NONE;
    std::atomic<bool> isImeTerminating = false;
    std::atomic_bool isShowAfterCreate_ { false };
    std::atomic<int32_t> securityMode_ = -1;
    std::mutex msgHandlerMutex_;
    std::shared_ptr<MsgHandlerCallbackInterface> msgHandler_;

    std::mutex systemAppCheckMutex_;
    bool isSystemApp_ = false;
    
    std::mutex bindClientInfoLock_;
    HiSysEventClientInfo bindClientInfo_;

    bool IsDisplayChanged(uint64_t oldDisplayId, uint64_t newDisplayId);
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_ABILITY_INCLUDE_INPUT_METHOD_ABILITY_H
