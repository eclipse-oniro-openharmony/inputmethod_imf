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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_CONTROLLER_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_CONTROLLER_H

#include <atomic>
#include <chrono>
#include <ctime>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <variant>

#include "block_queue.h"
#include "controller_listener.h"
#include "element_name.h"
#include "event_handler.h"
#include "global.h"
#include "i_input_method_agent.h"
#include "i_input_method_system_ability.h"
#include "ime_event_listener.h"
#include "input_client_info.h"
#include "input_method_property.h"
#include "input_method_status.h"
#include "input_method_utils.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "key_event.h"
#include "message_handler.h"
#include "panel_info.h"
#include "private_command_interface.h"
#include "visibility.h"

namespace OHOS {
namespace MiscServices {
class OnTextChangedListener : public virtual RefBase {
public:
    virtual ~OnTextChangedListener() {}
    virtual void InsertText(const std::u16string &text) = 0;
    virtual void DeleteForward(int32_t length) = 0;
    virtual void DeleteBackward(int32_t length) = 0;
    virtual void SendKeyEventFromInputMethod(const KeyEvent &event) = 0;
    virtual void SendKeyboardStatus(const KeyboardStatus &keyboardStatus) = 0;
    virtual void NotifyPanelStatusInfo(const PanelStatusInfo &info)
    {
    }
    virtual void NotifyKeyboardHeight(uint32_t height)
    {
    }
    virtual void SendFunctionKey(const FunctionKey &functionKey) = 0;
    virtual void SetKeyboardStatus(bool status) = 0;
    virtual void MoveCursor(const Direction direction) = 0;
    virtual void HandleSetSelection(int32_t start, int32_t end) = 0;
    virtual void HandleExtendAction(int32_t action) = 0;
    virtual void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip) = 0;
    virtual std::u16string GetLeftTextOfCursor(int32_t number) = 0;
    virtual std::u16string GetRightTextOfCursor(int32_t number) = 0;
    virtual int32_t GetTextIndexAtCursor() = 0;
    virtual int32_t ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
    {
        return ErrorCode::NO_ERROR;
    }
    /**
     * @brief Set preview text.
     *
     * When InputMethod app sends request to set preview text, the function will be called.
     *
     * @param text Indicates the text to be previewed.
     * @param range Indicates the range of text to be replaced.
     * @return
     *     If success, please return 0.
     *     If parameter range check error, please return -1.
     *     If other failure, no specific requirement.
     * @since 12
     */
    virtual int32_t SetPreviewText(const std::u16string &text, const Range &range)
    {
        return ErrorCode::NO_ERROR;
    }
    /**
     * @brief Finish text preview.
     *
     * When InputMethod app sends request to finish text preview, the function will be called.
     *
     * @since 12
     */
    virtual void FinishTextPreview()
    {
    }
};
using PrivateDataValue = std::variant<std::string, bool, int32_t>;
using KeyEventCallback = std::function<void(std::shared_ptr<MMI::KeyEvent> &keyEvent, bool isConsumed)>;
class InputMethodController : public RefBase, public PrivateCommandInterface {
public:
    /**
     * @brief Get the instance of InputMethodController.
     *
     * This function is used to get the instance of InputMethodController.
     *
     * @return The instance of InputMethodController.
     * @since 6
     */
    IMF_API static sptr<InputMethodController> GetInstance();

    /**
     * @brief Show soft keyboard, set listener and bind IMSA with default states and attribute.
     *
     * This function is used to show soft keyboard,  set listener and bind IMSA,
     * default state is 'true', default attribute is 'InputAttribute::PATTERN_TEXT'.
     *
     * @param listener Indicates the listener in order to manipulate text.
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t Attach(sptr<OnTextChangedListener> listener);

    /**
     * @brief Set listener and bind IMSA with given states and default attribute.
     *
     * This function is used to set listener and bind IMSA,
     * default attribute is 'InputAttribute::PATTERN_TEXT'. Show soft keyboard when state is true.
     *
     * @param listener          Indicates the listener in order to manipulate text.
     * @param isShowKeyboard    Indicates the state, if you want to show soft keyboard, please pass in true.
     * @return Returns 0 for success, others for failure.
     * @since 8
     */
    IMF_API int32_t Attach(sptr<OnTextChangedListener> listener, bool isShowKeyboard);

