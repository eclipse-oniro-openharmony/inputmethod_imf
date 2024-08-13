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

#include <unistd.h>

#include "ability_manager_client.h"
#include "application_info.h"
#include "combination_key.h"
#include "common_event_support.h"
#include "errors.h"
#include "full_ime_info_manager.h"
#include "global.h"
#include "im_common_event_manager.h"
#include "ime_cfg_manager.h"
#include "ime_info_inquirer.h"
#include "input_client_info.h"
#include "input_method_utils.h"
#include "input_type_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "itypes_util.h"
#include "key_event.h"
#include "mem_mgr_client.h"
#include "message_handler.h"
#include "native_token_info.h"
#include "os_account_adapter.h"
#include "scene_board_judgement.h"
#include "system_ability_definition.h"
#include "system_language_observer.h"
#include "user_session_manager.h"
#include "wms_connection_observer.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
using namespace AppExecFwk;
using namespace Security::AccessToken;
REGISTER_SYSTEM_ABILITY_BY_ID(InputMethodSystemAbility, INPUT_METHOD_SYSTEM_ABILITY_ID, true);
constexpr std::int32_t INIT_INTERVAL = 10000L;
static const std::string PERMISSION_CONNECT_IME_ABILITY = "ohos.permission.CONNECT_IME_ABILITY";
std::shared_ptr<AppExecFwk::EventHandler> InputMethodSystemAbility::serviceHandler_;

InputMethodSystemAbility::InputMethodSystemAbility(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate), state_(ServiceRunningState::STATE_NOT_START)
{
}

InputMethodSystemAbility::InputMethodSystemAbility() : state_(ServiceRunningState::STATE_NOT_START)
{
}

InputMethodSystemAbility::~InputMethodSystemAbility()
{
    stop_ = true;
    Message *msg = new Message(MessageID::MSG_ID_QUIT_WORKER_THREAD, nullptr);
    MessageHandler::Instance()->SendMessage(msg);
    if (workThreadHandler.joinable()) {
        workThreadHandler.join();
    }
}

void InputMethodSystemAbility::OnStart()
{
    IMSA_HILOGI("InputMethodSystemAbility::OnStart start.");
    if (!InputMethodSysEvent::GetInstance().StartTimerForReport()) {
        IMSA_HILOGE("start sysevent timer failed!");
    }
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        IMSA_HILOGI("imsa service is already running.");
        return;
    }
    InitServiceHandler();
    Initialize();
    int32_t ret = Init();
    if (ret != ErrorCode::NO_ERROR) {
        InputMethodSysEvent::GetInstance().ServiceFaultReporter("imf", ret);
        auto callback = [=]() { Init(); };
        serviceHandler_->PostTask(callback, INIT_INTERVAL);
        IMSA_HILOGE("init failed. try again 10s later!");
    }
    InitHiTrace();
    InputMethodSyncTrace tracer("InputMethodController Attach trace.");
    InputmethodDump::GetInstance().AddDumpAllMethod([this](int fd) { this->DumpAllMethod(fd); });
    IMSA_HILOGI("start imsa service success.");
    return;
}

int InputMethodSystemAbility::Dump(int fd, const std::vector<std::u16string> &args)
{
    IMSA_HILOGD("InputMethodSystemAbility::Dump start.");
    std::vector<std::string> argsStr;
    for (auto item : args) {
        argsStr.emplace_back(Str16ToStr8(item));
    }
    InputmethodDump::GetInstance().Dump(fd, argsStr);
    return ERR_OK;
}

void InputMethodSystemAbility::DumpAllMethod(int fd)
{
    IMSA_HILOGD("InputMethodSystemAbility::DumpAllMethod start.");
    auto ids = OsAccountAdapter::QueryActiveOsAccountIds();
    if (ids.empty()) {
        dprintf(fd, "\n - InputMethodSystemAbility::DumpAllMethod get Active Id failed.\n");
        return;
    }
    dprintf(fd, "\n - DumpAllMethod get Active Id succeed,count=%zu,", ids.size());
    for (auto id : ids) {
        const auto &params = ImeInfoInquirer::GetInstance().GetDumpInfo(id);
        if (params.empty()) {
            IMSA_HILOGD("userId: %{public}d the IME properties is empty.", id);
            dprintf(fd, "\n - The IME properties about the Active Id %d is empty.\n", id);
            continue;
        }
        dprintf(fd, "\n - The Active Id:%d get input method:\n%s\n", id, params.c_str());
    }
    IMSA_HILOGD("InputMethodSystemAbility::DumpAllMethod end.");
}

