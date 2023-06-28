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

#include "tdd_util.h"

#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "accesstoken_kit.h"
#include "global.h"
#include "if_system_ability_manager.h"
#include "input_manager.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "os_account_manager.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "window_manager.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::Security::AccessToken;
using namespace OHOS::AccountSA;
using namespace Rosen;
constexpr int32_t INVALID_USER_ID = -1;
constexpr int32_t MAIN_USER_ID = 100;
constexpr const uint16_t EACH_LINE_LENGTH = 500;
constexpr int32_t SEC_TO_NANOSEC = 1000000000;
constexpr int32_t NANOSECOND_TO_MILLISECOND = 1000000;
constexpr int32_t DEFAULT_DEVICE_ID = -1;
constexpr int32_t DEFAULT_UNICODE = 0x0000;
uint64_t TddUtil::selfTokenID_ = 0;
uint64_t TddUtil::testTokenID_ = 0;
int64_t TddUtil::selfUid_ = -1;
int32_t TddUtil::userID_ = INVALID_USER_ID;
int32_t TddUtil::GetCurrentUserId()
{
    if (userID_ != INVALID_USER_ID) {
        return userID_;
    }
    std::vector<int32_t> userIds;
    auto ret = OsAccountManager::QueryActiveOsAccountIds(userIds);
    if (ret != ErrorCode::NO_ERROR || userIds.empty()) {
        IMSA_HILOGE("query active os account id failed");
        userIds[0] = MAIN_USER_ID;
    }
    return userIds[0];
}
void TddUtil::StorageSelfTokenID()
{
    selfTokenID_ = GetSelfTokenID();
}

void TddUtil::AllocTestTokenID(const std::string &bundleName)
{
    IMSA_HILOGI("bundleName: %{public}s", bundleName.c_str());
    HapInfoParams infoParams = { .userID = GetCurrentUserId(),
        .bundleName = bundleName,
        .instIndex = 0,
        .appIDDesc = "ohos.inputmethod_test.demo",
        .isSystemApp = true };
    PermissionStateFull permissionState = { .permissionName = "ohos.permission.CONNECT_IME_ABILITY",
        .isGeneral = true,
        .resDeviceID = { "local" },
        .grantStatus = { PermissionState::PERMISSION_GRANTED },
        .grantFlags = { 1 } };
    HapPolicyParams policyParams = {
        .apl = APL_NORMAL, .domain = "test.domain.inputmethod", .permList = {}, .permStateList = { permissionState }
    };

    auto tokenInfo = AccessTokenKit::AllocHapToken(infoParams, policyParams);
    testTokenID_ = tokenInfo.tokenIDEx;
}

void TddUtil::DeleteTestTokenID()
{
    AccessTokenKit::DeleteToken(testTokenID_);
}

void TddUtil::SetTestTokenID()
{
    auto ret = SetSelfTokenID(testTokenID_);
    IMSA_HILOGI("SetSelfTokenID ret: %{public}d", ret);
}

void TddUtil::RestoreSelfTokenID()
{
    auto ret = SetSelfTokenID(selfTokenID_);
    IMSA_HILOGI("SetSelfTokenID ret = %{public}d", ret);
}

void TddUtil::StorageSelfUid()
{
    selfUid_ = getuid();
}

void TddUtil::SetTestUid()
{
    FocusChangeInfo info;
    WindowManager::GetInstance().GetFocusWindowInfo(info);
    IMSA_HILOGI("uid: %{public}d", info.uid_);
    setuid(info.uid_);
}
void TddUtil::RestoreSelfUid()
{
    setuid(selfUid_);
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

bool TddUtil::SimulateKeyEvent(int32_t keyCode)
{
    auto keyDown = CreateKeyEvent(keyCode, MMI::KeyEvent::KEY_ACTION_DOWN);
    auto keyUp = CreateKeyEvent(keyCode, MMI::KeyEvent::KEY_ACTION_UP);
    if (keyDown == nullptr || keyUp == nullptr) {
        IMSA_HILOGE("failed to create key event: %{public}d", keyCode);
        return false;
    }
    MMI::InputManager::GetInstance()->SimulateInputEvent(keyDown);
    MMI::InputManager::GetInstance()->SimulateInputEvent(keyUp);
    return true;
}

bool TddUtil::SimulateKeyEvents(const std::vector<int32_t> &keys)
{
    if (keys.empty()) {
        IMSA_HILOGE("keys is empty");
        return false;
    }
    std::vector<std::shared_ptr<MMI::KeyEvent>> downKeys_;
    std::vector<std::shared_ptr<MMI::KeyEvent>> upKeys_;
    for (auto &key : keys) {
        auto keyDown = CreateKeyEvent(key, MMI::KeyEvent::KEY_ACTION_DOWN);
        auto keyUp = CreateKeyEvent(key, MMI::KeyEvent::KEY_ACTION_UP);
        if (keyDown == nullptr || keyUp == nullptr) {
            IMSA_HILOGE("failed to create key event: %{public}d", key);
            return false;
        }
        downKeys_.push_back(keyDown);
        upKeys_.push_back(keyUp);
    }
    // first pressed last lift.
    std::reverse(upKeys_.begin(), upKeys_.end());
    for (auto &downKey : downKeys_) {
        MMI::InputManager::GetInstance()->SimulateInputEvent(downKey);
    }
    for (auto &upkey : upKeys_) {
        MMI::InputManager::GetInstance()->SimulateInputEvent(upkey);
    }
    return true;
}

std::shared_ptr<MMI::KeyEvent> TddUtil::CreateKeyEvent(int32_t keyCode, int32_t keyAction)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    int64_t downTime = GetNanoTime() / NANOSECOND_TO_MILLISECOND;
    MMI::KeyEvent::KeyItem keyItem;
    keyItem.SetKeyCode(keyCode);
    keyItem.SetPressed(keyAction == MMI::KeyEvent::KEY_ACTION_DOWN);
    keyItem.SetDownTime(downTime);
    keyItem.SetDeviceId(DEFAULT_DEVICE_ID);
    keyItem.SetUnicode(DEFAULT_UNICODE);
    if (keyEvent != nullptr) {
        keyEvent->SetKeyCode(keyCode);
        keyEvent->SetKeyAction(keyAction);
        keyEvent->AddPressedKeyItems(keyItem);
    }
    return keyEvent;
}

int64_t TddUtil::GetNanoTime()
{
    struct timespec time = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &time);
    return static_cast<int64_t>(time.tv_sec) * SEC_TO_NANOSEC + time.tv_nsec;
}
} // namespace MiscServices
} // namespace OHOS
