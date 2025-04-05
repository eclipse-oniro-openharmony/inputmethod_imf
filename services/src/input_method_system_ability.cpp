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

#include "unordered_map"
#include "variant"
#include "input_method_system_ability.h"
#include "ability_manager_client.h"
#include "combination_key.h"
#include "full_ime_info_manager.h"
#include "im_common_event_manager.h"
#include "imsa_hisysevent_reporter.h"
#include "input_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "itypes_util.h"
#include "mem_mgr_client.h"
#include "inputmethod_message_handler.h"
#include "os_account_adapter.h"
#include "scene_board_judgement.h"
#include "system_ability_definition.h"
#ifdef IMF_SCREENLOCK_MGR_ENABLE
#include "screenlock_manager.h"
#endif
#include "system_language_observer.h"
#include "wms_connection_observer.h"
#include "xcollie/xcollie.h"
#ifdef IMF_ON_DEMAND_START_STOP_SA_ENABLE
#include "on_demand_start_stop_sa.h"
#endif
#include "window_adapter.h"

namespace OHOS {
namespace MiscServices {
using namespace MessageID;
using namespace AppExecFwk;
using namespace Security::AccessToken;
REGISTER_SYSTEM_ABILITY_BY_ID(InputMethodSystemAbility, INPUT_METHOD_SYSTEM_ABILITY_ID, true);
constexpr std::int32_t INIT_INTERVAL = 10000L;
constexpr const char *UNDEFINED = "undefined";
static const char *PERMISSION_CONNECT_IME_ABILITY = "ohos.permission.CONNECT_IME_ABILITY";
std::shared_ptr<AppExecFwk::EventHandler> InputMethodSystemAbility::serviceHandler_;
constexpr uint32_t START_SA_TIMEOUT = 6; // 6s
constexpr const char *SELECT_DIALOG_ACTION = "action.system.inputmethodchoose";
constexpr const char *SELECT_DIALOG_HAP = "com.ohos.inputmethodchoosedialog";
constexpr const char *SELECT_DIALOG_ABILITY = "InputMethod";
#ifdef IMF_ON_DEMAND_START_STOP_SA_ENABLE
constexpr const char *UNLOAD_SA_TASK = "unloadInputMethodSaTask";
constexpr int64_t DELAY_UNLOAD_SA_TIME = 20000; // 20s
constexpr int32_t REFUSE_UNLOAD_DELAY_TIME = 1000; // 1s
#endif

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

#ifdef IMF_ON_DEMAND_START_STOP_SA_ENABLE
int64_t InputMethodSystemAbility::GetTickCount()
{
    auto now = std::chrono::steady_clock::now();
    auto durationSinceEpoch = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(durationSinceEpoch).count();
}
void InputMethodSystemAbility::ResetDelayUnloadTask(uint32_t code)
{
    auto task = [this]() {
        IMSA_HILOGI("start unload task");
        auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
        if (session != nullptr) {
            session->TryUnloadSystemAbility();
        }
    };

    static std::mutex lastPostTimeLock;
    std::lock_guard<std::mutex> lock(lastPostTimeLock);
    static int64_t lastPostTime = 0;
    if (code == static_cast<uint32_t>(InputMethodInterfaceCode::RELEASE_INPUT) ||
        code == static_cast<uint32_t>(InputMethodInterfaceCode::REQUEST_HIDE_INPUT)) {
        if (lastPostTime != 0 && (GetTickCount() - lastPostTime) < DELAY_UNLOAD_SA_TIME) {
            IMSA_HILOGD("no need post unload task repeat");
            return;
        }
    }

    if (serviceHandler_ == nullptr) {
        IMSA_HILOGE("serviceHandler_ is nullptr code:%{public}u", code);
        return;
    }

    serviceHandler_->RemoveTask(std::string(UNLOAD_SA_TASK));
    IMSA_HILOGD("post unload task");
    lastPostTime = GetTickCount();
    bool ret = serviceHandler_->PostTask(task, std::string(UNLOAD_SA_TASK), DELAY_UNLOAD_SA_TIME);
    if (!ret) {
        IMSA_HILOGE("post unload task fail code:%{public}u", code);
    }
}
bool InputMethodSystemAbility::IsImeInUse()
{
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("session is nullptr userId: %{public}d", userId_);
        return false;
    }

    auto data = session->GetReadyImeData(ImeType::IME);
    if (data == nullptr || data->freezeMgr == nullptr) {
        IMSA_HILOGE("data or freezeMgr is nullptr");
        return false;
    }
    return data->freezeMgr->IsImeInUse();
}
#endif

int32_t InputMethodSystemAbility::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
#ifdef IMF_ON_DEMAND_START_STOP_SA_ENABLE
    OnDemandStartStopSa::IncreaseProcessingIpcCnt();
#endif
    auto ret = InputMethodSystemAbilityStub::OnRemoteRequest(code, data, reply, option);
#ifdef IMF_ON_DEMAND_START_STOP_SA_ENABLE
    OnDemandStartStopSa::DecreaseProcessingIpcCnt();
    ResetDelayUnloadTask(code);
#endif
    return ret;
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
    auto id = HiviewDFX::XCollie::GetInstance().SetTimer(
        "IMSA OnStart timeout", START_SA_TIMEOUT, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_DEFAULT);
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
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    IMSA_HILOGI("start imsa service success.");
    return;
}

bool InputMethodSystemAbility::IsValidBundleName(const std::string &bundleName)
{
    if (bundleName.empty()) {
        IMSA_HILOGE("bundleName is empty.");
        return false;
    }
    std::vector<Property> props;
    auto ret = ListInputMethod(InputMethodStatus::ALL, props);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("ListInputMethod failed, ret=%{public}d", ret);
        return false;
    }

    for (auto &prop : props) {
        if (prop.name == bundleName) {
            return true;
        }
    }
    return false;
}

std::string InputMethodSystemAbility::GetRestoreBundleName(MessageParcel &data)
{
    std::string jsonString = data.ReadString();
    if (jsonString.empty()) {
        IMSA_HILOGE("jsonString is empty.");
        return "";
    }
    IMSA_HILOGI("restore jsonString=%{public}s", jsonString.c_str());

    cJSON *root = cJSON_Parse(jsonString.c_str());
    if (root == NULL) {
        IMSA_HILOGE("cJSON_Parse fail");
        return "";
    }
    std::string bundleName = "";
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, root)
    {
        cJSON *type = cJSON_GetObjectItem(item, "type");
        cJSON *detail = cJSON_GetObjectItem(item, "detail");
        if (type == NULL || detail == NULL || type->valuestring == NULL || detail->valuestring == NULL) {
            IMSA_HILOGE("type or detail is null");
            continue;
        }

        if (strcmp(type->valuestring, "default_input_method") == 0) {
            bundleName = std::string(detail->valuestring);
            break;
        }
    }
    cJSON_Delete(root);
    return bundleName;
}

