/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef IMF_SA_STUB_FUZZ_UTIL_H
#define IMF_SA_STUB_FUZZ_UTIL_H

#define private public
#define protected public
#include "input_method_system_ability.h"
#include "input_method_system_ability_proxy.h"
#undef private

#include <string_ex.h>

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "accesstoken_kit.h"
#include "global.h"
#include "ime_cfg_manager.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "text_listener.h"
#include "token_setproc.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::Security::AccessToken;
const std::u16string SYSTEMABILITY_INTERFACE_TOKEN = u"ohos.miscservices.inputmethod.IInputMethodSystemAbility";
constexpr const int32_t USER_ID = 100;
class ImfSaStubFuzzUtil {
public:
    static bool FuzzInputMethodSystemAbility(const uint8_t *rawData, size_t size, InputMethodInterfaceCode code);

private:
    static void InitKeyboardDelegate();
    static void Initialize();
    static void GrantNativePermission();
    static bool isInitialize_;
    static std::mutex initMutex_;
};
bool ImfSaStubFuzzUtil::isInitialize_ = false;
std::mutex ImfSaStubFuzzUtil::initMutex_;

void ImfSaStubFuzzUtil::GrantNativePermission()
{
    const char **perms = new const char *[1];
    perms[0] = "ohos.permission.CONNECT_IME_ABILITY";
    TokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "broker",
        .aplStr = "system_basic",
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

bool ImfSaStubFuzzUtil::FuzzInputMethodSystemAbility(const uint8_t *rawData, size_t size, InputMethodInterfaceCode code)
{
    if (!isInitialize_) {
        Initialize();
    }
    GrantNativePermission();

    MessageParcel datas;
    datas.WriteInterfaceToken(SYSTEMABILITY_INTERFACE_TOKEN);
    datas.WriteBuffer(rawData, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->OnRemoteRequest(
        static_cast<int32_t>(code), datas, reply, option);
    return true;
}

void ImfSaStubFuzzUtil::Initialize()
{
    std::lock_guard<std::mutex> lock(initMutex_);
    if (isInitialize_) {
        return;
    }
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->Initialize();
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->InitServiceHandler();
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->state_ = ServiceRunningState::STATE_RUNNING;
    ImeCfgManager::GetInstance().Init();
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->userSession_->UpdateCurrentUserId(USER_ID);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->StartUserIdListener();
    int32_t ret = DelayedSingleton<InputMethodSystemAbility>::GetInstance()->InitKeyEventMonitor();
    IMSA_HILOGI("init KeyEvent monitor %{public}s", ret == ErrorCode::NO_ERROR ? "success" : "failed");
    ret = DelayedSingleton<InputMethodSystemAbility>::GetInstance()->InitFocusChangeMonitor();
    IMSA_HILOGI("init focus change monitor %{public}s", ret ? "success" : "failed");
    isInitialize_ = true;
}
} // namespace MiscServices
} // namespace OHOS
#endif // IMF_SA_STUB_FUZZ_UTIL_H