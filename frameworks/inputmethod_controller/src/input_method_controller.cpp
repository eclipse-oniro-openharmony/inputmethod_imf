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
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "global.h"

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
        if (msgHandler != nullptr) {
            delete msgHandler;
            msgHandler = nullptr;
            stop_ = false;
        }
    }

    sptr<InputMethodController> InputMethodController::GetInstance()
    {
        if (instance_ == nullptr) {
            std::lock_guard<std::mutex> autoLock(instanceLock_);
            if (instance_ == nullptr) {
                IMSA_HILOGI("InputMethodController::GetInstance instance_ is nullptr");
                instance_ = new InputMethodController();
            }
        }
        return instance_;
    }

    bool InputMethodController::Initialize()
    {
        msgHandler = new MessageHandler();

        mClient = new InputClientStub();
        mClient->SetHandler(msgHandler);

        mInputDataChannel = new InputDataChannelStub();
        mInputDataChannel->SetHandler(msgHandler);

        workThreadHandler = std::thread([this] {WorkThread();});
        mAttribute.SetInputPattern(InputAttribute::PATTERN_TEXT);

        textListener = nullptr;
        PrepareInput(0, mClient, mInputDataChannel, mAttribute);
        return true;
    }

    sptr<IInputMethodSystemAbility> InputMethodController::GetSystemAbilityProxy()
    {
        IMSA_HILOGD("start");
        std::lock_guard<std::mutex> lock(abilityLock_);
        if (systemAbility_ != nullptr) {
          return systemAbility_;
        }
        IMSA_HILOGI("get input method service proxy");
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityManager == nullptr) {
            IMSA_HILOGE("systemAbilityManager is nullptr");
            return nullptr;
        }
        auto systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
        if (systemAbility == nullptr) {
            IMSA_HILOGE("systemAbility is nullptr");
            return nullptr;
        }
        if (deathRecipient_ == nullptr) {
            deathRecipient_ = new (std::nothrow) ImsaDeathRecipient();
            if (deathRecipient_ == nullptr) {
              IMSA_HILOGE("failed to new death recipient");
              return nullptr;
            }
        }
        if (systemAbility->IsProxyObject() && (!systemAbility->AddDeathRecipient(deathRecipient_))) {
            IMSA_HILOGE("failed to add death recipient");
            return nullptr;
        }
        systemAbility_ = iface_cast<IInputMethodSystemAbility>(systemAbility);
        return systemAbility_;
    }

    void InputMethodController::WorkThread()
    {
        while (!stop_) {
            Message *msg = msgHandler->GetMessage();
            switch (msg->msgId_) {
                case MSG_ID_INSERT_CHAR: {
                    MessageParcel *data = msg->msgContent_;
                    std::u16string text = data->ReadString16();
                    if (textListener != nullptr) {
                        textListener->InsertText(text);
                    }
                    break;
                }

                case MSG_ID_DELETE_FORWARD: {
                    IMSA_HILOGI("InputMethodController::MSG_ID_DELETE_FORWARD");
                    MessageParcel *data = msg->msgContent_;
                    int32_t length = data->ReadInt32();
                    if (textListener != nullptr) {
                        textListener->DeleteForward(length);
                    }
                    break;
                }
                case MSG_ID_DELETE_BACKWARD: {
                    IMSA_HILOGI("InputMethodController::MSG_ID_DELETE_BACKWARD");
                    MessageParcel *data = msg->msgContent_;
                    int32_t length = data->ReadInt32();
                    if (textListener != nullptr) {
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
                    if (object != nullptr) {
                        SetInputMethodAgent(object);
                    }
                    break;
                }
                case MSG_ID_EXIT_SERVICE: {
                    MessageParcel *data = msg->msgContent_;
                    int32_t ret = data->ReadInt32();
                    textListener = nullptr;
                    IMSA_HILOGI("MSG_ID_EXIT_SERVICE : %{public}d", ret);
                    break;
                }
                case MSG_ID_SEND_KEYBOARD_STATUS: {
                    MessageParcel *data = msg->msgContent_;
                    int32_t ret = data->ReadInt32();
                    IMSA_HILOGI("MSG_ID_SEND_KEYBOARD_STATUS : %{public}d", ret);
                    KeyboardInfo *info = new KeyboardInfo();
                    info->SetKeyboardStatus(ret);
                    if (textListener != nullptr) {
                        textListener->SendKeyboardInfo(*info);
                    }
                    delete info;
                    break;
                }
                case MSG_ID_SEND_FUNCTION_KEY: {
                    MessageParcel *data = msg->msgContent_;
                    int32_t ret = data->ReadInt32();
                    IMSA_HILOGI("MSG_ID_SEND_FUNCTION_KEY : %{public}d", ret);
                    KeyboardInfo *info = new KeyboardInfo();
                    info->SetFunctionKey(ret);
                    if (textListener != nullptr) {
                        textListener->SendKeyboardInfo(*info);
                    }
                    delete info;
                    break;
                }
                case MSG_ID_MOVE_CURSOR: {
                    MessageParcel *data = msg->msgContent_;
                    int32_t ret = data->ReadInt32();
                    IMSA_HILOGI("MSG_ID_MOVE_CURSOR : %{public}d", ret);
                    if (textListener != nullptr) {
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
        textListener = listener;
        PrepareInput(0, mClient, mInputDataChannel, mAttribute);
        StartInput(mClient);
    }

    void InputMethodController::ShowTextInput()
    {
        IMSA_HILOGI("InputMethodController::ShowTextInput");
        StartInput(mClient);
    }

    void InputMethodController::HideTextInput()
    {
        IMSA_HILOGI("InputMethodController::HideTextInput");
        StopInput(mClient);
    }

    void InputMethodController::HideCurrentInput()
    {
        IMSA_HILOGI("InputMethodController::HideCurrentInput");
        auto proxy = GetSystemAbilityProxy();
        if (proxy == nullptr) {
            IMSA_HILOGE("proxy is nullptr");
            return;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(proxy->GetDescriptor()))) {
            return;
        }
        proxy->HideCurrentInput(data);
    }

    void InputMethodController::Close()
    {
        ReleaseInput(mClient);
        textListener = nullptr;
    }

    void InputMethodController::PrepareInput(int32_t displayId, sptr<InputClientStub> &client,
                                             sptr<InputDataChannelStub> &channel, InputAttribute &attribute)
    {
        IMSA_HILOGI("InputMethodController::PrepareInput");
        auto proxy = GetSystemAbilityProxy();
        if (proxy == nullptr) {
            IMSA_HILOGE("proxy is nullptr");
            return;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(proxy->GetDescriptor())
            && data.WriteInt32(displayId)
            && data.WriteRemoteObject(client->AsObject())
            && data.WriteRemoteObject(channel->AsObject())
            && data.WriteParcelable(&attribute))) {
            return;
        }
        proxy->prepareInput(data);
    }

    void InputMethodController::DisplayOptionalInputMethod()
    {
        IMSA_HILOGI("InputMethodController::DisplayOptionalInputMethod");
        auto proxy = GetSystemAbilityProxy();
        if (proxy == nullptr) {
            IMSA_HILOGE("proxy is nullptr");
            return;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(proxy->GetDescriptor()))) {
            return;
        }
        proxy->displayOptionalInputMethod(data);
    }

    std::vector<InputMethodProperty*> InputMethodController::ListInputMethod()
    {
        IMSA_HILOGI("InputMethodController::listInputMethod");
        std::vector<InputMethodProperty*> properties;
        auto proxy = GetSystemAbilityProxy();
        if (proxy == nullptr) {
            IMSA_HILOGE("proxy is nullptr");
            return {};
        }
        proxy->listInputMethod(&properties);
        return properties;
    }

    void InputMethodController::StartInput(sptr<InputClientStub> &client)
    {
        IMSA_HILOGI("InputMethodController::StartInput");
        auto proxy = GetSystemAbilityProxy();
        if (proxy == nullptr) {
            IMSA_HILOGE("proxy is nullptr");
            return;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(proxy->GetDescriptor())
            && data.WriteRemoteObject(client->AsObject()))) {
            return;
        }
        proxy->startInput(data);
    }

    void InputMethodController::ReleaseInput(sptr<InputClientStub> &client)
    {
        IMSA_HILOGI("InputMethodController::ReleaseInput");
        auto proxy = GetSystemAbilityProxy();
        if (proxy == nullptr) {
            IMSA_HILOGE("proxy is nullptr");
            return;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(proxy->GetDescriptor())
            && data.WriteRemoteObject(client->AsObject().GetRefPtr()))) {
            return;
        }
        proxy->releaseInput(data);
    }

    void InputMethodController::StopInput(sptr<InputClientStub> &client)
    {
        IMSA_HILOGI("InputMethodController::StopInput");
        auto proxy = GetSystemAbilityProxy();
        if (proxy == nullptr) {
            IMSA_HILOGE("proxy is nullptr");
            return;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(proxy->GetDescriptor())
            && data.WriteRemoteObject(client->AsObject().GetRefPtr()))) {
            return;
        }
        proxy->stopInput(data);
    }

    void InputMethodController::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
    {
        IMSA_HILOGE("input method service died");
        std::lock_guard<std::mutex> lock(abilityLock_);
        systemAbility_ = nullptr;
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
        if (!mTextString.empty()) {
            int32_t endPos = (mSelectNewEnd+number<mTextString.size()) ? (mSelectNewEnd + number) : mTextString.size();
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
        agent->SetCallingWindow(windowId);
        return;
    }

    std::shared_ptr<IInputMethodAgent> InputMethodController::GetInputMethodAgent()
    {
        std::lock_guard<std::mutex> lock(agentLock_);
        return mAgent;
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
} // namespace MiscServices
} // namespace OHOS