int32_t InputMethodSystemAbility::Init()
{
    bool isSuccess = Publish(this);
    if (!isSuccess) {
        return -1;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    ImeCfgManager::GetInstance().Init();
    ImeInfoInquirer::GetInstance().InitSystemConfig();
    InitMonitors();
    return ErrorCode::NO_ERROR;
}

void InputMethodSystemAbility::UpdateUserInfo(int32_t userId)
{
    IMSA_HILOGI("%{public}d switch to %{public}d.", userId_, userId);
    userId_ = userId;
    UserSessionManager::GetInstance().AddUserSession(userId_);
    InputMethodSysEvent::GetInstance().SetUserId(userId_);
    if (enableImeOn_) {
        EnableImeDataParser::GetInstance()->OnUserChanged(userId_);
    }
    if (enableSecurityMode_) {
        SecurityModeParser::GetInstance()->UpdateFullModeList(userId_);
    }
}

void InputMethodSystemAbility::OnStop()
{
    IMSA_HILOGI("OnStop start.");
    FreezeManager::SetEventHandler(nullptr);
    serviceHandler_ = nullptr;
    state_ = ServiceRunningState::STATE_NOT_START;
    Memory::MemMgrClient::GetInstance().NotifyProcessStatus(getpid(), 1, 0, INPUT_METHOD_SYSTEM_ABILITY_ID);
}

void InputMethodSystemAbility::InitServiceHandler()
{
    IMSA_HILOGI("InitServiceHandler start.");
    if (serviceHandler_ != nullptr) {
        IMSA_HILOGE("InputMethodSystemAbility already init.");
        return;
    }
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("OS_InputMethodSystemAbility");
    serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    FreezeManager::SetEventHandler(serviceHandler_);

    IMSA_HILOGI("InitServiceHandler succeeded.");
}

/**
 * Initialization of Input method management service
 * \n It's called after the service starts, before any transaction.
 */
void InputMethodSystemAbility::Initialize()
{
    IMSA_HILOGI("InputMethodSystemAbility::Initialize.");
    // init work thread to handle the messages
    workThreadHandler = std::thread([this] { this->WorkThread(); });
    identityChecker_ = std::make_shared<IdentityCheckerImpl>();
    userId_ = OsAccountAdapter::MAIN_USER_ID;
    UserSessionManager::GetInstance().SetEventHandler(serviceHandler_);
    UserSessionManager::GetInstance().AddUserSession(userId_);
    InputMethodSysEvent::GetInstance().SetUserId(userId_);
    isScbEnable_ = Rosen::SceneBoardJudgement::IsSceneBoardEnabled();
}

void InputMethodSystemAbility::SubscribeCommonEvent()
{
    sptr<ImCommonEventManager> imCommonEventManager = ImCommonEventManager::GetInstance();
    bool isSuccess = imCommonEventManager->SubscribeEvent();
    if (isSuccess) {
        IMSA_HILOGI("initialize subscribe service event success.");
        return;
    }

    IMSA_HILOGE("failed, try again 10s later");
    auto callback = [this]() { SubscribeCommonEvent(); };
    serviceHandler_->PostTask(callback, INIT_INTERVAL);
}

int32_t InputMethodSystemAbility::PrepareInput(int32_t userId, InputClientInfo &clientInfo)
{
    auto ret = GenerateClientInfo(userId, clientInfo);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnPrepareInput(clientInfo);
}

int32_t InputMethodSystemAbility::GenerateClientInfo(int32_t userId, InputClientInfo &clientInfo)
{
    if (clientInfo.client == nullptr || clientInfo.channel == nullptr) {
        IMSA_HILOGE("client or channel is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto deathRecipient = new (std::nothrow) InputDeathRecipient();
    if (deathRecipient == nullptr) {
        IMSA_HILOGE("failed to new deathRecipient!");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    clientInfo.pid = IPCSkeleton::GetCallingPid();
    clientInfo.uid = IPCSkeleton::GetCallingUid();
    clientInfo.userID = userId;
    clientInfo.deathRecipient = deathRecipient;
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (identityChecker_->IsFocusedUIExtension(tokenId)) {
        clientInfo.uiExtensionTokenId = tokenId;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::ReleaseInput(sptr<IInputClient> client)
{
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnReleaseInput(client);
};

int32_t InputMethodSystemAbility::StartInput(InputClientInfo &inputClientInfo, sptr<IRemoteObject> &agent)
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->IsBroker(tokenId)) {
        if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId)) {
            return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
        }
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (session->GetCurrentClientPid() != IPCSkeleton::GetCallingPid()
        && session->GetInactiveClientPid() != IPCSkeleton::GetCallingPid()) {
        // notify inputStart when caller pid different from both current client and inactive client
        inputClientInfo.isNotifyInputStart = true;
    }
    if (!session->IsProxyImeEnable()) {
        CheckInputTypeOption(userId, inputClientInfo);
    }
    int32_t ret = PrepareInput(userId, inputClientInfo);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to PrepareInput");
        return ret;
    }
    return session->OnStartInput(inputClientInfo, agent);
};

void InputMethodSystemAbility::CheckInputTypeOption(int32_t userId, InputClientInfo &inputClientInfo)
{
    IMSA_HILOGI("SecurityFlag: %{public}d, IsSameTextInput: %{public}d, IsStarted: %{public}d, "
                "IsSecurityImeStarted: %{public}d.",
        inputClientInfo.config.inputAttribute.GetSecurityFlag(), !inputClientInfo.isNotifyInputStart,
        InputTypeManager::GetInstance().IsStarted(), InputTypeManager::GetInstance().IsSecurityImeStarted());
    if (inputClientInfo.config.inputAttribute.GetSecurityFlag()) {
        if (!InputTypeManager::GetInstance().IsStarted()) {
            StartInputType(userId, InputType::SECURITY_INPUT);
            IMSA_HILOGI("SecurityFlag, input type is not started.");
            return;
        }
        if (!inputClientInfo.isNotifyInputStart) {
            IMSA_HILOGI("SecurityFlag, same text input.");
            return;
        }
        if (!InputTypeManager::GetInstance().IsSecurityImeStarted()) {
            StartInputType(userId, InputType::SECURITY_INPUT);
            IMSA_HILOGI("SecurityFlag, input type is started, but not security.");
            return;
        }
        IMSA_HILOGI("SecurityFlag others.");
        return;
    }
    if (inputClientInfo.isNotifyInputStart && InputTypeManager::GetInstance().IsStarted()) {
        IMSA_HILOGI("NormalFlag diff text input, input type started.");
        StartInputType(userId, InputType::NONE);
        return;
    }
    IMSA_HILOGI("NormalFlag success.");
}

int32_t InputMethodSystemAbility::ShowInput(sptr<IInputClient> client)
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (!identityChecker_->IsBroker(tokenId)) {
        if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId, session->GetCurrentClientPid())) {
            return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
        }
    }
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return session->OnShowInput(client);
}

int32_t InputMethodSystemAbility::HideInput(sptr<IInputClient> client)
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (!identityChecker_->IsBroker(tokenId)) {
        if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId, session->GetCurrentClientPid())) {
            return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
        }
    }
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return session->OnHideInput(client);
};

int32_t InputMethodSystemAbility::StopInputSession()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (!identityChecker_->IsBroker(tokenId)) {
        if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId, session->GetCurrentClientPid())) {
            return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
        }
    }
    return session->OnHideCurrentInput();
}

int32_t InputMethodSystemAbility::RequestShowInput()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId) &&
        !identityChecker_->HasPermission(tokenId, PERMISSION_CONNECT_IME_ABILITY)) {
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnRequestShowInput();
}

int32_t InputMethodSystemAbility::RequestHideInput()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId) &&
        !identityChecker_->HasPermission(tokenId, PERMISSION_CONNECT_IME_ABILITY)) {
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnRequestHideInput();
}

int32_t InputMethodSystemAbility::SetCoreAndAgent(const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent)
{
    IMSA_HILOGD("InputMethodSystemAbility start.");
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (IsCurrentIme(userId)) {
        return session->OnSetCoreAndAgent(core, agent);
    }
    if (identityChecker_->IsNativeSa(IPCSkeleton::GetCallingTokenID())) {
        return session->OnRegisterProxyIme(core, agent);
    }
    return ErrorCode::ERROR_NOT_CURRENT_IME;
}

int32_t InputMethodSystemAbility::HideCurrentInput()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (identityChecker_->IsBroker(tokenId)) {
        return session->OnHideCurrentInput();
    }
    if (!identityChecker_->HasPermission(tokenId, PERMISSION_CONNECT_IME_ABILITY)) {
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    return session->OnHideCurrentInput();
};

int32_t InputMethodSystemAbility::ShowCurrentInput()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (identityChecker_->IsBroker(tokenId)) {
        return session->OnShowCurrentInput();
    }
    if (!identityChecker_->HasPermission(tokenId, PERMISSION_CONNECT_IME_ABILITY)) {
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    return session->OnShowCurrentInput();
};

int32_t InputMethodSystemAbility::PanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info)
{
    auto userId = GetCallingUserId();
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    if (!identityChecker_->IsBundleNameValid(IPCSkeleton::GetCallingTokenID(), currentImeCfg->bundleName)) {
        IMSA_HILOGE("not current ime!");
        return ErrorCode::ERROR_NOT_CURRENT_IME;
    }
    auto commonEventManager = ImCommonEventManager::GetInstance();
    if (commonEventManager != nullptr) {
        auto ret = ImCommonEventManager::GetInstance()->PublishPanelStatusChangeEvent(userId, status, info);
        IMSA_HILOGD("public panel status change event: %{public}d", ret);
    }
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnPanelStatusChange(status, info);
}

