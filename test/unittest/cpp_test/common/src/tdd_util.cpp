/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#define private public
#define protected public
#include "full_ime_info_manager.h"
#include "settings_data_utils.h"
#include "user_session_manager.h"
#undef private

#include <unistd.h>

#include <csignal>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "accesstoken_kit.h"
#include "datashare_helper.h"
#include "global.h"
#include "if_system_ability_manager.h"
#include "input_method_controller.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "os_account_manager.h"
#include "scope_utils.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "tdd_util.h"
#include "token_setproc.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::Security::AccessToken;
using namespace OHOS::AccountSA;
using namespace Rosen;
constexpr int32_t INVALID_USER_ID = -1;
constexpr int32_t MAIN_USER_ID = 100;
constexpr const uint16_t EACH_LINE_LENGTH = 500;
constexpr int32_t PERMISSION_NUM = 6;
constexpr int32_t FIRST_PARAM_INDEX = 0;
constexpr int32_t SECOND_PARAM_INDEX = 1;
constexpr int32_t THIRD_PARAM_INDEX = 2;
constexpr int32_t FOURTH_PARAM_INDEX = 3;
constexpr int32_t FIFTH_PARAM_INDEX = 4;
constexpr int32_t SIXTH_PARAM_INDEX = 5;
static constexpr int32_t MAX_TIMEOUT_WAIT_FOCUS = 2000;
uint64_t TddUtil::selfTokenID_ = 0;
int32_t TddUtil::userID_ = INVALID_USER_ID;
std::string TddUtil::currentBundleNameMock_;
sptr<Window> TddUtil::WindowManager::window_ = nullptr;
int32_t TddUtil::WindowManager::currentWindowId_ = 0;
uint64_t TddUtil::WindowManager::windowTokenId_ = 0;
std::shared_ptr<BlockData<bool>> FocusChangedListenerTestImpl::isFocused_ =
    std::make_shared<BlockData<bool>>(MAX_TIMEOUT_WAIT_FOCUS, false);
std::shared_ptr<BlockData<bool>> FocusChangedListenerTestImpl::unFocused_ =
    std::make_shared<BlockData<bool>>(MAX_TIMEOUT_WAIT_FOCUS, false);
static constexpr const char *ENABLE_IME = "settings.inputmethod.enable_ime";
void FocusChangedListenerTestImpl::OnFocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo)
{
    IMSA_HILOGI("get onFocus information from window manager.");
    if (focusChangeInfo->windowId_ == TddUtil::WindowManager::currentWindowId_) {
        getFocus_ = true;
        isFocused_->SetValue(getFocus_);
        unFocused_->Clear(false);
    }
}

void FocusChangedListenerTestImpl::OnUnfocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo)
{
    IMSA_HILOGI("get unfocus information from window manager.");
    if (focusChangeInfo->windowId_ == TddUtil::WindowManager::currentWindowId_) {
        getFocus_ = false;
        isFocused_->Clear(false);
        bool unFocus = !getFocus_;
        unFocused_->SetValue(unFocus);
    }
}

int32_t TddUtil::GetCurrentUserId()
{
    if (userID_ != INVALID_USER_ID) {
        return userID_;
    }
    int32_t userId = -1;
    auto ret = OsAccountManager::GetForegroundOsAccountLocalId(userId);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("GetForegroundOsAccountLocalId failed");
        return MAIN_USER_ID;
    }
    return userId;
}
void TddUtil::StorageSelfTokenID()
{
    selfTokenID_ = GetSelfTokenID();
}

uint64_t TddUtil::AllocTestTokenID(
    bool isSystemApp, const std::string &bundleName, const std::vector<std::string> &premission)
{
    IMSA_HILOGI("bundleName: %{public}s", bundleName.c_str());
    HapInfoParams infoParams = { .userID = GetCurrentUserId(),
        .bundleName = bundleName,
        .instIndex = 0,
        .appIDDesc = bundleName,
        .isSystemApp = isSystemApp };
    std::vector<PermissionStateFull> permStateList;
    for (const auto &prem : premission) {
        PermissionStateFull permissionState = { .permissionName = prem,
            .isGeneral = true,
            .resDeviceID = { "local" },
            .grantStatus = { PermissionState::PERMISSION_GRANTED },
            .grantFlags = { 1 } };
        permStateList.push_back(permissionState);
    }
    HapPolicyParams policyParams = {
        .apl = APL_NORMAL, .domain = bundleName, .permList = {}, .permStateList = permStateList
    };
    if (premission.empty()) {
        policyParams = { .apl = APL_NORMAL, .domain = bundleName, .permList = {}, .permStateList = {} };
    }
    auto tokenInfo = AccessTokenKit::AllocHapToken(infoParams, policyParams);
    return tokenInfo.tokenIDEx;
}

