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
#include "input_method_agent_proxy.h"
#include "input_method_core_proxy.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "para_handle.h"
#include "parcel.h"
#include "iservice_registry.h"
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
            std::lock_guard<std::recursive_mutex> lock(mtx);
            switch (msg->msgId_) {
                case MSG_ID_HIDE_KEYBOARD_SELF: {
                    int flag = msg->msgContent_->ReadInt32();
                    OnHideKeyboardSelf(flag);
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

    int PerUserSession::AddClient(sptr<IRemoteObject> inputClient, const ClientInfo &clientInfo)
    {
        IMSA_HILOGD("PerUserSession::AddClient");
        std::lock_guard<std::recursive_mutex> lock(mtx);
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

    void PerUserSession::UpdateClient(sptr<IRemoteObject> inputClient, bool isShowKeyboard)
    {
        IMSA_HILOGD("PerUserSession::start");
        std::lock_guard<std::recursive_mutex> lock(mtx);
        auto it = mapClients.find(inputClient);
        if (it == mapClients.end()) {
            IMSA_HILOGE("PerUserSession::client not found");
            return;
        }
        it->second->isShowKeyBoard = isShowKeyboard;
    }

    /*! Remove an input client
    \param inputClient remote object handler of the input client
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_CLIENT_NOT_FOUND client is not found
    */
    void PerUserSession::RemoveClient(sptr<IRemoteObject> inputClient)
    {
        IMSA_HILOGD("PerUserSession::RemoveClient");
        auto client = GetCurrentClient();
        if (client != nullptr && client->AsObject() == inputClient) {
            SetCurrentClient(nullptr);
        }
        std::lock_guard<std::recursive_mutex> lock(mtx);
        auto it = mapClients.find(inputClient);
        if (it == mapClients.end()) {
            IMSA_HILOGE("PerUserSession::RemoveClient client not found");
            return;
        }
        auto info = it->second;
        info->client->AsObject()->RemoveDeathRecipient(info->deathRecipient);
        mapClients.erase(it);
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
        IMSA_HILOGD("PerUserSession::ShowKeyboard");
        auto clientInfo = GetClientInfo(inputClient->AsObject());
        int index = GetImeIndex(inputClient);
        if (index == -1 || clientInfo == nullptr) {
            IMSA_HILOGE("PerUserSession::ShowKeyboard Aborted! index = -1 or clientInfo is nullptr");
            return ErrorCode::ERROR_CLIENT_NOT_FOUND;
        }
        sptr<IInputMethodCore> core = GetImsCore(DEFAULT_IME);
        if (core == nullptr) {
            IMSA_HILOGE("PerUserSession::ShowKeyboard Aborted! imsCore[%{public}d] is nullptr", index);
            return ErrorCode::ERROR_NULL_POINTER;
        }

        auto subProperty = GetCurrentSubProperty();
        int32_t ret = core->showKeyboard(clientInfo->channel, isShowKeyboard, subProperty);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("PerUserSession::showKeyboard failed ret: %{public}d", ret);
            return ErrorCode::ERROR_KBD_SHOW_FAILED;
        }
        UpdateClient(inputClient->AsObject(), isShowKeyboard);
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
        IMSA_HILOGD("PerUserSession::HideKeyboard");
        sptr<IInputMethodCore> core = GetImsCore(DEFAULT_IME);
        if (core == nullptr) {
            IMSA_HILOGE("PerUserSession::HideKeyboard imsCore is nullptr");
            return ErrorCode::ERROR_IME_NOT_STARTED;
        }
        UpdateClient(inputClient->AsObject(), false);
        bool ret = core->hideKeyboard(1);
        if (!ret) {
            IMSA_HILOGE("PerUserSession::HideKeyboard [imsCore->hideKeyboard] failed");
            return ErrorCode::ERROR_KBD_HIDE_FAILED;
        }
        return ErrorCode::NO_ERROR;
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
            sptr<IInputMethodCore> core = GetImsCore(i);
            if (core == remote) {
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
        IMSA_HILOGD("End...[%{public}d]\n", userId_);
    }

    /*! Hide current keyboard
    \param flag the flag to hide keyboard.
    */
    int PerUserSession::OnHideKeyboardSelf(int flags)
    {
        IMSA_HILOGD("PerUserSession::OnHideKeyboardSelf");
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
        IMSA_HILOGD("PerUserSession::OnShowKeyboardSelf");
        sptr<IInputClient> client = GetCurrentClient();
        if (client == nullptr) {
            IMSA_HILOGE("current client is nullptr");
            return ErrorCode::ERROR_CLIENT_NOT_FOUND;
        }
        return ShowKeyboard(client, true);
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

    /*! Get ClientInfo
    \param inputClient the IRemoteObject remote handler of given input client
    \return a pointer of ClientInfo if client is found
    \n      null if client is not found
    \note the clientInfo pointer should not be freed by caller
    */
    std::shared_ptr<ClientInfo> PerUserSession::GetClientInfo(sptr<IRemoteObject> inputClient)
    {
        std::lock_guard<std::recursive_mutex> lock(mtx);
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

    /*! Prepare input. Called by an input client.
    \n Run in work thread of this user
    \param the parameters from remote client
    \return ErrorCode
    */
    int32_t PerUserSession::OnPrepareInput(const ClientInfo &clientInfo)
    {
        IMSA_HILOGD("PerUserSession::OnPrepareInput Start\n");
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
        IMSA_HILOGD("PerUserSession::SendAgentToSingleClient");
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
        IMSA_HILOGI("PerUserSession::OnReleaseInput Start zll");
        RemoveClient(client->AsObject());
        auto ret = HideKeyboard(client);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("failed to hide keyboard ret %{public}d", ret);
            return ret;
        }
        IMSA_HILOGD("PerUserSession::OnReleaseInput End...[%{public}d]\n", userId_);
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
        return ShowKeyboard(client, isShowKeyboard);
    }

    int32_t PerUserSession::OnSetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent)
    {
        IMSA_HILOGD("PerUserSession::SetCoreAndAgent Start\n");
        if (core == nullptr || agent == nullptr) {
            IMSA_HILOGE("PerUserSession::SetCoreAndAgent core or agent nullptr");
            return ErrorCode::ERROR_EX_NULL_POINTER;
        }
        SetImsCore(DEFAULT_IME, core);
        if (imsDeathRecipient != nullptr) {
            imsDeathRecipient->SetDeathRecipient([this, core](const wptr<IRemoteObject> &) { this->OnImsDied(core); });
            bool ret = core->AsObject()->AddDeathRecipient(imsDeathRecipient);
            IMSA_HILOGI("Add death recipient %{public}s", ret ? "success" : "failed");
        }
        imsAgent = agent;
        InitInputControlChannel();
        SendAgentToAllClients();
        auto client = GetCurrentClient();
        if (client != nullptr) {
            auto it = mapClients.find(client->AsObject());
            if (it != mapClients.end()) {
                IMSA_HILOGI("PerUserSession::Bind IMC to IMA");
                OnStartInput(it->second->client, it->second->isShowKeyBoard);
            }
        }
        return ErrorCode::NO_ERROR;
    }

    void PerUserSession::SendAgentToAllClients()
    {
        IMSA_HILOGD("PerUserSession::SendAgentToAllClients");
        std::lock_guard<std::recursive_mutex> lock(mtx);
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
        IMSA_HILOGD("PerUserSession::InitInputControlChannel");
        sptr<IInputControlChannel> inputControlChannel = new InputControlChannelStub(userId_);
        sptr<IInputMethodCore> core = GetImsCore(DEFAULT_IME);
        if (core == nullptr) {
            IMSA_HILOGE("PerUserSession::InitInputControlChannel core is nullptr");
            return;
        }
        int ret = core->InitInputControlChannel(inputControlChannel);
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
        IMSA_HILOGD("PerUserSession::OnStopInput");
        return HideKeyboard(client);
    }

    void PerUserSession::StopInputService(std::string imeId)
    {
        IMSA_HILOGI("PerUserSession::StopInputService");
        sptr<IInputMethodCore> core = GetImsCore(DEFAULT_IME);
        if (core == nullptr) {
            IMSA_HILOGE("imsCore[0] is nullptr");
            return;
        }
        IMSA_HILOGI("Remove death recipient");
        core->AsObject()->RemoveDeathRecipient(imsDeathRecipient);
        core->StopInputService(imeId);
    }

    bool PerUserSession::IsRestartIme(uint32_t index)
    {
        IMSA_HILOGD("PerUserSession::IsRestartIme");
        std::lock_guard<std::mutex> lock(resetLock);
        auto now = time(nullptr);
        if (difftime(now, manager[index].last) > IME_RESET_TIME_OUT) {
            manager[index] = { 0, now };
        }
        ++manager[index].num;
        return manager[index].num <= MAX_RESTART_NUM;
    }

    void PerUserSession::ClearImeData(uint32_t index)
    {
        IMSA_HILOGI("Clear ime...index = %{public}d", index);
        sptr<IInputMethodCore> core = GetImsCore(index);
        if (core != nullptr) {
            core->AsObject()->RemoveDeathRecipient(imsDeathRecipient);
            SetImsCore(index, nullptr);
        }
        inputControlChannel[index] = nullptr;
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
        IMSA_HILOGD("PerUserSession::OnInputMethodSwitched");
        std::lock_guard<std::recursive_mutex> lock(mtx);
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
        if (subProperty.id != currentSubProperty.id) {
            SetCurrentSubProperty(subProperty);
            return ErrorCode::NO_ERROR;
        }
        SetCurrentSubProperty(subProperty);
        sptr<IInputMethodCore> core = GetImsCore(DEFAULT_IME);
        if (core == nullptr) {
            IMSA_HILOGE("imsCore is nullptr");
            return ErrorCode::ERROR_EX_NULL_POINTER;
        }
        int32_t ret = core->SetSubtype(subProperty);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("PerUserSession::SetSubtype failed, ret %{public}d", ret);
            return ret;
        }
        return ErrorCode::NO_ERROR;
    }

    SubProperty PerUserSession::GetCurrentSubProperty()
    {
        IMSA_HILOGD("PerUserSession::GetCurrentSubProperty");
        std::lock_guard<std::mutex> lock(propertyLock_);
        return currentSubProperty;
    }

    void PerUserSession::SetCurrentSubProperty(const SubProperty &subProperty)
    {
        IMSA_HILOGD("PerUserSession::SetCurrentSubProperty");
        std::lock_guard<std::mutex> lock(propertyLock_);
        currentSubProperty = subProperty;
    }

    sptr<IInputMethodCore> PerUserSession::GetImsCore(int32_t index)
    {
        std::lock_guard<std::mutex> lock(imsCoreLock_);
        if (!IsValid(index)) {
            return nullptr;
        }
        return imsCore[index];
    }

    void PerUserSession::SetImsCore(int32_t index, sptr<IInputMethodCore> core)
    {
        std::lock_guard<std::mutex> lock(imsCoreLock_);
        if (!IsValid(index)) {
            return;
        }
        imsCore[index] = core;
    }
} // namespace MiscServices
} // namespace OHOS