int32_t InputMethodSystemAbility::UpdateListenEventFlag(InputClientInfo &clientInfo, uint32_t eventFlag)
{
    IMSA_HILOGI("finalEventFlag: %{public}u, eventFlag: %{public}u.", clientInfo.eventFlag, eventFlag);
    if (EventStatusManager::IsImeHideOn(eventFlag) || EventStatusManager::IsImeShowOn(eventFlag)) {
        if (!identityChecker_->IsSystemApp(IPCSkeleton::GetCallingFullTokenID()) &&
            !identityChecker_->IsNativeSa(IPCSkeleton::GetCallingTokenID())) {
            IMSA_HILOGE("not system application!");
            return ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION;
        }
    }
    auto userId = GetCallingUserId();
    auto ret = GenerateClientInfo(userId, clientInfo);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnUpdateListenEventFlag(clientInfo);
}

bool InputMethodSystemAbility::IsCurrentIme()
{
    return IsCurrentIme(GetCallingUserId());
}

bool InputMethodSystemAbility::IsInputTypeSupported(InputType type)
{
    return InputTypeManager::GetInstance().IsSupported(type);
}

int32_t InputMethodSystemAbility::StartInputType(InputType type)
{
    return StartInputType(GetCallingUserId(), type);
}

int32_t InputMethodSystemAbility::ExitCurrentInputType()
{
    auto userId = GetCallingUserId();
    auto ret = IsDefaultImeFromTokenId(userId, IPCSkeleton::GetCallingTokenID());
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("not default ime.");
        return ErrorCode::ERROR_NOT_DEFAULT_IME;
    }
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (session->CheckSecurityMode()) {
        return StartInputType(userId, InputType::SECURITY_INPUT);
    }
    return StartInputType(userId, InputType::NONE);
}

int32_t InputMethodSystemAbility::IsDefaultIme()
{
    return IsDefaultImeFromTokenId(GetCallingUserId(), IPCSkeleton::GetCallingTokenID());
}