int32_t InputMethodSystemAbility::RestoreInputmethod(std::string &bundleName)
{
    std::shared_ptr<Property> prop = GetCurrentInputMethod();
    if (prop == nullptr) {
        IMSA_HILOGE("GetCurrentInputMethod failed");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    std::string currentInputMethod = prop->name;
    if (currentInputMethod == bundleName) {
        IMSA_HILOGW("currentInputMethod=%{public}s, has been set", currentInputMethod.c_str());
        return ErrorCode::NO_ERROR;
    }

    int32_t userId = GetCallingUserId();
    bool result = SettingsDataUtils::GetInstance()->EnableIme(userId, bundleName);
    if (!result) {
        IMSA_HILOGE("EnableIme failed");
        return ErrorCode::ERROR_ENABLE_IME;
    }

    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("session[ userId=%{public}d ] is nullptr", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    SwitchInfo switchInfo = { std::chrono::system_clock::now(), bundleName, "" };
    switchInfo.timestamp = std::chrono::system_clock::now();
    session->GetSwitchQueue().Push(switchInfo);
    auto ret = OnSwitchInputMethod(userId, switchInfo, SwitchTrigger::IMSA);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("SwitchInputMethod failed, ret=%{public}d.", ret);
        return ret;
    }
    IMSA_HILOGI("restore success");
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::OnExtension(const std::string &extension, MessageParcel &data, MessageParcel &reply)
{
    IMSA_HILOGI("extension=%{public}s", extension.c_str());
    if (extension == "restore") {
        (void)data.ReadFileDescriptor();
        std::string bundleName = GetRestoreBundleName(data);
        if (!IsValidBundleName(bundleName)) {
            IMSA_HILOGE("bundleName=%{public}s is invalid", bundleName.c_str());
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }

        return RestoreInputmethod(bundleName);
    }
    return 0;
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
    IMSA_HILOGI("InputMethodSystemAbility::Init start.");
#ifdef IMF_ON_DEMAND_START_STOP_SA_ENABLE
    ImeCfgManager::GetInstance().Init();
    ImeInfoInquirer::GetInstance().InitSystemConfig();
    bool isSuccess = Publish(this);
    if (!isSuccess) {
        IMSA_HILOGE("publish failed");
        return -1;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    IMSA_HILOGI("publish success");
#else
    bool isSuccess = Publish(this);
    if (!isSuccess) {
        IMSA_HILOGE("publish failed");
        return -1;
    }
    IMSA_HILOGI("publish success");
    state_ = ServiceRunningState::STATE_RUNNING;
    ImeCfgManager::GetInstance().Init();
    ImeInfoInquirer::GetInstance().InitSystemConfig();
#endif
    InitMonitors();
    return ErrorCode::NO_ERROR;
}

void InputMethodSystemAbility::UpdateUserInfo(int32_t userId)
{
    IMSA_HILOGI("%{public}d switch to %{public}d.", userId_, userId);
    userId_ = userId;
    UserSessionManager::GetInstance().AddUserSession(userId_);
    InputMethodSysEvent::GetInstance().SetUserId(userId_);
    if (enableImeOn_.load()) {
        EnableImeDataParser::GetInstance()->OnUserChanged(userId_);
    }
    if (enableSecurityMode_.load()) {
        SecurityModeParser::GetInstance()->UpdateFullModeList(userId_);
    }
}

int32_t InputMethodSystemAbility::OnIdle(const SystemAbilityOnDemandReason &idleReason)
{
    IMSA_HILOGI("OnIdle start.");
    (void)idleReason;
#ifdef IMF_ON_DEMAND_START_STOP_SA_ENABLE
    if (OnDemandStartStopSa::IsSaBusy() || IsImeInUse()) {
        IMSA_HILOGW("sa is busy, refuse stop imsa.");
        return REFUSE_UNLOAD_DELAY_TIME;
    }
#endif
    return 0;
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
        IMSA_HILOGE("InputMethodSystemAbility already init!");
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
    IMSA_HILOGI("start get scene board enable status");
    isScbEnable_.store(Rosen::SceneBoardJudgement::IsSceneBoardEnabled());
    IMSA_HILOGI("InputMethodSystemAbility::Initialize end.");
}

void InputMethodSystemAbility::RestartSessionIme(std::shared_ptr<PerUserSession> &session)
{
    if (session == nullptr) {
        UserSessionManager::GetInstance().AddUserSession(userId_);
    }
    session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId_);
        return;
    }
#ifndef IMF_ON_DEMAND_START_STOP_SA_ENABLE
    session->AddRestartIme();
#endif
    StopImeInBackground();
}

std::shared_ptr<PerUserSession> InputMethodSystemAbility::GetSessionFromMsg(const Message *msg)
{
    if (msg == nullptr || msg->msgContent_ == nullptr) {
        IMSA_HILOGE("Aborted! Message is nullptr!");
        return nullptr;
    }
    auto userId = msg->msgContent_->ReadInt32();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return nullptr;
    }
    return session;
}

int32_t InputMethodSystemAbility::PrepareForOperateKeyboard(std::shared_ptr<PerUserSession> &session)
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto userId = GetCallingUserId();
    session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (!identityChecker_->IsBroker(tokenId)) {
        if (!identityChecker_->IsFocused(
            IPCSkeleton::GetCallingPid(), tokenId, session->GetCurrentClientPid(GetCallingDisplayId()))) {
            return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::SwitchByCondition(const Condition &condition,
    const std::shared_ptr<ImeInfo> &info)
{
    auto target = ImeInfoInquirer::GetInstance().FindTargetSubtypeByCondition(info->subProps, condition);
    if (target == nullptr) {
        IMSA_HILOGE("target is empty!");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    SwitchInfo switchInfo = { std::chrono::system_clock::now(), target->name, target->id };
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId_);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    session->GetSwitchQueue().Push(switchInfo);
    return OnSwitchInputMethod(userId_, switchInfo, SwitchTrigger::IMSA);
}

void InputMethodSystemAbility::SubscribeCommonEvent()
{
    sptr<ImCommonEventManager> imCommonEventManager = ImCommonEventManager::GetInstance();
    bool isSuccess = imCommonEventManager->SubscribeEvent();
    if (isSuccess) {
        IMSA_HILOGI("initialize subscribe service event success.");
        return;
    }

    IMSA_HILOGE("failed, try again 10s later!");
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
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND;
    }
    return session->OnPrepareInput(clientInfo);
}

int32_t InputMethodSystemAbility::GenerateClientInfo(int32_t userId, InputClientInfo &clientInfo)
{
    if (clientInfo.client == nullptr || clientInfo.channel == nullptr) {
        IMSA_HILOGE("client or channel is nullptr!");
        return ErrorCode::ERROR_IMSA_NULLPTR;
    }
    auto deathRecipient = new (std::nothrow) InputDeathRecipient();
    if (deathRecipient == nullptr) {
        IMSA_HILOGE("failed to new deathRecipient!");
        return ErrorCode::ERROR_IMSA_MALLOC_FAILED;
    }
    clientInfo.pid = IPCSkeleton::GetCallingPid();
    clientInfo.uid = IPCSkeleton::GetCallingUid();
    clientInfo.displayId = GetCallingDisplayId();
    clientInfo.userID = userId;
    clientInfo.deathRecipient = deathRecipient;
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (identityChecker_->IsFocusedUIExtension(tokenId)) {
        clientInfo.uiExtensionTokenId = tokenId;
    }
    auto callingDisplayId = identityChecker_->GetDisplayIdByWindowId(clientInfo.config.windowId);
    clientInfo.config.privateCommand.insert_or_assign("displayId",
        PrivateDataValue(static_cast<int32_t>(callingDisplayId)));
    clientInfo.name = ImfHiSysEventUtil::GetAppName(tokenId);
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session != nullptr) {
        auto callingWindowInfo = session->GetCallingWindowInfo(clientInfo);
        clientInfo.config.windowId = callingWindowInfo.windowId;
        clientInfo.config.inputAttribute.windowId = callingWindowInfo.windowId;
        clientInfo.config.inputAttribute.callingDisplayId = callingWindowInfo.displayId;
        IMSA_HILOGD("result:%{public}s,wid:%{public}d", clientInfo.config.inputAttribute.ToString().c_str(),
            clientInfo.config.windowId);
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::ReleaseInput(sptr<IInputClient> client, uint32_t sessionId)
{
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnReleaseInput(client, sessionId);
}

int32_t InputMethodSystemAbility::StartInput(
    InputClientInfo &inputClientInfo, sptr<IRemoteObject> &agent, std::pair<int64_t, std::string> &imeInfo)
{
    auto ret = StartInputInner(inputClientInfo, agent, imeInfo);
    IMSA_HILOGD("HiSysEvent report start!");
    auto evenInfo = HiSysOriginalInfo::Builder()
                        .SetPeerName(ImfHiSysEventUtil::GetAppName(IPCSkeleton::GetCallingTokenID()))
                        .SetPeerPid(IPCSkeleton::GetCallingPid())
                        .SetPeerUserId(GetCallingUserId())
                        .SetClientType(inputClientInfo.type)
                        .SetInputPattern(inputClientInfo.attribute.inputPattern)
                        .SetIsShowKeyboard(inputClientInfo.isShowKeyboard)
                        .SetImeName(imeInfo.second)
                        .SetErrCode(ret)
                        .Build();
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_ATTACH, *evenInfo);
    IMSA_HILOGE("HiSysEvent report end!");
    return ret;
}

int32_t InputMethodSystemAbility::StartInputInner(
    InputClientInfo &inputClientInfo, sptr<IRemoteObject> &agent, std::pair<int64_t, std::string> &imeInfo)
{
    auto userId = GetCallingUserId();
    imeInfo = GetCurrentImeInfoForHiSysEvent(userId);
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (ImeInfoInquirer::GetInstance().IsInputMethodExtension(IPCSkeleton::GetCallingPid()) ||
        (!identityChecker_->IsBroker(tokenId) &&
         !identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId))) {
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND;
    }
    auto displayId = GetCallingDisplayId();
    if (session->GetCurrentClientPid(displayId) != IPCSkeleton::GetCallingPid()
        && session->GetInactiveClientPid(displayId) != IPCSkeleton::GetCallingPid()) {
        // notify inputStart when caller pid different from both current client and inactive client
        inputClientInfo.isNotifyInputStart = true;
    }
    auto displayGroupId = session->GetDisplayGroupId(displayId);
    if (session->CheckPwdInputPatternConv(inputClientInfo, displayId)) {
        inputClientInfo.needHide = true;
        inputClientInfo.isNotifyInputStart = true;
    }
    if (displayGroupId == DEFAULT_DISPLAY_ID && !session->IsProxyImeEnable()) {
        auto ret = CheckInputTypeOption(userId, inputClientInfo);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("%{public}d failed to CheckInputTypeOption!", userId);
            return ret;
        }
    }
    inputClientInfo.config.inputAttribute.bundleName = identityChecker_->GetBundleNameByToken(tokenId);
    int32_t ret = PrepareInput(userId, inputClientInfo);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to PrepareInput!");
        return ret;
    }
    session->SetInputType();
    return session->OnStartInput(inputClientInfo, agent, imeInfo);
}