uint64_t TddUtil::GetTestTokenID(const std::string &bundleName)
{
    HapInfoParams infoParams = { .userID = GetUserIdByBundleName(bundleName, GetCurrentUserId()),
        .bundleName = bundleName,
        .instIndex = 0,
        .appIDDesc = "ohos.inputmethod_test.demo" };
    return AccessTokenKit::GetHapTokenID(infoParams.userID, infoParams.bundleName, infoParams.instIndex);
}

void TddUtil::DeleteTestTokenID(uint64_t tokenId)
{
    AccessTokenKit::DeleteToken(tokenId);
}

void TddUtil::SetTestTokenID(uint64_t tokenId)
{
    auto ret = SetSelfTokenID(tokenId);
    IMSA_HILOGI("SetSelfTokenID ret: %{public}d", ret);
}

void TddUtil::RestoreSelfTokenID()
{
    auto ret = SetSelfTokenID(selfTokenID_);
    IMSA_HILOGI("SetSelfTokenID ret = %{public}d", ret);
}

uint64_t TddUtil::GetCurrentTokenID()
{
    return GetSelfTokenID();
}

int32_t TddUtil::GetUid(const std::string &bundleName)
{
    auto bundleMgr = GetBundleMgr();
    if (bundleMgr == nullptr) {
        IMSA_HILOGE("bundleMgr nullptr");
        return -1;
    }
    auto uid = bundleMgr->GetUidByBundleName(bundleName, GetCurrentUserId());
    if (uid == -1) {
        IMSA_HILOGE("failed to get information and the parameters may be wrong.");
        return -1;
    }
    IMSA_HILOGI("bundleName: %{public}s, uid: %{public}d", bundleName.c_str(), uid);
    return uid;
}

void TddUtil::SetSelfUid(int32_t uid)
{
    setuid(uid);
    IMSA_HILOGI("set uid to: %{public}d", uid);
}

bool TddUtil::ExecuteCmd(const std::string &cmd, std::string &result)
{
    char buff[EACH_LINE_LENGTH] = { 0x00 };
    std::stringstream output;
    FILE *ptr = popen(cmd.c_str(), "r");
    if (ptr != nullptr) {
        IMSA_HILOGI("Execute cmd: %{public}s", cmd.c_str());
        while (fgets(buff, sizeof(buff), ptr) != nullptr) {
            output << buff;
        }
        pclose(ptr);
        ptr = nullptr;
    } else {
        return false;
    }
    result = output.str();
    return true;
}

pid_t TddUtil::GetImsaPid()
{
    auto currentToken = GetSelfTokenID();
    GrantNativePermission();
    auto saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemProcessInfo info;
    int32_t ret = saMgr->GetSystemProcessInfo(INPUT_METHOD_SYSTEM_ABILITY_ID, info);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to get sa info, ret: %{public}d", ret);
        return -1;
    }
    SetSelfTokenID(currentToken);
    return info.pid;
}

bool TddUtil::KillImsaProcess()
{
    pid_t pid = GetImsaPid();
    if (pid == -1) {
        IMSA_HILOGE("failed to get pid");
        return false;
    }
    auto ret = kill(pid, SIGTERM);
    if (ret != 0) {
        IMSA_HILOGE("Kill failed, ret: %{public}d", ret);
        return false;
    }
    IMSA_HILOGI("Kill [%{public}d] success", pid);
    return true;
}

sptr<OHOS::AppExecFwk::IBundleMgr> TddUtil::GetBundleMgr()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGE("systemAbilityManager is nullptr");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        IMSA_HILOGE("remoteObject is nullptr");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
}

int TddUtil::GetUserIdByBundleName(const std::string &bundleName, const int currentUserId)
{
    auto bundleMgr = TddUtil::GetBundleMgr();
    if (bundleMgr == nullptr) {
        IMSA_HILOGE("Get bundleMgr failed.");
        return -1;
    }
    auto uid = bundleMgr->GetUidByBundleName(bundleName, currentUserId);
    if (uid == -1) {
        IMSA_HILOGE("failed to get information and the parameters may be wrong.");
        return -1;
    }
    // 200000 means userId = uid / 200000.
    return uid / 200000;
}

