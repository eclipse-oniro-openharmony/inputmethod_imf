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
#include "global.h"
#include "im_common_event_manager.h"
#include "ime_cfg_manager.h"
#include "ime_info_inquirer.h"
#include "input_method_utils.h"
#include "input_type_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "itypes_util.h"
#include "key_event.h"
#include "message_handler.h"
#include "native_token_info.h"
#include "os_account_manager.h"
#include "scene_board_judgement.h"
#include "sys/prctl.h"
#include "system_ability_definition.h"
#include "system_language_observer.h"
#include "wms_connection_observer.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
using namespace AccountSA;
REGISTER_SYSTEM_ABILITY_BY_ID(InputMethodSystemAbility, INPUT_METHOD_SYSTEM_ABILITY_ID, true);
constexpr std::int32_t INIT_INTERVAL = 10000L;
constexpr std::int32_t MAIN_USER_ID = 100;
constexpr uint32_t RETRY_INTERVAL = 100;
constexpr uint32_t BLOCK_RETRY_TIMES = 100;
constexpr uint32_t SWITCH_BLOCK_TIME = 150000;
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
    IMSA_HILOGI("InputMethodSystemAbility::OnStart.");
    if (!InputMethodSysEvent::GetInstance().StartTimerForReport()) {
        IMSA_HILOGE("Start sysevent timer failed!");
    }
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        IMSA_HILOGI("ImsaService is already running.");
        return;
    }
    Initialize();
    InitServiceHandler();
    int32_t ret = Init();
    if (ret != ErrorCode::NO_ERROR) {
        InputMethodSysEvent::GetInstance().ServiceFaultReporter("imf", ret);
        auto callback = [=]() { Init(); };
        serviceHandler_->PostTask(callback, INIT_INTERVAL);
        IMSA_HILOGE("Init failed. Try again 10s later");
    }
    InitHiTrace();
    InputMethodSyncTrace tracer("InputMethodController Attach trace.");
    InputmethodDump::GetInstance().AddDumpAllMethod(
        std::bind(&InputMethodSystemAbility::DumpAllMethod, this, std::placeholders::_1));
    IMSA_HILOGI("Start ImsaService ErrorCode::NO_ERROR.");
    return;
}

int InputMethodSystemAbility::Dump(int fd, const std::vector<std::u16string> &args)
{
    IMSA_HILOGD("InputMethodSystemAbility::Dump");
    std::vector<std::string> argsStr;
    for (auto item : args) {
        argsStr.emplace_back(Str16ToStr8(item));
    }
    InputmethodDump::GetInstance().Dump(fd, argsStr);
    return ERR_OK;
}

