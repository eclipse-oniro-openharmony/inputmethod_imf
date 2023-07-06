/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "input_method_system_ability.h"
#include "input_method_system_ability_proxy.h"
#undef private

#include "systemabilitystub_fuzzer.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string_ex.h>

#include "accesstoken_kit.h"
#include "global.h"
#include "input_method_controller.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "text_listener.h"
#include "token_setproc.h"

using namespace OHOS::Security::AccessToken;
using namespace OHOS::MiscServices;
namespace OHOS {
std::atomic_bool g_isInitialize = false;
constexpr uint32_t TARGET_REMOTE_CODE_NUMS = 21;
void GrantNativePermission()
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
        .processName = "inputmethod_imf",
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
constexpr size_t THRESHOLD = 10;
constexpr int32_t OFFSET = 4;
const std::u16string SYSTEMABILITY_INTERFACE_TOKEN = u"ohos.miscservices.inputmethod.IInputMethodSystemAbility";

uint32_t ConvertToUint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return bigVar;
}
bool FuzzInputMethodSystemAbility(const uint8_t *rawData, size_t size)
{
    GrantNativePermission();
    uint32_t code = ConvertToUint32(rawData) % TARGET_REMOTE_CODE_NUMS;
    rawData = rawData + OFFSET;
    size = size - OFFSET;

    if (!g_isInitialize.load()) {
        DelayedSingleton<InputMethodSystemAbility>::GetInstance()->Initialize();
        g_isInitialize.store(true);
    }

    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    sptr<OnTextChangedListener> textListener = new TextListener();
    imc->Attach(textListener);

    MessageParcel datas;
    datas.WriteInterfaceToken(SYSTEMABILITY_INTERFACE_TOKEN);
    datas.WriteBuffer(rawData, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool TestDump(const uint8_t *rawData, size_t size)
{
    std::vector<std::u16string> args;
    std::string str(reinterpret_cast<const char *>(rawData), size);
    args.push_back(Str8ToStr16(str));
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->Dump(static_cast<int32_t>(size), args);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->DumpAllMethod(static_cast<int32_t>(size));
    return true;
}

} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return 0;
    }
    /* Run your code on data */
    OHOS::FuzzInputMethodSystemAbility(data, size);
    OHOS::TestDump(data, size);
    return 0;
}
