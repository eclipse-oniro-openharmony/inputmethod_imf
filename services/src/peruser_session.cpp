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

#include "peruser_session.h"

#include <vector>

#include "ability_connect_callback_proxy.h"
#include "ability_manager_interface.h"
#include "element_name.h"
#include "input_client_proxy.h"
#include "input_control_channel_proxy.h"
#include "input_data_channel_proxy.h"
#include "input_method_ability_connection_stub.h"
#include "input_method_agent_proxy.h"
#include "input_method_core_proxy.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "para_handle.h"
#include "parcel.h"
#include "platform.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "unistd.h"
#include "utils.h"
#include "want.h"

namespace OHOS {
namespace MiscServices {
    using namespace MessageID;

    void RemoteObjectDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
    {
        IMSA_HILOGE("Start");
        if (handler_ != nullptr) {
            handler_(remote);
        }
    }

    void RemoteObjectDeathRecipient::SetDeathRecipient(RemoteDiedHandler handler)
    {
        handler_ = handler;
    }

    PerUserSession::PerUserSession(int userId) : userId_(userId), imsDeathRecipient(new RemoteObjectDeathRecipient())
    {
    }

    /*! Destructor
    */
    PerUserSession::~PerUserSession()
    {
        if (userState == UserState::USER_STATE_UNLOCKED) {
            OnUserLocked();
        }
        imsDeathRecipient = nullptr;
        if (workThreadHandler.joinable()) {
            workThreadHandler.join();
        }
    }


    /*! Create work thread for this user
    \param handle message handle to receive the message
    */
    void PerUserSession::CreateWorkThread(MessageHandler& handler)
    {
        msgHandler = &handler;
        workThreadHandler = std::thread([this] {WorkThread();});
    }

    /*! Wait till work thread exits
    */
    void PerUserSession::JoinWorkThread()
    {
        if (workThreadHandler.joinable()) {
            workThreadHandler.join();
        }
    }