void InputMethodSystemAbility::DumpAllMethod(int fd)
{
    IMSA_HILOGD("InputMethodSystemAbility::DumpAllMethod");
    std::vector<int32_t> ids;
    int errCode = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (errCode != ERR_OK) {
        dprintf(fd, "\n - InputMethodSystemAbility::DumpAllMethod get Active Id failed.\n");
        return;
    }
    dprintf(fd, "\n - DumpAllMethod get Active Id succeed,count=%zu,", ids.size());
    for (auto id : ids) {
        const auto &params = ImeInfoInquirer::GetInstance().GetDumpInfo(id);
        if (params.empty()) {
            IMSA_HILOGD("userId: %{public}d The IME properties is empty.", id);
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

int32_t InputMethodSystemAbility::GetCurrentUserIdFromOsAccount()
{
    std::vector<int32_t> userIds;
    if (!BlockRetry(RETRY_INTERVAL, BLOCK_RETRY_TIMES, [&userIds]() -> bool {
            return OsAccountManager::QueryActiveOsAccountIds(userIds) == ERR_OK && !userIds.empty();
        })) {
        IMSA_HILOGE("get userId failed");
        return MAIN_USER_ID;
    }
    return userIds[0];
}

void InputMethodSystemAbility::HandleUserChanged(int32_t userId)
{
    IMSA_HILOGI("%{public}d switch to %{public}d", userId_, userId);
    userId_ = userId;
    userSession_->UpdateCurrentUserId(userId_);
    InputMethodSysEvent::GetInstance().SetUserId(userId_);
    if (enableImeOn_) {
        EnableImeDataParser::GetInstance()->OnUserChanged(userId_);
    }
    if (enableSecurityMode_) {
        SecurityModeParser::GetInstance()->GetFullModeList(userId_);
    }
    ImeInfoInquirer::GetInstance().SetCurrentImeInfo(nullptr);
}

int32_t InputMethodSystemAbility::StartImeWhenWmsReady()
{
    if (imeStarting_.exchange(true)) {
        IMSA_HILOGD("do starting");
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("start ime, userId: %{public}d", userId_);
    userSession_->StopCurrentIme();
    auto ret = userSession_->StartCurrentIme(userId_, true);
    if (!ret) {
        IMSA_HILOGE("start ime failed");
    }
    imeStarting_.store(false);
    return ret ? ErrorCode::NO_ERROR : ErrorCode::ERROR_IME_START_FAILED;
}

void InputMethodSystemAbility::HandleWmsReady(int32_t userId)
{
    // wms ready scene: user switch(in scb enable), device boots, wms reboot
    if (userId != userId_) {
        HandleUserChanged(userId);
    }
    StartImeWhenWmsReady();
}

void InputMethodSystemAbility::OnStop()
{
    IMSA_HILOGI("OnStop started.");
    serviceHandler_ = nullptr;
    state_ = ServiceRunningState::STATE_NOT_START;
}

void InputMethodSystemAbility::InitServiceHandler()
{
    IMSA_HILOGI("InitServiceHandler started.");
    if (serviceHandler_ != nullptr) {
        IMSA_HILOGE("InputMethodSystemAbility, already init.");
        return;
    }
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("OS_InputMethodSystemAbility");
    serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);

    IMSA_HILOGI("InitServiceHandler succeeded.");
}

/**
 * Initialization of Input method management service
 * \n It's called after the service starts, before any transaction.
 */
void InputMethodSystemAbility::Initialize()
{
    IMSA_HILOGI("InputMethodSystemAbility::Initialize");
    // init work thread to handle the messages
    workThreadHandler = std::thread([this] { WorkThread(); });
    identityChecker_ = std::make_shared<IdentityCheckerImpl>();
    userId_ = MAIN_USER_ID;
    userSession_ = std::make_shared<PerUserSession>(userId_);
    InputMethodSysEvent::GetInstance().SetUserId(userId_);
    isScbEnable_ = Rosen::SceneBoardJudgement::IsSceneBoardEnabled();
}

void InputMethodSystemAbility::StartUserIdListener()
{
    sptr<ImCommonEventManager> imCommonEventManager = ImCommonEventManager::GetInstance();
    bool isSuccess = imCommonEventManager->SubscribeEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    if (isSuccess) {
        IMSA_HILOGI("Initialize subscribe service event success");
        return;
    }

    IMSA_HILOGE("failed. Try again 10s later");
    auto callback = [this]() { StartUserIdListener(); };
    serviceHandler_->PostTask(callback, INIT_INTERVAL);
}

bool InputMethodSystemAbility::StartInputService(const std::shared_ptr<ImeNativeCfg> &imeId)
{
    return userSession_->StartInputService(imeId, true);
}

void InputMethodSystemAbility::StopInputService()
{
    userSession_->StopCurrentIme();
}

int32_t InputMethodSystemAbility::PrepareInput(InputClientInfo &clientInfo)
{
    auto ret = GenerateClientInfo(clientInfo);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    return userSession_->OnPrepareInput(clientInfo);
}

int32_t InputMethodSystemAbility::GenerateClientInfo(InputClientInfo &clientInfo)
{
    if (clientInfo.client == nullptr || clientInfo.channel == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto deathRecipient = new (std::nothrow) InputDeathRecipient();
    if (deathRecipient == nullptr) {
        IMSA_HILOGE("failed to new deathRecipient");
        return ErrorCode::ERROR_EX_NULL_POINTER;
    }
    clientInfo.pid = IPCSkeleton::GetCallingPid();
    clientInfo.uid = IPCSkeleton::GetCallingUid();
    clientInfo.userID = userId_;
    clientInfo.deathRecipient = deathRecipient;
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::ReleaseInput(sptr<IInputClient> client)
{
    if (client == nullptr) {
        IMSA_HILOGE("InputMethodSystemAbility::client is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return userSession_->OnReleaseInput(client);
};

int32_t InputMethodSystemAbility::StartInput(InputClientInfo &inputClientInfo, sptr<IRemoteObject> &agent)
{
    if (userSession_->GetCurrentClientPid() != IPCSkeleton::GetCallingPid()) {
        //进程变化时需要通知inputstart
        inputClientInfo.isNotifyInputStart = true;
    }
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->IsBroker(tokenId)) {
        if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId)) {
            return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
        }
    }

    if (!userSession_->IsProxyImeEnable()) {
        CheckSecurityMode(inputClientInfo);
    }
    int32_t ret = PrepareInput(inputClientInfo);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("PrepareInput failed");
        return ret;
    }
    return userSession_->OnStartInput(inputClientInfo, agent);
};

void InputMethodSystemAbility::CheckSecurityMode(InputClientInfo &inputClientInfo)
{
    if (inputClientInfo.config.inputAttribute.GetSecurityFlag()) {
        if (InputTypeManager::GetInstance().IsStarted()) {
            IMSA_HILOGD("security ime has started.");
            return;
        }
        auto ret = StartInputType(InputType::SECURITY_INPUT);
        IMSA_HILOGD("switch to security ime ret = %{public}d.", ret);
        return;
    }
    if (!InputTypeManager::GetInstance().IsStarted() || InputTypeManager::GetInstance().IsCameraImeStarted()) {
        IMSA_HILOGD("security ime is not start or camera ime started, keep current.");
        return;
    }
    auto ret = StartInputType(InputType::NONE);
    IMSA_HILOGD("Exit security ime ret = %{public}d.", ret);
}

int32_t InputMethodSystemAbility::ShowInput(sptr<IInputClient> client)
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->IsBroker(tokenId)) {
        if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId, userSession_->GetCurrentClientPid())) {
            return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
        }
    }
    if (client == nullptr) {
        IMSA_HILOGE("IMSA, client is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return userSession_->OnShowInput(client);
}

int32_t InputMethodSystemAbility::HideInput(sptr<IInputClient> client)
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->IsBroker(tokenId)) {
        if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId, userSession_->GetCurrentClientPid())) {
            return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
        }
    }
    if (client == nullptr) {
        IMSA_HILOGE("IMSA, client is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return userSession_->OnHideInput(client);
};

int32_t InputMethodSystemAbility::StopInputSession()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->IsBroker(tokenId)) {
        if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId, userSession_->GetCurrentClientPid())) {
            return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
        }
    }
    return userSession_->OnHideCurrentInput();
}

