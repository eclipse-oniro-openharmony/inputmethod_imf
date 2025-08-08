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

#include "imf_sa_stub_fuzz_util.h"

#include "accesstoken_kit.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "global.h"
#include "ime_cfg_manager.h"
#include "input_client_service_impl.h"
#include "input_method_agent_service_impl.h"
#include "input_method_core_service_impl.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "text_listener.h"
#include "token_setproc.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::Security::AccessToken;
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

bool ImfSaStubFuzzUtil::SwitchIpcCode(IInputMethodSystemAbilityIpcCode code, MessageParcel &datas, int32_t fuzzedInt32)
{
    auto writeInputClient = [&datas]() {
        sptr<IInputClient> client = new (std::nothrow) InputClientServiceImpl();
        if (client == nullptr || !datas.WriteRemoteObject(client->AsObject())) {
            return false;
        }
        return true;
    };

    auto writeInputMethodCore = [&datas]() {
        sptr<IInputMethodCore> core = new (std::nothrow) InputMethodCoreServiceImpl();
        if (core == nullptr || !datas.WriteRemoteObject(core->AsObject())) {
            return false;
        }
        return true;
    };

    auto writeInputMethodAgent = [&datas]() {
        sptr<IInputMethodAgent> agent = new (std::nothrow) InputMethodAgentServiceImpl();
        if (agent == nullptr || !datas.WriteRemoteObject(agent->AsObject())) {
            return false;
        }
        return true;
    };
    switch (code) {
        case IInputMethodSystemAbilityIpcCode::COMMAND_START_INPUT: {
            InputClientInfoInner clientInfoInner = {};
            if (!datas.WriteParcelable(&clientInfoInner)) {
                return false;
            }
            break;
        }
        case IInputMethodSystemAbilityIpcCode::COMMAND_SHOW_INPUT:
            return writeInputClient();
        case IInputMethodSystemAbilityIpcCode::COMMAND_SET_CORE_AND_AGENT:
            return writeInputMethodCore() && writeInputMethodAgent();
        case IInputMethodSystemAbilityIpcCode::COMMAND_UN_REGISTERED_PROXY_IME:
            if (!datas.WriteInt32(fuzzedInt32)) {
                return false;
            }
            return writeInputMethodCore();
        case IInputMethodSystemAbilityIpcCode::COMMAND_RELEASE_INPUT:
            return writeInputClient();
        case IInputMethodSystemAbilityIpcCode::COMMAND_HIDE_INPUT:
            return writeInputClient();
        case IInputMethodSystemAbilityIpcCode::COMMAND_BIND_IME_MIRROR:
            return writeInputMethodCore() && writeInputMethodAgent();
        default:
            return true;
    }
    return true;
}

bool ImfSaStubFuzzUtil::FuzzInputMethodSystemAbility(const uint8_t *rawData, size_t size,
    IInputMethodSystemAbilityIpcCode code)
{
    if (!isInitialize_) {
        Initialize();
    }
    FuzzedDataProvider provider(rawData, size);
    auto fuzzedInt32 = provider.ConsumeIntegral<int32_t>();
    GrantNativePermission();

    MessageParcel datas;
    datas.WriteInterfaceToken(SYSTEMABILITY_INTERFACE_TOKEN);
    SwitchIpcCode(code, datas, fuzzedInt32);
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
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->SubscribeCommonEvent();
    int32_t ret = DelayedSingleton<InputMethodSystemAbility>::GetInstance()->InitKeyEventMonitor();
    IMSA_HILOGI("init KeyEvent monitor %{public}s", ret == ErrorCode::NO_ERROR ? "success" : "failed");
    ret = DelayedSingleton<InputMethodSystemAbility>::GetInstance()->InitWmsMonitor();
    ImeInfoInquirer::GetInstance().InitSystemConfig();
    IMSA_HILOGI("init wms monitor %{public}s", ret ? "success" : "failed");
    isInitialize_ = true;
}
} // namespace MiscServices
} // namespace OHOS