    /**
     * @brief Set listener and bind IMSA with given states and attribute.
     *
     * This function is used to set listener and bind IMSA.
     * Show soft keyboard when state is true, and customized attribute.
     *
     * @param listener          Indicates the listener in order to manipulate text.
     * @param isShowKeyboard    Indicates the state, if you want to show soft keyboard, please pass in true.
     * @param attribute         Indicates the attribute, such as input pattern, enter eyType, input option.
     * @return Returns 0 for success, others for failure.
     * @since 8
     */
    IMF_API int32_t Attach(sptr<OnTextChangedListener> listener, bool isShowKeyboard, const InputAttribute &attribute);

    /**
     * @brief Set listener and bind IMSA with given states and textConfig.
     *
     * This function is used to set listener and bind IMSA.
     * Show soft keyboard when state is true, and customized attribute.
     *
     * @param listener          Indicates the listener in order to manipulate text.
     * @param isShowKeyboard    Indicates the state, if you want to show soft keyboard, please pass in true.
     * @param textConfig        Indicates the textConfig, such as input attribute, cursorInfo, range of text selection,
     *                          windowId.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    IMF_API int32_t Attach(sptr<OnTextChangedListener> listener, bool isShowKeyboard, const TextConfig &textConfig);

    /**
     * @brief Show soft keyboard.
     *
     * This function is used to show soft keyboard of current client.
     *
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t ShowTextInput();

    /**
     * @brief Hide soft keyboard.
     *
     * This function is used to hide soft keyboard of current client, and keep binding.
     *
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t HideTextInput();

    /**
     * @brief Hide current input method, clear text listener and unbind IMSA.
     *
     * This function is used to stop input, whick will set listener to nullptr,
     * hide current soft keyboard and unbind IMSA.
     *
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t Close();

    /**
     * @brief A callback function when the cursor changes.
     *
     * This function is the callback when the cursor changes.
     *
     * @param cursorInfo Indicates the information of current cursor changes.
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t OnCursorUpdate(CursorInfo cursorInfo);

    /**
     * @brief A callback function when the cursor changes.
     *
     * This function is the callback when the cursor changes.
     *
     * @param text  Indicates the currently selected text.
     * @param start Indicates the coordinates of the current start.
     * @param end   Indicates the coordinates of the current end.
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t OnSelectionChange(std::u16string text, int start, int end);

    /**
     * @brief Changing the configuration of soft keyboard.
     *
     * This function is used to change the configuration of soft keyboard.
     *
     * @param info Indicates the current configuration.
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t OnConfigurationChange(Configuration info);
    IMF_API void SetControllerListener(std::shared_ptr<ControllerListener> controllerListener);

    /**
     * @brief Dispatch keyboard event.
     *
     * This function is used to Dispatch events of keyboard.
     *
     * @param keyEvent Indicates the events keyboard.
     * @return Returns true for success otherwise for failure.
     * @since 6
     */
    /**
     * @brief Dispatch keyboard event.
     *
     * This function is used to Dispatch events of keyboard.
     *
     * @param keyEvent Indicates the events keyboard.
     * @param callback Indicates the consumption result of key event.
     * @return Returns 0 for success, others for failure.
     * @since 11
     */
    IMF_API int32_t DispatchKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent, KeyEventCallback callback);

    /**
     * @brief List input methods.
     *
     * This function is used to list all of input methods.
     *
     * @param props Indicates the input methods that will be listed.
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t ListInputMethod(std::vector<Property> &props);

    /**
     * @brief List input methods.
     *
     * This function is used to list enabled or disabled input methods.
     *
     * @param props     Indicates the input methods that will be listed.
     * @param enable    Indicates the state of input method.
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t ListInputMethod(bool enable, std::vector<Property> &props);

    /**
     * @brief List input method subtypes.
     *
     * This function is used to list specified input method subtypes.
     *
     * @param property      Indicates the specified input method property.
     * @param subProperties Indicates the subtypes of specified input method that will be listed.
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t ListInputMethodSubtype(const Property &property, std::vector<SubProperty> &subProperties);

    /**
     * @brief List current input method subtypes.
     *
     * This function is used to list current input method subtypes.
     *
     * @param subProperties Indicates the subtypes of current input method that will be listed.
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProperties);

    /**
     * @brief Get enter key type.
     *
     * This function is used to get enter key type of current client.
     *
     * @param keyType Indicates the enter key type of current client that will be obtained, such as SEND, SEARCH...
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    int32_t GetEnterKeyType(int32_t &keyType);

    /**
     * @brief Get input pattern.
     *
     * This function is used to get text input type of current client.
     *
     * @param inputPattern Indicates the text input type of current client that will be obtained, such as TEXT, URL...
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    int32_t GetInputPattern(int32_t &inputPattern);

    /**
     * @brief Get text config.
     *
     * This function is used to get text config of current client.
     *
     * @param textConfig Indicates the text config of current client that will be obtained.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    int32_t GetTextConfig(TextTotalConfig &config);

    /**
     * @brief Get current input method property.
     *
     * This function is used to get current input method property.
     *
     * @return The property of current input method.
     * @since 6
     */
    IMF_API std::shared_ptr<Property> GetCurrentInputMethod();