int32_t InputMethodSystemAbility::RequestShowInput()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId)
        && !identityChecker_->HasPermission(tokenId, PERMISSION_CONNECT_IME_ABILITY)) {
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    return userSession_->OnRequestShowInput();
}

int32_t InputMethodSystemAbility::RequestHideInput()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId)
        && !identityChecker_->HasPermission(tokenId, PERMISSION_CONNECT_IME_ABILITY)) {
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    return userSession_->OnRequestHideInput();
}

int32_t InputMethodSystemAbility::SetCoreAndAgent(
    const sptr<IInputMethodCore> &core, const sptr<IInputMethodAgent> &agent)
{
    IMSA_HILOGD("InputMethodSystemAbility run in");
    if (IsCurrentIme()) {
        return userSession_->OnSetCoreAndAgent(core, agent);
    }
    if (identityChecker_->IsNativeSa(IPCSkeleton::GetCallingTokenID())) {
        return userSession_->OnRegisterProxyIme(core, agent);
    }
    return ErrorCode::ERROR_NOT_CURRENT_IME;
}

int32_t InputMethodSystemAbility::HideCurrentInput()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (identityChecker_->IsBroker(tokenId)) {
        return userSession_->OnHideCurrentInput();
    }
    if (!identityChecker_->HasPermission(tokenId, PERMISSION_CONNECT_IME_ABILITY)) {
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    return userSession_->OnHideCurrentInput();
};

int32_t InputMethodSystemAbility::ShowCurrentInput()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (identityChecker_->IsBroker(tokenId)) {
        return userSession_->OnShowCurrentInput();
    }

    if (!identityChecker_->HasPermission(tokenId, PERMISSION_CONNECT_IME_ABILITY)) {
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    return userSession_->OnShowCurrentInput();
};

int32_t InputMethodSystemAbility::PanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info)
{
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    if (!identityChecker_->IsBundleNameValid(IPCSkeleton::GetCallingTokenID(), currentImeCfg->bundleName)) {
        IMSA_HILOGE("not current ime");
        return ErrorCode::ERROR_NOT_CURRENT_IME;
    }
    return userSession_->OnPanelStatusChange(status, info);
}

int32_t InputMethodSystemAbility::UpdateListenEventFlag(InputClientInfo &clientInfo, uint32_t eventFlag)
{
    IMSA_HILOGI("finalEventFlag: %{public}u, eventFlag: %{public}u", clientInfo.eventFlag, eventFlag);
    if (EventStatusManager::IsImeHideOn(eventFlag) || EventStatusManager::IsImeShowOn(eventFlag)) {
        if (!identityChecker_->IsSystemApp(IPCSkeleton::GetCallingFullTokenID())
            && !identityChecker_->IsNativeSa(IPCSkeleton::GetCallingTokenID())) {
            IMSA_HILOGE("not system application");
            return ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION;
        }
    }
    auto ret = GenerateClientInfo(clientInfo);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    return userSession_->OnUpdateListenEventFlag(clientInfo);
}

bool InputMethodSystemAbility::IsCurrentIme()
{
    if (InputTypeManager::GetInstance().IsStarted()) {
        auto currentTypeIme = InputTypeManager::GetInstance().GetCurrentIme();
        return identityChecker_->IsBundleNameValid(IPCSkeleton::GetCallingTokenID(), currentTypeIme.bundleName);
    }
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    return identityChecker_->IsBundleNameValid(IPCSkeleton::GetCallingTokenID(), currentImeCfg->bundleName);
}

bool InputMethodSystemAbility::IsInputTypeSupported(InputType type)
{
    return InputTypeManager::GetInstance().IsSupported(type);
}

int32_t InputMethodSystemAbility::StartInputType(InputType type)
{
    if (type != InputType::NONE) {
        ImeIdentification ime;
        int32_t ret = InputTypeManager::GetInstance().GetImeByInputType(type, ime);
        if (ret != ErrorCode::NO_ERROR) {
            return ret;
        }
        SwitchInfo switchInfo = { std::chrono::system_clock::now(), ime.bundleName, ime.subName };
        switchQueue_.Push(switchInfo);
        IMSA_HILOGI("start input type: %{public}d", type);
        return type == InputType::SECURITY_INPUT ? OnStartInputType(switchInfo, false)
                                                 : OnStartInputType(switchInfo, true);
    }
    auto cfgIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    SwitchInfo switchInfo = { std::chrono::system_clock::now(), cfgIme->bundleName, cfgIme->subName };
    switchQueue_.Push(switchInfo);
    return OnStartInputType(switchInfo, false);
}

int32_t InputMethodSystemAbility::ExitCurrentInputType()
{
    auto defaultIme = ImeInfoInquirer::GetInstance().GetDefaultImeInfo(userId_);
    if (defaultIme == nullptr) {
        IMSA_HILOGE("failed to get default ime");
        return ErrorCode::ERROR_PERSIST_CONFIG;
    }
    if (!identityChecker_->IsBundleNameValid(IPCSkeleton::GetCallingTokenID(), defaultIme->prop.name)) {
        return ErrorCode::ERROR_NOT_DEFAULT_IME;
    }
    if (userSession_->CheckSecurityMode()) {
        return StartInputType(InputType::SECURITY_INPUT);
    }
    return userSession_->ExitCurrentInputType();
}

