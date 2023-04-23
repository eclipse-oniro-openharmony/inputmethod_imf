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
#include "input_method_system_ability_proxy.h"
#undef private

#include "systemabilitystub_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "accesstoken_kit.h"
#include "global.h"
#include "input_method_controller.h"
#include "input_method_system_ability.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

using namespace OHOS::Security::AccessToken;
using namespace OHOS::MiscServices;
namespace OHOS {
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

class TextListener : public OnTextChangedListener {
public:
    TextListener() {}
    ~TextListener() {}
    void InsertText(const std::u16string &text) {}
    void DeleteBackward(int32_t length) {}
    void SetKeyboardStatus(bool status) {}
    void DeleteForward(int32_t length) {}
    void SendKeyEventFromInputMethod(const KeyEvent &event) {}
    void SendKeyboardStatus(const KeyboardStatus &status) {}
    void SendFunctionKey(const FunctionKey &functionKey) {}
    void MoveCursor(const Direction direction) {}
    void HandleSetSelection(int32_t start, int32_t end) {}
    void HandleExtendAction(int32_t action) {}
    void HandleSelect(int32_t keyCode, int32_t cursorMoveSkip) {}
};
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
    uint32_t code = ConvertToUint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;

    sptr<InputMethodController> imc = InputMethodController::GetInstance();
    sptr<OnTextChangedListener> textListener = new TextListener();
    imc->Attach(textListener);

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGI("systemAbilityManager is nullptr");
        return false;
    }
    auto systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
    if (systemAbility == nullptr) {
        IMSA_HILOGI("systemAbility is nullptr");
        return false;
    }
    sptr<InputMethodSystemAbilityProxy> iface = new InputMethodSystemAbilityProxy(systemAbility);
    iface->SendRequest(code, [&rawData, &size](MessageParcel &data) {
        data.WriteInterfaceToken(SYSTEMABILITY_INTERFACE_TOKEN);
        data.WriteBuffer(rawData, size);
        data.RewindRead(0);
        return true;
    });
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
    return 0;
}