int32_t InputMethodSystemAbility::CheckInputTypeOption(int32_t userId, InputClientInfo &inputClientInfo)
{
    IMSA_HILOGI("SecurityFlag: %{public}d, IsSameTextInput: %{public}d, IsStarted: %{public}d, "
                "IsSecurityImeStarted: %{public}d.",
        inputClientInfo.config.inputAttribute.GetSecurityFlag(), !inputClientInfo.isNotifyInputStart,
        InputTypeManager::GetInstance().IsStarted(), InputTypeManager::GetInstance().IsSecurityImeStarted());
    if (inputClientInfo.config.inputAttribute.GetSecurityFlag()) {
        if (!InputTypeManager::GetInstance().IsStarted()) {
            IMSA_HILOGD("SecurityFlag, input type is not started, start.");
            // if need to switch ime, no need to hide panel first.
            NeedHideWhenSwitchInputType(userId, inputClientInfo.needHide);
            return StartInputType(userId, InputType::SECURITY_INPUT);
        }
        if (!inputClientInfo.isNotifyInputStart) {
            IMSA_HILOGD("SecurityFlag, same textField, input type is started, not deal.");
            return ErrorCode::NO_ERROR;
        }
        if (!InputTypeManager::GetInstance().IsSecurityImeStarted()) {
            IMSA_HILOGD("SecurityFlag, new textField, input type is started, but it is not security, switch.");
            NeedHideWhenSwitchInputType(userId, inputClientInfo.needHide);
            return StartInputType(userId, InputType::SECURITY_INPUT);
        }
        IMSA_HILOGD("SecurityFlag, other condition, not deal.");
        return ErrorCode::NO_ERROR;
    }
    if (!inputClientInfo.isNotifyInputStart) {
        IMSA_HILOGD("NormalFlag, same textField, not deal.");
        return ErrorCode::NO_ERROR;
    }
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND;
    }
    if (InputTypeManager::GetInstance().IsStarted()) {
        IMSA_HILOGD("NormalFlag, diff textField, input type started, restore.");
        session->RestoreCurrentImeSubType();
    }
#ifdef IMF_SCREENLOCK_MGR_ENABLE
    if (ScreenLock::ScreenLockManager::GetInstance()->IsScreenLocked()) {
        std::string ime;
        if (GetScreenLockIme(ime) != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("not ime screenlocked");
            return ErrorCode::ERROR_IMSA_IME_TO_START_NULLPTR;
        }
        ImeCfgManager::GetInstance().ModifyTempScreenLockImeCfg(userId_, ime);
    }
#endif
    return session->RestoreCurrentIme();
}

int32_t InputMethodSystemAbility::ShowInputInner(sptr<IInputClient> client, int32_t requestKeyboardReason)
{
    std::shared_ptr<PerUserSession> session = nullptr;
    auto result = PrepareForOperateKeyboard(session);
    if (result != ErrorCode::NO_ERROR) {
        return result;
    }
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return session->OnShowInput(client, requestKeyboardReason);
}

int32_t InputMethodSystemAbility::HideInput(sptr<IInputClient> client)
{
    std::shared_ptr<PerUserSession> session = nullptr;
    auto result = PrepareForOperateKeyboard(session);
    if (result != ErrorCode::NO_ERROR) {
        return result;
    }
    if (client == nullptr) {
        IMSA_HILOGE("client is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    return session->OnHideInput(client);
}

int32_t InputMethodSystemAbility::StopInputSession()
{
    std::shared_ptr<PerUserSession> session = nullptr;
    auto result = PrepareForOperateKeyboard(session);
    if (result != ErrorCode::NO_ERROR) {
        return result;
    }
    return session->OnHideCurrentInput(GetCallingDisplayId());
}

int32_t InputMethodSystemAbility::RequestShowInput()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId) &&
        !identityChecker_->HasPermission(tokenId, std::string(PERMISSION_CONNECT_IME_ABILITY))) {
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND;
    }
    return session->OnRequestShowInput(GetCallingDisplayId());
}

int32_t InputMethodSystemAbility::RequestHideInput(bool isFocusTriggered)
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto pid = IPCSkeleton::GetCallingPid();
    if (isFocusTriggered) {
        if (!identityChecker_->IsFocused(pid, tokenId)) {
            return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
        }
    } else {
        if (!identityChecker_->IsFocused(pid, tokenId) &&
            !identityChecker_->HasPermission(tokenId, std::string(PERMISSION_CONNECT_IME_ABILITY))) {
            return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
        }
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnRequestHideInput(pid, GetCallingDisplayId());
}

int32_t InputMethodSystemAbility::SetCoreAndAgent(const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent)
{
    IMSA_HILOGD("InputMethodSystemAbility start.");
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (identityChecker_->IsNativeSa(IPCSkeleton::GetCallingTokenID())) {
        return session->OnRegisterProxyIme(core, agent);
    }
    if (!IsCurrentIme(userId)) {
        IMSA_HILOGE("not current ime, userId:%{public}d", userId);
        return ErrorCode::ERROR_NOT_CURRENT_IME;
    }
    return session->OnSetCoreAndAgent(core, agent);
}