    /**
     * @brief Get current input method subtypes.
     *
     * This function is used to get current input method's current subtype.
     *
     * @return The subtypes of current input method.
     * @since 6
     */
    IMF_API std::shared_ptr<SubProperty> GetCurrentInputMethodSubtype();

    /**
     * @brief Get default input method property.
     *
     * This function is used to get default input method property.
     *
     * @return The property of default input method.
     * @since 10
     */
    IMF_API int32_t GetDefaultInputMethod(std::shared_ptr<Property> &prop);

    /**
     * @brief get input method config ability.
     *
     * This function is used to get input method config ability.
     *
     * @return The info of input settings.
     * @since 10
     */
    IMF_API int32_t GetInputMethodConfig(AppExecFwk::ElementName &inputMethodConfig);

    /**
     * @brief Set calling window id.
     *
     * This function is used to set calling window id to input method.
     *
     * @param windowId Indicates the window id.
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t SetCallingWindow(uint32_t windowId);

    /**
     * @brief Switch input method or subtype.
     *
     * This function is used to switch input method or subtype.
     *
     * @param name      Indicates the id of target input method.
     * @param subName   Optional parameter. Indicates the subtype of target input method.
     * @return Returns 0 for success, others for failure.
     * @since 11
     */
    IMF_API int32_t SwitchInputMethod(SwitchTrigger trigger, const std::string &name, const std::string &subName = "");

    /**
     * @brief Show soft keyboard.
     *
     * This function is used to show soft keyboard of current client.
     *
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t ShowSoftKeyboard();

    /**
     * @brief Hide soft keyboard.
     *
     * This function is used to hide soft keyboard of current client, and keep binding.
     *
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t HideSoftKeyboard();

    /**
     * @brief Stop current input session.
     *
     * This function is used to stop current input session.
     *
     * @return Returns 0 for success, others for failure.
     * @since 6
     */
    IMF_API int32_t StopInputSession();

    /**
     * @brief Show input method setting extension dialog.
     *
     * This function is used to show input method setting extension dialog.
     *
     * @return Returns 0 for success, others for failure.
     * @since 8
     */
    IMF_API int32_t ShowOptionalInputMethod();

    // Deprecated innerkits with no permission check, kept for compatibility

    /**
     * @brief Show soft keyboard.
     *
     * This function is used to show soft keyboard of current client.
     *
     * @return Returns 0 for success, others for failure.
     * @deprecated since 9
     * @since 6
     */
    IMF_API int32_t ShowCurrentInput();

    /**
     * @brief Hide soft keyboard.
     *
     * This function is used to hide soft keyboard of current client, and keep binding.
     *
     * @return Returns 0 for success, others for failure.
     * @deprecated since 9
     * @since 6
     */
    IMF_API int32_t HideCurrentInput();

    /**
     * @brief Request to show input method.
     *
     * This function is used to request to show input method.
     *
     * @return Returns 0 for success, others for failure.
     * @since 11
     */
    IMF_API int32_t RequestShowInput();

    /**
     * @brief Request to hide input method.
     *
     * This function is used to request to hide input method.
     *
     * @return Returns 0 for success, others for failure.
     * @since 11
     */
    IMF_API int32_t RequestHideInput();

    /**
     * @brief Show input method setting extension dialog.
     *
     * This function is used to show input method setting extension dialog.
     *
     * @return Returns 0 for success, others for failure.
     * @deprecated since 9
     * @since 6
     */
    IMF_API int32_t DisplayOptionalInputMethod();

    /**
     * @brief Get attach status.
     *
     * This function is used to get status of attach.
     *
     * @return Returns true for attached otherwise for detached.
     * @since 10
     */
    IMF_API bool WasAttached();

    /**
     * @brief Set agent which will be used to communicate with IMA.
     *
     * This function is used to Set agent.
     *
     * @since 10
     */
    void OnInputReady(sptr<IRemoteObject> agentObject);

    /**
     * @brief Unbind IMC with Service.
     *
     * This function is unbind imc with service.
     *
     * @since 10
     */
    void OnInputStop();

