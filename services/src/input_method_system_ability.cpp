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

#include "input_method_system_ability.h"

#include <global.h>
#include <utils.h>
#include <key_event.h>

#include "../adapter/keyboard/keyboard_event.h"
#include "ability_connect_callback_proxy.h"
#include "ability_manager_interface.h"
#include "application_info.h"
#include "bundle_mgr_proxy.h"
#include "common_event_support.h"
#include "errors.h"
#include "global.h"
#include "im_common_event_manager.h"
#include "input_method_status.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "message_handler.h"
#include "os_account_manager.h"
#include "para_handle.h"
#include "resource_manager.h"
#include "sa_mgr_client.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "ui_service_mgr_client.h"

namespace OHOS {
namespace MiscServices {
    using namespace MessageID;
    using namespace AccountSA;
    REGISTER_SYSTEM_ABILITY_BY_ID(InputMethodSystemAbility, INPUT_METHOD_SYSTEM_ABILITY_ID, true);
    const std::int32_t INIT_INTERVAL = 10000L;
    const std::int32_t MAIN_USER_ID = 100;

    std::shared_ptr<AppExecFwk::EventHandler> InputMethodSystemAbility::serviceHandler_;

    /**
     * constructor
     * @param systemAbilityId
     * @param runOnCreate
     */
    InputMethodSystemAbility::InputMethodSystemAbility(int32_t systemAbilityId, bool runOnCreate)
        : SystemAbility(systemAbilityId, runOnCreate), state_(ServiceRunningState::STATE_NOT_START)
    {
    }

    /**
     * constructor
     */
    InputMethodSystemAbility::InputMethodSystemAbility() : state_(ServiceRunningState::STATE_NOT_START)
    {
    }

    /**
     * Destructor
     */
    InputMethodSystemAbility::~InputMethodSystemAbility()
    {
        if (workThreadHandler.joinable()) {
            workThreadHandler.join();
        }
        userSessions.clear();
        std::map<int32_t, PerUserSetting*>::const_iterator it1;
        for (it1 = userSettings.cbegin(); it1 != userSettings.cend();) {
            PerUserSetting *setting = it1->second;
            it1 = userSettings.erase(it1);
            delete setting;
            setting = nullptr;
        }
        userSettings.clear();
        std::map<int32_t, MessageHandler*>::const_iterator it2;
        for (it2 = msgHandlers.cbegin(); it2 != msgHandlers.cend();) {
            MessageHandler *handler = it2->second;
            it2 = msgHandlers.erase(it2);
            delete handler;
            handler = nullptr;
        }
        msgHandlers.clear();
    }

    void InputMethodSystemAbility::OnStart()
    {
        IMSA_HILOGI("InputMethodSystemAbility::OnStart.");
        if (state_ == ServiceRunningState::STATE_RUNNING) {
            IMSA_HILOGI("ImsaService is already running.");
            return;
        }
        Initialize();
        InitServiceHandler();
        if (Init() != ErrorCode::NO_ERROR) {
            auto callback = [=]() { Init(); };
            serviceHandler_->PostTask(callback, INIT_INTERVAL);
            IMSA_HILOGE("Init failed. Try again 10s later");
            return;
        }
        InitHiTrace();
        InputmethodTrace tracer("InputMethodController Attach trace.");
        InputmethodDump::GetInstance().AddDumpAllMethod(
            std::bind(&InputMethodSystemAbility::DumpAllMethod, this, std::placeholders::_1));
        IMSA_HILOGI("Start ImsaService ErrorCode::NO_ERROR.");
        return;
    }

    int InputMethodSystemAbility::Dump(int fd, const std::vector<std::u16string> &args)
    {
        IMSA_HILOGI("InputMethodSystemAbility::Dump");
        std::vector<std::string> argsStr;
        for (auto item : args) {
            argsStr.emplace_back(Str16ToStr8(item));
        }
        InputmethodDump::GetInstance().Dump(fd, argsStr);
        return ERR_OK;
    }

    std::string InputMethodSystemAbility::GetInputMethodParam(const std::vector<InputMethodInfo> &properties)
    {
        std::string defaultIme = ParaHandle::GetDefaultIme(userId_);
        bool isBegin = true;
        std::string params = "{\"imeList\":[";
        for (const auto &property : properties) {
            params += isBegin ? "" : "},";
            isBegin = false;

            std::string imeId = Str16ToStr8(property.mPackageName) + "/" + Str16ToStr8(property.mAbilityName);
            params += "{\"ime\": \"" + imeId + "\",";
            params += "\"labelId\": \"" + std::to_string(property.labelId) + "\",";
            params += "\"descriptionId\": \"" + std::to_string(property.descriptionId) + "\",";
            std::string isDefaultIme = defaultIme == imeId ? "true" : "false";
            params += "\"isDefaultIme\": \"" + isDefaultIme + "\",";
            params += "\"label\": \"" + Str16ToStr8(property.label) + "\",";
            params += "\"description\": \"" + Str16ToStr8(property.description) + "\"";
        }
        params += "}]}";
        return params;
    }

    void InputMethodSystemAbility::DumpAllMethod(int fd)
    {
        IMSA_HILOGI("InputMethodSystemAbility::DumpAllMethod");
        std::vector<int32_t> ids;
        int errCode = OsAccountManager::QueryActiveOsAccountIds(ids);
        if (errCode != ERR_OK) {
            dprintf(fd, "\n - InputMethodSystemAbility::DumpAllMethod get Active Id failed.\n");
            return;
        }
        dprintf(fd, "\n - DumpAllMethod get Active Id succeed,count=%zu,", ids.size());
        for (auto id : ids) {
            const auto &properties = ListInputMethodInfo(id);
            if (properties.empty()) {
                IMSA_HILOGI("The IME properties is empty.");
                dprintf(fd, "\n - The IME properties about the Active Id %d is empty.\n", id);
                continue;
            }
            const auto &params = GetInputMethodParam(properties);
            dprintf(fd, "\n - The Active Id:%d get input method:\n%s\n", id, params.c_str());
        }
        IMSA_HILOGI("InputMethodSystemAbility::DumpAllMethod end.");
    }

    int32_t InputMethodSystemAbility::Init()
    {
        bool isSuccess = Publish(this);
        if (!isSuccess) {
            return -1;
        }
        IMSA_HILOGI("Publish ErrorCode::NO_ERROR.");
        state_ = ServiceRunningState::STATE_RUNNING;
        std::string defaultIme = ParaHandle::GetDefaultIme(userId_);
        StartInputService(defaultIme);
        StartUserIdListener();
        int32_t ret = SubscribeKeyboardEvent();
        IMSA_HILOGI("subscribe key event ret %{public}d", ret);
        return ErrorCode::NO_ERROR;
    }