int32_t InputMethodSystemAbility::IsDefaultImeFromTokenId(int32_t userId, uint32_t tokenId)
{
    auto prop = std::make_shared<Property>();
    auto ret = ImeInfoInquirer::GetInstance().GetDefaultInputMethod(userId, prop);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to get default ime!");
        return ErrorCode::ERROR_PERSIST_CONFIG;
    }
    if (!identityChecker_->IsBundleNameValid(tokenId, prop->name)) {
        return ErrorCode::ERROR_NOT_DEFAULT_IME;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::IsPanelShown(const PanelInfo &panelInfo, bool &isShown)
{
    if (!identityChecker_->IsSystemApp(IPCSkeleton::GetCallingFullTokenID())) {
        IMSA_HILOGE("not system application!");
        return ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION;
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->IsPanelShown(panelInfo, isShown);
}

int32_t InputMethodSystemAbility::DisplayOptionalInputMethod()
{
    IMSA_HILOGD("InputMethodSystemAbility start.");
    return OnDisplayOptionalInputMethod();
}

int32_t InputMethodSystemAbility::SwitchInputMethod(const std::string &bundleName, const std::string &subName,
    SwitchTrigger trigger)
{
    // IMSA not check permission, add this verify for prevent counterfeit
    if (trigger == SwitchTrigger::IMSA) {
        IMSA_HILOGW("caller counterfeit!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    SwitchInfo switchInfo = { std::chrono::system_clock::now(), bundleName, subName };
    int32_t userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (enableImeOn_ && !EnableImeDataParser::GetInstance()->CheckNeedSwitch(switchInfo, userId)) {
        IMSA_HILOGW("Enable mode off or switch is not enable, stopped!");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    switchInfo.timestamp = std::chrono::system_clock::now();
    session->GetSwitchQueue().Push(switchInfo);
    return InputTypeManager::GetInstance().IsInputType({ bundleName, subName })
               ? OnStartInputType(userId, switchInfo, true)
               : OnSwitchInputMethod(userId, switchInfo, trigger);
}

int32_t InputMethodSystemAbility::OnSwitchInputMethod(int32_t userId, const SwitchInfo &switchInfo,
    SwitchTrigger trigger)
{
    IMSA_HILOGD("start, switchInfo: %{public}s|%{public}s", switchInfo.bundleName.c_str(), switchInfo.subName.c_str());
    InputMethodSysEvent::GetInstance().RecordEvent(IMEBehaviour::CHANGE_IME);
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (!session->GetSwitchQueue().IsReady(switchInfo)) {
        IMSA_HILOGD("start wait.");
        session->GetSwitchQueue().Wait(switchInfo);
    }
    IMSA_HILOGI("start switch %{public}s|%{public}s.", switchInfo.bundleName.c_str(), switchInfo.subName.c_str());
    int32_t ret = CheckSwitchPermission(userId, switchInfo, trigger);
    if (ret != ErrorCode::NO_ERROR) {
        InputMethodSysEvent::GetInstance().InputmethodFaultReporter(ErrorCode::ERROR_STATUS_PERMISSION_DENIED,
            switchInfo.bundleName, "switch input method failed!");
        session->GetSwitchQueue().Pop();
        return ret;
    }

    if (!InputTypeManager::GetInstance().IsStarted()
        && !IsNeedSwitch(userId, switchInfo.bundleName, switchInfo.subName)) {
        session->GetSwitchQueue().Pop();
        return ErrorCode::NO_ERROR;
    }
    auto info = ImeInfoInquirer::GetInstance().GetImeInfo(userId, switchInfo.bundleName, switchInfo.subName);
    if (info == nullptr) {
        session->GetSwitchQueue().Pop();
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    {
        InputMethodSyncTrace tracer("InputMethodSystemAbility_OnSwitchInputMethod");
        ret = info->isNewIme ? Switch(userId, switchInfo.bundleName, info) : SwitchExtension(userId, info);
    }
    if (InputTypeManager::GetInstance().IsStarted()) {
        InputTypeManager::GetInstance().Set(false);
    }
    session->GetSwitchQueue().Pop();
    if (ret != ErrorCode::NO_ERROR) {
        InputMethodSysEvent::GetInstance().InputmethodFaultReporter(ret, switchInfo.bundleName,
            "switch input method failed!");
    }
    return ret;
}

int32_t InputMethodSystemAbility::OnStartInputType(int32_t userId, const SwitchInfo &switchInfo,
    bool isCheckPermission)
{
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (!session->GetSwitchQueue().IsReady(switchInfo)) {
        IMSA_HILOGD("start wait.");
        session->GetSwitchQueue().Wait(switchInfo);
    }
    auto cfgIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    if (switchInfo.bundleName == cfgIme->bundleName && switchInfo.subName == cfgIme->subName) {
        IMSA_HILOGD("input type is current ime, exit input type.");
        int32_t ret = session->ExitCurrentInputType();
        session->GetSwitchQueue().Pop();
        return ret;
    }
    IMSA_HILOGD("start switch %{public}s|%{public}s.", switchInfo.bundleName.c_str(), switchInfo.subName.c_str());
    if (isCheckPermission && !IsStartInputTypePermitted(userId)) {
        IMSA_HILOGE("not permitted to start input type!");
        session->GetSwitchQueue().Pop();
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    if (!IsNeedSwitch(userId, switchInfo.bundleName, switchInfo.subName)) {
        IMSA_HILOGI("no need to switch.");
        session->GetSwitchQueue().Pop();
        return ErrorCode::NO_ERROR;
    }
    int32_t ret = SwitchInputType(userId, switchInfo);
    session->GetSwitchQueue().Pop();
    return ret;
}

bool InputMethodSystemAbility::IsNeedSwitch(int32_t userId, const std::string &bundleName,
    const std::string &subName)
{
    if (InputTypeManager::GetInstance().IsStarted()) {
        ImeIdentification target = { bundleName, subName };
        return !(target == InputTypeManager::GetInstance().GetCurrentIme());
    }
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    IMSA_HILOGI("currentIme: %{public}s/%{public}s, targetIme: %{public}s/%{public}s.",
        currentImeCfg->bundleName.c_str(), currentImeCfg->subName.c_str(), bundleName.c_str(), subName.c_str());
    if ((subName.empty() && bundleName == currentImeCfg->bundleName) ||
        (!subName.empty() && subName == currentImeCfg->subName && currentImeCfg->bundleName == bundleName)) {
        IMSA_HILOGI("no need to switch.");
        return false;
    }
    return true;
}

int32_t InputMethodSystemAbility::Switch(int32_t userId, const std::string &bundleName,
    const std::shared_ptr<ImeInfo> &info)
{
    auto currentImeBundleName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId)->bundleName;
    if (bundleName != currentImeBundleName) {
        IMSA_HILOGI("switch input method to: %{public}s", bundleName.c_str());
        return SwitchExtension(userId, info);
    }
    auto currentInputType = InputTypeManager::GetInstance().GetCurrentIme();
    auto isInputTypeStarted = InputTypeManager::GetInstance().IsStarted();
    if (isInputTypeStarted && bundleName != currentInputType.bundleName) {
        IMSA_HILOGI("right click on state, switch input method to: %{public}s", bundleName.c_str());
        return SwitchExtension(userId, info);
    }
    return SwitchSubType(userId, info);
}

// Switch the current InputMethodExtension to the new InputMethodExtension
int32_t InputMethodSystemAbility::SwitchExtension(int32_t userId, const std::shared_ptr<ImeInfo> &info)
{
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    session->StopCurrentIme();
    std::string targetImeName = info->prop.name + "/" + info->prop.id;
    ImeCfgManager::GetInstance().ModifyImeCfg({ userId, targetImeName, info->subProp.id });
    ImeNativeCfg targetIme = { targetImeName, info->prop.name, info->subProp.id, info->prop.id };
    if (!session->StartInputService(std::make_shared<ImeNativeCfg>(targetIme))) {
        IMSA_HILOGE("start input method failed!");
        return ErrorCode::ERROR_IME_START_FAILED;
    }
    session->NotifyImeChangeToClients(info->prop, info->subProp);
    return ErrorCode::NO_ERROR;
}

// Inform current InputMethodExtension to switch subtype
int32_t InputMethodSystemAbility::SwitchSubType(int32_t userId, const std::shared_ptr<ImeInfo> &info)
{
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto ret = session->SwitchSubtype(info->subProp);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to inform ime to switch subtype, ret: %{public}d!", ret);
        return ret;
    }
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId)->imeId;
    ImeCfgManager::GetInstance().ModifyImeCfg({ userId, currentIme, info->subProp.id });
    session->NotifyImeChangeToClients(info->prop, info->subProp);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::SwitchInputType(int32_t userId, const SwitchInfo &switchInfo)
{
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    bool checkSameIme = InputTypeManager::GetInstance().IsStarted()
                            ? switchInfo.bundleName == InputTypeManager::GetInstance().GetCurrentIme().bundleName
                            : switchInfo.bundleName == currentIme->bundleName;
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (checkSameIme) {
        IMSA_HILOGD("only need to switch subtype: %{public}s.", switchInfo.subName.c_str());
        auto ret = session->SwitchSubtype({ .name = switchInfo.bundleName, .id = switchInfo.subName });
        if (ret == ErrorCode::NO_ERROR) {
            InputTypeManager::GetInstance().Set(true, { switchInfo.bundleName, switchInfo.subName });
        }
        return ret;
    }
    IMSA_HILOGD("need to switch ime: %{public}s|%{public}s.", switchInfo.bundleName.c_str(),
        switchInfo.subName.c_str());
    auto targetImeProperty = ImeInfoInquirer::GetInstance().GetImeProperty(userId, switchInfo.bundleName);
    if (targetImeProperty == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    session->StopCurrentIme();
    std::string targetName = switchInfo.bundleName + "/" + targetImeProperty->id;
    ImeNativeCfg targetIme = { targetName, switchInfo.bundleName, switchInfo.subName, targetImeProperty->id };
    InputTypeManager::GetInstance().Set(true, { switchInfo.bundleName, switchInfo.subName });
    if (!session->StartInputService(std::make_shared<ImeNativeCfg>(targetIme))) {
        IMSA_HILOGE("start input method failed!");
        InputTypeManager::GetInstance().Set(false);
        return ErrorCode::ERROR_IME_START_FAILED;
    }
    int32_t ret = session->SwitchSubtype({ .name = switchInfo.bundleName, .id = switchInfo.subName });
    if (ret != ErrorCode::NO_ERROR) {
        InputTypeManager::GetInstance().Set(false);
        return ret;
    }
    return ErrorCode::NO_ERROR;
}

// Deprecated because of no permission check, kept for compatibility
int32_t InputMethodSystemAbility::HideCurrentInputDeprecated()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    int32_t userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (!identityChecker_->IsBroker(tokenId)) {
        if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId, session->GetCurrentClientPid())) {
            return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
        }
    }
    return session->OnHideCurrentInput();
};

int32_t InputMethodSystemAbility::ShowCurrentInputDeprecated()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    int32_t userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (!identityChecker_->IsBroker(tokenId)) {
        if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId, session->GetCurrentClientPid())) {
            return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
        }
    }
    return session->OnShowCurrentInput();
};

std::shared_ptr<Property> InputMethodSystemAbility::GetCurrentInputMethod()
{
    return ImeInfoInquirer::GetInstance().GetCurrentInputMethod(GetCallingUserId());
}

std::shared_ptr<SubProperty> InputMethodSystemAbility::GetCurrentInputMethodSubtype()
{
    return ImeInfoInquirer::GetInstance().GetCurrentSubtype(GetCallingUserId());
}

int32_t InputMethodSystemAbility::GetDefaultInputMethod(std::shared_ptr<Property> &prop, bool isBrief)
{
    return ImeInfoInquirer::GetInstance().GetDefaultInputMethod(GetCallingUserId(), prop, isBrief);
}

int32_t InputMethodSystemAbility::GetInputMethodConfig(OHOS::AppExecFwk::ElementName &inputMethodConfig)
{
    return ImeInfoInquirer::GetInstance().GetInputMethodConfig(GetCallingUserId(), inputMethodConfig);
}

int32_t InputMethodSystemAbility::ListInputMethod(InputMethodStatus status, std::vector<Property> &props)
{
    return ImeInfoInquirer::GetInstance().ListInputMethod(GetCallingUserId(), status, props, enableImeOn_);
}