int32_t InputMethodSystemAbility::IsPanelShown(const PanelInfo &panelInfo, bool &isShown)
{
    if (!identityChecker_->IsSystemApp(IPCSkeleton::GetCallingFullTokenID())) {
        IMSA_HILOGE("not system application");
        return ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION;
    }
    return userSession_->IsPanelShown(panelInfo, isShown);
}

int32_t InputMethodSystemAbility::DisplayOptionalInputMethod()
{
    IMSA_HILOGD("InputMethodSystemAbility run in");
    return OnDisplayOptionalInputMethod();
}

int32_t InputMethodSystemAbility::SwitchInputMethod(
    const std::string &bundleName, const std::string &subName, SwitchTrigger trigger)
{
    // IMSA not check permission, add this verify for prevent counterfeit
    if (trigger == SwitchTrigger::IMSA) {
        IMSA_HILOGW("caller counterfeit!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    SwitchInfo switchInfo = { std::chrono::system_clock::now(), bundleName, subName };
    if (enableImeOn_ && !EnableImeDataParser::GetInstance()->CheckNeedSwitch(switchInfo, userId_)) {
        IMSA_HILOGW("Enable mode off or switch is not enable, stoped!");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    switchInfo.timestamp = std::chrono::system_clock::now();
    switchQueue_.Push(switchInfo);
    return InputTypeManager::GetInstance().IsInputType({ bundleName, subName })
               ? OnStartInputType(switchInfo, true)
               : OnSwitchInputMethod(switchInfo, trigger);
}

int32_t InputMethodSystemAbility::OnSwitchInputMethod(const SwitchInfo &switchInfo, SwitchTrigger trigger)
{
    IMSA_HILOGD("run in, switchInfo: %{public}s|%{public}s", switchInfo.bundleName.c_str(), switchInfo.subName.c_str());
    InputMethodSysEvent::GetInstance().RecordEvent(IMEBehaviour::CHANGE_IME);
    if (!switchQueue_.IsReady(switchInfo)) {
        IMSA_HILOGD("start wait");
        switchQueue_.Wait(switchInfo);
        usleep(SWITCH_BLOCK_TIME);
    }
    IMSA_HILOGI("start switch %{public}s|%{public}s", switchInfo.bundleName.c_str(), switchInfo.subName.c_str());
    int32_t ret = CheckSwitchPermission(switchInfo, trigger);
    if (ret != ErrorCode::NO_ERROR) {
        InputMethodSysEvent::GetInstance().InputmethodFaultReporter(
            ErrorCode::ERROR_STATUS_PERMISSION_DENIED, switchInfo.bundleName, "switch inputmethod failed!");
        switchQueue_.Pop();
        return ret;
    }

    if (!InputTypeManager::GetInstance().IsStarted() && !IsNeedSwitch(switchInfo.bundleName, switchInfo.subName)) {
        switchQueue_.Pop();
        return ErrorCode::NO_ERROR;
    }
    auto info = ImeInfoInquirer::GetInstance().GetImeInfo(userId_, switchInfo.bundleName, switchInfo.subName);
    if (info == nullptr) {
        switchQueue_.Pop();
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    ret = info->isNewIme ? Switch(switchInfo.bundleName, info) : SwitchExtension(info);
    if (InputTypeManager::GetInstance().IsStarted()) {
        InputTypeManager::GetInstance().Set(false);
    }
    switchQueue_.Pop();
    if (ret != ErrorCode::NO_ERROR) {
        InputMethodSysEvent::GetInstance().InputmethodFaultReporter(
            ret, switchInfo.bundleName, "switch inputmethod failed!");
    }
    return ret;
}

int32_t InputMethodSystemAbility::OnStartInputType(const SwitchInfo &switchInfo, bool isCheckPermission)
{
    if (!switchQueue_.IsReady(switchInfo)) {
        IMSA_HILOGD("start wait");
        switchQueue_.Wait(switchInfo);
        usleep(SWITCH_BLOCK_TIME);
    }
    auto cfgIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    if (switchInfo.bundleName == cfgIme->bundleName && switchInfo.subName == cfgIme->subName) {
        IMSA_HILOGD("start input type is current ime, exit input type.");
        int32_t ret = userSession_->ExitCurrentInputType();
        switchQueue_.Pop();
        return ret;
    }
    IMSA_HILOGD("start switch %{public}s|%{public}s", switchInfo.bundleName.c_str(), switchInfo.subName.c_str());
    if (isCheckPermission && !IsStartInputTypePermitted()) {
        IMSA_HILOGE("not permitted to start input type");
        switchQueue_.Pop();
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    if (!IsNeedSwitch(switchInfo.bundleName, switchInfo.subName)) {
        IMSA_HILOGI("no need to switch");
        switchQueue_.Pop();
        return ErrorCode::NO_ERROR;
    }
    int32_t ret = SwitchInputType(switchInfo);
    switchQueue_.Pop();
    return ret;
}

bool InputMethodSystemAbility::IsNeedSwitch(const std::string &bundleName, const std::string &subName)
{
    if (InputTypeManager::GetInstance().IsStarted()) {
        ImeIdentification target = { bundleName, subName };
        return !(target == InputTypeManager::GetInstance().GetCurrentIme());
    }
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    IMSA_HILOGI("currentIme: %{public}s/%{public}s, targetIme: %{public}s/%{public}s",
        currentImeCfg->bundleName.c_str(), currentImeCfg->subName.c_str(), bundleName.c_str(), subName.c_str());
    if ((subName.empty() && bundleName == currentImeCfg->bundleName)
        || (!subName.empty() && subName == currentImeCfg->subName && currentImeCfg->bundleName == bundleName)) {
        IMSA_HILOGI("no need to switch");
        return false;
    }
    return true;
}

int32_t InputMethodSystemAbility::Switch(const std::string &bundleName, const std::shared_ptr<ImeInfo> &info)
{
    auto currentImeBundleName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName;
    return bundleName != currentImeBundleName ? SwitchExtension(info) : SwitchSubType(info);
}

// Switch the current InputMethodExtension to the new InputMethodExtension
int32_t InputMethodSystemAbility::SwitchExtension(const std::shared_ptr<ImeInfo> &info)
{
    StopInputService();
    std::string targetImeName = info->prop.name + "/" + info->prop.id;
    ImeCfgManager::GetInstance().ModifyImeCfg({ userId_, targetImeName, info->subProp.id });
    ImeInfoInquirer::GetInstance().SetCurrentImeInfo(info);
    ImeNativeCfg targetIme = { targetImeName, info->prop.name, info->subProp.id, info->prop.id };
    if (!StartInputService(std::make_shared<ImeNativeCfg>(targetIme))) {
        IMSA_HILOGE("start input method failed");
        return ErrorCode::ERROR_IME_START_FAILED;
    }
    userSession_->NotifyImeChangeToClients(info->prop, info->subProp);
    return ErrorCode::NO_ERROR;
}

// Inform current InputMethodExtension to switch subtype
int32_t InputMethodSystemAbility::SwitchSubType(const std::shared_ptr<ImeInfo> &info)
{
    auto ret = userSession_->SwitchSubtype(info->subProp);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to inform ime to switch subtype, ret: %{public}d", ret);
        return ret;
    }
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->imeId;
    ImeCfgManager::GetInstance().ModifyImeCfg({ userId_, currentIme, info->subProp.id });
    ImeInfoInquirer::GetInstance().SetCurrentImeInfo(info);
    userSession_->NotifyImeChangeToClients(info->prop, info->subProp);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::SwitchInputType(const SwitchInfo &switchInfo)
{
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    bool checkSameIme = InputTypeManager::GetInstance().IsStarted()
                            ? switchInfo.bundleName == InputTypeManager::GetInstance().GetCurrentIme().bundleName
                            : switchInfo.bundleName == currentIme->bundleName;
    if (checkSameIme) {
        IMSA_HILOGD("only need to switch subtype: %{public}s", switchInfo.subName.c_str());
        auto ret = userSession_->SwitchSubtype({ .name = switchInfo.bundleName, .id = switchInfo.subName });
        if (ret == ErrorCode::NO_ERROR) {
            InputTypeManager::GetInstance().Set(true, { switchInfo.bundleName, switchInfo.subName });
        }
        return ret;
    }
    IMSA_HILOGD("need to switch ime: %{public}s|%{public}s", switchInfo.bundleName.c_str(), switchInfo.subName.c_str());
    auto targetImeProperty = ImeInfoInquirer::GetInstance().GetImeByBundleName(userId_, switchInfo.bundleName);
    if (targetImeProperty == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }

    StopInputService();
    std::string targetName = switchInfo.bundleName + "/" + targetImeProperty->id;
    ImeNativeCfg targetIme = { targetName, switchInfo.bundleName, switchInfo.subName, targetImeProperty->id };
    InputTypeManager::GetInstance().Set(true, { switchInfo.bundleName, switchInfo.subName });
    if (!StartInputService(std::make_shared<ImeNativeCfg>(targetIme))) {
        IMSA_HILOGE("start input method failed");
        InputTypeManager::GetInstance().Set(false);
        return ErrorCode::ERROR_IME_START_FAILED;
    }
    int32_t ret = userSession_->SwitchSubtype({ .name = switchInfo.bundleName, .id = switchInfo.subName });
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
    if (!identityChecker_->IsBroker(tokenId)) {
        if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId, userSession_->GetCurrentClientPid())) {
            return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
        }
    }
    return userSession_->OnHideCurrentInput();
};

int32_t InputMethodSystemAbility::ShowCurrentInputDeprecated()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->IsBroker(tokenId)) {
        if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId, userSession_->GetCurrentClientPid())) {
            return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
        }
    }
    return userSession_->OnShowCurrentInput();
};

