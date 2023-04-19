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

#include "ability_connect_callback_proxy.h"
#include "ability_manager_errors.h"
#include "ability_manager_interface.h"
#include "application_info.h"
#include "bundle_checker.h"
#include "combination_key.h"
#include "common_event_support.h"
#include "errors.h"
#include "global.h"
#include "im_common_event_manager.h"
#include "ime_cfg_manager.h"
#include "ime_info_inquirer.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "itypes_util.h"
#include "key_event.h"
#include "message_handler.h"
#include "os_account_manager.h"
#include "sys/prctl.h"
#include "system_ability.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
using namespace AccountSA;
REGISTER_SYSTEM_ABILITY_BY_ID(InputMethodSystemAbility, INPUT_METHOD_SYSTEM_ABILITY_ID, true);
constexpr std::int32_t INIT_INTERVAL = 10000L;
constexpr std::int32_t MAIN_USER_ID = 100;
constexpr uint32_t RETRY_INTERVAL = 100;
constexpr uint32_t BLOCK_RETRY_TIMES = 100;
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
    if (workThreadHandler.joinable()) {
        workThreadHandler.join();
    }
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
        const auto &params = ImeInfoInquirer::GetInstance().GetInputMethodParam(id);
        if (params.empty()) {
            IMSA_HILOGI("userId: %{public}d The IME properties is empty.", id);
            dprintf(fd, "\n - The IME properties about the Active Id %d is empty.\n", id);
            continue;
        }
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
    state_ = ServiceRunningState::STATE_RUNNING;
    ImeCfgManager::GetInstance().Init();
    std::vector<int32_t> userIds;
    if (BlockRetry(RETRY_INTERVAL, BLOCK_RETRY_TIMES, [&userIds]() -> bool {
            return OsAccountManager::QueryActiveOsAccountIds(userIds) == ERR_OK && !userIds.empty();
        })) {
        userId_ = userIds[0];
        userSession_->UpdateCurrentUserId(userId_);
    }
    StartInputService(ImeInfoInquirer::GetInstance().GetStartedIme(userId_));
    StartUserIdListener();
    int32_t ret = InitKeyEventMonitor();
    IMSA_HILOGI("init KeyEvent monitor %{public}s", ret == ErrorCode::NO_ERROR ? "success" : "failed");
    ret = InitFocusChangeMonitor();
    IMSA_HILOGI("init focus change monitor %{public}s", ret ? "success" : "failed");
    return ErrorCode::NO_ERROR;
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
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("InputMethodSystemAbility");
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
    userSession_ = std::make_shared<PerUserSession>(MAIN_USER_ID);
    userId_ = MAIN_USER_ID;
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

