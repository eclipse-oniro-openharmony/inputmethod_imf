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
#include "enable_ime_data_parser.h"
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
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "os_account_manager.h"
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
constexpr int32_t BUFF_LENGTH = 10;
constexpr int32_t PERMISSION_NUM = 2;
constexpr const char *CMD_PIDOF_IMS = "pidof inputmethod_ser";
constexpr const char *SETTING_COLUMN_KEYWORD = "KEYWORD";
constexpr const char *SETTING_COLUMN_VALUE = "VALUE";
static constexpr int32_t MAX_TIMEOUT_WAIT_FOCUS = 2000;
uint64_t TddUtil::selfTokenID_ = 0;
int32_t TddUtil::userID_ = INVALID_USER_ID;
sptr<Window> TddUtil::WindowManager::window_ = nullptr;
int32_t TddUtil::WindowManager::currentWindowId_ = 0;
std::shared_ptr<BlockData<bool>> FocusChangedListenerTestImpl::isFocused_ =
    std::make_shared<BlockData<bool>>(MAX_TIMEOUT_WAIT_FOCUS, false);
std::shared_ptr<BlockData<bool>> FocusChangedListenerTestImpl::unFocused_ =
    std::make_shared<BlockData<bool>>(MAX_TIMEOUT_WAIT_FOCUS, false);
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
    std::vector<int32_t> userIds;
    auto ret = OsAccountManager::QueryActiveOsAccountIds(userIds);
    if (ret != ErrorCode::NO_ERROR || userIds.empty()) {
        IMSA_HILOGE("query active os account id failed");
        return MAIN_USER_ID;
    }
    return userIds[0];
}
void TddUtil::StorageSelfTokenID()
{
    selfTokenID_ = GetSelfTokenID();
}

uint64_t TddUtil::AllocTestTokenID(bool isSystemApp, bool needPermission, const std::string &bundleName)
{
    IMSA_HILOGI("bundleName: %{public}s", bundleName.c_str());
    HapInfoParams infoParams = { .userID = GetCurrentUserId(),
        .bundleName = bundleName,
        .instIndex = 0,
        .appIDDesc = bundleName,
        .isSystemApp = isSystemApp };
    PermissionStateFull permissionState = { .permissionName = "ohos.permission.CONNECT_IME_ABILITY",
        .isGeneral = true,
        .resDeviceID = { "local" },
        .grantStatus = { PermissionState::PERMISSION_GRANTED },
        .grantFlags = { 1 } };
    HapPolicyParams policyParams = {
        .apl = APL_NORMAL, .domain = bundleName, .permList = {}, .permStateList = { permissionState }
    };
    if (!needPermission) {
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

bool TddUtil::ExecuteCmd(const std::string &cmd, std::string &result)
{
    char buff[EACH_LINE_LENGTH] = { 0x00 };
    std::stringstream output;
    FILE *ptr = popen(cmd.c_str(), "r");
    if (ptr != nullptr) {
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
    char buff[BUFF_LENGTH] = { 0 };
    FILE *fp = popen(CMD_PIDOF_IMS, "r");
    if (fp == nullptr) {
        IMSA_HILOGI("get pid failed.");
        return -1;
    }
    fgets(buff, sizeof(buff), fp);
    pid_t pid = atoi(buff);
    pclose(fp);
    fp = nullptr;
    return pid;
}

void TddUtil::KillImsaProcess()
{
    pid_t pid = GetImsaPid();
    if (pid == -1) {
        IMSA_HILOGE("Pid of Imsa is not exist.");
        return;
    }
    auto ret = kill(pid, SIGTERM);
    if (ret != 0) {
        IMSA_HILOGE("Kill failed, ret: %{public}d", ret);
        return;
    }
    IMSA_HILOGI("Kill success.");
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
    const char *perms[PERMISSION_NUM] = {
        "ohos.permission.MANAGE_SECURE_SETTINGS",
        "ohos.permission.CONNECT_IME_ABILITY",
    };
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

void TddUtil::PutEnableImeValue(const std::string &key, const std::string &value)
{
    auto helper = EnableImeDataParser::GetInstance()->CreateDataShareHelper();
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
    Uri uri(EnableImeDataParser::GetInstance()->GenerateTargetUri(key));
    if (helper->Update(uri, predicates, bucket) <= 0) {
        IMSA_HILOGD("no data exist, insert one row");
        helper->Insert(uri, bucket);
    }
    EnableImeDataParser::GetInstance()->ReleaseDataShareHelper(helper);
}

int32_t TddUtil::CheckEnableOn(std::string &value)
{
    return EnableImeDataParser::GetInstance()->GetStringValue(EnableImeDataParser::ENABLE_IME, value);
}

void TddUtil::WindowManager::CreateWindow()
{
    std::string windowName = "inputmethod_test_window";
    sptr<WindowOption> winOption = new OHOS::Rosen::WindowOption();
    winOption->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    winOption->SetFocusable(true);
    std::shared_ptr<AbilityRuntime::Context> context = nullptr;
    WMError wmError = WMError::WM_OK;
    window_ = Window::Create(windowName, winOption, context, wmError);
    IMSA_HILOGI("Create window ret:%{public}d", wmError);
    currentWindowId_ = window_->GetWindowId();
}

void TddUtil::WindowManager::ShowWindow()
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window is not exist.");
        return;
    }
    auto ret = window_->Show();
    IMSA_HILOGI("Show window end, ret = %{public}d", ret);
}

void TddUtil::WindowManager::HideWindow()
{
    if (window_ == nullptr) {
        IMSA_HILOGE("window is not exist.");
        return;
    }
    auto ret = window_->Hide();
    IMSA_HILOGI("Hide window end, ret = %{public}d", ret);
}

void TddUtil::WindowManager::DestroyWindow()
{
    if (window_ != nullptr) {
        window_->Destroy();
    }
}

void TddUtil::WindowManager::RegisterFocusChangeListener()
{
    auto listener = new (std::nothrow) FocusChangedListenerTestImpl();
    WMError ret = Rosen::WindowManager::GetInstance().RegisterFocusChangedListener(listener);
    IMSA_HILOGI("register focus changed listener ret: %{public}d", ret);
}
} // namespace MiscServices
} // namespace OHOS
