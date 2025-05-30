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

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string_ex.h>

#include "accesstoken_kit.h"
#include "global.h"
#include "ime_cfg_manager.h"
#include "input_method_controller.h"
#include "inputmethodsystemability_fuzzer.h"
#include "input_method_core_service_impl.h"
#include "system_cmd_channel_service_impl.h"
#include "ime_enabled_info_manager.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "text_listener.h"
#include "token_setproc.h"

using namespace OHOS::MiscServices;
namespace OHOS {
constexpr const int32_t MSG_ID_USER_ONE = 50;
constexpr const int32_t MSG_ID_USER_TWO = 60;
void FuzzOnUser(int32_t userId, const std::string &packageName)
{
    // onUserStarted
    MessageParcel *parcel = new MessageParcel();
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->isScbEnable_ = false;
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->userId_ = MSG_ID_USER_ONE;
    parcel->WriteInt32(MSG_ID_USER_ONE);
    auto msg = std::make_shared<Message>(MessageID::MSG_ID_USER_START, parcel);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->OnUserStarted(msg.get());

    // onUserRemoved
    MessageParcel *parcel1 = new MessageParcel();
    parcel1->WriteInt32(MSG_ID_USER_TWO);
    auto msg1 = std::make_shared<Message>(MessageID::MSG_ID_USER_REMOVED, parcel1);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->OnUserRemoved(msg1.get());

    // HandlePackageEvent
    MessageParcel *parcel2 = new (std::nothrow) MessageParcel();
    auto bundleName = "testBundleName1";
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->userId_ = MSG_ID_USER_TWO;
    parcel2->WriteInt32(MSG_ID_USER_ONE);
    parcel2->WriteString(bundleName);
    auto msg2 = std::make_shared<Message>(MessageID::MSG_ID_PACKAGE_REMOVED, parcel2);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->HandlePackageEvent(msg2.get());

    // OnPackageRemoved
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->userId_ = userId;
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->OnPackageRemoved(userId, bundleName);
}

void FuzzOnScreenUnlock()
{
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->OnScreenUnlock(nullptr);

    MessageParcel *parcel = nullptr;
    auto msg = std::make_shared<Message>(MessageID::MSG_ID_SCREEN_UNLOCK, parcel);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->OnScreenUnlock(msg.get());

    MessageParcel *parcel1 = new (std::nothrow) MessageParcel();
    msg = std::make_shared<Message>(MessageID::MSG_ID_SCREEN_UNLOCK, parcel1);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->OnScreenUnlock(msg.get());

    MessageParcel *parcel2 = new (std::nothrow) MessageParcel();
    msg = std::make_shared<Message>(MessageID::MSG_ID_SCREEN_UNLOCK, parcel2);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->OnScreenUnlock(msg.get());
}

void SystemAbility(const uint8_t *data, size_t size)
{
    auto fuzzedUint32 = static_cast<uint32_t>(size);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->ReleaseInput(nullptr, fuzzedUint32);
}

void FuzzSwitchOperation(const uint8_t *data, size_t size)
{
    auto fuzzedUint64 = static_cast<uint64_t>(size);
    auto fuzzedUint32 = static_cast<uint32_t>(size);
    auto fuzzedInt32 = static_cast<int32_t>(size);
    sptr<IInputMethodCore> core = new InputMethodCoreServiceImpl();
    sptr<SystemCmdChannelStub> stub = new SystemCmdChannelServiceImpl();
    auto info = std::make_shared<ImeInfo>();

    MessageParcel messData;
    messData.WriteRemoteObject(stub->AsObject());
    sptr<IRemoteObject> remoteObject = messData.ReadRemoteObject();
    auto fuzzedBool = static_cast<bool>(data[0] % 2);
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);

    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->Init();
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->OnStop();
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->InitMemMgrMonitor();
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->InitFocusChangedMonitor();
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->InitWmsConnectionMonitor();
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->IsValidBundleName(fuzzedString);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->UpdateUserInfo(fuzzedInt32);

    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->RegisterProxyIme(fuzzedUint64, core, remoteObject);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->SwitchByCombinationKey(fuzzedUint32);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->HandleFocusChanged(fuzzedBool,
        fuzzedUint64, fuzzedInt32, fuzzedInt32);
}

void FuzzHandleOperation(const uint8_t *data, size_t size)
{
    auto fuzzedInt32 = static_cast<int32_t>(size);
    InputType inputType = static_cast<InputType>(size);
    InputClientInfo clientInfo {};

    auto fuzzedBool = static_cast<bool>(data[0] % 2);
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);

    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->StopImeInBackground();
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->HandleOsAccountStarted();
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->HandleMemStarted();
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->HandleWmsStarted();

    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->HandleWmsConnected(fuzzedInt32, fuzzedInt32);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->HandleScbStarted(fuzzedInt32, fuzzedInt32);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->HandleWmsDisconnected(fuzzedInt32, fuzzedInt32);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->NeedHideWhenSwitchInputType(fuzzedInt32, inputType,
        fuzzedBool);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->GetAlternativeIme(fuzzedInt32, fuzzedString);
}
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    const int32_t userId = static_cast<int32_t>(size);
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);

    OHOS::FuzzOnUser(userId, fuzzedString);
    OHOS::FuzzOnScreenUnlock();
    OHOS::SystemAbility(data, size);
    OHOS::FuzzSwitchOperation(data, size);
    OHOS::FuzzHandleOperation(data, size);
    return 0;
}