    /**
     * @brief Insert text.
     *
     * This function is used to insert text into editor.
     *
     * @param text Indicates the text which will be inserted.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    int32_t InsertText(const std::u16string &text);

    /**
     * @brief Move cursor.
     *
     * This function is used to move cursor according to the direction.
     *
     * @param direction Indicates the direction according to which the cursor will be moved.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    int32_t MoveCursor(Direction direction);

    /**
     * @brief Delete forward.
     *
     * This function is used to delete text at the left of cursor.
     *
     * @param length Indicates the length of deleted text.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    int32_t DeleteForward(int32_t length);

    /**
     * @brief Delete backward.
     *
     * This function is used to delete text at the right of cursor.
     *
     * @param length Indicates the length of deleted text.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    int32_t DeleteBackward(int32_t length);

    /**
     * @brief Get text at the left of cursor.
     *
     * This function is used to get text at the left of cursor.
     *
     * @param length Indicates the length of text.
     * @param text Indicates the text which will be get.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    int32_t GetLeft(int32_t length, std::u16string &text);

    /**
     * @brief Get text at the right of cursor.
     *
     * This function is used to get text at the right of cursor.
     *
     * @param length Indicates the length of text.
     * @param text Indicates the text which will be get.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    int32_t GetRight(int32_t length, std::u16string &text);

    /**
     * @brief Select text in editor by range.
     *
     * This function is used to select text in editor by range.
     *
     * @param start Indicates the beginning of the range.
     * @param start Indicates the end of the range.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    void SelectByRange(int32_t start, int32_t end);

    /**
     * @brief Select text in editor by cursor movement.
     *
     * This function is used to select text in editor by cursor movement.
     *
     * @param direction Indicates the direction of cursor movement.
     * @param cursorMoveSkip Indicates the skip of cursor movement.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    void SelectByMovement(int32_t direction, int32_t cursorMoveSkip);

    /**
     * @brief Handle extend action code.
     *
     * This function is used to handle extend action code.
     *
     * @param action Indicates the action code which will be handled.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    int32_t HandleExtendAction(int32_t action);

    /**
     * @brief Get the index number of text at cursor.
     *
     * This function is used to get the index number of text at cursor.
     *
     * @param index Indicates the index number of text at cursor.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    int32_t GetTextIndexAtCursor(int32_t &index);

    /**
     * @brief Send keyboard status.
     *
     * This function is used to send keyboard status to editor.
     *
     * @param status Indicates the status of keyboard.
     * @since 10
     */
    void SendKeyboardStatus(KeyboardStatus status);

    /**
     * @brief Send panel status info.
     *
     * This function is used to send panel status info to editor.
     * Only notify the status info of soft keyboard(not contain candidate column) at present
     *
     * @param info Indicates the status info of panel.
     * @since 11
     */
    void NotifyPanelStatusInfo(const PanelStatusInfo &info);

    /**
     * @brief Send panel height.
     *
     * This function is used to send panel height to editor.
     *
     * @param info Indicates the panel height.
     * @since 11
     */
    void NotifyKeyboardHeight(uint32_t height);

    /**
     * @brief Send function key.
     *
     * This function is used to send function key to editor.
     *
     * @param functionKey Indicates the function key.
     * @return Returns 0 for success, others for failure.
     * @since 10
     */
    int32_t SendFunctionKey(int32_t functionKey);

    /**
     * @brief Deactivate the input client.
     *
     * This function is used to deactivate the input client.
     *
     * @since 11
     */
    void DeactivateClient();

    /**
     * @brief Query whether an input type is supported.
     *
     * This function is used to query whether an input type is supported.
     *
     * @param type Indicates the input type being queried.
     * @return Returns true for supported, false for not supported.
     * @since 11
     */
    IMF_API bool IsInputTypeSupported(InputType type);

    /**
     * @brief Start the input method which provides the specific input type.
     *
     * This function is used to start the input method which provides the specific input type.
     *
     * @param type Indicates the input type being specified.
     * @return Returns 0 for success, others for failure.
     * @since 11
     */
    IMF_API int32_t StartInputType(InputType type);

    /**
     * @brief Query whether the specific type panel is shown.
     *
     * This function is used to query whether the specific type panel is shown.
     *
     * @param panelInfo Indicates the info of the panel.
     * @param isShown Indicates the state of the specific panel.
     * @return Returns 0 for success, others for failure.
     * @since 11
     */
    IMF_API int32_t IsPanelShown(const PanelInfo &panelInfo, bool &isShown);
    int32_t UpdateListenEventFlag(uint32_t finalEventFlag, uint32_t eventFlag, bool isOn);