void TddUtil::GrantNativePermission()
{
    const char **perms = new const char *[PERMISSION_NUM];
    perms[FIRST_PARAM_INDEX] = "ohos.permission.MANAGE_SECURE_SETTINGS";
    perms[SECOND_PARAM_INDEX] = "ohos.permission.CONNECT_IME_ABILITY";
    perms[THIRD_PARAM_INDEX] = "ohos.permission.MANAGE_SETTINGS";
    perms[FOURTH_PARAM_INDEX] = "ohos.permission.INJECT_INPUT_EVENT";
    perms[FIFTH_PARAM_INDEX] = "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED";
    perms[SIXTH_PARAM_INDEX] = "ohos.permission.GET_RUNNING_INFO";
    TokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = PERMISSION_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "imf_test",
        .aplStr = "system_core",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    int res = SetSelfTokenID(tokenId);
    if (res == 0) {
        IMSA_HILOGI("SetSelfTokenID success!");
    } else {
        IMSA_HILOGE("SetSelfTokenID fail!");
    }
    AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

void TddUtil::PushEnableImeValue(const std::string &key, const std::string &value)
{
    IMSA_HILOGI("key: %{public}s, value: %{public}s", key.c_str(), value.c_str());
    auto helper = SettingsDataUtils::GetInstance().CreateDataShareHelper(SETTING_URI_PROXY);
    if (helper == nullptr) {
        IMSA_HILOGE("helper is nullptr.");
        return;
    }
    DataShare::DataShareValueObject keyObj(key);
    DataShare::DataShareValueObject valueObj(value);
    DataShare::DataShareValuesBucket bucket;
    bucket.Put(SETTING_COLUMN_KEYWORD, keyObj);
    bucket.Put(SETTING_COLUMN_VALUE, valueObj);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(SettingsDataUtils::GetInstance().GenerateTargetUri(SETTING_URI_PROXY, key));
    if (helper->Update(uri, predicates, bucket) <= 0) {
        int index = helper->Insert(uri, bucket);
        IMSA_HILOGI("no data exists, insert ret index: %{public}d", index);
    } else {
        IMSA_HILOGI("data exits");
    }
    bool ret = SettingsDataUtils::GetInstance().ReleaseDataShareHelper(helper);
    IMSA_HILOGI("ReleaseDataShareHelper isSuccess: %{public}d", ret);
}

void TddUtil::DeleteGlobalTable(const std::string &key)
{
    IMSA_HILOGI("key: %{public}s", key.c_str());
    std::string uriProxy = SETTING_URI_PROXY;
    DeleteTable(key, uriProxy);
}

void TddUtil::DeleteUserTable(int32_t userId, const std::string &key)
{
    IMSA_HILOGI("userId:%{public}d, key:%{public}s", userId, key.c_str());
    std::string uriProxy = SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true";
    DeleteTable(key, uriProxy);
}

void TddUtil::DeleteTable(const std::string &key, const std::string &uriProxy)
{
    auto helper = SettingsDataUtils::GetInstance().CreateDataShareHelper(uriProxy);
    if (helper == nullptr) {
        IMSA_HILOGE("helper is nullptr.");
        return;
    }
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(SettingsDataUtils::GetInstance().GenerateTargetUri(uriProxy, key));
    auto result = helper->DeleteEx(uri, predicates);
    IMSA_HILOGI("Delete ret: [%{public}d, %{public}d]", result.first, result.second);
    bool ret = SettingsDataUtils::GetInstance().ReleaseDataShareHelper(helper);
    IMSA_HILOGI("ReleaseDataShareHelper isSuccess: %{public}d", ret);
}

void TddUtil::SetGlobalTable(const std::string &key, const std::string &content)
{
    std::string uriProxy = SETTING_URI_PROXY;
    SetTable(key, uriProxy, content);
}
void TddUtil::SetUserTable(int32_t userId, const std::string &key, const std::string &content)
{
    std::string uriProxy = SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true";
    SetTable(key, uriProxy, content);
}

void TddUtil::SetTable(const std::string &key, const std::string &uriProxy, const std::string &content)
{
    SettingsDataUtils::GetInstance().SetStringValue(uriProxy, key, content);
}

std::string TddUtil::GetTable(const std::string &key, const std::string &uriProxy)
{
    std::string content;
    auto ret = SettingsDataUtils::GetInstance().GetStringValue(uriProxy, key, content);
    IMSA_HILOGI("%{public}s/%{public}s ret:%{public}d.", key.c_str(), uriProxy.c_str(), ret);
    return content;
}

std::string TddUtil::GetGlobalTable(const std::string &key)
{
    std::string uriProxy = SETTING_URI_PROXY;
    return GetTable(key, uriProxy);
}

std::string TddUtil::GetUserTable(int32_t userId, const std::string &key)
{
    std::string uriProxy = SETTINGS_USER_DATA_URI + std::to_string(userId) + "?Proxy=true";
    return GetTable(key, uriProxy);
}

int32_t TddUtil::GetEnableData(std::string &value)
{
    auto ret = SettingsDataUtils::GetInstance().GetStringValue(SETTING_URI_PROXY, ENABLE_IME, value);
    if (ret == ErrorCode::NO_ERROR) {
        IMSA_HILOGI("success, value: %{public}s", value.c_str());
    }
    IMSA_HILOGI("GetStringValue ret: %{public}d", ret);
    return ret;
}

void TddUtil::InitWindow(bool isShow)
{
    WindowManager::RegisterFocusChangeListener();
    WindowManager::CreateWindow();
    if (!isShow) {
        return;
    }
    WindowManager::ShowWindow();
    bool isFocused = FocusChangedListenerTestImpl::isFocused_->GetValue();
    IMSA_HILOGI("getFocus end, isFocused = %{public}d", isFocused);
}

void TddUtil::DestroyWindow()
{
    WindowManager::HideWindow();
    WindowManager::DestroyWindow();
}

bool TddUtil::GetFocused()
{
    WindowManager::ShowWindow();
    bool isFocused = FocusChangedListenerTestImpl::isFocused_->GetValue();
    IMSA_HILOGI("getFocus end, isFocused = %{public}d", isFocused);
    return isFocused;
}

bool TddUtil::GetUnfocused()
{
    WindowManager::HideWindow();
    bool unFocused = FocusChangedListenerTestImpl::unFocused_->GetValue();
    IMSA_HILOGI("unFocused end, unFocused = %{public}d", unFocused);
    return unFocused;
}

void TddUtil::InitCurrentImePermissionInfo()
{
    auto userId = GetCurrentUserId();
    auto session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        UserSessionManager::GetInstance().AddUserSession(userId);
    }
    session = UserSessionManager::GetInstance().GetUserSession(userId);
    if (session == nullptr) {
        IMSA_HILOGE("session is nullptr.");
        return;
    }
    std::shared_ptr<Property> property = nullptr;
    InputMethodController::GetInstance()->GetDefaultInputMethod(property);
    if (property == nullptr) {
        IMSA_HILOGI("default ime is nullptr.");
        return;
    }
    currentBundleNameMock_ = property->name;
    session->InitImeData({ property->name, property->id });
    ImeCfgManager::GetInstance().imeConfigs_ = {
        { userId, property->name + "/" + property->id, "", false }
    };
}