int32_t InputMethodSystemAbility::ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProps)
{
    return ImeInfoInquirer::GetInstance().ListCurrentInputMethodSubtype(GetCallingUserId(), subProps);
}

int32_t InputMethodSystemAbility::ListInputMethodSubtype(const std::string &bundleName,
    std::vector<SubProperty> &subProps)
{
    return ImeInfoInquirer::GetInstance().ListInputMethodSubtype(GetCallingUserId(), bundleName, subProps);
}

/**
 * Work Thread of input method management service
 * \n Remote commands which may change the state or data in the service will be handled sequentially in this thread.
 */
void InputMethodSystemAbility::WorkThread()
{
    pthread_setname_np(pthread_self(), "OS_IMSAWorkThread");
    while (!stop_) {
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
            case MSG_ID_USER_STOP: {
                OnUserStop(msg);
                break;
            }
            case MSG_ID_HIDE_KEYBOARD_SELF: {
                OnHideKeyboardSelf(msg);
                break;
            }
            case MSG_ID_BUNDLE_SCAN_FINISHED: {
                RegisterDataShareObserver();
                FullImeInfoManager::GetInstance().Init();
                break;
            }
            case MSG_ID_PACKAGE_ADDED:
            case MSG_ID_PACKAGE_CHANGED:
            case MSG_ID_PACKAGE_REMOVED: {
                HandlePackageEvent(msg);
                break;
            }
            case MSG_ID_SYS_LANGUAGE_CHANGED: {
                FullImeInfoManager::GetInstance().Update();
                break;
            }
            case MSG_ID_BOOT_COMPLETED:
            case MSG_ID_OS_ACCOUNT_STARTED: {
                FullImeInfoManager::GetInstance().Init();
                break;
            }
            default: {
                IMSA_HILOGD("the message is %{public}d.", msg->msgId_);
                break;
            }
        }
        delete msg;
    }
}

/**
 * Called when a user is started. (EVENT_USER_STARTED is received)
 * \n Run in work thread of input method management service
 * \param msg the parameters are saved in msg->msgContent_
 * \return ErrorCode
 */