int32_t InputMethodSystemAbility::RegisterProxyIme(
    uint64_t displayId, const sptr<IInputMethodCore> &core, const sptr<IRemoteObject> &agent)
{
    if (!ImeInfoInquirer::GetInstance().IsEnableAppAgent()) {
        IMSA_HILOGE("current device does not support app agent");
        return ErrorCode::ERROR_DEVICE_UNSUPPORTED;
    }
    if (!identityChecker_->IsValidVirtualIme(IPCSkeleton::GetCallingUid())) {
        IMSA_HILOGE("not agent sa");
        return ErrorCode::ERROR_NOT_AI_APP_IME;
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnRegisterProxyIme(displayId, core, agent);
}

int32_t InputMethodSystemAbility::UnregisterProxyIme(uint64_t displayId)
{
    if (!ImeInfoInquirer::GetInstance().IsEnableAppAgent()) {
        IMSA_HILOGE("current device does not support app agent");
        return ErrorCode::ERROR_DEVICE_UNSUPPORTED;
    }
    if (!identityChecker_->IsValidVirtualIme(IPCSkeleton::GetCallingUid())) {
        IMSA_HILOGE("not agent sa");
        return ErrorCode::ERROR_NOT_AI_APP_IME;
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnUnregisterProxyIme(displayId);
}

int32_t InputMethodSystemAbility::InitConnect()
{
    IMSA_HILOGD("InputMethodSystemAbility init connect.");
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (!IsCurrentIme(userId)) {
        return ErrorCode::ERROR_NOT_CURRENT_IME;
    }
    return session->InitConnect(IPCSkeleton::GetCallingPid());
}

int32_t InputMethodSystemAbility::HideCurrentInput()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (identityChecker_->IsBroker(tokenId)) {
        return session->OnHideCurrentInput(GetCallingDisplayId());
    }
    if (!identityChecker_->HasPermission(tokenId, std::string(PERMISSION_CONNECT_IME_ABILITY))) {
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    return session->OnHideCurrentInput(GetCallingDisplayId());
}

int32_t InputMethodSystemAbility::ShowCurrentInputInner()
{
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND;
    }
    if (identityChecker_->IsBroker(tokenId)) {
        return session->OnShowCurrentInput(GetCallingDisplayId());
    }
    if (!identityChecker_->HasPermission(tokenId, std::string(PERMISSION_CONNECT_IME_ABILITY))) {
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    return session->OnShowCurrentInput(GetCallingDisplayId());
}

int32_t InputMethodSystemAbility::PanelStatusChange(const InputWindowStatus &status, const ImeWindowInfo &info)
{
    auto userId = GetCallingUserId();
    if (!IsCurrentIme(userId)) {
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
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnPanelStatusChange(status, info, GetCallingDisplayId());
}

int32_t InputMethodSystemAbility::UpdateListenEventFlag(InputClientInfo &clientInfo, uint32_t eventFlag)
{
    IMSA_HILOGI("finalEventFlag: %{public}u, eventFlag: %{public}u.", clientInfo.eventFlag, eventFlag);
    if (EventStatusManager::IsImeHideOn(eventFlag) || EventStatusManager::IsImeShowOn(eventFlag) ||
        EventStatusManager::IsInputStatusChangedOn(eventFlag)) {
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
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return session->OnUpdateListenEventFlag(clientInfo);
}

int32_t InputMethodSystemAbility::SetCallingWindow(uint32_t windowId, sptr<IInputClient> client)
{
    IMSA_HILOGD("IMF SA setCallingWindow enter");
    if (identityChecker_ == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->IsBroker(tokenId) &&
        !identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId)) {
        return ErrorCode::ERROR_CLIENT_NOT_FOCUSED;
    }
    auto callingUserId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(callingUserId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", callingUserId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto callingDisplayId = identityChecker_->GetDisplayIdByWindowId(windowId);
    return session->OnSetCallingWindow(windowId, callingDisplayId, client);
}

int32_t InputMethodSystemAbility::GetInputStartInfo(bool& isInputStart,
    uint32_t& callingWndId, int32_t &requestKeyboardReason)
{
    if (!identityChecker_->IsNativeSa(IPCSkeleton::GetCallingTokenID())) {
        IMSA_HILOGE("not native sa!");
        return ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION;
    }
    auto callingUserId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(callingUserId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", callingUserId);
        return false;
    }
    return session->GetInputStartInfo(GetCallingDisplayId(), isInputStart, callingWndId, requestKeyboardReason);
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
        IMSA_HILOGE("not default ime!");
        return ErrorCode::ERROR_NOT_DEFAULT_IME;
    }
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (session->CheckSecurityMode()) {
        return StartInputType(userId, InputType::SECURITY_INPUT);
    }
    auto typeIme = InputTypeManager::GetInstance().GetCurrentIme();
    auto cfgIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    if (cfgIme->bundleName == typeIme.bundleName) {
        return session->RestoreCurrentImeSubType();
    }
    return session->RestoreCurrentIme();
}

int32_t InputMethodSystemAbility::IsDefaultIme()
{
    return IsDefaultImeFromTokenId(GetCallingUserId(), IPCSkeleton::GetCallingTokenID());
}

bool InputMethodSystemAbility::IsSystemApp()
{
    return identityChecker_->IsSystemApp(IPCSkeleton::GetCallingFullTokenID());
}

int32_t InputMethodSystemAbility::IsDefaultImeFromTokenId(int32_t userId, uint32_t tokenId)
{
    auto prop = std::make_shared<Property>();
    auto ret = ImeInfoInquirer::GetInstance().GetDefaultInputMethod(userId, prop, true);
    if (ret != ErrorCode::NO_ERROR || prop == nullptr) {
        IMSA_HILOGE("failed to get default ime!");
        return ErrorCode::ERROR_PERSIST_CONFIG;
    }
    if (!identityChecker_->IsBundleNameValid(tokenId, prop->name)) {
        return ErrorCode::ERROR_NOT_DEFAULT_IME;
    }
    return ErrorCode::NO_ERROR;
}

bool InputMethodSystemAbility::IsCurrentImeByPid(int32_t pid)
{
    if (!identityChecker_->IsSystemApp(IPCSkeleton::GetCallingFullTokenID()) &&
        !identityChecker_->IsNativeSa(IPCSkeleton::GetCallingTokenID())) {
        IMSA_HILOGE("not system application or system ability!");
        return false;
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return false;
    }
    return session->IsCurrentImeByPid(pid);
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
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
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
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (enableImeOn_.load() && !EnableImeDataParser::GetInstance()->CheckNeedSwitch(switchInfo, userId)) {
        IMSA_HILOGW("Enable mode off or switch is not enable, stopped!");
        return ErrorCode::ERROR_ENABLE_IME;
    }
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    if (switchInfo.subName.empty() && switchInfo.bundleName == currentImeCfg->bundleName) {
        switchInfo.subName = currentImeCfg->subName;
    }
    switchInfo.timestamp = std::chrono::system_clock::now();
    session->GetSwitchQueue().Push(switchInfo);
    return InputTypeManager::GetInstance().IsInputType({ bundleName, subName })
               ? OnStartInputType(userId, switchInfo, true)
               : OnSwitchInputMethod(userId, switchInfo, trigger);
}

bool InputMethodSystemAbility::EnableIme(const std::string &bundleName)
{
    if (CheckEnableAndSwitchPermission() != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Check enable ime value failed!");
        return false;
    }
    int32_t userId = GetCallingUserId();
    return SettingsDataUtils::GetInstance()->EnableIme(userId, bundleName);
}

int32_t InputMethodSystemAbility::OnSwitchInputMethod(int32_t userId, const SwitchInfo &switchInfo,
    SwitchTrigger trigger)
{
    InputMethodSysEvent::GetInstance().RecordEvent(IMEBehaviour::CHANGE_IME);
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
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
    auto info = ImeInfoInquirer::GetInstance().GetImeInfo(userId, switchInfo.bundleName, switchInfo.subName);
    if (info == nullptr) {
        session->GetSwitchQueue().Pop();
        return ErrorCode::ERROR_IMSA_GET_IME_INFO_FAILED;
    }
    InputTypeManager::GetInstance().Set(false);
    {
        InputMethodSyncTrace tracer("InputMethodSystemAbility_OnSwitchInputMethod");
        std::string targetImeName = info->prop.name + "/" + info->prop.id;
        ImeCfgManager::GetInstance().ModifyImeCfg({ userId, targetImeName, info->subProp.id, true });
        auto targetIme = std::make_shared<ImeNativeCfg>(ImeNativeCfg{
            targetImeName, info->prop.name, switchInfo.subName.empty() ? "" : info->subProp.id, info->prop.id });
        ret = session->StartIme(targetIme);
        if (ret != ErrorCode::NO_ERROR) {
            InputMethodSysEvent::GetInstance().InputmethodFaultReporter(
                ret, switchInfo.bundleName, "switch input method failed!");
            session->GetSwitchQueue().Pop();
            return ret;
        }
        GetValidSubtype(switchInfo.subName, info);
        session->NotifyImeChangeToClients(info->prop, info->subProp);
        ret = session->SwitchSubtype(info->subProp);
    }
    session->GetSwitchQueue().Pop();
    ret = info->isSpecificSubName ? ret : ErrorCode::NO_ERROR;
    if (ret != ErrorCode::NO_ERROR) {
        InputMethodSysEvent::GetInstance().InputmethodFaultReporter(ret, switchInfo.bundleName,
            "switch input method subtype failed!");
    }
    return ret;
}

void InputMethodSystemAbility::GetValidSubtype(const std::string &subName, const std::shared_ptr<ImeInfo> &info)
{
    if (subName.empty()) {
        IMSA_HILOGW("undefined subtype");
        info->subProp.id = UNDEFINED;
        info->subProp.name = UNDEFINED;
    }
}

int32_t InputMethodSystemAbility::OnStartInputType(int32_t userId, const SwitchInfo &switchInfo,
    bool isCheckPermission)
{
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND;
    }
    if (!session->GetSwitchQueue().IsReady(switchInfo)) {
        IMSA_HILOGD("start wait.");
        session->GetSwitchQueue().Wait(switchInfo);
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
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    std::string targetImeName = info->prop.name + "/" + info->prop.id;
    ImeCfgManager::GetInstance().ModifyImeCfg({ userId, targetImeName, info->subProp.id, false });
    ImeNativeCfg targetIme = { targetImeName, info->prop.name, info->subProp.id, info->prop.id };
    auto ret = session->StartIme(std::make_shared<ImeNativeCfg>(targetIme));
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("start input method failed!");
        return ret;
    }
    session->NotifyImeChangeToClients(info->prop, info->subProp);
    GetValidSubtype("", info);
    session->SwitchSubtype(info->subProp);
    return ErrorCode::NO_ERROR;
}

// Inform current InputMethodExtension to switch subtype
int32_t InputMethodSystemAbility::SwitchSubType(int32_t userId, const std::shared_ptr<ImeInfo> &info)
{
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto ret = session->SwitchSubtype(info->subProp);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to inform ime to switch subtype, ret: %{public}d!", ret);
        return ret;
    }
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId)->imeId;
    ImeCfgManager::GetInstance().ModifyImeCfg({ userId, currentIme, info->subProp.id, false });
    session->NotifyImeChangeToClients(info->prop, info->subProp);
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::SwitchInputType(int32_t userId, const SwitchInfo &switchInfo)
{
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND;
    }
    auto targetIme = session->GetImeNativeCfg(userId, switchInfo.bundleName, switchInfo.subName);
    if (targetIme == nullptr) {
        IMSA_HILOGE("targetIme is nullptr!");
        return ErrorCode::ERROR_IMSA_GET_IME_INFO_FAILED;
    }
    auto ret = session->StartIme(targetIme);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("start input method failed!");
        return ret;
    }
    ret = session->SwitchSubtype({ .name = switchInfo.bundleName, .id = switchInfo.subName });
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("switch subtype failed!");
        return ret;
    }
    InputTypeManager::GetInstance().Set(true, { switchInfo.bundleName, switchInfo.subName });
    session->SetInputType();
    return ErrorCode::NO_ERROR;
}