std::shared_ptr<Property> InputMethodSystemAbility::GetCurrentInputMethod()
{
    return ImeInfoInquirer::GetInstance().GetCurrentInputMethod(userId_);
}

std::shared_ptr<SubProperty> InputMethodSystemAbility::GetCurrentInputMethodSubtype()
{
    return ImeInfoInquirer::GetInstance().GetCurrentSubtype(userId_);
}

int32_t InputMethodSystemAbility::GetDefaultInputMethod(std::shared_ptr<Property> &prop)
{
    return ImeInfoInquirer::GetInstance().GetDefaultInputMethod(userId_, prop);
}

int32_t InputMethodSystemAbility::GetInputMethodConfig(OHOS::AppExecFwk::ElementName &inputMethodConfig)
{
    return ImeInfoInquirer::GetInstance().GetInputMethodConfig(userId_, inputMethodConfig);
}

int32_t InputMethodSystemAbility::ListInputMethod(InputMethodStatus status, std::vector<Property> &props)
{
    return ImeInfoInquirer::GetInstance().ListInputMethod(userId_, status, props, enableImeOn_);
}

int32_t InputMethodSystemAbility::ListCurrentInputMethodSubtype(std::vector<SubProperty> &subProps)
{
    return ImeInfoInquirer::GetInstance().ListCurrentInputMethodSubtype(userId_, subProps);
}