int32_t InputMethodSystemAbility::OnUserStarted(const Message *msg)
{
    if (msg->msgContent_ == nullptr) {
        IMSA_HILOGE("msgContent_ is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto newUserId = msg->msgContent_->ReadInt32();
    FullImeInfoManager::GetInstance().Add(newUserId);
    // if scb enable, deal when receive wmsConnected.
    if (isScbEnable_) {
        return ErrorCode::NO_ERROR;
    }
    if (newUserId == userId_) {
        return ErrorCode::NO_ERROR;
    }
    HandleUserSwitched(newUserId);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::OnUserRemoved(const Message *msg)
{
    if (msg == nullptr || msg->msgContent_ == nullptr) {
        IMSA_HILOGE("Aborted! Message is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto userId = msg->msgContent_->ReadInt32();
    IMSA_HILOGI("start: %{public}d", userId);
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session != nullptr) {
        session->StopCurrentIme();
        UserSessionManager::GetInstance().RemoveUserSession(userId);
    }
    ImeCfgManager::GetInstance().DeleteImeCfg(userId);
    FullImeInfoManager::GetInstance().Delete(userId);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::OnUserStop(const Message *msg)
{
    if (msg == nullptr || msg->msgContent_ == nullptr) {
        IMSA_HILOGE("Aborted! Message is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto userId = msg->msgContent_->ReadInt32();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    session->StopCurrentIme();
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::OnHideKeyboardSelf(const Message *msg)
{
    if (msg == nullptr || msg->msgContent_ == nullptr) {
        IMSA_HILOGE("Aborted! Message is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto userId = msg->msgContent_->ReadInt32();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    session->OnHideSoftKeyBoardSelf();
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::HandlePackageEvent(const Message *msg)
{
    MessageParcel *data = msg->msgContent_;
    if (data == nullptr) {
        IMSA_HILOGD("data is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    int32_t userId = 0;
    std::string packageName;
    if (!ITypesUtil::Unmarshal(*data, userId, packageName)) {
        IMSA_HILOGE("Failed to read message parcel");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    if (msg->msgId_ == MSG_ID_PACKAGE_CHANGED) {
        return FullImeInfoManager::GetInstance().Update(userId, packageName);
    }
    if (msg->msgId_ == MSG_ID_PACKAGE_ADDED) {
        return FullImeInfoManager::GetInstance().Add(userId, packageName);
    }
    if (msg->msgId_ == MSG_ID_PACKAGE_REMOVED) {
        return OnPackageRemoved(userId, packageName);
    }
    return ErrorCode::NO_ERROR;
}

/**
 *  Called when a package is removed.
 *  \n Run in work thread of input method management service
 *  \param msg the parameters are saved in msg->msgContent_
 *  \return ErrorCode::NO_ERROR
 *  \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
 *  \return ErrorCode::ERROR_BAD_PARAMETERS bad parameter
 */
int32_t InputMethodSystemAbility::OnPackageRemoved(int32_t userId, const std::string &packageName)
{
    FullImeInfoManager::GetInstance().Delete(userId, packageName);
    // if the app that doesn't belong to current user is removed, ignore it
    if (userId != userId_) {
        IMSA_HILOGD("userId: %{public}d, currentUserId: %{public}d.", userId, userId_);
        return ErrorCode::NO_ERROR;
    }
    auto currentImeBundle = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId)->bundleName;
    if (packageName == currentImeBundle) {
        // Switch to the default ime
        IMSA_HILOGI("user[%{public}d] ime: %{public}s is uninstalled.", userId, packageName.c_str());
        auto info = ImeInfoInquirer::GetInstance().GetDefaultImeInfo(userId);
        if (info == nullptr) {
            return ErrorCode::ERROR_PERSIST_CONFIG;
        }
        int32_t ret = SwitchExtension(userId, info);
        IMSA_HILOGI("switch ret: %{public}d", ret);
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::OnDisplayOptionalInputMethod()
{
    IMSA_HILOGD("InputMethodSystemAbility::OnDisplayOptionalInputMethod start.");
    AAFwk::Want want;
    want.SetAction(SELECT_DIALOG_ACTION);
    want.SetElementName(SELECT_DIALOG_HAP, SELECT_DIALOG_ABILITY);
    int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want);
    if (ret != ErrorCode::NO_ERROR && ret != START_SERVICE_ABILITY_ACTIVATING) {
        IMSA_HILOGE("start InputMethod ability failed, err: %{public}d", ret);
        return ErrorCode::ERROR_EX_SERVICE_SPECIFIC;
    }
    IMSA_HILOGI("start InputMethod ability success.");
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::SwitchByCombinationKey(uint32_t state)
{
    IMSA_HILOGD("InputMethodSystemAbility::SwitchByCombinationKey start.");
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId_);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (session->IsProxyImeEnable()) {
        IMSA_HILOGI("proxy enable, not switch.");
        return ErrorCode::NO_ERROR;
    }
    if (CombinationKey::IsMatch(CombinationKeyFunction::SWITCH_MODE, state)) {
        IMSA_HILOGI("switch mode.");
        return SwitchMode();
    }
    if (CombinationKey::IsMatch(CombinationKeyFunction::SWITCH_LANGUAGE, state)) {
        IMSA_HILOGI("switch language.");
        return SwitchLanguage();
    }
    if (CombinationKey::IsMatch(CombinationKeyFunction::SWITCH_IME, state)) {
        IMSA_HILOGI("switch ime.");
        DealSwitchRequest();
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGE("keycode is undefined");
    return ErrorCode::ERROR_EX_UNSUPPORTED_OPERATION;
}

void InputMethodSystemAbility::DealSecurityChange()
{
    {
        std::lock_guard<std::mutex> lock(modeChangeMutex_);
        if (isChangeHandling_) {
            IMSA_HILOGI("already has mode change task.");
            hasPendingChanges_ = true;
            return;
        } else {
            isChangeHandling_ = true;
            hasPendingChanges_ = true;
        }
    }
    auto changeTask = [this]() {
        pthread_setname_np(pthread_self(), "SecurityChange");
        auto checkChangeCount = [this]() {
            std::lock_guard<std::mutex> lock(modeChangeMutex_);
            if (hasPendingChanges_) {
                return true;
            }
            isChangeHandling_ = false;
            return false;
        };
        do {
            OnSecurityModeChange();
        } while (checkChangeCount());
    };
    std::thread(changeTask).detach();
}

void InputMethodSystemAbility::DealSwitchRequest()
{
    {
        std::lock_guard<std::mutex> lock(switchImeMutex_);
        // 0 means current swich ime task count.
        if (switchTaskExecuting_.load()) {
            IMSA_HILOGI("already has switch ime task.");
            ++targetSwitchCount_;
            return;
        } else {
            switchTaskExecuting_.store(true);
            ++targetSwitchCount_;
        }
    }
    auto switchTask = [this]() {
        auto checkSwitchCount = [this]() {
            std::lock_guard<std::mutex> lock(switchImeMutex_);
            if (targetSwitchCount_ > 0) {
                return true;
            }
            switchTaskExecuting_.store(false);
            return false;
        };
        do {
            SwitchType();
        } while (checkSwitchCount());
    };
    // 0 means delay time is 0.
    serviceHandler_->PostTask(switchTask, "SwitchImeTask", 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

int32_t InputMethodSystemAbility::SwitchMode()
{
    auto bundleName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName;
    auto subName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->subName;
    auto info = ImeInfoInquirer::GetInstance().GetImeInfo(userId_, bundleName, subName);
    if (info == nullptr) {
        IMSA_HILOGE("current ime is abnormal!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    if (info->isNewIme) {
        IMSA_HILOGD("the switching operation is handed over to ime.");
        return ErrorCode::NO_ERROR;
    }
    auto condition = info->subProp.mode == "upper" ? Condition::LOWER : Condition::UPPER;
    auto target = ImeInfoInquirer::GetInstance().FindTargetSubtypeByCondition(info->subProps, condition);
    if (target == nullptr) {
        IMSA_HILOGE("target is empty!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    SwitchInfo switchInfo = { std::chrono::system_clock::now(), target->name, target->id };
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId_);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    session->GetSwitchQueue().Push(switchInfo);
    return OnSwitchInputMethod(userId_, switchInfo, SwitchTrigger::IMSA);
}

int32_t InputMethodSystemAbility::SwitchLanguage()
{
    auto bundleName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName;
    auto subName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->subName;
    auto info = ImeInfoInquirer::GetInstance().GetImeInfo(userId_, bundleName, subName);
    if (info == nullptr) {
        IMSA_HILOGE("current ime is abnormal!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    if (info->isNewIme) {
        IMSA_HILOGD("the switching operation is handed over to ime.");
        return ErrorCode::NO_ERROR;
    }
    if (info->subProp.language != "chinese" && info->subProp.language != "english") {
        return ErrorCode::NO_ERROR;
    }
    auto condition = info->subProp.language == "chinese" ? Condition::ENGLISH : Condition::CHINESE;
    auto target = ImeInfoInquirer::GetInstance().FindTargetSubtypeByCondition(info->subProps, condition);
    if (target == nullptr) {
        IMSA_HILOGE("target is empty!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    SwitchInfo switchInfo = { std::chrono::system_clock::now(), target->name, target->id };
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId_);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    session->GetSwitchQueue().Push(switchInfo);
    return OnSwitchInputMethod(userId_, switchInfo, SwitchTrigger::IMSA);
}

int32_t InputMethodSystemAbility::SwitchType()
{
    SwitchInfo switchInfo = { std::chrono::system_clock::now(), "", "" };
    uint32_t cacheCount = 0;
    {
        std::lock_guard<std::mutex> lock(switchImeMutex_);
        cacheCount = targetSwitchCount_.exchange(0);
    }
    int32_t ret =
        ImeInfoInquirer::GetInstance().GetSwitchInfoBySwitchCount(switchInfo, userId_, enableImeOn_, cacheCount);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("get next SwitchInfo failed, stop switching ime.");
        return ret;
    }
    IMSA_HILOGD("switch to: %{public}s.", switchInfo.bundleName.c_str());
    switchInfo.timestamp = std::chrono::system_clock::now();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId_);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    session->GetSwitchQueue().Push(switchInfo);
    return OnSwitchInputMethod(userId_, switchInfo, SwitchTrigger::IMSA);
}

void InputMethodSystemAbility::InitMonitors()
{
    int32_t ret = InitAccountMonitor();
    IMSA_HILOGI("init account monitor, ret: %{public}d.", ret);
    SubscribeCommonEvent();
    ret = InitMemMgrMonitor();
    IMSA_HILOGI("init MemMgr monitor, ret: %{public}d.", ret);
    ret = InitKeyEventMonitor();
    IMSA_HILOGI("init KeyEvent monitor, ret: %{public}d.", ret);
    ret = InitWmsMonitor();
    IMSA_HILOGI("init wms monitor, ret: %{public}d.", ret);
    InitSystemLanguageMonitor();
    if (ImeInfoInquirer::GetInstance().IsEnableInputMethod()) {
        IMSA_HILOGW("Enter enable mode.");
        EnableImeDataParser::GetInstance()->Initialize(userId_);
        enableImeOn_ = true;
    }
    if (ImeInfoInquirer::GetInstance().IsEnableSecurityMode()) {
        IMSA_HILOGW("Enter security mode.");
        enableSecurityMode_ = true;
    }
}

int32_t InputMethodSystemAbility::RegisterDataShareObserver()
{
    IMSA_HILOGD("start.");
    if (enableImeOn_) {
        RegisterEnableImeObserver();
    }
    if (enableSecurityMode_) {
        RegisterSecurityModeObserver();
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::InitAccountMonitor()
{
    IMSA_HILOGI("InputMethodSystemAbility::InitAccountMonitor start.");
    return ImCommonEventManager::GetInstance()->SubscribeAccountManagerService([this]() { HandleOsAccountStarted(); });
}

int32_t InputMethodSystemAbility::InitKeyEventMonitor()
{
    IMSA_HILOGI("InputMethodSystemAbility::InitKeyEventMonitor start.");
    bool ret = ImCommonEventManager::GetInstance()->SubscribeKeyboardEvent(
        [this](uint32_t keyCode) { return SwitchByCombinationKey(keyCode); });
    return ret ? ErrorCode::NO_ERROR : ErrorCode::ERROR_SERVICE_START_FAILED;
}

bool InputMethodSystemAbility::InitWmsMonitor()
{
    return ImCommonEventManager::GetInstance()->SubscribeWindowManagerService([this]() { HandleWmsStarted(); });
}

bool InputMethodSystemAbility::InitMemMgrMonitor()
{
    return ImCommonEventManager::GetInstance()->SubscribeMemMgrService([this]() { HandleMemStarted(); });
}

void InputMethodSystemAbility::InitWmsConnectionMonitor()
{
    WmsConnectionMonitorManager::GetInstance().RegisterWMSConnectionChangedListener(
        [this](bool isConnected, int32_t userId, int32_t screenId) {
            isConnected ? HandleWmsConnected(userId, screenId) : HandleWmsDisconnected(userId, screenId);
        });
}

void InputMethodSystemAbility::InitSystemLanguageMonitor()
{
    SystemLanguageObserver::GetInstance().Watch();
}

void InputMethodSystemAbility::InitFocusChangedMonitor()
{
    FocusMonitorManager::GetInstance().RegisterFocusChangedListener(
        [this](bool isOnFocused, int32_t pid, int32_t uid) { HandleFocusChanged(isOnFocused, pid, uid); });
}

void InputMethodSystemAbility::RegisterEnableImeObserver()
{
    int32_t ret = SettingsDataUtils::GetInstance()->CreateAndRegisterObserver(EnableImeDataParser::ENABLE_IME,
        [this]() { DatashareCallback(EnableImeDataParser::ENABLE_IME); });
    IMSA_HILOGI("register enable ime observer, ret: %{public}d.", ret);
    ret = SettingsDataUtils::GetInstance()->CreateAndRegisterObserver(EnableImeDataParser::ENABLE_KEYBOARD,
        [this]() { DatashareCallback(EnableImeDataParser::ENABLE_KEYBOARD); });
    IMSA_HILOGI("register enable keyboard observer, ret: %{public}d.", ret);
}

void InputMethodSystemAbility::RegisterSecurityModeObserver()
{
    int32_t ret = SettingsDataUtils::GetInstance()->CreateAndRegisterObserver(SecurityModeParser::SECURITY_MODE,
        [this]() { DatashareCallback(SecurityModeParser::SECURITY_MODE); });
    IMSA_HILOGI("register security mode observer, ret: %{public}d", ret);
}

void InputMethodSystemAbility::DatashareCallback(const std::string &key)
{
    IMSA_HILOGI("start.");
    if (key == EnableImeDataParser::ENABLE_KEYBOARD || key == EnableImeDataParser::ENABLE_IME) {
        EnableImeDataParser::GetInstance()->OnConfigChanged(userId_, key);
        std::lock_guard<std::mutex> autoLock(checkMutex_);
        SwitchInfo switchInfo;
        if (EnableImeDataParser::GetInstance()->CheckNeedSwitch(key, switchInfo, userId_)) {
            auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
            if (session == nullptr) {
                IMSA_HILOGE("%{public}d session is nullptr", userId_);
                return;
            }
            switchInfo.timestamp = std::chrono::system_clock::now();
            session->GetSwitchQueue().Push(switchInfo);
            OnSwitchInputMethod(userId_, switchInfo, SwitchTrigger::IMSA);
        }
    }
    if (key == SecurityModeParser::SECURITY_MODE) {
        DealSecurityChange();
    }
}

void InputMethodSystemAbility::OnSecurityModeChange()
{
    {
        std::lock_guard<std::mutex> lock(modeChangeMutex_);
        hasPendingChanges_ = false;
    }
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    auto oldMode = SecurityModeParser::GetInstance()->GetSecurityMode(currentIme->bundleName, userId_);
    SecurityModeParser::GetInstance()->UpdateFullModeList(userId_);
    auto newMode = SecurityModeParser::GetInstance()->GetSecurityMode(currentIme->bundleName, userId_);
    if (oldMode == newMode) {
        IMSA_HILOGD("current ime mode not changed.");
        return;
    }
    IMSA_HILOGI("ime: %{public}s securityMode change to: %{public}d.", currentIme->bundleName.c_str(),
        static_cast<int32_t>(newMode));
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId_);
        return;
    }
    session->OnSecurityChange(static_cast<int32_t>(newMode));
    session->AddRestartIme();
}

int32_t InputMethodSystemAbility::GetSecurityMode(int32_t &security)
{
    IMSA_HILOGD("InputMethodSystemAbility start.");
    if (!enableSecurityMode_) {
        security = static_cast<int32_t>(SecurityMode::FULL);
        return ErrorCode::NO_ERROR;
    }
    auto userId = GetCallingUserId();
    auto bundleName = FullImeInfoManager::GetInstance().Get(userId, IPCSkeleton::GetCallingTokenID());
    if (bundleName.empty()) {
        bundleName = identityChecker_->GetBundleNameByToken(IPCSkeleton::GetCallingTokenID());
        IMSA_HILOGW("%{public}s tokenId not find.", bundleName.c_str());
    }
    SecurityMode mode = SecurityModeParser::GetInstance()->GetSecurityMode(bundleName, userId);
    security = static_cast<int32_t>(mode);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::UnRegisteredProxyIme(UnRegisteredType type, const sptr<IInputMethodCore> &core)
{
    if (!identityChecker_->IsNativeSa(IPCSkeleton::GetCallingTokenID())) {
        IMSA_HILOGE("not native sa!");
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnUnRegisteredProxyIme(type, core);
}

int32_t InputMethodSystemAbility::CheckSwitchPermission(int32_t userId, const SwitchInfo &switchInfo,
    SwitchTrigger trigger)
{
    IMSA_HILOGD("trigger: %{public}d.", static_cast<int32_t>(trigger));
    if (trigger == SwitchTrigger::IMSA) {
        return ErrorCode::NO_ERROR;
    }
    if (trigger == SwitchTrigger::SYSTEM_APP) {
        if (!identityChecker_->IsSystemApp(IPCSkeleton::GetCallingFullTokenID())) {
            IMSA_HILOGE("not system app!");
            return ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION;
        }
        if (!identityChecker_->HasPermission(IPCSkeleton::GetCallingTokenID(), PERMISSION_CONNECT_IME_ABILITY)) {
            IMSA_HILOGE("have not PERMISSION_CONNECT_IME_ABILITY");
            return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
        }
        return ErrorCode::NO_ERROR;
    }
    if (trigger == SwitchTrigger::CURRENT_IME) {
        // PERMISSION_CONNECT_IME_ABILITY check temporarily reserved for application adaptation, will be deleted soon
        if (identityChecker_->HasPermission(IPCSkeleton::GetCallingTokenID(), PERMISSION_CONNECT_IME_ABILITY)) {
            return ErrorCode::NO_ERROR;
        }
        IMSA_HILOGE("have not PERMISSION_CONNECT_IME_ABILITY");
        // switchInfo.subName.empty() check temporarily reserved for application adaptation, will be deleted soon
        auto currentBundleName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId)->bundleName;
        if (identityChecker_->IsBundleNameValid(IPCSkeleton::GetCallingTokenID(), currentBundleName) &&
            !switchInfo.subName.empty()) {
            return ErrorCode::NO_ERROR;
        }
        IMSA_HILOGE("not current ime!");
        /* return ErrorCode::ERROR_STATUS_PERMISSION_DENIED temporarily reserved for application adaptation,
        will be replaced by ERROR_NOT_CURRENT_IME soon */
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    return ErrorCode::ERROR_BAD_PARAMETERS;
}

bool InputMethodSystemAbility::IsStartInputTypePermitted(int32_t userId)
{
    auto defaultIme = ImeInfoInquirer::GetInstance().GetDefaultImeInfo(userId);
    if (defaultIme == nullptr) {
        IMSA_HILOGE("failed to get default ime!");
        return false;
    }
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (identityChecker_->IsBundleNameValid(tokenId, defaultIme->prop.name)) {
        return true;
    }
    if (identityChecker_->HasPermission(tokenId, PERMISSION_CONNECT_IME_ABILITY)) {
        return true;
    }
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return false;
    }
    return identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId) && session->IsBoundToClient();
}

int32_t InputMethodSystemAbility::ConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent)
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->HasPermission(tokenId, PERMISSION_CONNECT_IME_ABILITY)) {
        IMSA_HILOGE("have not PERMISSION_CONNECT_IME_ABILITY");
        return ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION;
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnConnectSystemCmd(channel, agent);
}

void InputMethodSystemAbility::HandleWmsConnected(int32_t userId, int32_t screenId)
{
    if (userId == userId_) {
        // device boot or scb in foreground reboot
        HandleScbStarted(userId, screenId);
        return;
    }
    // user switched
    HandleUserSwitched(userId);
}

void InputMethodSystemAbility::HandleScbStarted(int32_t userId, int32_t screenId)
{
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        UserSessionManager::GetInstance().AddUserSession(userId);
    }
    session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return;
    }
    session->AddRestartIme();
}

void InputMethodSystemAbility::HandleUserSwitched(int32_t userId)
{
    UpdateUserInfo(userId);
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        UserSessionManager::GetInstance().AddUserSession(userId);
    }
    session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return;
    }
    auto imeData = session->GetImeData(ImeType::IME);
    if (imeData == nullptr && session->IsWmsReady()) {
        session->StartCurrentIme();
    }
}

void InputMethodSystemAbility::HandleWmsDisconnected(int32_t userId, int32_t screenId)
{
    // clear client
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session != nullptr) {
        session->RemoveCurrentClient();
    }

    if (userId == userId_) {
        // user switched or scb in foreground died, not deal
        return;
    }
    // scb in background died, stop ime
    if (session == nullptr) {
        return;
    }
    session->StopCurrentIme();
}

void InputMethodSystemAbility::HandleWmsStarted()
{
    // singleton, device boot, wms reboot
    IMSA_HILOGI("Wms start.");
    InitFocusChangedMonitor();
    if (isScbEnable_) {
        IMSA_HILOGI("scb enable, register WMS connection listener.");
        InitWmsConnectionMonitor();
        return;
    }
    // clear client
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session != nullptr) {
        session->RemoveCurrentClient();
    }
    if (session == nullptr) {
        UserSessionManager::GetInstance().AddUserSession(userId_);
    }
    session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId_);
        return;
    }
    session->AddRestartIme();
    StopImeInBackground();
}

void InputMethodSystemAbility::HandleFocusChanged(bool isFocused, int32_t pid, int32_t uid)
{
    int32_t userId = GetUserId(uid);
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("[%{public}d, %{public}d] session is nullptr", uid, userId);
        return;
    }
    isFocused ? session->OnFocused(pid, uid) : session->OnUnfocused(pid, uid);
}

void InputMethodSystemAbility::HandleMemStarted()
{
    // singleton
    IMSA_HILOGI("MemMgr start.");
    Memory::MemMgrClient::GetInstance().NotifyProcessStatus(getpid(), 1, 1, INPUT_METHOD_SYSTEM_ABILITY_ID);
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        UserSessionManager::GetInstance().AddUserSession(userId_);
    }
    session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId_);
        return;
    }
    session->AddRestartIme();
    StopImeInBackground();
}

void InputMethodSystemAbility::HandleOsAccountStarted()
{
    auto userId = OsAccountAdapter::GetForegroundOsAccountLocalId();
    if (userId_ != userId) {
        UpdateUserInfo(userId);
    }
    Message *msg = new (std::nothrow) Message(MessageID::MSG_ID_OS_ACCOUNT_STARTED, nullptr);
    if (msg == nullptr) {
        return;
    }
    MessageHandler::Instance()->SendMessage(msg);
}

void InputMethodSystemAbility::StopImeInBackground()
{
    auto task = [this]() {
        auto sessions = UserSessionManager::GetInstance().GetUserSessions();
        for (const auto &tempSession : sessions) {
            if (tempSession.first != userId_) {
                tempSession.second->StopCurrentIme();
            }
        }
    };
    serviceHandler_->PostTask(task, "StopImeInBackground", 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

int32_t InputMethodSystemAbility::GetUserId(int32_t uid)
{
    IMSA_HILOGD("uid:%{public}d", uid);
    auto userId = OsAccountAdapter::GetOsAccountLocalIdFromUid(uid);
    // 0 represents user 0 in the system
    if (userId == 0) {
        IMSA_HILOGI("user 0");
        return userId_;
    }
    return userId;
}

int32_t InputMethodSystemAbility::GetCallingUserId()
{
    auto uid = IPCSkeleton::GetCallingUid();
    return GetUserId(uid);
}

bool InputMethodSystemAbility::IsCurrentIme(int32_t userId)
{
    if (InputTypeManager::GetInstance().IsStarted()) {
        auto currentTypeIme = InputTypeManager::GetInstance().GetCurrentIme();
        return identityChecker_->IsBundleNameValid(IPCSkeleton::GetCallingTokenID(), currentTypeIme.bundleName);
    }
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    return identityChecker_->IsBundleNameValid(IPCSkeleton::GetCallingTokenID(), currentImeCfg->bundleName);
}

int32_t InputMethodSystemAbility::StartInputType(int32_t userId, InputType type)
{
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (type != InputType::NONE) {
        ImeIdentification ime;
        int32_t ret = InputTypeManager::GetInstance().GetImeByInputType(type, ime);
        if (ret != ErrorCode::NO_ERROR) {
            return ret;
        }
        SwitchInfo switchInfo = { std::chrono::system_clock::now(), ime.bundleName, ime.subName };
        session->GetSwitchQueue().Push(switchInfo);
        IMSA_HILOGI("start input type: %{public}d.", type);
        return type == InputType::SECURITY_INPUT ? OnStartInputType(userId, switchInfo, false)
                                                 : OnStartInputType(userId, switchInfo, true);
    }
    auto cfgIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    SwitchInfo switchInfo = { std::chrono::system_clock::now(), cfgIme->bundleName, cfgIme->subName };
    session->GetSwitchQueue().Push(switchInfo);
    return OnStartInputType(userId, switchInfo, false);
}
} // namespace MiscServices
} // namespace OHOS