// Deprecated because of no permission check, kept for compatibility
int32_t InputMethodSystemAbility::HideCurrentInputDeprecated()
{
    std::shared_ptr<PerUserSession> session = nullptr;
    auto result = PrepareForOperateKeyboard(session);
    if (result != ErrorCode::NO_ERROR) {
        return result;
    }
    return session->OnHideCurrentInput(GetCallingDisplayId());
}

int32_t InputMethodSystemAbility::ShowCurrentInputDeprecated()
{
    std::shared_ptr<PerUserSession> session = nullptr;
    auto result = PrepareForOperateKeyboard(session);
    if (result != ErrorCode::NO_ERROR) {
        return result;
    }
    return session->OnShowCurrentInput(GetCallingDisplayId());
}

std::shared_ptr<Property> InputMethodSystemAbility::GetCurrentInputMethod()
{
    return ImeInfoInquirer::GetInstance().GetCurrentInputMethod(GetCallingUserId());
}

bool InputMethodSystemAbility::IsDefaultImeSet()
{
    return ImeInfoInquirer::GetInstance().IsDefaultImeSet(GetCallingUserId());
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
    return ImeInfoInquirer::GetInstance().ListInputMethod(GetCallingUserId(), status, props, enableImeOn_.load());
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
                HandleBundleScanFinished();
                break;
            }
            case MSG_ID_DATA_SHARE_READY: {
                HandleDataShareReady();
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
            case MSG_ID_SCREEN_UNLOCK: {
                OnScreenUnlock(msg);
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
        IMSA_HILOGE("msgContent_ is nullptr!");
        return ErrorCode::ERROR_NULL_POINTER;
    }
    auto newUserId = msg->msgContent_->ReadInt32();
    FullImeInfoManager::GetInstance().Add(newUserId);
    // if scb enable, deal when receive wmsConnected.
    if (isScbEnable_.load()) {
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
        IMSA_HILOGE("Aborted! Message is nullptr!");
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
    auto session = GetSessionFromMsg(msg);
    if (session == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    session->StopCurrentIme();
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::OnHideKeyboardSelf(const Message *msg)
{
    auto session = GetSessionFromMsg(msg);
    if (session == nullptr) {
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
        IMSA_HILOGE("Failed to read message parcel!");
        return ErrorCode::ERROR_EX_PARCELABLE;
    }
    if (msg->msgId_ == MSG_ID_PACKAGE_CHANGED) {
        return FullImeInfoManager::GetInstance().Update(userId, packageName);
    }
    if (msg->msgId_ == MSG_ID_PACKAGE_ADDED) {
        auto instance = EnableImeDataParser::GetInstance();
        if (instance != nullptr) {
            instance->OnPackageAdded(userId, packageName);
        }
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

void InputMethodSystemAbility::OnScreenUnlock(const Message *msg)
{
    if (msg == nullptr || msg->msgContent_ == nullptr) {
        IMSA_HILOGE("message is nullptr");
        return;
    }
    int32_t userId = 0;
    if (!ITypesUtil::Unmarshal(*msg->msgContent_, userId)) {
        IMSA_HILOGE("failed to read message");
        return;
    }
    IMSA_HILOGI("userId: %{public}d", userId);
    if (userId != userId_) {
        return;
    }
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        UserSessionManager::GetInstance().AddUserSession(userId_);
    }
    session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId_);
        return;
    }
    session->OnScreenUnlock();
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
        IMSA_HILOGE("%{public}d session is nullptr!", userId_);
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
    bool isScreenLocked = false;
#ifdef IMF_SCREENLOCK_MGR_ENABLE
    if (ScreenLock::ScreenLockManager::GetInstance()->IsScreenLocked()) {
        IMSA_HILOGI("isScreenLocked  now.");
        isScreenLocked = true;
    }
#endif
    if (CombinationKey::IsMatch(CombinationKeyFunction::SWITCH_IME, state) && !isScreenLocked) {
        IMSA_HILOGI("switch ime.");
        DealSwitchRequest();
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGE("keycode is undefined!");
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
    return SwitchByCondition(condition, info);
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
    return SwitchByCondition(condition, info);
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
        ImeInfoInquirer::GetInstance().GetSwitchInfoBySwitchCount(switchInfo, userId_, enableImeOn_.load(), cacheCount);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("get next SwitchInfo failed, stop switching ime.");
        return ret;
    }
    IMSA_HILOGD("switch to: %{public}s.", switchInfo.bundleName.c_str());
    switchInfo.timestamp = std::chrono::system_clock::now();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId_);
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
}

void InputMethodSystemAbility::HandleDataShareReady()
{
    if (ImeInfoInquirer::GetInstance().IsEnableInputMethod()) {
        IMSA_HILOGW("Enter enable mode.");
        RegisterEnableImeObserver();
        EnableImeDataParser::GetInstance()->Initialize(userId_);
        enableImeOn_.store(true);
    }
    if (ImeInfoInquirer::GetInstance().IsEnableSecurityMode()) {
        IMSA_HILOGW("Enter security mode.");
        RegisterSecurityModeObserver();
        SecurityModeParser::GetInstance()->Initialize(userId_);
        enableSecurityMode_.store(true);
    }
    FullImeInfoManager::GetInstance().Init();
}

int32_t InputMethodSystemAbility::InitAccountMonitor()
{
    IMSA_HILOGI("InputMethodSystemAbility::InitAccountMonitor start.");
    return ImCommonEventManager::GetInstance()->SubscribeAccountManagerService([this]() { HandleOsAccountStarted(); });
}

int32_t InputMethodSystemAbility::InitKeyEventMonitor()
{
    IMSA_HILOGI("InputMethodSystemAbility::InitKeyEventMonitor start.");
    auto handler = [this]() {
        auto switchTrigger = [this](uint32_t keyCode) {
            return SwitchByCombinationKey(keyCode);
        };
        int32_t ret = KeyboardEvent::GetInstance().AddKeyEventMonitor(switchTrigger);
        IMSA_HILOGI(
            "SubscribeKeyboardEvent add monitor: %{public}s.", ret == ErrorCode::NO_ERROR ? "success" : "failed");
        // Check device capslock status and ime cfg corrent, when device power-up.
        HandleImeCfgCapsState();
    };
    bool ret = ImCommonEventManager::GetInstance()->SubscribeKeyboardEvent(handler);
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
        [this](bool isOnFocused, uint64_t displayId, int32_t pid, int32_t uid) {
            HandleFocusChanged(isOnFocused, displayId, pid, uid);
        });
}
void InputMethodSystemAbility::InitWindowDisplayChangedMonitor()
{
    IMSA_HILOGD("enter.");
    auto callBack = [this](OHOS::Rosen::CallingWindowInfo callingWindowInfo) {
        IMSA_HILOGD("WindowDisplayChanged callbak.");
        int32_t userId = callingWindowInfo.userId_;
        auto session = UserSessionManager::GetInstance().GetUserSession(userId);
        if (session == nullptr) {
            IMSA_HILOGE("[%{public}d] session is nullptr!", userId);
            return;
        };
        session->OnCallingDisplayIdChanged(
            callingWindowInfo.windowId_, callingWindowInfo.callingPid_, callingWindowInfo.displayId_);
    };
    WindowAdapter::GetInstance().RegisterCallingWindowInfoChangedListener(callBack);
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
                IMSA_HILOGE("%{public}d session is nullptr!", userId_);
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
        IMSA_HILOGE("%{public}d session is nullptr!", userId_);
        return;
    }
    session->OnSecurityChange(static_cast<int32_t>(newMode));
    session->AddRestartIme();
}