    /*! Work thread for this user
    */
    void PerUserSession::WorkThread()
    {
        if (!msgHandler) {
            return;
        }
        while (1) {
            Message *msg = msgHandler->GetMessage();
            std::unique_lock<std::mutex> lock(mtx);
            switch (msg->msgId_) {
                case MSG_ID_USER_LOCK:
                case MSG_ID_EXIT_SERVICE: {
                    OnUserLocked();
                    delete msg;
                    msg = nullptr;
                    return;
                }
                case MSG_ID_HIDE_KEYBOARD_SELF: {
                    int flag = msg->msgContent_->ReadInt32();
                    OnHideKeyboardSelf(flag);
                    break;
                }
                case MSG_ID_ADVANCE_TO_NEXT: {
                    OnAdvanceToNext();
                    break;
                }
                case MSG_ID_SET_DISPLAY_MODE: {
                    int mode = msg->msgContent_->ReadInt32();
                    OnSetDisplayMode(mode);
                    break;
                }
                case MSG_ID_RESTART_IMS: {
                    int index = msg->msgContent_->ReadInt32();
                    std::u16string imeId = msg->msgContent_->ReadString16();
                    OnRestartIms(index, imeId);
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

    /*! Set display Id
    \param displayId the Id of display screen on which the input method keyboard show.
    */
    void PerUserSession::SetDisplayId(int displayId)
    {
        this->displayId = displayId;
    }

    /*! Set the current input method engine
    \param ime the current (default) IME pointer referred to the instance in PerUserSetting.
    */
    void PerUserSession::SetCurrentIme(InputMethodInfo *ime)
    {
        currentIme[DEFAULT_IME] = ime;
        userState = UserState::USER_STATE_UNLOCKED;
    }

    /*! Set the system security input method engine
    \param ime system security IME pointer referred to the instance in PerUserSetting.
    */
    void PerUserSession::SetSecurityIme(InputMethodInfo *ime)
    {
        currentIme[SECURITY_IME] = ime;
    }

    /*! Set the input method setting data
    \param setting InputMethodSetting pointer referred to the instance in PerUserSetting.
    */
    void PerUserSession::SetInputMethodSetting(InputMethodSetting *setting)
    {
        inputMethodSetting = setting;
    }

    /*! Reset input method engine
    \param defaultIme default ime pointer referred to the instance in PerUserSetting
    \param  security security ime pointer referred to the instance in PerUserSetting
    \n Two input method engines can be running at the same time for one user.
    \n One is the default ime, another is security ime
    */
    void PerUserSession::ResetIme(InputMethodInfo *defaultIme, InputMethodInfo *securityIme)
    {
        IMSA_HILOGI("PerUserSession::ResetIme");
        std::unique_lock<std::mutex> lock(mtx);
        InputMethodInfo *ime[] = {defaultIme, securityIme};
        for (int i = 0; i < MIN_IME; i++) {
            if (currentIme[i] == ime[i] && ime[i]) {
                continue;
            }
            if (imsCore[i]) {
                StopInputMethod(i);
            }
            ResetImeError(i);
            currentIme[i] = ime[i];
            if (!currentIme[i]) {
                if (needReshowClient && GetImeIndex(needReshowClient) == i) {
                    needReshowClient = nullptr;
                }
                continue;
            }

            bool flag = false;
            for (auto it = mapClients.cbegin(); it != mapClients.cend(); ++it) {
                if ((i == DEFAULT_IME && !it->second->attribute.GetSecurityFlag()) ||
                        (i == SECURITY_IME && it->second->attribute.GetSecurityFlag())) {
                    flag = true;
                    break;
                }
            }
            if (flag) {
                int ret = StartInputMethod(i);
                if (ret != ErrorCode::NO_ERROR) {
                    needReshowClient = nullptr;
                    break;
                }
                if (needReshowClient && GetImeIndex(needReshowClient) == i) {
                    ShowKeyboard(needReshowClient, true);
                    needReshowClient = nullptr;
                }
            }
        }
    }

    /*! Called when a package is removed
    \param packageName the name of package removed
    */
    void PerUserSession::OnPackageRemoved(const std::u16string& packageName)
    {
        IMSA_HILOGI("PerUserSession::OnPackageRemoved");
        InputMethodSetting tmpSetting;
        bool flag = false;
        std::unique_lock<std::mutex> lock(mtx);
        for (int i = 0; i < MAX_IME; i++) {
            sptr<IInputClient> client = GetCurrentClient();
            if (currentIme[i] && currentIme[i]->mPackageName == packageName) {
                if (client != nullptr && GetImeIndex(client) == i) {
                    needReshowClient = client;
                    HideKeyboard(client);
                }
                StopInputMethod(i);
                currentIme[i] = nullptr;
                if (i == DEFAULT_IME) {
                    tmpSetting.SetCurrentKeyboardType(-1);
                    inputMethodSetting->SetCurrentKeyboardType(-1);
                } else if (i == SECURITY_IME) {
                    tmpSetting.SetCurrentSysKeyboardType(-1);
                    inputMethodSetting->SetCurrentSysKeyboardType(-1);
                }
                currentKbdIndex[i] = 0;
                flag = true;
            }
        }
        if (flag) {
            Platform::Instance()->SetInputMethodSetting(userId_, tmpSetting);
        }
    }

    int PerUserSession::AddClient(sptr<IRemoteObject> inputClient, const ClientInfo &clientInfo)
    {
        IMSA_HILOGI("PerUserSession::AddClient");
        auto cacheClient = GetClientInfo(inputClient);
        if (cacheClient != nullptr) {
            IMSA_HILOGE("PerUserSession::AddClient info is exist, not need add.");
            return ErrorCode::NO_ERROR;
        }
        auto info = std::make_shared<ClientInfo>(clientInfo);
        if (info == nullptr) {
            IMSA_HILOGE("info is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        mapClients.insert({ inputClient, info });
        info->deathRecipient->SetDeathRecipient(
            [this, info](const wptr<IRemoteObject> &) { this->OnClientDied(info->client); });
        sptr<IRemoteObject> obj = info->client->AsObject();
        if (obj == nullptr) {
            IMSA_HILOGE("PerUserSession::AddClient inputClient AsObject is nullptr");
            return ErrorCode::ERROR_REMOTE_CLIENT_DIED;
        }
        bool ret = obj->AddDeathRecipient(info->deathRecipient);
        IMSA_HILOGI("Add death recipient %{public}s", ret ? "success" : "failed");
        return ErrorCode::NO_ERROR;
    }

    /*! Remove an input client
    \param inputClient remote object handler of the input client
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_CLIENT_NOT_FOUND client is not found
    */
    void PerUserSession::RemoveClient(sptr<IRemoteObject> inputClient)
    {
        IMSA_HILOGE("PerUserSession::RemoveClient");
        auto it = mapClients.find(inputClient);
        if (it == mapClients.end()) {
            IMSA_HILOGE("PerUserSession::RemoveClient client not found");
            return;
        }
        auto info = it->second;
        info->client->AsObject()->RemoveDeathRecipient(info->deathRecipient);
        mapClients.erase(it);
    }

    /*! Start input method service
    \param index it can be 0 or 1. 0 - default ime, 1 - security ime
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_IME_BIND_FAILED failed to bind ime
    \return ErrorCode::ERROR_IME_NOT_AVAILABLE no ime is available
    \return ErrorCode::ERROR_SECURITY_IME_NOT_AVAILABLE no security ime is available
    \return ErrorCode::ERROR_TOKEN_CREATE_FAILED failed to create window token
    \return other errors returned by binder driver
    */
    int PerUserSession::StartInputMethod(int index)
    {
        IMSA_HILOGI("PerUserSession::StartInputMethod index = %{public}d [%{public}d]\n", index, userId_);

        if (!imsCore[index]) {
            IMSA_HILOGI("PerUserSession::StartInputMethod imscore is null");
            return ErrorCode::ERROR_IME_BIND_FAILED;
        }

        sptr<IRemoteObject> b = imsCore[index]->AsObject();
        inputMethodToken[index] = IPCSkeleton::GetInstance().GetContextObject();
        localControlChannel[index] = new InputControlChannelStub(userId_);
        inputControlChannel[index] = localControlChannel[index];
        int ret_init = imsCore[index]->initializeInput(inputMethodToken[index], displayId, inputControlChannel[index]);
        if (ret_init != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("PerUserSession::StartInputMethod initializeInput fail %{public}s", ErrorCode::ToString(ret_init));
            localControlChannel[index] = nullptr;
            inputControlChannel[index] = nullptr;
            return ret_init;
        }
        return ErrorCode::NO_ERROR;
    }

    /*! Stop input method service
    \param index it can be 0 or 1. 0 - default ime, 1 - security ime
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_IME_NOT_STARTED ime not started
    \return ErrorCode::ERROR_IME_UNBIND_FAILED failed to unbind ime
    \return ErrorCode::ERROR_TOKEN_DESTROY_FAILED failed to destroy window token
    \return other errors returned by binder driver
    */
    int PerUserSession::StopInputMethod(int index)
    {
        IMSA_HILOGI("Start... index = %{public}d [%{public}d]\n", index, userId_);
        if (index >= MAX_IME || index < 0) {
            IMSA_HILOGE("Aborted! %{public}s", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        if (!imsCore[index] || !currentIme[index]) {
            IMSA_HILOGE("Aborted! %{public}s", ErrorCode::ToString(ErrorCode::ERROR_IME_NOT_STARTED));
            return ErrorCode::ERROR_IME_NOT_STARTED;
        }
        if (currentIme[index] == currentIme[1 - index] && imsCore[1 - index]) {
            imsCore[index] = nullptr;
            inputControlChannel[index] = nullptr;
            localControlChannel[index] = nullptr;
            IMSA_HILOGI("End...[%{public}d]\n", userId_);
            return ErrorCode::NO_ERROR;
        }

        IMSA_HILOGD("unbindInputMethodService...\n");

        IMSA_HILOGD("destroyWindowTaskId...\n");
        int errorCode = ErrorCode::NO_ERROR;
        int ret = Platform::Instance()->DestroyWindowToken(userId_, currentIme[index]->mPackageName);
        inputMethodToken[index] = nullptr;
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("destroyWindowTaskId return : %{public}s [%{public}d]\n", ErrorCode::ToString(ret), userId_);
            errorCode = ErrorCode::ERROR_TOKEN_DESTROY_FAILED;
        }
        sptr<IRemoteObject> b = imsCore[index]->AsObject();
        ret = b->RemoveDeathRecipient(imsDeathRecipient);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("RemoveDeathRecipient return : %{public}s [%{public}d]\n", ErrorCode::ToString(ret), userId_);
        }
        imsCore[index] = nullptr;
        inputControlChannel[index] = nullptr;
        localControlChannel[index] = nullptr;
        IMSA_HILOGI("End...[%{public}d]\n", userId_);
        return errorCode;
    }

    /*! Show keyboard
    \param inputClient the remote object handler of the input client.
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_IME_NOT_STARTED ime not started
    \return ErrorCode::ERROR_KBD_IS_OCCUPIED keyboard is showing by other client
    \return ErrorCode::ERROR_CLIENT_NOT_FOUND the input client is not found
    \return ErrorCode::ERROR_IME_START_FAILED failed to start input method service
    \return ErrorCode::ERROR_KBD_SHOW_FAILED failed to show keyboard
    \return other errors returned by binder driver
    */
    int PerUserSession::ShowKeyboard(const sptr<IInputClient>& inputClient, bool isShowKeyboard)
    {
        IMSA_HILOGI("PerUserSession::ShowKeyboard");
        auto clientInfo = GetClientInfo(inputClient->AsObject());
        int index = GetImeIndex(inputClient);
        if (index == -1 || clientInfo == nullptr) {
            IMSA_HILOGE("PerUserSession::ShowKeyboard Aborted! index = -1 or clientInfo is nullptr");
            return ErrorCode::ERROR_CLIENT_NOT_FOUND;
        }

        if (imsCore[0] == nullptr) {
            IMSA_HILOGE("PerUserSession::ShowKeyboard Aborted! imsCore[%{public}d] is nullptr", index);
            return ErrorCode::ERROR_NULL_POINTER;
        }

        imsCore[0]->showKeyboard(clientInfo->channel, isShowKeyboard);

        SetCurrentClient(inputClient);
        return ErrorCode::NO_ERROR;
    }

    /*! hide keyboard
    \param inputClient the remote object handler of the input client.
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_IME_NOT_STARTED ime not started
    \return ErrorCode::ERROR_KBD_IS_NOT_SHOWING keyboard has not been showing
    \return ErrorCode::ERROR_CLIENT_NOT_FOUND the input client is not found
    \return ErrorCode::ERROR_KBD_HIDE_FAILED failed to hide keyboard
    \return other errors returned by binder driver
    */
    int PerUserSession::HideKeyboard(const sptr<IInputClient>& inputClient)
    {
        IMSA_HILOGI("PerUserSession::HideKeyboard");
        int index = GetImeIndex(inputClient);
        if (index == -1) {
            IMSA_HILOGE("PerUserSession::HideKeyboard Aborted! ErrorCode::ERROR_CLIENT_NOT_FOUND");
            return ErrorCode::ERROR_CLIENT_NOT_FOUND;
        }
        auto clientInfo = GetClientInfo(inputClient->AsObject());
        if (clientInfo == nullptr) {
            IMSA_HILOGE("PerUserSession::HideKeyboard GetClientInfo pointer nullptr");
        }
        if (imsCore[0] == nullptr) {
            IMSA_HILOGE("PerUserSession::HideKeyboard imsCore[index] is nullptr");
            return ErrorCode::ERROR_IME_NOT_STARTED;
        }

        bool ret = imsCore[0]->hideKeyboard(1);
        if (!ret) {
            IMSA_HILOGE("PerUserSession::HideKeyboard [imsCore->hideKeyboard] failed");
            return ErrorCode::ERROR_KBD_HIDE_FAILED;
        }

        return ErrorCode::NO_ERROR;
    }

    /*! Get the keyboard window height
    \param[out] retHeight the height of keyboard window showing or showed returned to caller
    \return ErrorCode
    */
    int PerUserSession::OnGetKeyboardWindowHeight(int &retHeight)
    {
        if (imsCore[lastImeIndex]) {
            int ret = imsCore[lastImeIndex]->getKeyboardWindowHeight(retHeight);
            if (ret != ErrorCode::NO_ERROR) {
                IMSA_HILOGE("getKeyboardWindowHeight return : %{public}s", ErrorCode::ToString(ret));
            }
            return ret;
        }
        IMSA_HILOGW("No IME is started [%{public}d]\n", userId_);
        return ErrorCode::ERROR_IME_NOT_STARTED;
    }

    /*! Get the current keyboard type
    \return return the pointer of the object of current keyboard type.
    \n null if no keyboard type supported by the current ime.
    \note The returned pointer should NOT be freed by the caller.
    */
    KeyboardType *PerUserSession::GetCurrentKeyboardType()
    {
        if (!inputMethodSetting || !currentIme[DEFAULT_IME]) {
            IMSA_HILOGI("Ime has not started ! [%{public}d]\n", userId_);
            return nullptr;
        }
        if (currentIme[DEFAULT_IME] == currentIme[SECURITY_IME]) {
            return nullptr;
        }
        int hashCode = inputMethodSetting->GetCurrentKeyboardType();  // To be checked.
        if (hashCode == -1) {
            std::vector<int> hashCodeList = inputMethodSetting->GetEnabledKeyboardTypes(currentIme[DEFAULT_IME]->mImeId);
            if (!hashCodeList.size()) {
                IMSA_HILOGE("Cannot find any keyboard types for the current ime [%{public}d]\n", userId_);
                return nullptr;
            }
            hashCode = hashCodeList[0];
        }

        for (int i = 0; i < (int)currentIme[DEFAULT_IME]->mTypes.size(); i++) {
            if (currentIme[DEFAULT_IME]->mTypes[i]->getHashCode() == hashCode) {
                return currentIme[DEFAULT_IME]->mTypes[i];
            }
        }
        return nullptr;
    }

    /*! Handle the situation a remote input client died\n
    It's called when a remote input client died
    \param the remote object handler of the input client died.
    */
    void PerUserSession::OnClientDied(sptr<IInputClient> remote)
    {
        IMSA_HILOGI("PerUserSession::OnClientDied Start...[%{public}d]\n", userId_);
        sptr<IInputClient> client = GetCurrentClient();
        if (client == nullptr) {
            IMSA_HILOGE("current client is nullptr");
            RemoveClient(remote->AsObject());
            return;
        }
        if (client->AsObject() == remote->AsObject()) {
            int ret = HideKeyboard(client);
            IMSA_HILOGI("hide keyboard ret: %{public}s", ErrorCode::ToString(ret));
        }
        RemoveClient(remote->AsObject());
    }

    /*! Handle the situation a input method service died\n
    It's called when an input method service died
    \param the remote object handler of input method service who died.
    */
    void PerUserSession::OnImsDied(sptr<IInputMethodCore> remote)
    {
        (void)remote;
        IMSA_HILOGI("Start...[%{public}d]\n", userId_);
        int index = 0;
        for (int i = 0; i < MAX_IME; i++) {
            if (imsCore[i] == remote) {
                index = i;
                break;
            }
        }
        ClearImeData(index);
        if (!IsRestartIme(index)) {
            IMSA_HILOGI("Restart ime over max num");
            return;
        }
        IMSA_HILOGI("IME died. Restart input method...[%{public}d]\n", userId_);
        const auto &ime = ParaHandle::GetDefaultIme(userId_);
        auto *parcel = new (std::nothrow) MessageParcel();
        if (parcel == nullptr) {
            IMSA_HILOGE("parcel is nullptr");
            return;
        }
        parcel->WriteString(ime);
        auto *msg = new (std::nothrow) Message(MSG_ID_START_INPUT_SERVICE, parcel);
        if (msg == nullptr) {
            IMSA_HILOGE("msg is nullptr");
            delete parcel;
            return;
        }
        usleep(MAX_RESET_WAIT_TIME);
        MessageHandler::Instance()->SendMessage(msg);
        IMSA_HILOGI("End...[%{public}d]\n", userId_);
    }

    /*! It's called when input method setting data in the system is changed
    \param key the name of setting item changed.
    \param value the value of setting item changed.
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_SETTING_SAME_VALUE the current value is same as the one in the system.
    */
    int PerUserSession::OnSettingChanged(const std::u16string& key, const std::u16string& value)
    {
        IMSA_HILOGI("Start...[%{public}d]\n", userId_);
        std::unique_lock<std::mutex> lock(mtx);
        if (!inputMethodSetting) {
            return ErrorCode::ERROR_NULL_POINTER;
        }
        std::u16string currentValue = inputMethodSetting->GetValue(key);

        IMSA_HILOGD("PerUserSession::OnSettingChanged key = %{public}s", Utils::ToStr8(key).c_str());
        IMSA_HILOGD("PerUserSession::OnSettingChanged value = %{public}s", Utils::ToStr8(value).c_str());
        IMSA_HILOGD("PerUserSession::OnSettingChanged currentValue = %{public}s", Utils::ToStr8(currentValue).c_str());

        if (currentValue == value) {
            IMSA_HILOGI("End...[%{public}d]\n", userId_);
            return ErrorCode::ERROR_SETTING_SAME_VALUE;
        }
        sptr<IInputClient> client = GetCurrentClient();
        if (key == InputMethodSetting::CURRENT_KEYBOARD_TYPE_TAG) {
            return OnCurrentKeyboardTypeChanged(DEFAULT_IME, value);
        } else if (key == InputMethodSetting::CURRENT_SYS_KEYBOARD_TYPE_TAG) {
            return OnCurrentKeyboardTypeChanged(SECURITY_IME, value);
        } else if (key == InputMethodSetting::CURRENT_INPUT_METHOD_TAG) {
            if (currentIme[DEFAULT_IME] == nullptr || value == currentIme[DEFAULT_IME]->mImeId) {
                return ErrorCode::NO_ERROR;
            }
            if (client != nullptr && GetImeIndex(client) == DEFAULT_IME) {
                needReshowClient = client;
                HideKeyboard(client);
            }
            StopInputMethod(DEFAULT_IME);
            currentIme[DEFAULT_IME] = nullptr;
            currentKbdIndex[DEFAULT_IME] = 0;
            inputMethodSetting->SetCurrentKeyboardType(-1);
        } else if (key == InputMethodSetting::ENABLED_INPUT_METHODS_TAG) {
            if (currentIme[DEFAULT_IME] != nullptr && currentIme[DEFAULT_IME] != currentIme[SECURITY_IME]
                && value.find(currentIme[DEFAULT_IME]->mImeId) == std::string::npos) {
                if (client != nullptr && GetImeIndex(client) == DEFAULT_IME) {
                    needReshowClient = client;
                    HideKeyboard(client);
                }
                StopInputMethod(DEFAULT_IME);
                currentIme[DEFAULT_IME] = nullptr;
                currentKbdIndex[DEFAULT_IME] = 0;
                inputMethodSetting->SetCurrentKeyboardType(-1);
            }
        }
        IMSA_HILOGI("End...[%{public}d]\n", userId_);
        return ErrorCode::NO_ERROR;
    }

    /*! Change current keyboard type.
    \param index it can be 0 or 1. 0 - default ime, 1 - security ime.
    \param value the hash code of keyboard type
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_SETTING_SAME_VALUE the current value is same as the one in the system.
    */
    int PerUserSession::OnCurrentKeyboardTypeChanged(int index, const std::u16string& value)
    {
        std::string str = Utils::ToStr8(value);
        int hashCode = std::atoi(str.c_str());
        if (hashCode == -1) {
            return ErrorCode::ERROR_SETTING_SAME_VALUE;
        }
        // switch within the current ime.
        if (index == SECURITY_IME || currentIme[DEFAULT_IME] == currentIme[SECURITY_IME]) {
            int num = currentKbdIndex[index];
            if (currentIme[index]->mTypes[num]->getHashCode() == hashCode) {
                return ErrorCode::ERROR_SETTING_SAME_VALUE;
            }
            for (int i = 0; i < (int)currentIme[index]->mTypes.size(); i++) {
                if (currentIme[index]->mTypes[i]->getHashCode() == hashCode) {
                    currentKbdIndex[index] = i;
                    break;
                }
            }
        } else {
            std::u16string imeId = currentIme[index]->mImeId;
            std::vector<int> currentKbdTypes = inputMethodSetting->GetEnabledKeyboardTypes(imeId);
            int num = currentKbdIndex[index];
            if (currentKbdTypes[num] == hashCode) {
                return ErrorCode::ERROR_SETTING_SAME_VALUE;
            }
            for (int i = 0; i < (int)currentKbdTypes.size(); i++) {
                if (currentKbdTypes[i] == hashCode) {
                    currentKbdIndex[index] = i;
                    break;
                }
            }
        }
        KeyboardType *type = GetKeyboardType(index, currentKbdIndex[index]);
        if (type) {
            sptr<IInputClient> client = GetCurrentClient();
            if (client != nullptr) {
                int ret = imsCore[index]->setKeyboardType(*type);
                if (ret != ErrorCode::NO_ERROR) {
                    IMSA_HILOGE("setKeyboardType ret: %{public}s [%{public}d]\n", ErrorCode::ToString(ret), userId_);
                }
            }
            if (imsCore[index] == imsCore[1 - index]) {
                inputMethodSetting->SetCurrentKeyboardType(type->getHashCode());
                inputMethodSetting->SetCurrentSysKeyboardType(type->getHashCode());
                currentKbdIndex[1 - index] = currentKbdIndex[index];
            } else if (index == DEFAULT_IME) {
                inputMethodSetting->SetCurrentKeyboardType(type->getHashCode());
            } else {
                inputMethodSetting->SetCurrentSysKeyboardType(type->getHashCode());
            }
        }
        return ErrorCode::NO_ERROR;
    }

    /*! Hide current keyboard
    \param flag the flag to hide keyboard.
    */
    int PerUserSession::OnHideKeyboardSelf(int flags)
    {
        IMSA_HILOGW("PerUserSession::OnHideKeyboardSelf");
        (void)flags;
        sptr<IInputClient> client = GetCurrentClient();
        if (client == nullptr) {
            IMSA_HILOGE("current client is nullptr");
            return ErrorCode::ERROR_CLIENT_NOT_FOUND;
        }
        return HideKeyboard(client);
    }

    int PerUserSession::OnShowKeyboardSelf()
    {
        IMSA_HILOGI("PerUserSession::OnShowKeyboardSelf");
        sptr<IInputClient> client = GetCurrentClient();
        if (client == nullptr) {
            IMSA_HILOGE("current client is nullptr");
            return ErrorCode::ERROR_CLIENT_NOT_FOUND;
        }
        return ShowKeyboard(client, true);
    }

    /*! Switch to next keyboard type
    */
    void PerUserSession::OnAdvanceToNext()
    {
        sptr<IInputClient> client = GetCurrentClient();
        if (client == nullptr) {
            IMSA_HILOGE("current client is nullptr");
            return;
        }
        int index = GetImeIndex(client);
        if (index == -1) {
            IMSA_HILOGW("%{public}s [%{public}d]\n", ErrorCode::ToString(ErrorCode::ERROR_CLIENT_NOT_FOUND), userId_);
            return;
        }
        int size = 0;
        if (index == SECURITY_IME || currentIme[DEFAULT_IME] == currentIme[SECURITY_IME]) {
            size = currentIme[index]->mTypes.size();
        } else {
            std::u16string imeId = currentIme[index]->mImeId;
            std::vector<int> currentKbdTypes = inputMethodSetting->GetEnabledKeyboardTypes(imeId);
            size = currentKbdTypes.size();
        }
        if (size < MIN_IME) {
            IMSA_HILOGW("No next keyboard is available. [%{public}d]\n", userId_);
            return;
        }

        int num = currentKbdIndex[index]+1;
        if (size) {
            num %= size;
        }
        KeyboardType *type = GetKeyboardType(index, num);
        if (type == nullptr) {
            IMSA_HILOGW("No next keyboard is available. [%{public}d]\n", userId_);
            return;
        }
        InputMethodSetting tmpSetting;
        if (imsCore[index] == imsCore[1 - index]) {
            tmpSetting.SetCurrentKeyboardType(type->getHashCode());
            tmpSetting.SetCurrentSysKeyboardType(type->getHashCode());
        }
        else if (index == DEFAULT_IME) {
            tmpSetting.SetCurrentKeyboardType(type->getHashCode());
        } else {
            tmpSetting.SetCurrentSysKeyboardType(type->getHashCode());
        }
        Platform::Instance()->SetInputMethodSetting(userId_, tmpSetting);
    }

    /*! Set display mode
    \param mode the display mode of soft keyboard UI.
    \n 0 - part screen mode, 1 - full screen mode
    */
    void PerUserSession::OnSetDisplayMode(int mode)
    {
        currentDisplayMode = mode;
        sptr<IInputClient> client = GetCurrentClient();
        if (client == nullptr) {
            IMSA_HILOGE("current client is nullptr");
            return;
        }
        auto clientInfo = GetClientInfo(client->AsObject());
        if (clientInfo == nullptr) {
            IMSA_HILOGE("%{public}s [%{public}d]\n", ErrorCode::ToString(ErrorCode::ERROR_CLIENT_NOT_FOUND), userId_);
            return;
        }
        int ret = clientInfo->client->setDisplayMode(mode);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("setDisplayMode return : %{public}s [%{public}d]\n", ErrorCode::ToString(ret), userId_);
        }
    }

    /*! Restart input method service
    \param index it can be DEFAULT_IME or SECURITY_IME
    \param imeId the id of the input method service going to restart
    */
    void PerUserSession::OnRestartIms(int index, const std::u16string& imeId)
    {
        if (index < 0 || index >= MAX_IME) {
            return;
        }
        IMSA_HILOGI("Start...[%{public}d]\n", userId_);
        if (currentIme[index] && currentIme[index]->mImeId == imeId) {
            int ret = StartInputMethod(index);
            if (needReshowClient && GetImeIndex(needReshowClient) == index) {
                if (ret == ErrorCode::NO_ERROR) {
                    ShowKeyboard(needReshowClient, true);
                }
                needReshowClient = nullptr;
            }
        }
        IMSA_HILOGI("End...[%{public}d]\n", userId_);
    }

    /*! It's called when this user is locked
    */
    void PerUserSession::OnUserLocked()
    {
        IMSA_HILOGI("PerUserSession::OnUserLocked");
        if (userState == UserState::USER_STATE_STARTED) {
            IMSA_HILOGI("End...[%{public}d]\n", userId_);
            return;
        }
        userState = UserState::USER_STATE_STARTED;
        // hide current keyboard
        sptr<IInputClient> client = GetCurrentClient();
        if (client != nullptr) {
            HideKeyboard(client);
        }
        for (int i = 0; i < MIN_IME; i++) {
            StopInputMethod(i);
            currentIme[i] = nullptr;
        }
        // disconnect all clients.
        for (auto it = mapClients.begin(); it != mapClients.end();) {
            auto clientInfo = it->second;
            if (clientInfo != nullptr) {
                clientInfo->client->AsObject()->RemoveDeathRecipient(clientInfo->deathRecipient);
                int ret = clientInfo->client->onInputReleased(0);
                if (ret != ErrorCode::NO_ERROR) {
                    IMSA_HILOGE("2-onInputReleased return : %{public}s", ErrorCode::ToString(ret));
                }
            }
            IMSA_HILOGD("erase client..\n");
            it = mapClients.erase(it);
        }
        mapClients.clear();

        // reset values
        inputMethodSetting = nullptr;
        SetCurrentClient(nullptr);
        needReshowClient = nullptr;
    }

    /*! Get keyboard type
    \param imeIndex it can be 0 or 1.  0 - default ime, 1 - security ime
    \param typeIndex the index of keyboard type.
    \return a KeyboardType pointer when it's found.
    \return null when it's not found.
    \note The returned pointer should not be freed by caller.
    */
    KeyboardType *PerUserSession::GetKeyboardType(int imeIndex, int typeIndex)
    {
        if (typeIndex < 0) {
            return nullptr;
        }
        if (imeIndex == SECURITY_IME || currentIme[DEFAULT_IME] == currentIme[SECURITY_IME]) {
            if (typeIndex >= (int)currentIme[imeIndex]->mTypes.size()) {
                return nullptr;
            }
            return currentIme[imeIndex]->mTypes[typeIndex];
        } else {
            std::u16string imeId = currentIme[imeIndex]->mImeId;
            std::vector<int> currentKbdTypes = inputMethodSetting->GetEnabledKeyboardTypes(imeId);
            int size = currentKbdTypes.size();
            if (typeIndex >= size) {
                return nullptr;
            }
            int hashCode = currentKbdTypes[typeIndex];
            for (int i = 0; i < (int)currentIme[imeIndex]->mTypes.size(); i++) {
                if (currentIme[imeIndex]->mTypes[i]->getHashCode() == hashCode) {
                    return currentIme[imeIndex]->mTypes[i];
                }
            }
        }
        return nullptr;
    }

    /*! Reset current keyboard type
    \param imeIndex it can be 0 or 1. 0 - default ime, 1 - security ime
    */
    void PerUserSession::ResetCurrentKeyboardType(int imeIndex)
    {
        if (imeIndex < 0 || imeIndex > 1) {
            return;
        }
        currentKbdIndex[imeIndex] = 0;
        int hashCode = 0;
        if (imeIndex == DEFAULT_IME) {
            hashCode = inputMethodSetting->GetCurrentKeyboardType();
        } else {
            hashCode = inputMethodSetting->GetCurrentSysKeyboardType();
        }
        KeyboardType *type = nullptr;
        if (hashCode == -1) {
            type  = GetKeyboardType(imeIndex, currentKbdIndex[imeIndex]);
        } else {
            bool flag = false;
            if (imeIndex == SECURITY_IME || currentIme[DEFAULT_IME] == currentIme[SECURITY_IME]) {
                for (int i = 0; i < (int)currentIme[imeIndex]->mTypes.size(); i++) {
                    if (currentIme[imeIndex]->mTypes[i]->getHashCode() == hashCode) {
                        currentKbdIndex[imeIndex] = i;
                        flag = true;
                        break;
                    }
                }
            } else {
                std::vector<int> hashCodeList = inputMethodSetting->GetEnabledKeyboardTypes(currentIme[imeIndex]->mImeId);
                for (int i = 0; i < (int)hashCodeList.size(); i++) {
                    if (hashCode == hashCodeList[i]) {
                        currentKbdIndex[imeIndex] = i;
                        flag = true;
                        break;
                    }
                }
            }
            if (!flag) {
                IMSA_HILOGW("The current keyboard type is not found in the current IME. Reset it!");
                type = GetKeyboardType(imeIndex, currentKbdIndex[imeIndex]);
            } else if (imsCore[imeIndex] == imsCore[1 - imeIndex]) {
                currentKbdIndex[1 - imeIndex] = currentKbdIndex[imeIndex];
            }
        }
        if (type) {
            InputMethodSetting tmpSetting;
            if (imsCore[imeIndex] == imsCore[1 - imeIndex]) {
                inputMethodSetting->SetCurrentKeyboardType(type->getHashCode());
                inputMethodSetting->SetCurrentSysKeyboardType(type->getHashCode());
                currentKbdIndex[1 - imeIndex] = currentKbdIndex[imeIndex];
                tmpSetting.SetCurrentKeyboardType(type->getHashCode());
                tmpSetting.SetCurrentSysKeyboardType(type->getHashCode());
            } else if (imeIndex == DEFAULT_IME) {
                tmpSetting.SetCurrentKeyboardType(type->getHashCode());
                inputMethodSetting->SetCurrentKeyboardType(type->getHashCode());
            } else {
                tmpSetting.SetCurrentSysKeyboardType(type->getHashCode());
                inputMethodSetting->SetCurrentSysKeyboardType(type->getHashCode());
            }
            Platform::Instance()->SetInputMethodSetting(userId_, tmpSetting);
        }
    }

    /*! Get ime index for the input client
    \param inputClient the remote object handler of an input client.
    \return 0 - default ime
    \return 1 - security ime
    \return -1 - input client is not found
    */
    int PerUserSession::GetImeIndex(const sptr<IInputClient>& inputClient)
    {
        if (inputClient == nullptr) {
            IMSA_HILOGW("PerUserSession::GetImeIndex inputClient is nullptr");
            return -1;
        }

        auto clientInfo = GetClientInfo(inputClient->AsObject());
        if (clientInfo == nullptr) {
            IMSA_HILOGW("PerUserSession::GetImeIndex clientInfo is nullptr");
            return -1;
        }

        if (clientInfo->attribute.GetSecurityFlag()) {
            return SECURITY_IME;
        }
        return DEFAULT_IME;
    }

    /*! Copy session data from one IME to another IME
    \param imeIndex it can be 0 or 1.
    \n 0 - default ime, 1 - security ime
    */
    void PerUserSession::CopyInputMethodService(int imeIndex)
    {
        imsCore[imeIndex] = imsCore[1 - imeIndex];
        localControlChannel[imeIndex] = localControlChannel[1 - imeIndex];
        inputControlChannel[imeIndex] = inputControlChannel[1 - imeIndex];
        inputMethodToken[imeIndex] = inputMethodToken[1 - imeIndex];
        currentKbdIndex[imeIndex] = currentKbdIndex[1 - imeIndex];
        int hashCode[2];
        hashCode[0] = inputMethodSetting->GetCurrentKeyboardType();
        hashCode[1] = inputMethodSetting->GetCurrentSysKeyboardType();
        if (hashCode[imeIndex] != hashCode[1 - imeIndex]) {
            hashCode[imeIndex] = hashCode[1 - imeIndex];
            inputMethodSetting->SetCurrentKeyboardType(hashCode[0]);
            inputMethodSetting->SetCurrentSysKeyboardType(hashCode[1]);

            InputMethodSetting tmpSetting;
            tmpSetting.ClearData();
            tmpSetting.SetCurrentKeyboardType(hashCode[0]);
            tmpSetting.SetCurrentSysKeyboardType(hashCode[1]);
            Platform::Instance()->SetInputMethodSetting(userId_, tmpSetting);
        }
    }

    /*! Get ClientInfo
    \param inputClient the IRemoteObject remote handler of given input client
    \return a pointer of ClientInfo if client is found
    \n      null if client is not found
    \note the clientInfo pointer should not be freed by caller
    */
    std::shared_ptr<ClientInfo> PerUserSession::GetClientInfo(sptr<IRemoteObject> inputClient)
    {
        if (inputClient == nullptr) {
            IMSA_HILOGE("PerUserSession::GetClientInfo inputClient is nullptr");
            return nullptr;
        }
        auto it = mapClients.find(inputClient);
        if (it == mapClients.end()) {
            IMSA_HILOGE("PerUserSession::GetClientInfo client not found");
            return nullptr;
        }
        return it->second;
    }

    bool PerUserSession::StartInputService()
    {
        IMSA_HILOGE("PerUserSession::StartInputService");
        sptr<AAFwk::IAbilityManager> ams = GetAbilityManagerService();
        if (!ams) {
            return false;
        }
        AAFwk::Want want;
        want.SetAction("action.system.inputmethod");
        want.SetElementName("com.example.kikakeyboard", "com.example.kikakeyboard.ServiceExtAbility");
        int32_t result = ams->StartAbility(want);
        if (result) {
            IMSA_HILOGE("PerUserSession::StartInputService fail. result = %{public}d", result);
            return false;
        }
        return true;
    }

    sptr<AAFwk::IAbilityManager> PerUserSession::GetAbilityManagerService()
    {
        IMSA_HILOGE("GetAbilityManagerService start");
        sptr<IRemoteObject> abilityMsObj =
        OHOS::DelayedSingleton<AAFwk::SaMgrClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
        if (!abilityMsObj) {
            IMSA_HILOGE("failed to get ability manager service");
            return nullptr;
        }
        return iface_cast<AAFwk::IAbilityManager>(abilityMsObj);
    }

    /*! Prepare input. Called by an input client.
    \n Run in work thread of this user
    \param the parameters from remote client
    \return ErrorCode
    */
    int32_t PerUserSession::OnPrepareInput(const ClientInfo &clientInfo)
    {
        IMSA_HILOGI("PerUserSession::OnPrepareInput Start\n");
        int ret = AddClient(clientInfo.client->AsObject(), clientInfo);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("PerUserSession::OnPrepareInput %{public}s", ErrorCode::ToString(ret));
            return ErrorCode::ERROR_ADD_CLIENT_FAILED;
        }
        SendAgentToSingleClient(clientInfo);
        return ErrorCode::NO_ERROR;
    }

    void PerUserSession::SendAgentToSingleClient(const ClientInfo &clientInfo)
    {
        IMSA_HILOGI("PerUserSession::SendAgentToSingleClient");
        if (imsAgent == nullptr) {
            IMSA_HILOGI("PerUserSession::SendAgentToSingleClient imsAgent is nullptr");
            CreateComponentFailed(userId_, ErrorCode::ERROR_NULL_POINTER);
            return;
        }
        clientInfo.client->onInputReady(imsAgent);
    }

    /*! Release input. Called by an input client.
    \n Run in work thread of this user
    \param the parameters from remote client
    \return ErrorCode
    */
    int32_t PerUserSession::OnReleaseInput(sptr<IInputClient> client)
    {
        IMSA_HILOGI("PerUserSession::OnReleaseInput Start\n");
        if (imsCore[0] == nullptr) {
            return ErrorCode::ERROR_IME_NOT_AVAILABLE;
        }
        imsCore[0]->SetClientState(false);
        HideKeyboard(client);
        RemoveClient(client->AsObject());
        IMSA_HILOGI("PerUserSession::OnReleaseInput End...[%{public}d]\n", userId_);
        return ErrorCode::NO_ERROR;
    }

    /*! Start input. Called by an input client.
    \n Run in work thread of this user
    \param the parameters from remote client
    \return ErrorCode
    */
    int32_t PerUserSession::OnStartInput(sptr<IInputClient> client, bool isShowKeyboard)
    {
        IMSA_HILOGI("PerUserSession::OnStartInput");
        if (imsCore[0] == nullptr) {
            return ErrorCode::ERROR_IME_NOT_AVAILABLE;
        }
        imsCore[0]->SetClientState(true);
        return ShowKeyboard(client, isShowKeyboard);
    }

    int32_t PerUserSession::OnSetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent)
    {
        IMSA_HILOGI("PerUserSession::SetCoreAndAgent Start\n");
        if (core == nullptr || agent == nullptr) {
            IMSA_HILOGE("PerUserSession::SetCoreAndAgent core or agent nullptr");
            return ErrorCode::ERROR_EX_NULL_POINTER;
        }
        imsCore[0] = core;
        if (imsDeathRecipient != nullptr) {
            imsDeathRecipient->SetDeathRecipient([this, core](const wptr<IRemoteObject> &) { this->OnImsDied(core); });
            bool ret = core->AsObject()->AddDeathRecipient(imsDeathRecipient);
            IMSA_HILOGI("Add death recipient %{public}s", ret ? "success" : "failed");
        }
        imsAgent = agent;
        InitInputControlChannel();
        SendAgentToAllClients();
        return ErrorCode::NO_ERROR;
    }

    void PerUserSession::SendAgentToAllClients()
    {
        IMSA_HILOGI("PerUserSession::SendAgentToAllClients");
        if (imsAgent == nullptr) {
            IMSA_HILOGE("PerUserSession::SendAgentToAllClients imsAgent is nullptr");
            return;
        }

        for (auto it = mapClients.begin(); it != mapClients.end(); ++it) {
            auto clientInfo = it->second;
            if (clientInfo != nullptr) {
                clientInfo->client->onInputReady(imsAgent);
            }
        }
    }

    void PerUserSession::InitInputControlChannel()
    {
        IMSA_HILOGI("PerUserSession::InitInputControlChannel");
        sptr<IInputControlChannel> inputControlChannel = new InputControlChannelStub(userId_);
        int ret = imsCore[0]->InitInputControlChannel(inputControlChannel);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGI("PerUserSession::InitInputControlChannel fail %{public}s", ErrorCode::ToString(ret));
        }
    }

    /*! Stop input. Called by an input client.
    \n Run in work thread of this user
    \param the parameters from remote client
    \return ErrorCode
    */
    int32_t PerUserSession::OnStopInput(sptr<IInputClient> client)
    {
        IMSA_HILOGI("PerUserSession::OnStopInput");
        return HideKeyboard(client);
    }

    void PerUserSession::StopInputService(std::string imeId)
    {
        IMSA_HILOGI("PerUserSession::StopInputService");
        if (imsCore[0] == nullptr) {
            IMSA_HILOGE("imsCore[0] is nullptr");
            return;
        }
        IMSA_HILOGI("Remove death recipient");
        imsCore[0]->AsObject()->RemoveDeathRecipient(imsDeathRecipient);
        imsCore[0]->StopInputService(imeId);
    }

    bool PerUserSession::IsRestartIme(uint32_t index)
    {
        IMSA_HILOGI("PerUserSession::IsRestartIme");
        std::lock_guard<std::mutex> lock(resetLock);
        auto now = time(nullptr);
        if (difftime(now, manager[index].last) > IME_RESET_TIME_OUT) {
            manager[index] = { 0, now };
        }
        ++manager[index].num;
        return manager[index].num <= MAX_RESTART_NUM;
    }

    void PerUserSession::ResetImeError(uint32_t index)
    {
        IMSA_HILOGI("PerUserSession::ResetImeError index = %{public}d", index);
        std::lock_guard<std::mutex> lock(resetLock);
        manager[index] = { 0, 0 };
    }

    void PerUserSession::ClearImeData(uint32_t index)
    {
        IMSA_HILOGI("Clear ime...index = %{public}d", index);
        if (imsCore[index] != nullptr) {
            imsCore[index]->AsObject()->RemoveDeathRecipient(imsDeathRecipient);
            imsCore[index] = nullptr;
        }
        inputControlChannel[index] = nullptr;
        localControlChannel[index] = nullptr;
        inputMethodToken[index] = nullptr;
    }

    void PerUserSession::SetCurrentClient(sptr<IInputClient> client)
    {
        IMSA_HILOGI("set current client");
        std::lock_guard<std::mutex> lock(clientLock_);
        currentClient = client;
    }

    sptr<IInputClient> PerUserSession::GetCurrentClient()
    {
        std::lock_guard<std::mutex> lock(clientLock_);
        return currentClient;
    }

    int32_t PerUserSession::OnInputMethodSwitched(const Property &property, const SubProperty &subProperty)
    {
        IMSA_HILOGI("PerUserSession::OnInputMethodSwitched");
        for (const auto &client : mapClients) {
            auto clientInfo = client.second;
            if (clientInfo == nullptr) {
                IMSA_HILOGD("PerUserSession::clientInfo is nullptr");
                continue;
            }
            int32_t ret = clientInfo->client->OnSwitchInput(property, subProperty);
            if (ret != ErrorCode::NO_ERROR) {
                IMSA_HILOGE("PerUserSession::OnSwitchInput failed, ret %{public}d", ret);
                return ret;
            }
        }
        if (imsCore[0] == nullptr) {
            IMSA_HILOGE("imsCore is nullptr");
            return ErrorCode::ERROR_EX_NULL_POINTER;
        }
        int32_t ret = imsCore[0]->SetSubtype(subProperty);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("PerUserSession::SetSubtype failed, ret %{public}d", ret);
            return ret;
        }
        return ErrorCode::NO_ERROR;
    }
} // namespace MiscServices
} // namespace OHOS