int32_t InputMethodSystemAbility::ListInputMethodSubtype(
    const std::string &bundleName, std::vector<SubProperty> &subProps)
{
    return ImeInfoInquirer::GetInstance().ListInputMethodSubtype(userId_, bundleName, subProps);
}

/**
 * Work Thread of input method management service
 * \n Remote commands which may change the state or data in the service will be handled sequentially in this thread.
 */
void InputMethodSystemAbility::WorkThread()
{
    prctl(PR_SET_NAME, "OS_IMSAWorkThread");
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
            case MSG_ID_PACKAGE_REMOVED: {
                OnPackageRemoved(msg);
                break;
            }
            case MSG_ID_HIDE_KEYBOARD_SELF: {
                userSession_->OnHideSoftKeyBoardSelf();
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
    // if scb enable, deal when receive wmsConnected.
    if (isScbEnable_) {
        return ErrorCode::NO_ERROR;
    }
    if (msg->msgContent_ == nullptr) {
        IMSA_HILOGE("msgContent is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto newUserId = msg->msgContent_->ReadInt32();
    if (newUserId == userId_) {
        return ErrorCode::NO_ERROR;
    }
    HandleUserChanged(newUserId);
    if (!userSession_->IsWmsReady()) {
        IMSA_HILOGI("wms not ready, wait");
        return ErrorCode::NO_ERROR;
    }
    return StartImeWhenWmsReady();
}

int32_t InputMethodSystemAbility::OnUserRemoved(const Message *msg)
{
    if (msg->msgContent_ == nullptr) {
        IMSA_HILOGE("Aborted! Message is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto userId = msg->msgContent_->ReadInt32();
    IMSA_HILOGI("Start: %{public}d", userId);
    ImeCfgManager::GetInstance().DeleteImeCfg(userId);
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
int32_t InputMethodSystemAbility::OnPackageRemoved(const Message *msg)
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
    // if the app that doesn't belong to current user is removed, ignore it
    if (userId != userId_) {
        IMSA_HILOGD("userId: %{public}d, currentUserId: %{public}d,", userId, userId_);
        return ErrorCode::NO_ERROR;
    }
    auto currentImeBundle = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId)->bundleName;
    if (packageName == currentImeBundle) {
        // Switch to the default ime
        IMSA_HILOGI("user[%{public}d] ime: %{public}s is uninstalled", userId, packageName.c_str());
        auto info = ImeInfoInquirer::GetInstance().GetDefaultImeInfo(userId);
        if (info == nullptr) {
            return ErrorCode::ERROR_PERSIST_CONFIG;
        }
        int32_t ret = SwitchExtension(info);
        IMSA_HILOGI("switch ret = %{public}d", ret);
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::OnDisplayOptionalInputMethod()
{
    IMSA_HILOGD("InputMethodSystemAbility::OnDisplayOptionalInputMethod");
    AAFwk::Want want;
    want.SetAction(SELECT_DIALOG_ACTION);
    want.SetElementName(SELECT_DIALOG_HAP, SELECT_DIALOG_ABILITY);
    int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want);
    if (ret != ErrorCode::NO_ERROR && ret != START_SERVICE_ABILITY_ACTIVATING) {
        IMSA_HILOGE("Start InputMethod ability failed, err = %{public}d", ret);
        return ErrorCode::ERROR_EX_SERVICE_SPECIFIC;
    }
    IMSA_HILOGI("Start InputMethod ability success.");
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::SwitchByCombinationKey(uint32_t state)
{
    IMSA_HILOGD("InputMethodSystemAbility::SwitchByCombinationKey");
    if (userSession_->IsProxyImeEnable()) {
        IMSA_HILOGI("proxy enable, not switch");
        return ErrorCode::NO_ERROR;
    }
    if (CombinationKey::IsMatch(CombinationKeyFunction::SWITCH_MODE, state)) {
        IMSA_HILOGI("switch mode");
        return SwitchMode();
    }
    if (CombinationKey::IsMatch(CombinationKeyFunction::SWITCH_LANGUAGE, state)) {
        IMSA_HILOGI("switch language");
        return SwitchLanguage();
    }
    if (CombinationKey::IsMatch(CombinationKeyFunction::SWITCH_IME, state)) {
        IMSA_HILOGI("switch ime");
        return SwitchType();
    }
    IMSA_HILOGE("keycode undefined");
    return ErrorCode::ERROR_EX_UNSUPPORTED_OPERATION;
}

int32_t InputMethodSystemAbility::SwitchMode()
{
    auto bundleName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName;
    auto subName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->subName;
    auto info = ImeInfoInquirer::GetInstance().GetImeInfo(userId_, bundleName, subName);
    if (info == nullptr) {
        IMSA_HILOGE("current ime is abnormal");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    if (info->isNewIme) {
        IMSA_HILOGD("the switching operation is handed over to ime");
        return ErrorCode::NO_ERROR;
    }
    auto condition = info->subProp.mode == "upper" ? Condition::LOWER : Condition::UPPER;
    auto target = ImeInfoInquirer::GetInstance().FindTargetSubtypeByCondition(info->subProps, condition);
    if (target == nullptr) {
        IMSA_HILOGE("target is empty");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    SwitchInfo switchInfo = { std::chrono::system_clock::now(), target->name, target->id };
    switchQueue_.Push(switchInfo);
    return OnSwitchInputMethod(switchInfo, SwitchTrigger::IMSA);
}

int32_t InputMethodSystemAbility::SwitchLanguage()
{
    auto bundleName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName;
    auto subName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->subName;
    auto info = ImeInfoInquirer::GetInstance().GetImeInfo(userId_, bundleName, subName);
    if (info == nullptr) {
        IMSA_HILOGE("current ime is abnormal");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    if (info->isNewIme) {
        IMSA_HILOGD("the switching operation is handed over to ime");
        return ErrorCode::NO_ERROR;
    }
    if (info->subProp.language != "chinese" && info->subProp.language != "english") {
        return ErrorCode::NO_ERROR;
    }
    auto condition = info->subProp.language == "chinese" ? Condition::ENGLISH : Condition::CHINESE;
    auto target = ImeInfoInquirer::GetInstance().FindTargetSubtypeByCondition(info->subProps, condition);
    if (target == nullptr) {
        IMSA_HILOGE("target is empty");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    SwitchInfo switchInfo = { std::chrono::system_clock::now(), target->name, target->id };
    switchQueue_.Push(switchInfo);
    return OnSwitchInputMethod(switchInfo, SwitchTrigger::IMSA);
}

int32_t InputMethodSystemAbility::SwitchType()
{
    SwitchInfo switchInfo = { std::chrono::system_clock::now(), "", "" };
    int32_t ret = ImeInfoInquirer::GetInstance().GetNextSwitchInfo(switchInfo, userId_, enableImeOn_);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Get next SwitchInfo failed, stop switching ime.");
        return ret;
    }
    IMSA_HILOGD("switch to: %{public}s", switchInfo.bundleName.c_str());
    switchInfo.timestamp = std::chrono::system_clock::now();
    switchQueue_.Push(switchInfo);
    return OnSwitchInputMethod(switchInfo, SwitchTrigger::IMSA);
}

void InputMethodSystemAbility::InitMonitors()
{
    int32_t ret = InitAccountMonitor();
    IMSA_HILOGI("init account monitor, ret: %{public}d", ret);
    StartUserIdListener();
    ret = InitKeyEventMonitor();
    IMSA_HILOGI("init KeyEvent monitor, ret: %{public}d", ret);
    ret = InitWmsMonitor();
    IMSA_HILOGI("init wms monitor, ret: %{public}d", ret);
    InitSystemLanguageMonitor();
    if (ImeInfoInquirer::GetInstance().IsEnableInputMethod()) {
        IMSA_HILOGW("Enter enable mode");
        EnableImeDataParser::GetInstance()->Initialize(userId_);
        enableImeOn_ = true;
        RegisterEnableImeObserver();
    }
    if (ImeInfoInquirer::GetInstance().IsEnableSecurityMode()) {
        IMSA_HILOGW("Enter security mode");
        enableSecurityMode_ = true;
        RegisterSecurityModeObserver();
    }
}

int32_t InputMethodSystemAbility::InitAccountMonitor()
{
    IMSA_HILOGI("InputMethodSystemAbility::InitAccountMonitor");
    return ImCommonEventManager::GetInstance()->SubscribeAccountManagerService([this]() {
        auto userId = GetCurrentUserIdFromOsAccount();
        if (userId_ == userId) {
            return;
        }
        HandleUserChanged(userId);
    });
}

int32_t InputMethodSystemAbility::InitKeyEventMonitor()
{
    IMSA_HILOGI("InputMethodSystemAbility::InitKeyEventMonitor");
    bool ret = ImCommonEventManager::GetInstance()->SubscribeKeyboardEvent(
        [this](uint32_t keyCode) { return SwitchByCombinationKey(keyCode); });
    return ret ? ErrorCode::NO_ERROR : ErrorCode::ERROR_SERVICE_START_FAILED;
}

bool InputMethodSystemAbility::InitWmsMonitor()
{
    return ImCommonEventManager::GetInstance()->SubscribeWindowManagerService(
        [this](bool isOnFocused, int32_t pid, int32_t uid) {
            return isOnFocused ? userSession_->OnFocused(pid, uid) : userSession_->OnUnfocused(pid, uid);
        },
        [this]() {
            if (isScbEnable_) {
                IMSA_HILOGI("scb enable, register WMS connection listener");
                InitWmsConnectionMonitor();
                return;
            }
            HandleWmsReady(GetCurrentUserIdFromOsAccount());
        });
}

void InputMethodSystemAbility::InitWmsConnectionMonitor()
{
    WmsConnectionMonitorManager::GetInstance().RegisterWMSConnectionChangedListener(
        [this](int32_t userId, int32_t screenId) { HandleWmsReady(userId); });
}

void InputMethodSystemAbility::InitSystemLanguageMonitor()
{
    SystemLanguageObserver::GetInstance().Watch(
        [this]() { ImeInfoInquirer::GetInstance().RefreshCurrentImeInfo(userId_); });
}

void InputMethodSystemAbility::RegisterEnableImeObserver()
{
    int32_t ret = SettingsDataUtils::GetInstance()->CreateAndRegisterObserver(
        EnableImeDataParser::ENABLE_IME, [this]() { DatashareCallback(EnableImeDataParser::ENABLE_IME); });
    IMSA_HILOGI("Register enable ime observer, ret: %{public}d", ret);
    ret = SettingsDataUtils::GetInstance()->CreateAndRegisterObserver(
        EnableImeDataParser::ENABLE_KEYBOARD, [this]() { DatashareCallback(EnableImeDataParser::ENABLE_KEYBOARD); });
    IMSA_HILOGI("Register enable keyboard observer, ret: %{public}d", ret);
}

void InputMethodSystemAbility::RegisterSecurityModeObserver()
{
    int32_t ret = SettingsDataUtils::GetInstance()->CreateAndRegisterObserver(
        SecurityModeParser::SECURITY_MODE, [this]() { DatashareCallback(SecurityModeParser::SECURITY_MODE); });
    IMSA_HILOGI("Register security mode observer, ret: %{public}d", ret);
}

void InputMethodSystemAbility::DatashareCallback(const std::string &key)
{
    IMSA_HILOGI("run in.");
    if (key == EnableImeDataParser::ENABLE_KEYBOARD || key == EnableImeDataParser::ENABLE_IME) {
        std::lock_guard<std::mutex> autoLock(checkMutex_);
        SwitchInfo switchInfo;
        if (EnableImeDataParser::GetInstance()->CheckNeedSwitch(key, switchInfo, userId_)) {
            switchInfo.timestamp = std::chrono::system_clock::now();
            switchQueue_.Push(switchInfo);
            OnSwitchInputMethod(switchInfo, SwitchTrigger::IMSA);
        }
    }

    if (key == SecurityModeParser::SECURITY_MODE) {
        auto currentBundleName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName;
        if (SecurityModeParser::GetInstance()->IsSecurityChange(currentBundleName, userId_)) {
            int32_t security;
            SecurityModeParser::GetInstance()->GetSecurityMode(currentBundleName, security, userId_);
            userSession_->OnSecurityChange(security);
        }
    }
}

int32_t InputMethodSystemAbility::GetSecurityMode(int32_t &security)
{
    IMSA_HILOGD("GetSecurityMode");
    if (!enableSecurityMode_) {
        security = static_cast<int32_t>(SecurityMode::FULL);
        return ErrorCode::NO_ERROR;
    }
    auto callBundleName = identityChecker_->GetBundleNameByToken(IPCSkeleton::GetCallingTokenID());
    return SecurityModeParser::GetInstance()->GetSecurityMode(callBundleName, security, userId_);
}

int32_t InputMethodSystemAbility::UnRegisteredProxyIme(UnRegisteredType type, const sptr<IInputMethodCore> &core)
{
    if (!identityChecker_->IsNativeSa(IPCSkeleton::GetCallingTokenID())) {
        IMSA_HILOGE("not native sa");
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    return userSession_->OnUnRegisteredProxyIme(type, core);
}

int32_t InputMethodSystemAbility::CheckSwitchPermission(const SwitchInfo &switchInfo, SwitchTrigger trigger)
{
    IMSA_HILOGD("trigger: %{public}d", static_cast<int32_t>(trigger));
    if (trigger == SwitchTrigger::IMSA) {
        return ErrorCode::NO_ERROR;
    }
    if (trigger == SwitchTrigger::SYSTEM_APP) {
        if (!identityChecker_->IsSystemApp(IPCSkeleton::GetCallingFullTokenID())) {
            IMSA_HILOGE("not system app");
            return ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION;
        }
        if (!identityChecker_->HasPermission(IPCSkeleton::GetCallingTokenID(), PERMISSION_CONNECT_IME_ABILITY)) {
            IMSA_HILOGE("not have PERMISSION_CONNECT_IME_ABILITY");
            return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
        }
        return ErrorCode::NO_ERROR;
    }
    if (trigger == SwitchTrigger::CURRENT_IME) {
        // PERMISSION_CONNECT_IME_ABILITY check temporarily reserved for application adaptation, will be deleted soon
        if (identityChecker_->HasPermission(IPCSkeleton::GetCallingTokenID(), PERMISSION_CONNECT_IME_ABILITY)) {
            return ErrorCode::NO_ERROR;
        }
        IMSA_HILOGE("not have PERMISSION_CONNECT_IME_ABILITY");
        // switchInfo.subName.empty() check temporarily reserved for application adaptation, will be deleted soon
        auto currentBundleName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName;
        if (identityChecker_->IsBundleNameValid(IPCSkeleton::GetCallingTokenID(), currentBundleName)
            && !switchInfo.subName.empty()) {
            return ErrorCode::NO_ERROR;
        }
        IMSA_HILOGE("not current ime");
        /* return ErrorCode::ERROR_STATUS_PERMISSION_DENIED temporarily reserved for application adaptation,
        will be replaced by ERROR_NOT_CURRENT_IME soon */
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    return ErrorCode::ERROR_BAD_PARAMETERS;
}

bool InputMethodSystemAbility::IsStartInputTypePermitted()
{
    auto defaultIme = ImeInfoInquirer::GetInstance().GetDefaultImeInfo(userId_);
    if (defaultIme == nullptr) {
        IMSA_HILOGE("failed to get default ime");
        return false;
    }
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (identityChecker_->IsBundleNameValid(tokenId, defaultIme->prop.name)) {
        return true;
    }
    return identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId) && userSession_->IsBoundToClient();
}
} // namespace MiscServices
} // namespace OHOS