int32_t InputMethodSystemAbility::GetSecurityMode(int32_t &security)
{
    IMSA_HILOGD("InputMethodSystemAbility start.");
    if (!enableSecurityMode_.load()) {
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
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    if (type == UnRegisteredType::SWITCH_PROXY_IME_TO_IME) {
        int32_t ret = ErrorCode::NO_ERROR;
        if (session->CheckSecurityMode()) {
            ret = StartInputType(userId, InputType::SECURITY_INPUT);
        } else {
            ret = session->RestoreCurrentIme();
        }
        if (ret != ErrorCode::NO_ERROR) {
            return ret;
        }
    }
    return session->OnUnRegisteredProxyIme(type, core);
}

int32_t InputMethodSystemAbility::CheckEnableAndSwitchPermission()
{
    if (!identityChecker_->IsNativeSa(IPCSkeleton::GetCallingFullTokenID())) {
        IMSA_HILOGE("not native sa!");
        return ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION;
    }
    if (!identityChecker_->HasPermission(IPCSkeleton::GetCallingTokenID(),
        std::string(PERMISSION_CONNECT_IME_ABILITY))) {
        IMSA_HILOGE("have not PERMISSION_CONNECT_IME_ABILITY!");
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::CheckSwitchPermission(int32_t userId, const SwitchInfo &switchInfo,
    SwitchTrigger trigger)
{
    IMSA_HILOGD("trigger: %{public}d.", static_cast<int32_t>(trigger));
    if (trigger == SwitchTrigger::IMSA) {
        return ErrorCode::NO_ERROR;
    }
    if (trigger == SwitchTrigger::NATIVE_SA) {
        return CheckEnableAndSwitchPermission();
    }
    if (trigger == SwitchTrigger::SYSTEM_APP) {
        if (!identityChecker_->IsSystemApp(IPCSkeleton::GetCallingFullTokenID())) {
            IMSA_HILOGE("not system app!");
            return ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION;
        }
        if (!identityChecker_->HasPermission(IPCSkeleton::GetCallingTokenID(),
            std::string(PERMISSION_CONNECT_IME_ABILITY))) {
            IMSA_HILOGE("have not PERMISSION_CONNECT_IME_ABILITY!");
            return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
        }
        return ErrorCode::NO_ERROR;
    }
    if (trigger == SwitchTrigger::CURRENT_IME) {
        // PERMISSION_CONNECT_IME_ABILITY check temporarily reserved for application adaptation, will be deleted soon
        if (identityChecker_->HasPermission(IPCSkeleton::GetCallingTokenID(),
            std::string(PERMISSION_CONNECT_IME_ABILITY))) {
            return ErrorCode::NO_ERROR;
        }
        IMSA_HILOGE("have not PERMISSION_CONNECT_IME_ABILITY!");
        // switchInfo.subName.empty() check temporarily reserved for application adaptation, will be deleted soon
        auto currentBundleName = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId)->bundleName;
        if (identityChecker_->IsBundleNameValid(IPCSkeleton::GetCallingTokenID(), currentBundleName)) {
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
    if (identityChecker_->HasPermission(tokenId, std::string(PERMISSION_CONNECT_IME_ABILITY))) {
        return true;
    }
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return false;
    }
    return identityChecker_->IsFocused(IPCSkeleton::GetCallingPid(), tokenId)
           && session->IsBoundToClient(GetCallingDisplayId());
}

int32_t InputMethodSystemAbility::ConnectSystemCmd(const sptr<IRemoteObject> &channel, sptr<IRemoteObject> &agent)
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!identityChecker_->HasPermission(tokenId, std::string(PERMISSION_CONNECT_IME_ABILITY))) {
        IMSA_HILOGE("have not PERMISSION_CONNECT_IME_ABILITY!");
        return ErrorCode::ERROR_STATUS_SYSTEM_PERMISSION;
    }
    auto userId = GetCallingUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
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
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return;
    }
#ifndef IMF_ON_DEMAND_START_STOP_SA_ENABLE
    session->AddRestartIme();
#endif
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
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return;
    }
    auto imeData = session->GetReadyImeData(ImeType::IME);
    if (imeData == nullptr && session->IsWmsReady()) {
        session->StartCurrentIme();
    }
}