    /**
     * @brief Send private command to ime.
     *
     * This function is used to send private command to ime.
     *
     * @param privateCommand Indicates the private command which will be send.
     * @return Returns 0 for success, others for failure.
     * @since 12
     */
    IMF_API int32_t SendPrivateCommand(
        const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;

    /**
     * @brief Receive private command from ime.
     *
     * This function is used to receive private command from ime.
     *
     * @param privateCommand Indicates the private command which send from ime.
     * @return Returns 0 for success, others for failure.
     * @since 12
     */
    int32_t ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) override;

    /**
     * @brief Set preview text.
     *
     * This function is used to set preview text.
     *
     * @param text Indicates the text to be previewed.
     * @param range Indicates the range of text to be replaced.
     * @return Returns 0 for success, others for failure.
     * @since 12
     */
    int32_t SetPreviewText(const std::string &text, const Range &range);

    /**
     * @brief Finish text preview.
     *
     * This function is used to finish text preview.
     *
     * @since 12
     */
    int32_t FinishTextPreview();

private:
    InputMethodController();
    ~InputMethodController();

    int32_t Initialize();
    sptr<IInputMethodSystemAbility> GetSystemAbilityProxy();
    void RemoveDeathRecipient();
    int32_t StartInput(InputClientInfo &inputClientInfo, sptr<IRemoteObject> &agent);
    int32_t ShowInput(sptr<IInputClient> &client);
    int32_t HideInput(sptr<IInputClient> &client);
    int32_t ReleaseInput(sptr<IInputClient> &client);
    int32_t ListInputMethodCommon(InputMethodStatus status, std::vector<Property> &props);
    void ClearEditorCache(bool isNewEditor, sptr<OnTextChangedListener> lastListener);
    void OnRemoteSaDied(const wptr<IRemoteObject> &object);
    void RestoreListenInfoInSaDied();
    void RestoreAttachInfoInSaDied();
    int32_t RestoreListenEventFlag();
    void SaveTextConfig(const TextConfig &textConfig);
    sptr<OnTextChangedListener> GetTextListener();
    void SetTextListener(sptr<OnTextChangedListener> listener);
    bool IsEditable();
    bool IsBound();
    void SetAgent(sptr<IRemoteObject> &agentObject);
    std::shared_ptr<IInputMethodAgent> GetAgent();
    void PrintLogIfAceTimeout(int64_t start);
    void PrintKeyEventLog();

    std::shared_ptr<ControllerListener> controllerListener_;
    std::mutex abilityLock_;
    sptr<IInputMethodSystemAbility> abilityManager_ = nullptr;
    sptr<InputDeathRecipient> deathRecipient_;
    std::mutex agentLock_;
    sptr<IRemoteObject> agentObject_ = nullptr;
    std::shared_ptr<IInputMethodAgent> agent_ = nullptr;
    std::mutex textListenerLock_;
    sptr<OnTextChangedListener> textListener_ = nullptr;
    std::atomic_bool isDiedAttached_{ false };

    std::mutex cursorInfoMutex_;
    CursorInfo cursorInfo_;

    std::atomic_bool isTextNotified_{ false };
    std::mutex editorContentLock_;
    std::u16string textString_;
    int selectOldBegin_ = 0;
    int selectOldEnd_ = 0;
    int selectNewBegin_ = 0;
    int selectNewEnd_ = 0;

    static std::mutex instanceLock_;
    static sptr<InputMethodController> instance_;
    static std::shared_ptr<AppExecFwk::EventHandler> handler_;

    static std::mutex logLock_;
    static int keyEventCountInPeriod_;
    static std::chrono::system_clock::time_point startLogTime_;

    std::atomic_bool isEditable_{ false };
    std::atomic_bool isBound_{ false };
    std::atomic_bool bootCompleted_{ false };

    std::recursive_mutex clientInfoLock_;
    InputClientInfo clientInfo_;

    static constexpr int CURSOR_DIRECTION_BASE_VALUE = 2011;
    std::atomic_bool isDiedRestoreListen_{ false };

    std::mutex textConfigLock_;
    TextConfig textConfig_;

    struct KeyEventInfo {
        std::chrono::system_clock::time_point timestamp{};
        std::shared_ptr<MMI::KeyEvent> keyEvent;
        bool operator==(const KeyEventInfo &info) const
        {
            return (timestamp == info.timestamp && keyEvent == info.keyEvent);
        }
    };
    static constexpr int32_t MAX_WAIT_TIME = 5000;
    BlockQueue<KeyEventInfo> keyEventQueue_{ MAX_WAIT_TIME };
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_METHOD_CONTROLLER_H