    void InputMethodSystemAbility::OnStop()
    {
        IMSA_HILOGI("OnStop started.");
        if (state_ != ServiceRunningState::STATE_RUNNING) {
            return;
        }
        serviceHandler_ = nullptr;

        state_ = ServiceRunningState::STATE_NOT_START;
        IMSA_HILOGI("OnStop end.");
    }

    void InputMethodSystemAbility::InitServiceHandler()
    {
        IMSA_HILOGI("InitServiceHandler started.");
        if (serviceHandler_) {
            IMSA_HILOGI("InitServiceHandler already init.");
            return;
        }
        std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("InputMethodSystemAbility");
        serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);

        IMSA_HILOGI("InitServiceHandler succeeded.");
    }

    /*! Initialization of Input method management service
    \n It's called after the service starts, before any transaction.
    */
    void InputMethodSystemAbility::Initialize()
    {
        IMSA_HILOGI("InputMethodSystemAbility::Initialize");
        // init work thread to handle the messages
        workThreadHandler = std::thread([this] { WorkThread(); });
        PerUserSetting *setting = new PerUserSetting(MAIN_USER_ID);
        userSettings.insert(std::pair<int32_t, PerUserSetting *>(MAIN_USER_ID, setting));
        userSessions.insert({ MAIN_USER_ID, std::make_shared<PerUserSession>(MAIN_USER_ID) });

        userId_ = MAIN_USER_ID;
        setting->Initialize();
    }

    void InputMethodSystemAbility::StartUserIdListener()
    {
        sptr<ImCommonEventManager> imCommonEventManager = ImCommonEventManager::GetInstance();
        bool isSuccess = imCommonEventManager->SubscribeEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
        if (isSuccess) {
            IMSA_HILOGI("InputMethodSystemAbility::Initialize subscribe service event success");
            return;
        }

        IMSA_HILOGE("StartUserIdListener failed. Try again 10s later");
        auto callback = [this]() { StartUserIdListener(); };
        serviceHandler_->PostTask(callback, INIT_INTERVAL);
    }

    bool InputMethodSystemAbility::StartInputService(std::string imeId)
    {
        IMSA_HILOGE("InputMethodSystemAbility::StartInputService() ime:%{public}s", imeId.c_str());

        auto session = GetUserSession(MAIN_USER_ID);

        std::map<int32_t, MessageHandler*>::const_iterator it = msgHandlers.find(MAIN_USER_ID);
        if (it == msgHandlers.end()) {
            IMSA_HILOGE("InputMethodSystemAbility::StartInputService() need start handler");
            MessageHandler *handler = new MessageHandler();
            if (session) {
                IMSA_HILOGE("InputMethodSystemAbility::OnPrepareInput session is not nullptr");
                session->CreateWorkThread(*handler);
                msgHandlers.insert(std::pair<int32_t, MessageHandler*>(MAIN_USER_ID, handler));
            }
        }

        bool isStartSuccess = false;
        sptr<AAFwk::IAbilityManager> abms = GetAbilityManagerService();
        if (abms) {
            AAFwk::Want want;
            want.SetAction("action.system.inputmethod");
            std::string::size_type pos = imeId.find("/");
            want.SetElementName(imeId.substr(0, pos), imeId.substr(pos + 1));
            int32_t result = abms->StartAbility(want);
            if (result) {
                IMSA_HILOGE("InputMethodSystemAbility::StartInputService failed, result = %{public}d", result);
                isStartSuccess = false;
            } else {
                IMSA_HILOGE("InputMethodSystemAbility::StartInputService success.");
                isStartSuccess = true;
            }
        }

        if (!isStartSuccess) {
            IMSA_HILOGE("StartInputService failed. Try again 10s later");
            auto callback = [this, imeId]() { StartInputService(imeId); };
            serviceHandler_->PostTask(callback, INIT_INTERVAL);
        }
        return isStartSuccess;
    }

    void InputMethodSystemAbility::StopInputService(std::string imeId)
    {
        IMSA_HILOGE("InputMethodSystemAbility::StopInputService(%{public}s)", imeId.c_str());
        auto session = GetUserSession(MAIN_USER_ID);
        if (session == nullptr) {
            IMSA_HILOGE("InputMethodSystemAbility::StopInputService abort session is nullptr");
            return;
        }
        session->StopInputService(imeId);
    }

    /*! Get the state of user
    \n This API is added for unit test.
    \param userID the id of given user
    \return user state can be one of the values of UserState
    */
    int32_t InputMethodSystemAbility::GetUserState(int32_t userId)
    {
        PerUserSetting *setting = GetUserSetting(userId);
        if (!setting) {
            return UserState::USER_STATE_NOT_AVAILABLE;
        }
        return setting->GetUserState();
    }