void TddUtil::EnabledAllIme()
{
    auto imc = InputMethodController::GetInstance();
    if (imc == nullptr) {
        return;
    }
    std::vector<Property> props;
    imc->ListInputMethod(false, props);
    for (const auto &prop : props) {
        imc->EnableIme(prop.name, prop.id, EnabledStatus::BASIC_MODE);
    }
}

void TddUtil::DisabledAllIme()
{
    auto imc = InputMethodController::GetInstance();
    if (imc == nullptr) {
        return;
    }
    std::vector<Property> props;
    imc->ListInputMethod(true, props);
    for (const auto &prop : props) {
        imc->EnableIme(prop.name, prop.id, EnabledStatus::DISABLED);
    }
}

void TddUtil::WindowManager::CreateWindow()
{
    if (windowTokenId_ == 0) {
        windowTokenId_ = AllocTestTokenID(true, "TestWindow", { "ohos.permission.SYSTEM_FLOAT_WINDOW" });
    }
    TokenScope scope(windowTokenId_);
    std::string windowName = "inputmethod_test_window";
    sptr<WindowOption> winOption = new OHOS::Rosen::WindowOption();
    winOption->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    winOption->SetFocusable(true);
    std::shared_ptr<AbilityRuntime::Context> context = nullptr;
    WMError wmError = WMError::WM_OK;
    window_ = Window::Create(windowName, winOption, context, wmError);
    if (window_ == nullptr) {
        IMSA_HILOGE("failed to create window, ret: %{public}d", wmError);
        return;
    }
    IMSA_HILOGI("Create window ret:%{public}d", wmError);
    currentWindowId_ = window_->GetWindowId();
}

void TddUtil::WindowManager::ShowWindow()
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window is not exist.");
        return;
    }
    TokenScope scope(windowTokenId_);
    auto ret = window_->Show();
    IMSA_HILOGI("Show window end, ret = %{public}d", ret);
}

void TddUtil::WindowManager::HideWindow()
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window is not exist.");
        return;
    }
    TokenScope scope(windowTokenId_);
    auto ret = window_->Hide();
    IMSA_HILOGI("Hide window end, ret = %{public}d", ret);
}

void TddUtil::WindowManager::DestroyWindow()
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window_ nullptr");
        return;
    }
    TokenScope scope(windowTokenId_);
    auto wmError = window_->Destroy();
    IMSA_HILOGI("Destroy window ret: %{public}d", wmError);
}

void TddUtil::WindowManager::RegisterFocusChangeListener()
{
    auto listener = new (std::nothrow) FocusChangedListenerTestImpl();
    WMError ret = Rosen::WindowManager::GetInstance().RegisterFocusChangedListener(listener);
    IMSA_HILOGI("register focus changed listener ret: %{public}d", ret);
}
} // namespace MiscServices
} // namespace OHOS