bool InputMethodSystemAbility::StartInputService(const std::string &imeId)
{
    IMSA_HILOGI("InputMethodSystemAbility, ime:%{public}s", imeId.c_str());
    bool isStartSuccess = false;
    sptr<AAFwk::IAbilityManager> abms = GetAbilityManagerService();
    if (abms != nullptr) {
        AAFwk::Want want;
        want.SetAction("action.system.inputmethod");
        std::string::size_type pos = imeId.find("/");
        want.SetElementName(imeId.substr(0, pos), imeId.substr(pos + 1));
        int32_t result = abms->StartAbility(want);
        if (result != ErrorCode::NO_ERROR) {
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

void InputMethodSystemAbility::StopInputService(const std::string &imeId)
{
    IMSA_HILOGE("InputMethodSystemAbility::StopInputService(%{public}s)", imeId.c_str());
    userSession_->StopInputService(imeId);
}

int32_t InputMethodSystemAbility::PrepareInput(InputClientInfo &clientInfo)
{
    if (!clientInfo.isToNotify && !BundleChecker::IsFocused(IPCSkeleton::GetCallingTokenID())) {
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
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
    return userSession_->OnPrepareInput(clientInfo);
}

int32_t InputMethodSystemAbility::ReleaseInput(sptr<IInputClient> client)
{
    if (client == nullptr) {
        IMSA_HILOGE("InputMethodSystemAbility::client is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return userSession_->OnReleaseInput(client);
};

int32_t InputMethodSystemAbility::StartInput(sptr<IInputClient> client, bool isShowKeyboard)
{
    if (!BundleChecker::IsFocused(IPCSkeleton::GetCallingTokenID())) {
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    if (client == nullptr) {
        IMSA_HILOGE("InputMethodSystemAbility::client is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return userSession_->OnStartInput(client, isShowKeyboard);
};

int32_t InputMethodSystemAbility::StopInput(sptr<IInputClient> client)
{
    if (!BundleChecker::IsFocused(IPCSkeleton::GetCallingTokenID())) {
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    if (client == nullptr) {
        IMSA_HILOGE("InputMethodSystemAbility::client is nullptr");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return userSession_->OnStopInput(client);
};

int32_t InputMethodSystemAbility::StopInputSession()
{
    if (!BundleChecker::IsFocused(IPCSkeleton::GetCallingTokenID())) {
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    return userSession_->OnHideKeyboardSelf();
}

int32_t InputMethodSystemAbility::SetCoreAndAgent(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent)
{
    if (core == nullptr || agent == nullptr) {
        CreateComponentFailed(userId_, ErrorCode::ERROR_NULL_POINTER);
        IMSA_HILOGE("InputMethodSystemAbility::core or agent is nullptr");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return userSession_->OnSetCoreAndAgent(core, agent);
};

int32_t InputMethodSystemAbility::HideCurrentInput()
{
    if (!BundleChecker::IsFocused(IPCSkeleton::GetCallingTokenID())) {
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    return userSession_->OnHideKeyboardSelf();
};

int32_t InputMethodSystemAbility::ShowCurrentInput()
{
    if (!BundleChecker::IsFocused(IPCSkeleton::GetCallingTokenID())) {
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    return userSession_->OnShowKeyboardSelf();
};

int32_t InputMethodSystemAbility::DisplayOptionalInputMethod()
{
    return OnDisplayOptionalInputMethod();
};

int32_t InputMethodSystemAbility::SwitchInputMethod(const std::string &bundleName, const std::string &subName)
{
    if (!IsNeedSwitch(bundleName, subName)) {
        return ErrorCode::NO_ERROR;
    }
    ImeInfo info;
    auto ret = ImeInfoInquirer::GetInstance().GetImeInfo(userId_, bundleName, subName, info);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    return info.isNewIme ? SwitchIme(bundleName, info) : SwitchImeType(info);
}

bool InputMethodSystemAbility::IsNeedSwitch(const std::string &bundleName, const std::string &subName)
{
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    IMSA_HILOGI("currentIme: %{public}s, targetIme: %{public}s", currentImeCfg->imeId.c_str(),
        (bundleName + "/" + subName).c_str());
    if ((subName.empty() && bundleName == currentImeCfg->bundleName)
        || (!subName.empty() && subName == currentImeCfg->subName && currentImeCfg->bundleName == bundleName)) {
        IMSA_HILOGI("no need to switch");
        return false;
    }
    return true;
}

int32_t InputMethodSystemAbility::SwitchIme(const std::string &bundleName, const ImeInfo &info)
{
    auto currentImeBundleName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName;
    return bundleName != currentImeBundleName ? SwitchImeType(info) : SwitchImeSubType(info);
}

int32_t InputMethodSystemAbility::SwitchImeType(const ImeInfo &info)
{
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->imeId;
    StopInputService(currentIme);
    std::string targetIme = info.prop.name + "/" + info.prop.id;
    ImeCfgManager::GetInstance().ModifyImeCfg({ userId_, targetIme, info.subProp.id });
    ImeInfoInquirer::GetInstance().SetCurrentImeInfo(info);
    if (!StartInputService(targetIme)) {
        IMSA_HILOGE("start input method failed");
        return ErrorCode::ERROR_IME_START_FAILED;
    }
    userSession_->OnSwitchIme(info.prop, info.subProp, false);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::SwitchImeSubType(const ImeInfo &info)
{
    auto ret = userSession_->OnSwitchIme(info.prop, info.subProp, true);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->imeId;
    ImeCfgManager::GetInstance().ModifyImeCfg({ userId_, currentIme, info.subProp.id });
    ImeInfoInquirer::GetInstance().SetCurrentImeInfo(info);
    return ErrorCode::NO_ERROR;
}

// Deprecated because of no permission check, kept for compatibility
int32_t InputMethodSystemAbility::SetCoreAndAgentDeprecated(sptr<IInputMethodCore> core, sptr<IInputMethodAgent> agent)
{
    return SetCoreAndAgent(core, agent);
};

int32_t InputMethodSystemAbility::HideCurrentInputDeprecated()
{
    if (!BundleChecker::IsFocused(IPCSkeleton::GetCallingTokenID())) {
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    return HideCurrentInput();
};

int32_t InputMethodSystemAbility::ShowCurrentInputDeprecated()
{
    if (!BundleChecker::IsFocused(IPCSkeleton::GetCallingTokenID())) {
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    return ShowCurrentInput();
};

int32_t InputMethodSystemAbility::DisplayOptionalInputMethodDeprecated()
{
    return DisplayOptionalInputMethod();
};

std::shared_ptr<Property> InputMethodSystemAbility::GetCurrentInputMethod()
{
    return ImeInfoInquirer::GetInstance().GetCurrentInputMethod(userId_);
}

std::shared_ptr<SubProperty> InputMethodSystemAbility::GetCurrentInputMethodSubtype()
{
    return ImeInfoInquirer::GetInstance().GetCurrentInputMethodSubtype(userId_);
}

int32_t InputMethodSystemAbility::ListInputMethod(InputMethodStatus status, std::vector<Property> &props)
{
    return ImeInfoInquirer::GetInstance().ListInputMethod(userId_, status, props);
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
    prctl(PR_SET_NAME, "IMSAWorkThread");
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
                break;
            }
            case MSG_ID_HIDE_KEYBOARD_SELF: {
                userSession_->OnHideKeyboardSelf();
                break;
            }
            case MSG_ID_START_INPUT_SERVICE: {
                StartInputService(ImeInfoInquirer::GetInstance().GetStartedIme(userId_));
                break;
            }
            default: {
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
        IMSA_HILOGE("msgContent is nullptr.");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    int32_t oldUserId = userId_;
    userId_ = msg->msgContent_->ReadInt32();
    userSession_->UpdateCurrentUserId(userId_);
    if (oldUserId == userId_) {
        IMSA_HILOGI("device boot, userId: %{public}d", userId_);
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGI("%{public}d switch to %{public}d.", oldUserId, userId_);
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(oldUserId)->imeId;
    StopInputService(currentIme);
    // user switch, reset currentImeInfo_ = nullptr
    ImeInfoInquirer::GetInstance().ResetCurrentImeInfo();
    auto newIme = ImeInfoInquirer::GetInstance().GetStartedIme(userId_);
    StartInputService(newIme);
    return ErrorCode::NO_ERROR;
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
        IMSA_HILOGI("InputMethodSystemAbility::userId: %{public}d, currentUserId: %{public}d,", userId, userId_);
        return ErrorCode::NO_ERROR;
    }
    auto currentImeBundle = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId)->bundleName;
    if (packageName == currentImeBundle) {
        // Switch to the default ime
        auto info = ImeInfoInquirer::GetInstance().GetDefaultImeInfo(userId);
        if (info == nullptr) {
            return ErrorCode::ERROR_PERSIST_CONFIG;
        }
        int32_t ret = SwitchImeType(*info);
        IMSA_HILOGI("InputMethodSystemAbility::OnPackageRemoved ret = %{public}d", ret);
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::OnDisplayOptionalInputMethod()
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
    if (ret != ErrorCode::NO_ERROR && ret != START_SERVICE_ABILITY_ACTIVATING) {
        IMSA_HILOGE("InputMethodSystemAbility::Start InputMethod ability failed, err = %{public}d", ret);
        return ErrorCode::ERROR_EX_SERVICE_SPECIFIC;
    }
    IMSA_HILOGI("InputMethodSystemAbility::Start InputMethod ability success.");
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

int32_t InputMethodSystemAbility::SwitchByCombinationKey(uint32_t state)
{
    IMSA_HILOGI("InputMethodSystemAbility::SwitchByCombinationKey");
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
        return SwitchInputMethod();
    }
    IMSA_HILOGE("keycode undefined");
    return ErrorCode::ERROR_EX_UNSUPPORTED_OPERATION;
}

int32_t InputMethodSystemAbility::SwitchMode()
{
    ImeInfo info;
    auto bundleName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName;
    auto subName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->subName;
    auto ret = ImeInfoInquirer::GetInstance().GetImeInfo(userId_, bundleName, subName, info);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("current ime is abnormal, ret: %{public}d", ret);
        return ret;
    }
    auto condition = info.subProp.mode == "upper" ? Condition::LOWER : Condition::UPPER;
    auto target = ImeInfoInquirer::GetInstance().GetImeSubProp(info.subProps, condition);
    if (target == nullptr) {
        IMSA_HILOGE("target is empty");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    return SwitchInputMethod(target->name, target->id);
}

int32_t InputMethodSystemAbility::SwitchLanguage()
{
    ImeInfo info;
    auto bundleName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName;
    auto subName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->subName;
    auto ret = ImeInfoInquirer::GetInstance().GetImeInfo(userId_, bundleName, subName, info);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("current ime is abnormal, ret: %{public}d", ret);
        return ret;
    }
    if (info.subProp.language != "chinese" && info.subProp.language != "english") {
        return ErrorCode::NO_ERROR;
    }
    auto condition = info.subProp.language == "chinese" ? Condition::ENGLISH : Condition::CHINESE;
    auto target = ImeInfoInquirer::GetInstance().GetImeSubProp(info.subProps, condition);
    if (target == nullptr) {
        IMSA_HILOGE("target is empty");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    return SwitchInputMethod(target->name, target->id);
}

int32_t InputMethodSystemAbility::SwitchInputMethod()
{
    std::vector<Property> props = {};
    auto ret = ImeInfoInquirer::GetInstance().ListInputMethod(userId_, ALL, props);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("ListProperty failed");
        return ret;
    }
    auto currentImeBundle = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_)->bundleName;
    auto iter = std::find_if(props.begin(), props.end(),
        [&currentImeBundle](const Property &property) { return property.name != currentImeBundle; });
    if (iter != props.end()) {
        return SwitchInputMethod(iter->name, "");
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::InitKeyEventMonitor()
{
    IMSA_HILOGI("InputMethodSystemAbility::InitKeyEventMonitor");
    bool ret = ImCommonEventManager::GetInstance()->SubscribeKeyboardEvent(
        [this](uint32_t keyCode) { return SwitchByCombinationKey(keyCode); });
    return ret ? ErrorCode::NO_ERROR : ErrorCode::ERROR_SERVICE_START_FAILED;
}

bool InputMethodSystemAbility::InitFocusChangeMonitor()
{
    return ImCommonEventManager::GetInstance()->SubscribeWindowManagerService(
        [this](int32_t pid, int32_t uid) { return userSession_->OnUnfocused(pid, uid); });
}
} // namespace MiscServices
} // namespace OHOS