    /*! Handle the transaction from the remote binder
    \n Run in binder thread
    \param code transaction code number
    \param data the params from remote binder
    \param[out] reply the result of the transaction replied to the remote binder
    \param flags the flags of handling transaction
    \return int32_t
    */
    int32_t InputMethodSystemAbility::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option)
    {
        return InputMethodSystemAbilityStub::OnRemoteRequest(code, data, reply, option);
    }

    int32_t InputMethodSystemAbility::PrepareInput(int32_t displayId, sptr<IInputClient> client,
        sptr<IInputDataChannel> channel, InputAttribute &attribute)
    {
        int32_t pid = IPCSkeleton::GetCallingPid();
        int32_t uid = IPCSkeleton::GetCallingUid();
        auto session = GetUserSession(MAIN_USER_ID);
        if (session == nullptr) {
            IMSA_HILOGE("InputMethodSystemAbility::PrepareInput session is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        sptr<RemoteObjectDeathRecipient> clientDeathRecipient = new (std::nothrow) RemoteObjectDeathRecipient();
        if (clientDeathRecipient == nullptr) {
            IMSA_HILOGE("InputMethodSystemAbility::PrepareInput clientDeathRecipient is nullptr");
            return ErrorCode::ERROR_EX_NULL_POINTER;
        }
        return session->OnPrepareInput({ pid, uid, userId_, displayId, client, channel, clientDeathRecipient, attribute });
    };

    int32_t InputMethodSystemAbility::ReleaseInput(sptr<IInputClient> client)
    {
        auto session = GetUserSession(MAIN_USER_ID);
        if (session == nullptr) {
            IMSA_HILOGE("InputMethodSystemAbility::PrepareInput session is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        return session->OnReleaseInput(client);
    };

    int32_t InputMethodSystemAbility::StartInput(sptr<IInputClient> client, bool isShowKeyboard)
    {
        auto session = GetUserSession(MAIN_USER_ID);
        if (session == nullptr) {
            IMSA_HILOGE("InputMethodSystemAbility::PrepareInput session is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        auto currentSubtype = GetCurrentInputMethodSubtype();
        if (currentSubtype == nullptr) {
            IMSA_HILOGE("currentSubtype is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        session->SetCurrentSubProperty(*currentSubtype);
        return session->OnStartInput(client, isShowKeyboard);
    };

    int32_t InputMethodSystemAbility::StopInput(sptr<IInputClient> client)
    {
        auto session = GetUserSession(MAIN_USER_ID);
        if (session == nullptr) {
            IMSA_HILOGE("InputMethodSystemAbility::PrepareInput session is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        return session->OnStopInput(client);
    };

    int32_t InputMethodSystemAbility::StopInputSession()
    {
        return HideCurrentInput();
    }

    int32_t InputMethodSystemAbility::SetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent)
    {
        auto session = GetUserSession(MAIN_USER_ID);
        if (session == nullptr) {
            IMSA_HILOGE("InputMethodSystemAbility::PrepareInput session is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        return session->OnSetCoreAndAgent(core, agent);
    };

    int32_t InputMethodSystemAbility::HideCurrentInput()
    {
        auto session = GetUserSession(MAIN_USER_ID);
        if (session == nullptr) {
            IMSA_HILOGE("InputMethodSystemAbility::PrepareInput session is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        return session->OnHideKeyboardSelf(0);
    };

    int32_t InputMethodSystemAbility::ShowCurrentInput()
    {
        auto session = GetUserSession(MAIN_USER_ID);
        if (session == nullptr) {
            IMSA_HILOGE("InputMethodSystemAbility::PrepareInput session is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        return session->OnShowKeyboardSelf();
    };

    int32_t InputMethodSystemAbility::DisplayOptionalInputMethod()
    {
        return OnDisplayOptionalInputMethod(MAIN_USER_ID);
    };

    int32_t InputMethodSystemAbility::GetKeyboardWindowHeight(int32_t &retHeight)
    {
        auto session = GetUserSession(MAIN_USER_ID);
        if (session == nullptr) {
            IMSA_HILOGI("InputMethodSystemAbility::getKeyboardWindowHeight session is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        int32_t ret = session->OnGetKeyboardWindowHeight(retHeight);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("Failed to get keyboard window height. ErrorCode=%d\n", ret);
        }
        return ret;
    }

    int32_t InputMethodSystemAbility::ListInputMethod(InputMethodStatus status, std::vector<Property> &props)
    {
        return ListInputMethodByUserId(MAIN_USER_ID, status, props);
    }

    int32_t InputMethodSystemAbility::ListAllInputMethod(int32_t userId, std::vector<Property> &props)
    {
        IMSA_HILOGI("InputMethodSystemAbility::listAllInputMethod");
        return ListProperty(userId, props);
    }

    int32_t InputMethodSystemAbility::ListEnabledInputMethod(std::vector<Property> &props)
    {
        IMSA_HILOGI("InputMethodSystemAbility::listEnabledInputMethod");
        auto current = GetCurrentInputMethod();
        if (current == nullptr) {
            IMSA_HILOGE("GetCurrentInputMethod current is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        auto property = FindProperty(current->name);
        props = { property };
        return ErrorCode::NO_ERROR;
    }

    int32_t InputMethodSystemAbility::ListDisabledInputMethod(int32_t userId, std::vector<Property> &props)
    {
        IMSA_HILOGI("InputMethodSystemAbility::listDisabledInputMethod");
        auto ret = ListProperty(userId, props);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("list property is failed, ret = %{public}d", ret);
            return ret;
        }
        auto filter = GetCurrentInputMethod();
        if (filter == nullptr) {
            IMSA_HILOGE("GetCurrentInputMethod property is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        for (auto iter = props.begin(); iter != props.end();) {
            if (iter->name == filter->name && iter->id == filter->id) {
                iter = props.erase(iter);
                continue;
            }
            ++iter;
        }
        return ErrorCode::NO_ERROR;
    }

    int32_t InputMethodSystemAbility::ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProps)
    {
        IMSA_HILOGI("InputMethodSystemAbility::ListCurrentInputMethodSubtype");
        auto filter = GetCurrentInputMethod();
        if (filter == nullptr) {
            IMSA_HILOGE("GetCurrentInputMethod failed");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        return ListSubtypeByBundleName(MAIN_USER_ID, filter->name, subProps);
    }

    int32_t InputMethodSystemAbility::ListInputMethodSubtype(
        const std::string &name, std::vector<SubProperty> &subProps)
    {
        IMSA_HILOGI("InputMethodSystemAbility::ListInputMethodSubtype");
        return ListSubtypeByBundleName(MAIN_USER_ID, name, subProps);
    }

    int32_t InputMethodSystemAbility::ListSubtypeByBundleName(
        int32_t userId, const std::string &name, std::vector<SubProperty> &subProps)
    {
        IMSA_HILOGI("InputMethodSystemAbility::ListSubtypeByBundleName");
        std::vector<AppExecFwk::ExtensionAbilityInfo> subtypeInfos;
        if (!GetBundleMgr()->QueryExtensionAbilityInfos(AbilityType::INPUTMETHOD, userId, subtypeInfos)) {
            IMSA_HILOGE("QueryExtensionAbilityInfos failed");
            return ErrorCode::ERROR_PACKAGE_MANAGER;
        }
        std::vector<SubProperty> properties;
        for (const auto &subtypeInfo : subtypeInfos) {
            if (subtypeInfo.bundleName == name) {
                std::vector<Metadata> extends = subtypeInfo.metadata;
                auto property = GetExtends(extends);
                subProps.push_back({ .id = subtypeInfo.bundleName,
                    .label = subtypeInfo.name,
                    .name = subtypeInfo.moduleName,
                    .iconId = subtypeInfo.iconId,
                    .language = property.language,
                    .mode = property.mode,
                    .locale = property.locale,
                    .icon = property.icon });
            }
        }
        return ErrorCode::NO_ERROR;
    }

    SubProperty InputMethodSystemAbility::GetExtends(const std::vector<Metadata> &metaData)
    {
        IMSA_HILOGI("InputMethodSystemAbility::GetExtends");
        SubProperty property;
        for (const auto &data : metaData) {
            if (data.name == "language") {
                property.language = data.value;
                continue;
            }
            if (data.name == "mode") {
                property.mode = data.value;
                continue;
            }
            if (data.name == "locale") {
                property.locale = data.value;
                continue;
            }
            if (data.name == "icon") {
                property.icon = data.value;
            }
        }
        return property;
    }

    int32_t InputMethodSystemAbility::SwitchInputMethod(const std::string &name, const std::string &subName)
    {
        IMSA_HILOGI("InputMethodSystemAbility::SwitchInputMethod");
        return subName.empty() ? SwitchInputMethodType(name) : SwitchInputMethodSubtype(name, subName);
    }

    int32_t InputMethodSystemAbility::SwitchInputMethodType(const std::string &name)
    {
        IMSA_HILOGI("InputMethodSystemAbility::SwitchInputMethodType");
        std::vector<Property> properties = {};
        auto ret = ListInputMethodByUserId(MAIN_USER_ID, ALL, properties);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("ListInputMethodByUserId failed, ret = %{public}d", ret);
            return ret;
        }
        if (properties.empty()) {
            IMSA_HILOGE("InputMethodSystemAbility::SwitchInputMethodType has no ime");
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        for (const auto &property : properties) {
            if (property.name == name) {
                IMSA_HILOGI("target is installed, start switching");
                return OnSwitchInputMethod(property.name, property.id);
            }
        }
        IMSA_HILOGE("target is not installed, switch failed");
        return ErrorCode::ERROR_SWITCH_IME;
    }

    int32_t InputMethodSystemAbility::SwitchInputMethodSubtype(const std::string &bundleName, const std::string &name)
    {
        IMSA_HILOGI("InputMethodSystemAbility::SwitchInputMethodSubtype");
        std::vector<SubProperty> subProps = {};
        auto ret = ListSubtypeByBundleName(MAIN_USER_ID, bundleName, subProps);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("ListSubtypeByBundleName failed, ret = %{public}d", ret);
            return ret;
        }
        if (subProps.empty()) {
            IMSA_HILOGE("InputMethodSystemAbility::SwitchInputMethodSubtype has no ime");
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        for (const auto &subProp : subProps) {
            if (subProp.label == name) {
                IMSA_HILOGI("target is installed, start switching");
                return OnSwitchInputMethod(bundleName, name);
            }
        }
        IMSA_HILOGE("target is not installed, switch failed");
        return ErrorCode::ERROR_SWITCH_IME;
    }

    int32_t InputMethodSystemAbility::OnSwitchInputMethod(const std::string &bundleName, const std::string &name)
    {
        IMSA_HILOGI("InputMethodSystemAbility::OnSwitchInputMethod");
        std::string targetIme = bundleName + "/" + name;
        std::string defaultIme = ParaHandle::GetDefaultIme(userId_);
        IMSA_HILOGI("DefaultIme : %{public}s, TargetIme : %{public}s", defaultIme.c_str(), targetIme.c_str());
        if (defaultIme == targetIme) {
            IMSA_HILOGI("DefaultIme and TargetIme are the same one");
            return ErrorCode::NO_ERROR;
        }
        StopInputService(defaultIme);
        if (!StartInputService(targetIme)) {
            IMSA_HILOGE("start input method failed");
            return ErrorCode::ERROR_IME_START_FAILED;
        }
        if (!ParaHandle::SetDefaultIme(userId_, targetIme)) {
            IMSA_HILOGE("set default ime failed");
            return ErrorCode::ERROR_PERSIST_CONFIG;
        }
        auto session = GetUserSession(MAIN_USER_ID);
        if (session == nullptr) {
            IMSA_HILOGE("session is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        return session->OnInputMethodSwitched(FindProperty(bundleName), FindSubProperty(bundleName, name));
    }

    Property InputMethodSystemAbility::FindProperty(const std::string &name)
    {
        IMSA_HILOGI("InputMethodSystemAbility::FindProperty");
        std::vector<Property> props = {};
        auto ret = ListAllInputMethod(MAIN_USER_ID, props);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("ListAllInputMethod failed");
            return {};
        }
        for (const auto &prop : props) {
            if (prop.name == name) {
                return prop;
            }
        }
        IMSA_HILOGE("InputMethodSystemAbility::FindProperty failed");
        return {};
    }

    SubProperty InputMethodSystemAbility::FindSubProperty(const std::string &bundleName, const std::string &name)
    {
        IMSA_HILOGI("InputMethodSystemAbility::FindSubProperty");
        std::vector<SubProperty> subProps = {};
        auto ret = ListSubtypeByBundleName(MAIN_USER_ID, bundleName, subProps);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("ListSubtypeByBundleName failed, ret = %{public}d", ret);
            return {};
        }
        for (const auto &subProp : subProps) {
            if (subProp.label == name) {
                return subProp;
            }
        }
        IMSA_HILOGE("InputMethodSystemAbility::FindSubProperty failed");
        return {};
    }

    // Deprecated because of no permission check, kept for compatibility
    int32_t InputMethodSystemAbility::SetCoreAndAgentDeprecated(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent)
    {
        return SetCoreAndAgent(core, agent);
    };

    int32_t InputMethodSystemAbility::HideCurrentInputDeprecated()
    {
        return HideCurrentInput();
    };

    int32_t InputMethodSystemAbility::ShowCurrentInputDeprecated()
    {
        return ShowCurrentInput();
    };

    int32_t InputMethodSystemAbility::DisplayOptionalInputMethodDeprecated()
    {
        return DisplayOptionalInputMethod();
    };

    /*! Get all of the input method engine list installed in the system
    \n Run in binder thread
    \param[out] properties input method engine list returned to the caller
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    */
    int32_t InputMethodSystemAbility::ListInputMethodByUserId(
        int32_t userId, InputMethodStatus status, std::vector<Property> &props)
    {
        IMSA_HILOGI("InputMethodSystemAbility::ListInputMethodByUserId");
        if (status == InputMethodStatus::ALL) {
            return ListAllInputMethod(userId, props);
        }
        if (status == InputMethodStatus::ENABLE) {
            return ListEnabledInputMethod(props);
        }
        if (status == InputMethodStatus::DISABLE) {
            return ListDisabledInputMethod(userId, props);
        }
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }

    std::vector<InputMethodInfo> InputMethodSystemAbility::ListInputMethodInfo(int32_t userId)
    {
        IMSA_HILOGI("InputMethodSystemAbility::ListInputMethodInfo userId = %{public}d", userId);
        std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
        bool ret = GetBundleMgr()->QueryExtensionAbilityInfos(AbilityType::INPUTMETHOD, userId, extensionInfos);
        if (!ret) {
            IMSA_HILOGE("InputMethodSystemAbility::ListInputMethodInfo QueryExtensionAbilityInfos error");
            return {};
        }
        std::vector<InputMethodInfo> properties;
        for (auto extension : extensionInfos) {
            std::shared_ptr<Global::Resource::ResourceManager> resourceManager(
                Global::Resource::CreateResourceManager());
            if (resourceManager == nullptr) {
                IMSA_HILOGE("InputMethodSystemAbility::ListInputMethodInfo resourcemanager is nullptr");
                break;
            }
            AppExecFwk::ApplicationInfo applicationInfo = extension.applicationInfo;
            resourceManager->AddResource(extension.hapPath.c_str());
            std::string labelString;
            resourceManager->GetStringById(applicationInfo.labelId, labelString);
            std::string descriptionString;
            resourceManager->GetStringById(applicationInfo.descriptionId, descriptionString);
            InputMethodInfo property;
            property.mPackageName = Str8ToStr16(extension.bundleName);
            property.mAbilityName = Str8ToStr16(extension.name);
            property.labelId = applicationInfo.labelId;
            property.descriptionId = applicationInfo.descriptionId;
            property.label = Str8ToStr16(labelString);
            property.description = Str8ToStr16(descriptionString);
        }
        return properties;
    }

    int32_t InputMethodSystemAbility::ListProperty(int32_t userId, std::vector<Property> &props)
    {
        IMSA_HILOGI("InputMethodSystemAbility::ListProperty userId = %{public}d", userId);
        std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
        bool ret = GetBundleMgr()->QueryExtensionAbilityInfos(AbilityType::INPUTMETHOD, userId, extensionInfos);
        if (!ret) {
            IMSA_HILOGE("InputMethodSystemAbility::ListProperty QueryExtensionAbilityInfos error");
            return ErrorCode::ERROR_PACKAGE_MANAGER;
        }
        for (const auto &extension : extensionInfos) {
            bool isInVector = false;
            for (const auto &prop : props) {
                if (extension.bundleName == prop.name) {
                    isInVector = true;
                    break;
                }
            }
            if (isInVector) {
                continue;
            }
            props.push_back({ .name = extension.bundleName,
                .id = extension.name,
                .label = extension.applicationInfo.label,
                .icon = extension.applicationInfo.icon,
                .iconId = extension.applicationInfo.iconId });
        }
        return ErrorCode::NO_ERROR;
    }

    std::shared_ptr<Property> InputMethodSystemAbility::GetCurrentInputMethod()
    {
        IMSA_HILOGI("InputMethodSystemAbility::GetCurrentInputMethod");
        std::string ime = ParaHandle::GetDefaultIme(MAIN_USER_ID);
        if (ime.empty()) {
            IMSA_HILOGE("InputMethodSystemAbility::GetCurrentInputMethod ime is empty");
            return nullptr;
        }

        auto pos = ime.find('/');
        if (pos == std::string::npos) {
            IMSA_HILOGE("InputMethodSystemAbility::GetCurrentInputMethod ime can not find '/'");
            return nullptr;
        }

        auto property = std::make_shared<Property>();
        if (property == nullptr) {
            IMSA_HILOGE("InputMethodSystemAbility property is nullptr");
            return nullptr;
        }
        property->name = ime.substr(0, pos);
        property->id = ime.substr(pos + 1, ime.length() - pos - 1);
        return property;
    }

    std::shared_ptr<SubProperty> InputMethodSystemAbility::GetCurrentInputMethodSubtype()
    {
        IMSA_HILOGI("InputMethodSystemAbility::GetCurrentInputMethodSubtype");
        std::string ime = ParaHandle::GetDefaultIme(MAIN_USER_ID);
        if (ime.empty()) {
            IMSA_HILOGE("InputMethodSystemAbility ime is empty");
            return nullptr;
        }
        auto pos = ime.find('/');
        if (pos == std::string::npos) {
            IMSA_HILOGE("InputMethodSystemAbility:: ime can not find '/'");
            return nullptr;
        }
        auto property = std::make_shared<SubProperty>(
            FindSubProperty(ime.substr(0, pos), ime.substr(pos + 1, ime.length() - pos - 1)));
        if (property == nullptr) {
            IMSA_HILOGE("property is nullptr");
            return nullptr;
        }
        return property;
    }

    /*! Get the instance of PerUserSetting for the given user
    \param userId the user id of the given user
    \return a pointer of the instance if the user is found
    \return null if the user is not found
    */
    PerUserSetting *InputMethodSystemAbility::GetUserSetting(int32_t userId)
    {
        auto it = userSettings.find(userId);
        if (it == userSettings.end()) {
            return nullptr;
        }
        return it->second;
    }

    /*! Get the instance of PerUserSession for the given user
    \param userId the user id of the given user
    \return a pointer of the instance if the user is found
    \return null if the user is not found
    */
    std::shared_ptr<PerUserSession> InputMethodSystemAbility::GetUserSession(int32_t userId)
    {
        auto it = userSessions.find(userId);
        if (it == userSessions.end()) {
            IMSA_HILOGE("not found session");
            return nullptr;
        }
        return it->second;
    }

    /*! Work Thread of input method management service
    \n Remote commands which may change the state or data in the service will be handled sequentially in this thread.
    */
    void InputMethodSystemAbility::WorkThread()
    {
        while (1) {
            Message *msg = MessageHandler::Instance()->GetMessage();
            switch (msg->msgId_) {
                case MSG_ID_USER_START : {
                    OnUserStarted(msg);
                    break;
                }
                case MSG_ID_USER_STOP: {
                    OnUserStopped(msg);
                    break;
                }
                case MSG_ID_USER_UNLOCK: {
                    OnUserUnlocked(msg);
                    break;
                }
                case MSG_ID_USER_LOCK : {
                    OnUserLocked(msg);
                    break;
                }
                case MSG_ID_PACKAGE_ADDED: {
                    OnPackageAdded(msg);
                    break;
                }
                case MSG_ID_PACKAGE_REMOVED: {
                    OnPackageRemoved(msg);
                    break;
                }
                case MSG_ID_SETTING_CHANGED: {
                    OnSettingChanged(msg);
                    break;
                }
                case MSG_ID_DISPLAY_OPTIONAL_INPUT_METHOD: {
                    MessageParcel *data = msg->msgContent_;
                    int32_t userId = data->ReadInt32();
                    OnDisplayOptionalInputMethod(userId);
                    break;
                }
                case MSG_ID_PREPARE_INPUT:
                case MSG_ID_RELEASE_INPUT:
                case MSG_ID_START_INPUT:
                case MSG_ID_STOP_INPUT:
                case MSG_HIDE_CURRENT_INPUT:
                case MSG_SHOW_CURRENT_INPUT:
                case MSG_ID_SET_CORE_AND_AGENT:
                case MSG_ID_HIDE_KEYBOARD_SELF:
                case MSG_ID_SET_DISPLAY_MODE:
                case MSG_ID_CLIENT_DIED:
                case MSG_ID_IMS_DIED:
                case MSG_ID_RESTART_IMS: {
                    OnHandleMessage(msg);
                    break;
                }
                case MSG_ID_DISABLE_IMS: {
                    OnDisableIms(msg);
                    break;
                }
                case MSG_ID_ADVANCE_TO_NEXT: {
                    OnAdvanceToNext(msg);
                    break;
                }
                case MSG_ID_EXIT_SERVICE: {
                    std::map<int32_t, MessageHandler*>::const_iterator it;
                    for (it = msgHandlers.cbegin(); it != msgHandlers.cend();) {
                        MessageHandler *handler = it->second;
                        Message *destMsg = new Message(MSG_ID_EXIT_SERVICE, nullptr);
                        handler->SendMessage(destMsg);
                        auto userSession = GetUserSession(it->first);
                        if (!userSession) {
                            IMSA_HILOGE("getUserSession fail.");
                            return;
                        }
                        userSession->JoinWorkThread();
                        it = msgHandlers.erase(it);
                        delete handler;
                        handler = nullptr;
                    }
                    delete msg;
                    msg = nullptr;
                    return;
                }
                case MSG_ID_START_INPUT_SERVICE: {
                    MessageParcel *data = msg->msgContent_;
                    const auto &ime = data->ReadString();
                    StartInputService(ime);
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }

    /*! Called when a user is started. (EVENT_USER_STARTED is received)
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode
    */
    int32_t InputMethodSystemAbility::OnUserStarted(const Message *msg)
    {
        IMSA_HILOGI("InputMethodSystemAbility::OnUserStarted Start...\n");
        if (!msg->msgContent_) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        std::string currentDefaultIme = ParaHandle::GetDefaultIme(userId_);
        int32_t userId = msg->msgContent_->ReadInt32();
        userId_ = userId;
        IMSA_HILOGI("InputMethodSystemAbility::OnUserStarted userId = %{public}u", userId);

        std::string newDefaultIme = ParaHandle::GetDefaultIme(userId_);

        if (newDefaultIme != currentDefaultIme) {
            StopInputService(currentDefaultIme);
            StartInputService(newDefaultIme);
        }

        PerUserSetting *setting = GetUserSetting(userId);
        if (setting) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_ALREADY_STARTED), userId);
            return ErrorCode::ERROR_USER_ALREADY_STARTED;
        }

        setting = new PerUserSetting(userId);
        setting->Initialize();

        userSettings.insert(std::pair<int32_t, PerUserSetting *>(userId, setting));
        userSessions.insert({ userId, std::make_shared<PerUserSession>(userId) });
        return ErrorCode::NO_ERROR;
    }

    /*! Called when a user is stopped. (EVENT_USER_STOPPED is received)
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode
    */
    int32_t InputMethodSystemAbility::OnUserStopped(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        if (!msg->msgContent_) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        int32_t userId = msg->msgContent_->ReadInt32();
        PerUserSetting *setting = GetUserSetting(userId);
        auto session = GetUserSession(userId);
        if (!setting || !session) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_STARTED), userId);
            return ErrorCode::ERROR_USER_NOT_STARTED;
        }
        if (setting->GetUserState() == UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_LOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_LOCKED;
        }
        auto it = userSessions.find(userId);
        if (it != userSessions.end()) {
            userSessions.erase(it);
        }
        std::map<int32_t, PerUserSetting *>::iterator itSetting = userSettings.find(userId);
        userSettings.erase(itSetting);
        delete setting;
        setting = nullptr;
        IMSA_HILOGI("End...[%d]\n", userId);
        return ErrorCode::NO_ERROR;
    }

    /*! Called when a user is unlocked. (EVENT_USER_UNLOCKED is received)
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode
    */
    int32_t InputMethodSystemAbility::OnUserUnlocked(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        if (!msg->msgContent_) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        int32_t userId = msg->msgContent_->ReadInt32();
        PerUserSetting *setting = GetUserSetting(userId);
        auto session = GetUserSession(userId);
        if (!setting || !session) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_STARTED), userId);
            return ErrorCode::ERROR_USER_NOT_STARTED;
        }
        if (setting->GetUserState() == UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_ALREADY_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_ALREADY_UNLOCKED;
        }

        setting->Initialize();

        InputMethodInfo *ime = setting->GetSecurityInputMethod();
        session->SetSecurityIme(ime);
        ime = setting->GetCurrentInputMethod();
        session->SetCurrentIme(ime);
        session->SetInputMethodSetting(setting->GetInputMethodSetting());
        IMSA_HILOGI("End...[%d]\n", userId);
        return ErrorCode::NO_ERROR;
    }

    /*! Called when a user is locked. (EVENT_USER_LOCKED is received)
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode
    */
    int32_t InputMethodSystemAbility::OnUserLocked(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        if (!msg->msgContent_) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        int32_t userId = msg->msgContent_->ReadInt32();
        PerUserSetting *setting = GetUserSetting(userId);
        if (!setting || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        std::map<int32_t, MessageHandler*>::iterator it = msgHandlers.find(userId);
        if (it != msgHandlers.end()) {
            MessageHandler *handler = it->second;
            Message *destMsg = new Message(MSG_ID_USER_LOCK, nullptr);
            if (destMsg) {
                handler->SendMessage(destMsg);
                auto userSession = GetUserSession(userId);
                if (userSession) {
                    userSession->JoinWorkThread();
                }
                msgHandlers.erase(it);
                delete handler;
                handler = nullptr;
            }
        }
        setting->OnUserLocked();
        IMSA_HILOGI("End...[%d]\n", userId);
        return ErrorCode::NO_ERROR;
    }

    /*! Handle message
    \param msgId the id of message to run
    \msg the parameters are saved in msg->msgContent_
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    */
    int32_t InputMethodSystemAbility::OnHandleMessage(Message *msg)
    {
        MessageParcel *data = msg->msgContent_;
        int32_t userId = data->ReadInt32();
        PerUserSetting *setting = GetUserSetting(MAIN_USER_ID);
        if (!setting) {
            IMSA_HILOGE("InputMethodSystemAbility::OnHandleMessage Aborted! setting is nullptr");
        }
        if (!setting || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("InputMethodSystemAbility::OnHandleMessage Aborted! userId = %{public}d,", userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }

        std::map<int32_t, MessageHandler*>::const_iterator it = msgHandlers.find(MAIN_USER_ID);
        if (it != msgHandlers.end()) {
            MessageHandler *handler = it->second;
            handler->SendMessage(msg);
        }
        return ErrorCode::NO_ERROR;
    }

    /*! Called when a package is installed.
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    \return ErrorCode::ERROR_BAD_PARAMETERS bad parameter
    */
    int32_t InputMethodSystemAbility::OnPackageAdded(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        MessageParcel *data = msg->msgContent_;
        int32_t userId = data->ReadInt32();
        int32_t size = data->ReadInt32();

        if (size <= 0) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        std::u16string packageName = data->ReadString16();
        PerUserSetting *setting = GetUserSetting(userId);
        if (!setting || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        bool securityImeFlag = false;
        int32_t ret = setting->OnPackageAdded(packageName, securityImeFlag);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGI("End...\n");
            return ret;
        }
        if (securityImeFlag) {
            InputMethodInfo *securityIme = setting->GetSecurityInputMethod();
            InputMethodInfo *defaultIme = setting->GetCurrentInputMethod();
            auto session = GetUserSession(userId);
            if (session == nullptr) {
                IMSA_HILOGI("InputMethodSystemAbility::OnPackageAdded session is nullptr");
                return ErrorCode::ERROR_NULL_POINTER;
            }
            session->ResetIme(defaultIme, securityIme);
        }
        IMSA_HILOGI("End...\n");
        return 0;
    }

    /*! Called when a package is removed.
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    \return ErrorCode::ERROR_BAD_PARAMETERS bad parameter
    */
    int32_t InputMethodSystemAbility::OnPackageRemoved(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        MessageParcel *data = msg->msgContent_;
        if (!data) {
            IMSA_HILOGI("InputMethodSystemAbility::OnPackageRemoved data is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        int32_t userId = data->ReadInt32();
        int32_t size = data->ReadInt32();

        if (size <= 0) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        std::u16string packageName = data->ReadString16();
        PerUserSetting *setting = GetUserSetting(userId);
        if (!setting || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        auto session = GetUserSession(userId);
        if (session == nullptr) {
            IMSA_HILOGI("InputMethodSystemAbility::OnPackageRemoved session is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        session->OnPackageRemoved(packageName);
        bool securityImeFlag = false;
        int32_t ret = setting->OnPackageRemoved(packageName, securityImeFlag);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGI("End...\n");
            return ret;
        }
        if (securityImeFlag) {
            InputMethodInfo *securityIme = setting->GetSecurityInputMethod();
            InputMethodInfo *defaultIme = setting->GetCurrentInputMethod();
            session->ResetIme(defaultIme, securityIme);
        }
        return 0;
    }

    /*! Called when input method setting data is changed.
    \n Run in work thread of input method management service
    \param msg the parameters from remote binder are saved in msg->msgContent_
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    \return ErrorCode::ERROR_BAD_PARAMETERS bad parameter
    */
    int32_t InputMethodSystemAbility::OnSettingChanged(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        MessageParcel *data = msg->msgContent_;
        int32_t userId = data->ReadInt32();
        int32_t size = data->ReadInt32();
        if (size < 2) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        std::u16string updatedKey = data->ReadString16();
        std::u16string updatedValue = data->ReadString16();
        PerUserSetting *setting = GetUserSetting(userId);
        if (!setting || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        auto session = GetUserSession(userId);
        if (session == nullptr) {
            return ErrorCode::ERROR_NULL_POINTER;
        }
        int32_t ret = session->OnSettingChanged(updatedKey, updatedValue);
        if (ret == ErrorCode::ERROR_SETTING_SAME_VALUE) {
            IMSA_HILOGI("End...No need to update\n");
            return ret;
        }

        // PerUserSetting does not need handle keyboard type change notification
        if (updatedKey == InputMethodSetting::CURRENT_KEYBOARD_TYPE_TAG ||
            updatedKey == InputMethodSetting::CURRENT_SYS_KEYBOARD_TYPE_TAG) {
            IMSA_HILOGI("End...\n");
            return ErrorCode::NO_ERROR;
        }

        ret = setting->OnSettingChanged(updatedKey, updatedValue);
        if (ret) {
            IMSA_HILOGI("End...No need to update\n");
            return ret;
        }

        InputMethodInfo *securityIme = setting->GetSecurityInputMethod();
        InputMethodInfo *defaultIme = setting->GetCurrentInputMethod();
        session->ResetIme(defaultIme, securityIme);
        IMSA_HILOGI("End...\n");
        return ErrorCode::NO_ERROR;
    }

    int32_t InputMethodSystemAbility::OnDisplayOptionalInputMethod(int32_t userId)
    {
        IMSA_HILOGI("InputMethodSystemAbility::OnDisplayOptionalInputMethod");
        auto abilityManager = GetAbilityManagerService();
        if (abilityManager == nullptr) {
            IMSA_HILOGE("InputMethodSystemAbility::get ability manager failed");
            return ErrorCode::ERROR_EX_SERVICE_SPECIFIC;
        }
        AAFwk::Want want;
        want.SetAction(SELECT_DIALOG_ACTION);
        want.SetElementName(SELECT_DIALOG_HAP, SELECT_DIALOG_ABILITY);
        int32_t ret = abilityManager->StartAbility(want);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("InputMethodSystemAbility::Start InputMethod ability failed, err = %{public}d", ret);
            return ErrorCode::ERROR_EX_SERVICE_SPECIFIC;
        }
        IMSA_HILOGI("InputMethodSystemAbility::Start InputMethod ability success.");
        return ErrorCode::NO_ERROR;
    }

    /*! Disable input method service. Called from PerUserSession module
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    */
    int32_t InputMethodSystemAbility::OnDisableIms(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        MessageParcel *data = msg->msgContent_;
        int32_t userId = data->ReadInt32();
        std::u16string imeId = data->ReadString16();
        PerUserSetting *setting = GetUserSetting(userId);
        if (!setting || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }

        InputMethodSetting tmpSetting;
        std::u16string key = InputMethodSetting::ENABLED_INPUT_METHODS_TAG;
        tmpSetting.SetValue(key, setting->GetInputMethodSetting()->GetValue(key));
        tmpSetting.RemoveEnabledInputMethod(imeId);
        IMSA_HILOGI("End...\n");
        return ErrorCode::NO_ERROR;
    }

    /*! Switch to next ime or next keyboard type. It's called by input method service
    \n Run in work thread of input method management service or the work thread of PerUserSession
    \param msg the parameters from remote binder are saved in msg->msgContent_
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    */
    int32_t InputMethodSystemAbility::OnAdvanceToNext(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        MessageParcel *data = msg->msgContent_;
        int32_t userId = data->ReadInt32();
        bool isCurrentIme = data->ReadBool();
        PerUserSetting *setting = GetUserSetting(userId);
        if (!setting || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        if (isCurrentIme) {
            std::map<int32_t, MessageHandler*>::const_iterator it = msgHandlers.find(userId);
            if (it != msgHandlers.end()) {
                Message *destMsg = new Message(msg->msgId_, nullptr);
                it->second->SendMessage(destMsg);
            }
        } else {
            setting->OnAdvanceToNext();
        }
        IMSA_HILOGI("End...\n");
        return ErrorCode::NO_ERROR;
    }

    sptr<OHOS::AppExecFwk::IBundleMgr> InputMethodSystemAbility::GetBundleMgr()
    {
        IMSA_HILOGI("InputMethodSystemAbility::GetBundleMgr");
        sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityManager) {
            IMSA_HILOGI("InputMethodSystemAbility::GetBundleMgr systemAbilityManager is nullptr");
            return nullptr;
        }
        sptr<IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        return iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    }

    sptr<AAFwk::IAbilityManager> InputMethodSystemAbility::GetAbilityManagerService()
    {
        IMSA_HILOGE("InputMethodSystemAbility::GetAbilityManagerService start");
        sptr<IRemoteObject> abilityMsObj =
        OHOS::DelayedSingleton<AAFwk::SaMgrClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
        if (!abilityMsObj) {
            IMSA_HILOGE("failed to get ability manager service");
            return nullptr;
        }
        return iface_cast<AAFwk::IAbilityManager>(abilityMsObj);
    }

    SubProperty InputMethodSystemAbility::FindSubPropertyByCompare(const std::string &bundleName, CompareHandler compare)
    {
        IMSA_HILOGI("InputMethodSystemAbility::FindSubPropertyByCompare");
        std::vector<SubProperty> subProps = {};
        auto ret = ListSubtypeByBundleName(MAIN_USER_ID, bundleName, subProps);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("ListSubtypeByBundleName failed, ret = %{public}d", ret);
            return {};
        }
        for (const auto &subProp : subProps) {
            if (compare(subProp)) {
                return subProp;
            }
        }
        IMSA_HILOGE("InputMethodSystemAbility::FindSubPropertyByCompare failed");
        return {};
    }

    int32_t InputMethodSystemAbility::SwitchByCombinedKey(const CombineKeyCode &keyCode)
    {
        IMSA_HILOGI("InputMethodSystemAbility::SwitchByCombinedKey");
        auto current = GetCurrentInputMethodSubtype();
        if (current == nullptr) {
            IMSA_HILOGE("GetCurrentInputMethodSubtype failed");
            return ErrorCode::ERROR_EX_NULL_POINTER;
        }
        if (keyCode == CombineKeyCode::COMBINE_KEYCODE_CAPS) {
            IMSA_HILOGI("COMBINE_KEYCODE_CAPS press");
            auto target = current->mode == "upper"
                              ? FindSubPropertyByCompare(current->id,
                                  [&current](const SubProperty &property) { return property.mode == "lower"; })
                              : FindSubPropertyByCompare(current->id,
                                  [&current](const SubProperty &property) { return property.mode == "upper"; });
            return SwitchInputMethod(target.id, target.label);
        }
        if (keyCode == CombineKeyCode::COMBINE_KEYCODE_SHIFT) {
            IMSA_HILOGI("COMBINE_KEYCODE_SHIFT press");
            auto target = current->language == "chinese"
                              ? FindSubPropertyByCompare(current->id,
                                  [&current](const SubProperty &property) { return property.language == "english"; })
                              : FindSubPropertyByCompare(current->id,
                                  [&current](const SubProperty &property) { return property.language == "chinese"; });
            return SwitchInputMethod(target.id, target.label);
        }
        if (keyCode == CombineKeyCode::COMBINE_KEYCODE_CTRL_SHIFT) {
            IMSA_HILOGI("KEYCODE_CTRL_LEFT_SHIFT_LEFT press");
            std::vector<Property> props = {};
            auto ret = ListProperty(MAIN_USER_ID, props);
            if (ret != ErrorCode::NO_ERROR) {
                IMSA_HILOGE("ListProperty failed");
                return ret;
            }
            for (const auto &prop : props) {
                if (prop.name != current->id) {
                    return SwitchInputMethod(current->name, current->id);
                }
            }
        }
        IMSA_HILOGI("keycode undefined");
        return ErrorCode::ERROR_EX_UNSUPPORTED_OPERATION;
    }

    int32_t InputMethodSystemAbility::SubscribeKeyboardEvent()
    {
        ImCommonEventManager::GetInstance()->SubscribeKeyboardEvent(
            { { {
                    .preKeys = {},
                    .finalKey = MMI::KeyEvent::KEYCODE_CAPS_LOCK,
                },
                  [this]() { SwitchByCombinedKey(CombineKeyCode::COMBINE_KEYCODE_CAPS); } },
                { {
                      .preKeys = {},
                      .finalKey = MMI::KeyEvent::KEYCODE_SHIFT_LEFT,
                  },
                    [this]() { SwitchByCombinedKey(CombineKeyCode::COMBINE_KEYCODE_SHIFT); } },
                { {
                      .preKeys = {},
                      .finalKey = MMI::KeyEvent::KEYCODE_SHIFT_RIGHT,
                  },
                    [this]() { SwitchByCombinedKey(CombineKeyCode::COMBINE_KEYCODE_SHIFT); } },
                { {
                      .preKeys = { MMI::KeyEvent::KEYCODE_CTRL_LEFT },
                      .finalKey = MMI::KeyEvent::KEYCODE_SHIFT_LEFT,
                  },
                    [this]() { SwitchByCombinedKey(CombineKeyCode::COMBINE_KEYCODE_CTRL_SHIFT); } },
                { {
                      .preKeys = { MMI::KeyEvent::KEYCODE_CTRL_LEFT },
                      .finalKey = MMI::KeyEvent::KEYCODE_SHIFT_RIGHT,
                  },
                    [this]() { SwitchByCombinedKey(CombineKeyCode::COMBINE_KEYCODE_CTRL_SHIFT); } },
                { {
                      .preKeys = { MMI::KeyEvent::KEYCODE_CTRL_RIGHT },
                      .finalKey = MMI::KeyEvent::KEYCODE_SHIFT_LEFT,
                  },
                    [this]() { SwitchByCombinedKey(CombineKeyCode::COMBINE_KEYCODE_CTRL_SHIFT); } },
                { {
                      .preKeys = { MMI::KeyEvent::KEYCODE_CTRL_RIGHT },
                      .finalKey = MMI::KeyEvent::KEYCODE_SHIFT_RIGHT,
                  },
                    [this]() { SwitchByCombinedKey(CombineKeyCode::COMBINE_KEYCODE_CTRL_SHIFT); } } });
        return 0;
    }
} // namespace MiscServices
} // namespace OHOS
