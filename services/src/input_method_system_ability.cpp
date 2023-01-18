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

#include "ability_connect_callback_proxy.h"
#include "ability_manager_errors.h"
#include "ability_manager_interface.h"
#include "application_info.h"
#include "bundle_mgr_proxy.h"
#include "combination_key.h"
#include "common_event_support.h"
#include "errors.h"
#include "global.h"
#include "im_common_event_manager.h"
#include "ime_cfg_manager.h"
#include "input_method_status.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "itypes_util.h"
#include "key_event.h"
#include "message_handler.h"
#include "os_account_manager.h"
#include "resource_manager.h"
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
constexpr int32_t INVALID_USER_ID = -1;
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
    std::map<int32_t, MessageHandler *>::const_iterator it2;
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
    auto cfg = ImeCfgManager::GetInstance().GetImeCfg(userId_);
    auto &currentIme = cfg.currentIme;
    bool isBegin = true;
    std::string params = "{\"imeList\":[";
    for (const auto &property : properties) {
        params += isBegin ? "" : "},";
        isBegin = false;

        std::string imeId = Str16ToStr8(property.mPackageName) + "/" + Str16ToStr8(property.mAbilityName);
        params += "{\"ime\": \"" + imeId + "\",";
        params += "\"labelId\": \"" + std::to_string(property.labelId) + "\",";
        params += "\"descriptionId\": \"" + std::to_string(property.descriptionId) + "\",";
        std::string isCurrentIme = currentIme == imeId ? "true" : "false";
        params += "\"isCurrentIme\": \"" + isCurrentIme + "\",";
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
    ImeCfgManager::GetInstance().Init();
    // 服务异常重启后不会走OnUserStarted，但是可以获取到当前userId
    // 设备启动时可能获取不到当前userId,如果获取不到，则等OnUserStarted的时候处理.
    std::vector<int32_t> userIds;
    if (OsAccountManager::QueryActiveOsAccountIds(userIds) == ERR_OK && !userIds.empty()) {
        userId_ = userIds[0];
        IMSA_HILOGI("InputMethodSystemAbility::get current userId success, userId: %{public}d", userId_);

        auto cfg = ImeCfgManager::GetInstance().GetImeCfg(userId_);
        auto newUserIme = cfg.currentIme;
        if (newUserIme.empty()) {
            newUserIme = ImeCfgManager::GetDefaultIme();
            ImeCfgManager::GetInstance().AddImeCfg({ userId_, newUserIme });
        }
        StartInputService(newUserIme);
    }

    StartUserIdListener();
    int32_t ret = InitKeyEventMonitor();
    IMSA_HILOGI("init KeyEvent monitor %{public}s", ret == ErrorCode::NO_ERROR ? "success" : "failed");
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
    userSessions.insert({ MAIN_USER_ID, std::make_shared<PerUserSession>(MAIN_USER_ID) });
    userId_ = INVALID_USER_ID;
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

    std::map<int32_t, MessageHandler *>::const_iterator it = msgHandlers.find(MAIN_USER_ID);
    if (it == msgHandlers.end()) {
        IMSA_HILOGE("InputMethodSystemAbility::StartInputService() need start handler");
        MessageHandler *handler = new MessageHandler();
        if (session) {
            IMSA_HILOGE("InputMethodSystemAbility::OnPrepareInput session is not nullptr");
            session->CreateWorkThread(*handler);
            msgHandlers.insert(std::pair<int32_t, MessageHandler *>(MAIN_USER_ID, handler));
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

/*! Handle the transaction from the remote binder
    \n Run in binder thread
    \param code transaction code number
    \param data the params from remote binder
    \param[out] reply the result of the transaction replied to the remote binder
    \param flags the flags of handling transaction
    \return int32_t
    */
int32_t InputMethodSystemAbility::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    return InputMethodSystemAbilityStub::OnRemoteRequest(code, data, reply, option);
}

int32_t InputMethodSystemAbility::PrepareInput(
    int32_t displayId, sptr<IInputClient> client, sptr<IInputDataChannel> channel, InputAttribute &attribute)
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
    return OnDisplayOptionalInputMethod(userId_);
};

int32_t InputMethodSystemAbility::ListInputMethod(InputMethodStatus status, std::vector<Property> &props)
{
    return ListInputMethodByUserId(userId_, status, props);
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
        if (iter->name == filter->name) {
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
    return ListSubtypeByBundleName(userId_, filter->name, subProps);
}

int32_t InputMethodSystemAbility::ListInputMethodSubtype(const std::string &name, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGI("InputMethodSystemAbility::ListInputMethodSubtype");
    return ListSubtypeByBundleName(userId_, name, subProps);
}

int32_t InputMethodSystemAbility::ListSubtypeByBundleName(
    int32_t userId, const std::string &name, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGI("InputMethodSystemAbility::ListSubtypeByBundleName");
    std::vector<AppExecFwk::ExtensionAbilityInfo> subtypeInfos;
    int32_t ret = QueryImeInfos(userId, subtypeInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Failed to query inputmethod infos");
        return ret;
    }
    auto bundleMgr = GetBundleMgr();
    if (bundleMgr == nullptr) {
        IMSA_HILOGE("Failed to GetBundleMgr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    for (const auto &subtypeInfo : subtypeInfos) {
        if (subtypeInfo.bundleName == name) {
            std::vector<Metadata> extends = subtypeInfo.metadata;
            auto property = GetExtends(extends);
            auto label =
                bundleMgr->GetStringById(subtypeInfo.bundleName, subtypeInfo.moduleName, subtypeInfo.labelId, userId);
            subProps.push_back({ .id = subtypeInfo.bundleName,
                .label = subtypeInfo.name,
                .name = label,
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
    IMSA_HILOGD("InputMethodSystemAbility name: %{public}s, subName: %{public}s", name.c_str(), subName.c_str());
    return subName.empty() ? SwitchInputMethodType(name) : SwitchInputMethodSubtype(name, subName);
}

int32_t InputMethodSystemAbility::SwitchInputMethodType(const std::string &name)
{
    IMSA_HILOGI("InputMethodSystemAbility::SwitchInputMethodType");
    std::vector<Property> properties = {};
    auto ret = ListInputMethodByUserId(userId_, ALL, properties);
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
    auto ret = ListSubtypeByBundleName(userId_, bundleName, subProps);
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
    auto cfg = ImeCfgManager::GetInstance().GetImeCfg(userId_);
    auto &currentIme = cfg.currentIme;
    if (currentIme.empty()) {
        IMSA_HILOGE("currentIme is empty");
        return ErrorCode::ERROR_PERSIST_CONFIG;
    }

    IMSA_HILOGI("CurrentIme : %{public}s, TargetIme : %{public}s", currentIme.c_str(), targetIme.c_str());
    if (currentIme == targetIme) {
        IMSA_HILOGI("currentIme and TargetIme are the same one");
        return ErrorCode::NO_ERROR;
    }
    StopInputService(currentIme);
    if (!StartInputService(targetIme)) {
        IMSA_HILOGE("start input method failed");
        return ErrorCode::ERROR_IME_START_FAILED;
    }
    ImeCfgManager::GetInstance().ModifyImeCfg({ userId_, targetIme });

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
    auto ret = ListAllInputMethod(userId_, props);
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
    auto ret = ListSubtypeByBundleName(userId_, bundleName, subProps);
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
    if (QueryImeInfos(userId, extensionInfos) != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Failed to query inputmethod infos");
        return {};
    }
    std::vector<InputMethodInfo> properties;
    for (auto extension : extensionInfos) {
        std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
        if (resourceManager == nullptr) {
            IMSA_HILOGE("InputMethodSystemAbility::ListInputMethodInfo resourcemanager is nullptr");
            break;
        }
        AppExecFwk::ApplicationInfo applicationInfo = extension.applicationInfo;
        std::string path = extension.hapPath.empty() ? extension.resourcePath : extension.hapPath;
        resourceManager->AddResource(path.c_str());
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
        properties.emplace_back(property);
    }
    return properties;
}

int32_t InputMethodSystemAbility::ListProperty(int32_t userId, std::vector<Property> &props)
{
    IMSA_HILOGI("InputMethodSystemAbility::ListProperty userId = %{public}d", userId);
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    int32_t ret = QueryImeInfos(userId, extensionInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Failed to query inputmethod infos");
        return ret;
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
    auto cfg = ImeCfgManager::GetInstance().GetImeCfg(userId_);
    auto &currentIme = cfg.currentIme;
    if (currentIme.empty()) {
        IMSA_HILOGE("InputMethodSystemAbility::GetCurrentInputMethod currentIme is empty");
        return nullptr;
    }

    auto pos = currentIme.find('/');
    if (pos == std::string::npos) {
        IMSA_HILOGE("InputMethodSystemAbility::GetCurrentInputMethod currentIme can not find '/'");
        return nullptr;
    }

    auto property = std::make_shared<Property>();
    if (property == nullptr) {
        IMSA_HILOGE("InputMethodSystemAbility property is nullptr");
        return nullptr;
    }
    property->name = currentIme.substr(0, pos);
    property->id = currentIme.substr(pos + 1, currentIme.length() - pos - 1);
    return property;
}

std::shared_ptr<SubProperty> InputMethodSystemAbility::GetCurrentInputMethodSubtype()
{
    IMSA_HILOGI("InputMethodSystemAbility::GetCurrentInputMethodSubtype");

    auto cfg = ImeCfgManager::GetInstance().GetImeCfg(userId_);
    auto &currentIme = cfg.currentIme;
    if (currentIme.empty()) {
        IMSA_HILOGE("InputMethodSystemAbility currentIme is empty");
        return nullptr;
    }
    auto pos = currentIme.find('/');
    if (pos == std::string::npos) {
        IMSA_HILOGE("InputMethodSystemAbility:: currentIme can not find '/'");
        return nullptr;
    }
    auto property = std::make_shared<SubProperty>(
        FindSubProperty(currentIme.substr(0, pos), currentIme.substr(pos + 1, currentIme.length() - pos - 1)));
    if (property == nullptr) {
        IMSA_HILOGE("property is nullptr");
        return nullptr;
    }
    return property;
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
            case MSG_ID_USER_START: {
                OnUserStarted(msg);
                break;
            }
            case MSG_ID_USER_REMOVED: {
                OnUserRemoved(msg);
                break;
            }
            case MSG_ID_PACKAGE_REMOVED: {
                OnPackageRemoved(msg);
                delete msg;
                msg = nullptr;
                break;
            }
            case MSG_ID_HIDE_KEYBOARD_SELF:{
                OnHandleMessage(msg);
                break;
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

bool InputMethodSystemAbility::IsImeInstalled(int32_t userId, std::string &imeId)
{
    std::vector<Property> props;
    ListAllInputMethod(userId, props);
    for (auto const &prop : props) {
        std::string ime = prop.name + "/" + prop.id;
        if (ime == imeId) {
            IMSA_HILOGI("true");
            return true;
        }
    }
    IMSA_HILOGI("false");
    return false;
}

/*! Called when a user is started. (EVENT_USER_STARTED is received)
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode
    */
int32_t InputMethodSystemAbility::OnUserStarted(const Message *msg)
{
    if (!msg->msgContent_) {
        IMSA_HILOGE("InputMethodSystemAbility::Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    std::string lastUserIme;
    if (userId_ != INVALID_USER_ID) {
        auto cfg = ImeCfgManager::GetInstance().GetImeCfg(userId_);
        lastUserIme = cfg.currentIme;
        if (lastUserIme.empty()) {
            IMSA_HILOGE("InputMethodSystemAbility::lastUserIme is empty");
        }
    }
    auto defaultIme = ImeCfgManager::GetDefaultIme();
    if (defaultIme.empty()) {
        IMSA_HILOGE("InputMethodSystemAbility::defaultIme is empty");
        return ErrorCode::ERROR_PERSIST_CONFIG;
    }

    int32_t newUserId = msg->msgContent_->ReadInt32();
    IMSA_HILOGI("lastUserId: %{public}d, newUserId: %{public}d", userId_, newUserId);
    userId_ = newUserId;
    auto it = userSessions.find(MAIN_USER_ID);
    if (it != userSessions.end()) {
        it->second->UpdateCurrentUserId(newUserId);
    }
    auto cfg = ImeCfgManager::GetInstance().GetImeCfg(newUserId);
    auto newUserIme = cfg.currentIme;
    if (newUserIme.empty()) {
        newUserIme = defaultIme;
        ImeCfgManager::GetInstance().AddImeCfg({ newUserId, newUserIme });
    } else if (!IsImeInstalled(newUserId, newUserIme)) {
        newUserIme = defaultIme;
        ImeCfgManager::GetInstance().ModifyImeCfg({ newUserId, newUserIme });
    }
    if (!lastUserIme.empty()) {
        IMSA_HILOGI("service restart or user switch");
        StopInputService(lastUserIme);
    }
    StartInputService(newUserIme);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::OnUserRemoved(const Message *msg)
{
    if (!msg->msgContent_) {
        IMSA_HILOGE("Aborted! %s", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    auto userId = msg->msgContent_->ReadInt32();
    IMSA_HILOGI("Start: %{public}d", userId);
    ImeCfgManager::GetInstance().DeleteImeCfg(userId);
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
    std::map<int32_t, MessageHandler *>::const_iterator it = msgHandlers.find(MAIN_USER_ID);
    if (it != msgHandlers.end()) {
        MessageHandler *handler = it->second;
        handler->SendMessage(msg);
    }
    return ErrorCode::NO_ERROR;
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
    if (data == nullptr) {
        IMSA_HILOGI("InputMethodSystemAbility::OnPackageRemoved data is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    int32_t userId = 0;
    std::string packageName = "";
    if (!ITypesUtil::Unmarshal(*data, userId, packageName)) {
        IMSA_HILOGE("Failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    // 用户移除也会有该通知，如果移除的app用户不是当前用户，则不处理
    if (userId != userId_) {
        return ErrorCode::NO_ERROR;
    }
    auto cfg = ImeCfgManager::GetInstance().GetImeCfg(userId);
    auto &currentIme = cfg.currentIme;
    if (currentIme.empty()) {
        IMSA_HILOGE("InputMethodSystemAbility::currentIme is empty");
        return ErrorCode::ERROR_PERSIST_CONFIG;
    }
    std::string::size_type pos = currentIme.find("/");
    std::string currentImeBundle = currentIme.substr(0, pos);
    if (packageName == currentImeBundle) {
        std::string defaultIme = ImeCfgManager::GetDefaultIme();
        if (defaultIme.empty()) {
            IMSA_HILOGE("InputMethodSystemAbility::defaultIme is empty");
            return ErrorCode::ERROR_PERSIST_CONFIG;
        }
        pos = defaultIme.find("/");
        int32_t ret =
            OnSwitchInputMethod(defaultIme.substr(0, pos), defaultIme.substr(pos + 1, defaultIme.length() - pos - 1));
        IMSA_HILOGI("InputMethodSystemAbility::OnPackageRemoved ret = %{public}d", ret);
    }
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
    if (ret == START_SERVICE_ABILITY_ACTIVATING) {
        return ErrorCode::ERROR_ABILITY_ACTIVATING;
    }
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("InputMethodSystemAbility::Start InputMethod ability failed, err = %{public}d", ret);
        return ErrorCode::ERROR_EX_SERVICE_SPECIFIC;
    }
    IMSA_HILOGI("InputMethodSystemAbility::Start InputMethod ability success.");
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
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    return iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
}

int32_t InputMethodSystemAbility::QueryImeInfos(int32_t userId, std::vector<AppExecFwk::ExtensionAbilityInfo> &infos)
{
    IMSA_HILOGI("InputMethodSystemAbility::QueryImeInfos");
    auto bundleMgr = GetBundleMgr();
    if (bundleMgr == nullptr) {
        IMSA_HILOGE("Failed to GetBundleMgr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (!bundleMgr->QueryExtensionAbilityInfos(AbilityType::INPUTMETHOD, userId, infos)) {
        IMSA_HILOGE("QueryExtensionAbilityInfos failed");
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    return ErrorCode::NO_ERROR;
}

sptr<AAFwk::IAbilityManager> InputMethodSystemAbility::GetAbilityManagerService()
{
    IMSA_HILOGD("InputMethodSystemAbility::GetAbilityManagerService start");
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGE("SystemAbilityManager is nullptr.");
        return nullptr;
    }

    auto abilityMsObj = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (abilityMsObj == nullptr) {
        IMSA_HILOGE("Failed to get ability manager service.");
        return nullptr;
    }

    return iface_cast<AAFwk::IAbilityManager>(abilityMsObj);
}

SubProperty InputMethodSystemAbility::FindSubPropertyByCompare(const std::string &bundleName, CompareHandler compare)
{
    IMSA_HILOGI("InputMethodSystemAbility::FindSubPropertyByCompare");
    std::vector<SubProperty> subProps = {};
    auto ret = ListSubtypeByBundleName(userId_, bundleName, subProps);
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

int32_t InputMethodSystemAbility::SwitchByCombinationKey(uint32_t state)
{
    IMSA_HILOGI("InputMethodSystemAbility::SwitchByCombinationKey");
    auto current = GetCurrentInputMethodSubtype();
    if (current == nullptr) {
        IMSA_HILOGE("GetCurrentInputMethodSubtype failed");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    if (CombinationKey::IsMatch(CombinationKeyFunction::SWITCH_MODE, state)) {
        IMSA_HILOGI("switch mode");
        auto target = current->mode == "upper"
                          ? FindSubPropertyByCompare(current->id,
                              [&current](const SubProperty &property) { return property.mode == "lower"; })
                          : FindSubPropertyByCompare(current->id,
                              [&current](const SubProperty &property) { return property.mode == "upper"; });
        return SwitchInputMethod(target.id, target.label);
    }
    if (CombinationKey::IsMatch(CombinationKeyFunction::SWITCH_LANGUAGE, state)) {
        IMSA_HILOGI("switch language");
        auto target = current->language == "chinese"
                          ? FindSubPropertyByCompare(current->id,
                              [&current](const SubProperty &property) { return property.language == "english"; })
                          : FindSubPropertyByCompare(current->id,
                              [&current](const SubProperty &property) { return property.language == "chinese"; });
        return SwitchInputMethod(target.id, target.label);
    }
    if (CombinationKey::IsMatch(CombinationKeyFunction::SWITCH_IME, state)) {
        IMSA_HILOGI("switch ime");
        std::vector<Property> props = {};
        auto ret = ListProperty(userId_, props);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("ListProperty failed");
            return ret;
        }
        auto iter = std::find_if(
            props.begin(), props.end(), [&current](const Property &property) { return property.name != current->id; });
        if (iter != props.end()) {
            return SwitchInputMethod(iter->name, iter->id);
        }
    }
    IMSA_HILOGD("keycode undefined");
    return ErrorCode::ERROR_EX_UNSUPPORTED_OPERATION;
}

int32_t InputMethodSystemAbility::InitKeyEventMonitor()
{
    IMSA_HILOGI("InputMethodSystemAbility::InitKeyEventMonitor");
    bool ret = ImCommonEventManager::GetInstance()->SubscribeKeyboardEvent(
        [this](uint32_t keyCode) { return SwitchByCombinationKey(keyCode); });
    return ret ? ErrorCode::NO_ERROR : ErrorCode::ERROR_SERVICE_START_FAILED;
}
} // namespace MiscServices
} // namespace OHOS
