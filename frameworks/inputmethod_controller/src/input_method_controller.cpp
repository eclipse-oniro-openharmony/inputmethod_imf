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

#include "input_method_controller.h"

#include "global.h"
#include "input_method_status.h"
#include "inputmethod_sysevent.h"
#include "inputmethod_trace.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "input_client_stub.h"
#include "input_data_channel_stub.h"
#include "input_method_agent_proxy.h"
#include "input_method_system_ability_proxy.h"
#include "system_ability_definition.h"
#include "utils.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
    sptr<InputMethodController> InputMethodController::instance_;
    std::mutex InputMethodController::instanceLock_;

    InputMethodController::InputMethodController() : stop_(false)
    {
        IMSA_HILOGI("InputMethodController structure");
        Initialize();
    }

    InputMethodController::~InputMethodController()
    {
        if (msgHandler) {
            delete msgHandler;
            msgHandler = nullptr;
            stop_ = false;
        }
    }

    sptr<InputMethodController> InputMethodController::GetInstance()
    {
        if (!instance_) {
            std::lock_guard<std::mutex> autoLock(instanceLock_);
            if (!instance_) {
                IMSA_HILOGI("InputMethodController::GetInstance instance_ is nullptr");
                instance_ = new InputMethodController();
            }
        }
        return instance_;
    }

    bool InputMethodController::Initialize()
    {
        mImms = GetImsaProxy();

        msgHandler = new MessageHandler();

        InputClientStub *client = new (std::nothrow) InputClientStub();
        if (client == nullptr) {
            IMSA_HILOGE("InputMethodController::Initialize client is nullptr");
            return false;
        }
        client->SetHandler(msgHandler);
        mClient = client;

        InputDataChannelStub *channel = new (std::nothrow) InputDataChannelStub();
        if (channel == nullptr) {
            IMSA_HILOGE("InputMethodController::Initialize channel is nullptr");
            return false;
        }
        channel->SetHandler(msgHandler);
        mInputDataChannel = channel;

        workThreadHandler = std::thread([this] {WorkThread();});
        mAttribute.SetInputPattern(InputAttribute::PATTERN_TEXT);

        textListener = nullptr;
        IMSA_HILOGI("InputMethodController::Initialize textListener is nullptr");
        PrepareInput(0, mClient, mInputDataChannel, mAttribute);
        return true;
    }

    sptr<IInputMethodSystemAbility> InputMethodController::GetImsaProxy()
    {
        IMSA_HILOGI("InputMethodController::GetImsaProxy");
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityManager) {
            IMSA_HILOGI("InputMethodController::GetImsaProxy systemAbilityManager is nullptr");
            return nullptr;
        }

        int32_t uid = IPCSkeleton::GetCallingUid();
        std::string strBundleName = "com.inputmethod.default";
        auto systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
        if (!systemAbility) {
            IMSA_HILOGI("InputMethodController::GetImsaProxy systemAbility is nullptr");
            FaultReporter(uid, strBundleName, ErrorCode::ERROR_NULL_POINTER);
            return nullptr;
        }

        if (!deathRecipient_) {
            deathRecipient_ = new ImsaDeathRecipient();
        }
        systemAbility->AddDeathRecipient(deathRecipient_);

        return iface_cast<IInputMethodSystemAbility>(systemAbility);
    }

    void InputMethodController::WorkThread()
    {
        while (!stop_) {
            Message *msg = msgHandler->GetMessage();
            switch (msg->msgId_) {
                case MSG_ID_INSERT_CHAR: {
                    MessageParcel *data = msg->msgContent_;
                    std::u16string text = data->ReadString16();
                    IMSA_HILOGI("InputMethodController::WorkThread InsertText");
                    if (textListener) {
                        textListener->InsertText(text);
                    }
                    break;
                }

                case MSG_ID_DELETE_FORWARD: {
                    MessageParcel *data = msg->msgContent_;
                    int32_t length = data->ReadInt32();
                    IMSA_HILOGI("InputMethodController::WorkThread DeleteForward");
                    if (textListener) {
                        textListener->DeleteForward(length);
                    }
                    break;
                }
                case MSG_ID_DELETE_BACKWARD: {
                    MessageParcel *data = msg->msgContent_;
                    int32_t length = data->ReadInt32();
                    IMSA_HILOGI("InputMethodController::WorkThread DeleteBackward");
                    if (textListener) {
                        textListener->DeleteBackward(length);
                    }
                    break;
                }
                case MSG_ID_SET_DISPLAY_MODE: {
                    MessageParcel *data = msg->msgContent_;
                    int32_t ret = data->ReadInt32();
                    IMSA_HILOGI("MSG_ID_SET_DISPLAY_MODE : %{public}d", ret);
                    break;
                }
                case MSG_ID_ON_INPUT_READY: {
                    MessageParcel *data = msg->msgContent_;
                    sptr<IRemoteObject> object = data->ReadRemoteObject();
                    if (object) {
                        SetInputMethodAgent(object);
                    }
                    break;
                }
                case MSG_ID_EXIT_SERVICE: {
                    MessageParcel *data = msg->msgContent_;
                    int32_t ret = data->ReadInt32();
                    textListener = nullptr;
                    IMSA_HILOGI("InputMethodController::WorkThread MSG_ID_EXIT_SERVICE : %{public}d", ret);
                    break;
                }
                case MSG_ID_SEND_KEYBOARD_STATUS: {
                    MessageParcel *data = msg->msgContent_;
                    int32_t ret = data->ReadInt32();
                    KeyboardInfo *info = new KeyboardInfo();
                    info->SetKeyboardStatus(ret);
                    IMSA_HILOGI("InputMethodController::WorkThread SendKeyboardInfo");
                    if (textListener) {
                        textListener->SendKeyboardInfo(*info);
                    }
                    delete info;
                    break;
                }
                case MSG_ID_SEND_FUNCTION_KEY: {
                    MessageParcel *data = msg->msgContent_;
                    int32_t ret = data->ReadInt32();
                    KeyboardInfo *info = new KeyboardInfo();
                    info->SetFunctionKey(ret);
                    IMSA_HILOGI("InputMethodController::WorkThread SendKeyboardInfo");
                    if (textListener) {
                        textListener->SendKeyboardInfo(*info);
                    }
                    delete info;
                    break;
                }
                case MSG_ID_MOVE_CURSOR: {
                    MessageParcel *data = msg->msgContent_;
                    int32_t ret = data->ReadInt32();
                    IMSA_HILOGI("InputMethodController::WorkThread MoveCursor");
                    if (textListener) {
                        Direction direction = static_cast<Direction>(ret);
                        textListener->MoveCursor(direction);
                    }
                    break;
                }
                default: {
                    break;
                }
            }
            delete msg;
            msg = nullptr;
        }
    }

    void InputMethodController::Attach(sptr<OnTextChangedListener> &listener)
    {
        Attach(listener, true);
    }

    void InputMethodController::Attach(sptr<OnTextChangedListener> &listener, bool isShowKeyboard)
    {
        textListener = listener;
        IMSA_HILOGI("InputMethodController::Attach");
        InputmethodTrace tracer("InputMethodController Attach trace.");
        IMSA_HILOGI("InputMethodController::Attach isShowKeyboard %{public}s", isShowKeyboard ? "true" : "false");
        PrepareInput(0, mClient, mInputDataChannel, mAttribute);
        StartInput(mClient, isShowKeyboard);
    }

    void InputMethodController::ShowTextInput()
    {
        IMSA_HILOGI("InputMethodController::ShowTextInput");
        StartInput(mClient, true);
    }

    void InputMethodController::HideTextInput()
    {
        IMSA_HILOGI("InputMethodController::HideTextInput");
        StopInput(mClient);
    }

    int32_t InputMethodController::HideCurrentInput()
    {
        IMSA_HILOGI("InputMethodController::HideCurrentInput");
        if (mImms == nullptr) {
            IMSA_HILOGE("mImms is nullptr");
            return ErrorCode::ERROR_KBD_HIDE_FAILED;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(mImms->GetDescriptor()))) {
            IMSA_HILOGE("write descriptor failed");
            return ErrorCode::ERROR_KBD_HIDE_FAILED;
        }
        return mImms->HideCurrentInputDeprecated(data);
    }

    int32_t InputMethodController::ShowCurrentInput()
    {
        IMSA_HILOGI("InputMethodController::ShowCurrentInput");
        if (mImms == nullptr) {
            IMSA_HILOGE("mImms is nullptr");
            return ErrorCode::ERROR_KBD_SHOW_FAILED;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(mImms->GetDescriptor()))) {
            IMSA_HILOGE("write descriptor failed");
            return ErrorCode::ERROR_KBD_SHOW_FAILED;
        }
        return mImms->ShowCurrentInputDeprecated(data);
    }

    void InputMethodController::Close()
    {
        ReleaseInput(mClient);
        InputmethodTrace tracer("InputMethodController Close trace.");
        textListener = nullptr;
        IMSA_HILOGI("InputMethodController::Close");
    }

    void InputMethodController::PrepareInput(int32_t displayId, sptr<IInputClient> &client,
                                             sptr<IInputDataChannel> &channel, InputAttribute &attribute)
    {
        IMSA_HILOGI("InputMethodController::PrepareInput");
        if (!mImms) {
            return;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(mImms->GetDescriptor())
            && data.WriteInt32(displayId)
            && data.WriteRemoteObject(client->AsObject())
            && data.WriteRemoteObject(channel->AsObject())
            && data.WriteParcelable(&attribute))) {
            return;
        }
        mImms->prepareInput(data);
    }

    int32_t InputMethodController::DisplayOptionalInputMethod()
    {
        IMSA_HILOGI("InputMethodController::DisplayOptionalInputMethod");
        if (!mImms) {
            return ErrorCode::ERROR_STATUS_BAD_VALUE;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(mImms->GetDescriptor()))) {
            return ErrorCode::ERROR_STATUS_BAD_VALUE;
        }
        return mImms->displayOptionalInputMethod(data);
    }

    std::vector<Property> InputMethodController::ListInputMethodCommon(InputMethodStatus status)
    {
        IMSA_HILOGI("InputMethodController::ListInputMethodCommon");
        if (mImms == nullptr) {
            IMSA_HILOGE("mImms is nullptr");
            return {};
        }
        auto property = mImms->ListInputMethod(status);
        return Utils::GetProperty(property);
    }

    std::vector<Property> InputMethodController::ListInputMethod()
    {
        IMSA_HILOGI("InputMethodController::listInputMethod");
        return ListInputMethodCommon(ALL);
    }

    std::vector<Property> InputMethodController::ListInputMethod(bool enable)
    {
        IMSA_HILOGI("InputMethodController::listInputMethod enable = %{public}s", enable ? "ENABLE" : "DISABLE");
        return ListInputMethodCommon(enable ? ENABLE : DISABLE);
    }

    std::shared_ptr<Property> InputMethodController::GetCurrentInputMethod()
    {
        IMSA_HILOGI("InputMethodController::GetCurrentInputMethod");
        if (mImms == nullptr) {
            IMSA_HILOGE("InputMethodController::GetCurrentInputMethod mImms is nullptr");
            return nullptr;
        }

        auto property = mImms->GetCurrentInputMethod();
        if (property == nullptr) {
            IMSA_HILOGE("InputMethodController::GetCurrentInputMethod property is nullptr");
            return nullptr;
        }

        return { new Property({ Str16ToStr8(property->mPackageName), Str16ToStr8(property->mAbilityName) }),
            [](auto p) {} };
    }

    void InputMethodController::StartInput(sptr<IInputClient> &client, bool isShowKeyboard)
    {
        IMSA_HILOGI("InputMethodController::StartInput");
        if (!mImms) {
            return;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(mImms->GetDescriptor()) && data.WriteRemoteObject(client->AsObject())
                && data.WriteBool(isShowKeyboard))) {
            return;
        }
        isStopInput = false;
        mImms->startInput(data);
    }

    void InputMethodController::ReleaseInput(sptr<IInputClient> &client)
    {
        IMSA_HILOGI("InputMethodController::ReleaseInput");
        if (!mImms) {
            return;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(mImms->GetDescriptor())
            && data.WriteRemoteObject(client->AsObject().GetRefPtr()))) {
            return;
        }
        isStopInput = true;
        mImms->releaseInput(data);
    }

    void InputMethodController::StopInput(sptr<IInputClient> &client)
    {
        IMSA_HILOGI("InputMethodController::StopInput");
        if (!mImms) {
            return;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(mImms->GetDescriptor())
            && data.WriteRemoteObject(client->AsObject().GetRefPtr()))) {
            return;
        }
        isStopInput = true;
        mImms->stopInput(data);
    }

    void InputMethodController::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
    {
        mImms = GetImsaProxy();
    }

    ImsaDeathRecipient::ImsaDeathRecipient()
    {
    }

    void ImsaDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
    {
        InputMethodController::GetInstance()->OnRemoteSaDied(object);
    }

    void InputMethodController::OnCursorUpdate(CursorInfo cursorInfo)
    {
        std::shared_ptr<IInputMethodAgent> agent = GetInputMethodAgent();
        if (agent == nullptr) {
            IMSA_HILOGI("InputMethodController::OnCursorUpdate agent is nullptr");
            return;
        }

        if (isStopInput) {
            IMSA_HILOGD("InputMethodController::OnCursorUpdate isStopInput");
            return;
        }

        if (cursorInfo_.left == cursorInfo.left && cursorInfo_.top == cursorInfo.top
            && cursorInfo_.height == cursorInfo.height) {
            return;
        }
        cursorInfo_ = cursorInfo;
        agent->OnCursorUpdate(cursorInfo.left, cursorInfo.top, cursorInfo.height);
    }

    void InputMethodController::OnSelectionChange(std::u16string text, int start, int end)
    {
        if (mTextString == text && mSelectNewBegin == start && mSelectNewEnd == end) {
            return;
        }
        IMSA_HILOGI("InputMethodController::OnSelectionChange");
        mTextString = text;
        mSelectOldBegin = mSelectNewBegin;
        mSelectOldEnd = mSelectNewEnd;
        mSelectNewBegin = start;
        mSelectNewEnd = end;
        std::shared_ptr<IInputMethodAgent> agent = GetInputMethodAgent();
        if (agent == nullptr) {
            IMSA_HILOGI("InputMethodController::OnSelectionChange agent is nullptr");
            return;
        }

        if (isStopInput) {
            IMSA_HILOGD("InputMethodController::OnSelectionChange isStopInput");
            return;
        }

        agent->OnSelectionChange(mTextString, mSelectOldBegin, mSelectOldEnd, mSelectNewBegin, mSelectNewEnd);
    }

    void InputMethodController::OnConfigurationChange(Configuration info)
    {
        IMSA_HILOGI("InputMethodController::OnConfigurationChange");
        enterKeyType_ = static_cast<uint32_t>(info.GetEnterKeyType());
        inputPattern_ = static_cast<uint32_t>(info.GetTextInputType());
    }

    std::u16string InputMethodController::GetTextBeforeCursor(int32_t number)
    {
        IMSA_HILOGI("InputMethodController::GetTextBeforeCursor");
        if (!mTextString.empty()) {
            int32_t startPos = (mSelectNewBegin >= number ? (mSelectNewBegin - number + 1) : 0);
            return mTextString.substr(startPos, mSelectNewBegin);
        }
        return u"";
    }

    std::u16string InputMethodController::GetTextAfterCursor(int32_t number)
    {
        IMSA_HILOGI("InputMethodController::GetTextBeforeCursor");
        if (!mTextString.empty() && mTextString.size() <= INT_MAX) {
            int32_t endPos = (mSelectNewEnd + number < static_cast<int32_t>(mTextString.size()))
                                 ? (mSelectNewEnd + number)
                                 : mTextString.size();
            return mTextString.substr(mSelectNewEnd, endPos);
        }
        return u"";
    }

    bool InputMethodController::dispatchKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent)
    {
        IMSA_HILOGI("InputMethodController::dispatchKeyEvent");
        std::shared_ptr<IInputMethodAgent> agent = GetInputMethodAgent();
        if (agent == nullptr) {
            IMSA_HILOGI("InputMethodController::dispatchKeyEvent agent is nullptr");
            return false;
        }
        
        if (isStopInput) {
            IMSA_HILOGD("InputMethodController::dispatchKeyEvent isStopInput");
            return false;
        }

        MessageParcel data;
        if (!(data.WriteInterfaceToken(agent->GetDescriptor())
            && data.WriteInt32(keyEvent->GetKeyCode())
            && data.WriteInt32(keyEvent->GetKeyAction()))) {
            return false;
        }

        return agent->DispatchKeyEvent(data);
    }

    int32_t InputMethodController::GetEnterKeyType()
    {
        IMSA_HILOGI("InputMethodController::GetEnterKeyType");
        return enterKeyType_;
    }

    int32_t InputMethodController::GetInputPattern()
    {
        IMSA_HILOGI("InputMethodController::GetInputPattern");
        return inputPattern_;
    }

    void InputMethodController::SetCallingWindow(uint32_t windowId)
    {
        IMSA_HILOGI("InputMethodController::SetCallingWindow windowId = %{public}d", windowId);
        std::shared_ptr<IInputMethodAgent> agent = GetInputMethodAgent();
        if (agent == nullptr) {
            IMSA_HILOGI("InputMethodController::SetCallingWindow agent is nullptr");
            return;
        }
        
        if (isStopInput) {
            IMSA_HILOGD("InputMethodController::SetCallingWindow isStopInput");
            return;
        }
        
        agent->SetCallingWindow(windowId);
    }

    int32_t InputMethodController::SwitchInputMethod(const Property &target)
    {
        IMSA_HILOGI("InputMethodController::SwitchInputMethod");
        if (!mImms) {
            IMSA_HILOGE("InputMethodController mImms is nullptr");
            return false;
        }
        InputMethodProperty property;
        property.mPackageName = Str8ToStr16(target.packageName);
        property.mAbilityName = Str8ToStr16(target.abilityName);
        return mImms->SwitchInputMethod(property);
    }

    void InputMethodController::SetInputMethodAgent(sptr<IRemoteObject> &object)
    {
        IMSA_HILOGI("run in SetInputMethodAgent");
        std::lock_guard<std::mutex> lock(agentLock_);
        std::shared_ptr<IInputMethodAgent> agent = std::make_shared<InputMethodAgentProxy>(object);
        if (agent == nullptr) {
            IMSA_HILOGI("InputMethodController::SetInputMethodAgent agent is nullptr");
            return;
        }
        mAgent = agent;
    }

    std::shared_ptr<IInputMethodAgent> InputMethodController::GetInputMethodAgent()
    {
        std::lock_guard<std::mutex> lock(agentLock_);
        return mAgent;
    }

    int32_t InputMethodController::ShowSoftKeyboard()
    {
        IMSA_HILOGI("InputMethodController ShowSoftKeyboard");
        if (mImms == nullptr) {
            IMSA_HILOGE("mImms is nullptr");
            return ErrorCode::ERROR_KBD_SHOW_FAILED;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(mImms->GetDescriptor()))) {
            return ErrorCode::ERROR_KBD_SHOW_FAILED;
        }
        return mImms->ShowCurrentInput(data);
    }

    int32_t InputMethodController::HideSoftKeyboard()
    {
        IMSA_HILOGI("InputMethodController HideSoftKeyboard");
        if (mImms == nullptr) {
            IMSA_HILOGE("mImms is nullptr");
            return ErrorCode::ERROR_KBD_SHOW_FAILED;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(mImms->GetDescriptor()))) {
            return ErrorCode::ERROR_KBD_SHOW_FAILED;
        }
        return mImms->HideCurrentInput(data);
    }
} // namespace MiscServices
} // namespace OHOS