void InputMethodSystemAbility::HandleWmsDisconnected(int32_t userId, int32_t screenId)
{
    // clear client
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session != nullptr) {
        session->RemoveAllCurrentClient();
    }

#ifndef IMF_ON_DEMAND_START_STOP_SA_ENABLE
    if (userId == userId_) {
        // user switched or scb in foreground died, not deal
        return;
    }
#endif
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
    if (isScbEnable_.load()) {
        IMSA_HILOGI("scb enable, register WMS connection listener.");
        InitWmsConnectionMonitor();
        InitWindowDisplayChangedMonitor();
        return;
    }
    // clear client
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session != nullptr) {
        session->RemoveAllCurrentClient();
    }
    RestartSessionIme(session);
}

void InputMethodSystemAbility::HandleFocusChanged(bool isFocused, uint64_t displayId, int32_t pid, int32_t uid)
{
    int32_t userId = GetUserId(uid);
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("[%{public}d, %{public}d] session is nullptr!", uid, userId);
        return;
    }
    isFocused ? session->OnFocused(displayId, pid, uid) : session->OnUnfocused(displayId, pid, uid);
}

void InputMethodSystemAbility::HandleMemStarted()
{
    // singleton
    IMSA_HILOGI("MemMgr start.");
    Memory::MemMgrClient::GetInstance().NotifyProcessStatus(getpid(), 1, 1, INPUT_METHOD_SYSTEM_ABILITY_ID);
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    RestartSessionIme(session);
}

void InputMethodSystemAbility::HandleOsAccountStarted()
{
    IMSA_HILOGI("account start");
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

uint64_t InputMethodSystemAbility::GetCallingDisplayId()
{
    return identityChecker_->GetDisplayIdByPid(IPCSkeleton::GetCallingPid());
}

bool InputMethodSystemAbility::IsCurrentIme(int32_t userId)
{
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return false;
    }
    auto bundleName = FullImeInfoManager::GetInstance().Get(userId, IPCSkeleton::GetCallingTokenID());
    if (bundleName.empty()) {
        IMSA_HILOGW("user:%{public}d tokenId:%{public}d not find.", userId, IPCSkeleton::GetCallingTokenID());
        bundleName = identityChecker_->GetBundleNameByToken(IPCSkeleton::GetCallingTokenID());
    }
    auto imeData = session->GetImeData(ImeType::IME);
    return imeData != nullptr && bundleName == imeData->ime.first;
}

int32_t InputMethodSystemAbility::StartInputType(int32_t userId, InputType type)
{
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("%{public}d session is nullptr!", userId);
        return ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND;
    }
    ImeIdentification ime;
    int32_t ret = InputTypeManager::GetInstance().GetImeByInputType(type, ime);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGW("not find input type: %{public}d.", type);
        // add for not adapter for SECURITY_INPUT
        if (type == InputType::SECURITY_INPUT) {
            return session->RestoreCurrentIme();
        }
        return ret;
    }
    SwitchInfo switchInfo = { std::chrono::system_clock::now(), ime.bundleName, ime.subName };
    session->GetSwitchQueue().Push(switchInfo);
    IMSA_HILOGI("start input type: %{public}d.", type);
    return type == InputType::SECURITY_INPUT ? OnStartInputType(userId, switchInfo, false)
                                             : OnStartInputType(userId, switchInfo, true);
}

void InputMethodSystemAbility::NeedHideWhenSwitchInputType(int32_t userId, bool &needHide)
{
    ImeIdentification ime;
    InputTypeManager::GetInstance().GetImeByInputType(InputType::SECURITY_INPUT, ime);
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    if (currentImeCfg == nullptr) {
        IMSA_HILOGI("currentImeCfg is nullptr");
        return;
    }
    needHide = currentImeCfg->bundleName == ime.bundleName;
}

void InputMethodSystemAbility::HandleBundleScanFinished()
{
    isBundleScanFinished_.store(true);
    HandleImeCfgCapsState();
}

bool InputMethodSystemAbility::ModifyImeCfgWithWrongCaps()
{
    bool isCapsEnable = false;
    if (!GetDeviceFunctionKeyState(MMI::KeyEvent::CAPS_LOCK_FUNCTION_KEY, isCapsEnable)) {
        IMSA_HILOGE("Get capslock function key state failed!");
        return false;
    }
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    if (currentImeCfg == nullptr) {
        IMSA_HILOGE("currentImeCfg is nullptr!");
        return false;
    }
    auto info = ImeInfoInquirer::GetInstance().GetImeInfo(userId_, currentImeCfg->bundleName, currentImeCfg->subName);
    if (info == nullptr) {
        IMSA_HILOGE("ime info is nullptr!");
        return false;
    }
    bool imeCfgCapsEnable = info->subProp.mode == "upper";
    if (imeCfgCapsEnable == isCapsEnable) {
        IMSA_HILOGE("current caps state is correct.");
        return true;
    }
    auto condition = isCapsEnable ? Condition::UPPER : Condition::LOWER;
    auto correctIme = ImeInfoInquirer::GetInstance().FindTargetSubtypeByCondition(info->subProps, condition);
    if (correctIme == nullptr) {
        IMSA_HILOGE("correctIme is empty!");
        return false;
    }
    std::string correctImeName = info->prop.name + "/" + info->prop.id;
    ImeCfgManager::GetInstance().ModifyImeCfg({ userId_, correctImeName, correctIme->id, false });
    IMSA_HILOGD("Adjust imeCfg caps success! current imeName: %{public}s, subName: %{public}s",
        correctImeName.c_str(), correctIme->id.c_str());
    return true;
}

bool InputMethodSystemAbility::GetDeviceFunctionKeyState(int32_t functionKey, bool &isEnable)
{
    auto multiInputMgr = MMI::InputManager::GetInstance();
    if (multiInputMgr == nullptr) {
        IMSA_HILOGE("multiInputMgr is nullptr");
        return false;
    }
    int32_t ret = multiInputMgr->GetFunctionKeyState(functionKey, isEnable);
    IMSA_HILOGD("The function key: %{public}d, isEnable: %{public}d", functionKey, isEnable);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("multiInputMgr get function key state error: %{public}d", ret);
        return false;
    }
    return true;
}

void InputMethodSystemAbility::HandleImeCfgCapsState()
{
    if (!isBundleScanFinished_.load()) {
        IMSA_HILOGE("Bundle scan is not ready.");
        return;
    }
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("UserId: %{public}d session is nullptr!", userId_);
        return;
    }
    if (!session->IsSaReady(MULTIMODAL_INPUT_SERVICE_ID)) {
        IMSA_HILOGE("MMI service is not ready.");
        return;
    }
    if (!ModifyImeCfgWithWrongCaps()) {
        IMSA_HILOGE("Check ImeCfg capslock state correct failed!");
    }
}

