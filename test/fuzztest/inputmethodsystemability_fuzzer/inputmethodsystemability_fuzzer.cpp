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
#include "iservice_registry.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "inputmethodsystemability_fuzzer.h"
#include "text_listener.h"
#include "token_setproc.h"

using namespace OHOS::MiscServices;
namespace OHOS {
constexpr const int32_t MSG_ID_USER_ONE = 50;
constexpr const int32_t MSG_ID_USER_TWO = 60;
void FuzzOnUser(int32_t userId, const std::string &packageName)
{
    //onUserStarted
    MessageParcel *parcel = new MessageParcel();
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->isScbEnable_ = false;
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->userId_ = MSG_ID_USER_ONE;
    parcel->WriteInt32(MSG_ID_USER_ONE);
    auto msg = std::make_shared<Message>(MessageID::MSG_ID_USER_START, parcel);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->OnUserStarted(msg.get());

    //onUserRemoved
    MessageParcel *parcel1 = new MessageParcel();
    parcel1->WriteInt32(MSG_ID_USER_TWO);
    auto msg1 = std::make_shared<Message>(MessageID::MSG_ID_USER_REMOVED, parcel1);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->OnUserRemoved(msg1.get());

    //HandlePackageEvent
    MessageParcel *parcel2 = new (std::nothrow) MessageParcel();
    auto bundleName = "testBundleName1";
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->userId_ = MSG_ID_USER_TWO;
    parcel2->WriteInt32(MSG_ID_USER_ONE);
    parcel2->WriteString(bundleName);
    auto msg2 = std::make_shared<Message>(MessageID::MSG_ID_PACKAGE_REMOVED, parcel2);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->HandlePackageEvent(msg2.get());

    //OnPackageRemoved
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->userId_ = userId;
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->OnPackageRemoved(userId, bundleName);
}

} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    const int32_t userId = static_cast<int32_t>(size);
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);

    OHOS::FuzzOnUser(userId, fuzzedString);
    return 0;
}