int32_t InputMethodSystemAbility::GetInputMethodState(EnabledStatus &status)
{
    auto userId = GetCallingUserId();
    auto bundleName = FullImeInfoManager::GetInstance().Get(userId, IPCSkeleton::GetCallingTokenID());
    if (bundleName.empty()) {
        bundleName = identityChecker_->GetBundleNameByToken(IPCSkeleton::GetCallingTokenID());
        if (!ImeInfoInquirer::GetInstance().IsInputMethod(userId, bundleName)) {
            IMSA_HILOGE("[%{public}d, %{public}s] not an ime.", userId, bundleName.c_str());
            return ErrorCode::ERROR_NOT_IME;
        }
    }

    auto ret = GetInputMethodState(userId, bundleName, status);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::GetInputMethodState(
    int32_t userId, const std::string &bundleName, EnabledStatus &status)
{
    auto isDefaultBasicMode = !ImeInfoInquirer::GetInstance().IsEnableInputMethod();
    auto isDefaultFullExperience = !ImeInfoInquirer::GetInstance().IsEnableSecurityMode();
    IMSA_HILOGI("sys cfg:[%{public}d, %{public}d].", isDefaultBasicMode, isDefaultFullExperience);
    auto isSecurityMode = SecurityModeParser::GetInstance()->IsSecurityMode(userId, bundleName);
    if (isDefaultBasicMode) {
        if (isDefaultFullExperience) {
            status = EnabledStatus::FULL_EXPERIENCE_MODE;
            return ErrorCode::NO_ERROR;
        }
        status = isSecurityMode ? EnabledStatus::FULL_EXPERIENCE_MODE : EnabledStatus::BASIC_MODE;
        return ErrorCode::NO_ERROR;
    }

    if (isDefaultFullExperience) {
        auto ret = EnableImeDataParser::GetInstance()->GetImeEnablePattern(userId, bundleName, status);
        if (ret != ErrorCode::NO_ERROR) {
            return ret;
        }
        if (status == EnabledStatus::BASIC_MODE) {
            status = EnabledStatus::FULL_EXPERIENCE_MODE;
        }
        return ErrorCode::NO_ERROR;
    }

    if (isSecurityMode) {
        status = EnabledStatus::FULL_EXPERIENCE_MODE;
        return ErrorCode::NO_ERROR;
    }

    auto ret = EnableImeDataParser::GetInstance()->GetImeEnablePattern(userId, bundleName, status);
    return ret;
}

int32_t InputMethodSystemAbility::ShowCurrentInput(ClientType type)
{
    auto name = ImfHiSysEventUtil::GetAppName(IPCSkeleton::GetCallingTokenID());
    auto pid = IPCSkeleton::GetCallingPid();
    auto userId = GetCallingUserId();
    auto imeInfo = GetCurrentImeInfoForHiSysEvent(userId);
    auto ret = ShowCurrentInputInner();
    auto evenInfo = HiSysOriginalInfo::Builder()
                        .SetPeerName(name)
                        .SetPeerPid(pid)
                        .SetPeerUserId(userId)
                        .SetClientType(type)
                        .SetImeName(imeInfo.second)
                        .SetEventCode(static_cast<int32_t>(InputMethodInterfaceCode::SHOW_CURRENT_INPUT))
                        .SetErrCode(ret)
                        .Build();
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_SHOW, *evenInfo);
    return ret;
}

int32_t InputMethodSystemAbility::ShowInput(sptr<IInputClient> client, ClientType type, int32_t requestKeyboardReason)
{
    auto name = ImfHiSysEventUtil::GetAppName(IPCSkeleton::GetCallingTokenID());
    auto pid = IPCSkeleton::GetCallingPid();
    auto userId = GetCallingUserId();
    auto imeInfo = GetCurrentImeInfoForHiSysEvent(userId);
    auto ret = ShowInputInner(client, requestKeyboardReason);
    auto evenInfo = HiSysOriginalInfo::Builder()
                        .SetPeerName(name)
                        .SetPeerPid(pid)
                        .SetPeerUserId(userId)
                        .SetClientType(type)
                        .SetImeName(imeInfo.second)
                        .SetEventCode(static_cast<int32_t>(InputMethodInterfaceCode::SHOW_INPUT))
                        .SetErrCode(ret)
                        .Build();
    ImsaHiSysEventReporter::GetInstance().ReportEvent(ImfEventType::CLIENT_SHOW, *evenInfo);
    return ret;
}

std::pair<int64_t, std::string> InputMethodSystemAbility::GetCurrentImeInfoForHiSysEvent(int32_t userId)
{
    std::pair<int64_t, std::string> imeInfo{ 0, "" };
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
        imeInfo.second = currentImeCfg != nullptr ? currentImeCfg->bundleName : "";
        return imeInfo;
    }
    auto imeType = session->IsProxyImeEnable() ? ImeType::PROXY_IME : ImeType::IME;
    auto imeData = session->GetImeData(imeType);
    if (imeData != nullptr) {
        imeInfo.first = imeData->pid;
        imeInfo.second = imeData->ime.first;
    }
    return imeInfo;
}

int32_t InputMethodSystemAbility::GetScreenLockIme(std::string &ime)
{
    auto defaultIme = ImeInfoInquirer::GetInstance().GetDefaultImeCfg();
    if (defaultIme != nullptr) {
        ime = defaultIme->imeId;
        IMSA_HILOGD("GetDefaultIme screenlocked");
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGE("GetDefaultIme is failed!");
    auto currentIme = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId_);
    if (currentIme != nullptr) {
        ime = currentIme->imeId;
        IMSA_HILOGD("GetCurrentIme screenlocked");
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGE("GetCurrentIme is failed!");
    if (GetAlternativeIme(ime) != ErrorCode::NO_ERROR) {
        return ErrorCode::ERROR_NOT_IME;
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputMethodSystemAbility::GetAlternativeIme(std::string &ime)
{
    InputMethodStatus status = InputMethodStatus::ENABLE;
    std::vector<Property> props;
    int32_t ret = ListInputMethod(status, props);
    if (ret == ErrorCode::NO_ERROR && !props.empty()) {
        ime = props[0].name + "/" + props[0].id;
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGD("GetListEnableInputMethodIme is failed!");
    status = InputMethodStatus::DISABLE;
    ret = ListInputMethod(status, props);
    if (ret != ErrorCode::NO_ERROR || props.empty()) {
        IMSA_HILOGE("GetListDisableInputMethodIme is failed!");
        return ErrorCode::ERROR_NOT_IME;
    }
    if (EnableIme(props[0].name)) {
        ime = props[0].name + "/" + props[0].id;
        return ErrorCode::NO_ERROR;
    }
    IMSA_HILOGE("GetAlternativeIme is failed!");
    return ErrorCode::ERROR_NOT_IME;
}

int32_t InputMethodSystemAbility::SendPrivateData(
    const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
{
    if (privateCommand.empty()) {
        IMSA_HILOGE("privateCommand is empty!");
        return ErrorCode::ERROR_PRIVATE_COMMAND_IS_EMPTY;
    }
    if (!identityChecker_->IsSpecialSaUid()) {
        IMSA_HILOGE("uid failed, not permission!");
        return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
    }
    auto session = UserSessionManager::GetInstance().GetUserSession(userId_);
    if (session == nullptr) {
        IMSA_HILOGE("UserId: %{public}d session is nullptr!", userId_);
        return ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND;
    }
    if (!session->SpecialScenarioCheck()) {
        IMSA_HILOGE("Special check permission failed!");
        return ErrorCode::ERROR_SCENE_UNSUPPORTED;
    }
    auto ret = session->SpecialSendPrivateData(privateCommand);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("Special send private data failed, ret: %{public}d!", ret);
    }
    return ret;
}
} // namespace MiscServices
} // namespace OHOS