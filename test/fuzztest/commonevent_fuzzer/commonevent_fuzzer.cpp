/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "commonevent_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "input_method_agent_service_impl.h"
#include "string_utils.h"
#include "message_parcel.h"
#define private public
#define protected public
#include "on_demand_start_stop_sa.h"
#include "input_method_system_ability.h"
#undef private
#include "fuzzer/FuzzedDataProvider.h"
#include "input_method_controller.h"
#include "input_method_core_service_impl.h"
#include "inputmethod_message_handler.h"
#include "iremote_object.h"
#include "system_cmd_channel_service_impl.h"
using namespace OHOS::MiscServices;
using namespace MessageID;
namespace OHOS {
uint32_t ConvertToUint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return bigVar;
}

void FuzzStringUtils(const uint8_t *data, size_t size)
{
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto fuzzedInt32 = static_cast<int32_t>(size);
    std::u16string txt = u"insert text";
    StringUtils::ToHex(fuzzedString);
    StringUtils::ToHex(txt);
    StringUtils::CountUtf16Chars(txt);
    StringUtils::TruncateUtf16String(txt, fuzzedInt32);
}

void FuzzOnDemandStartStopSa(const uint8_t *data, size_t size)
{
    auto onDemandStartStopSa = std::make_shared<OnDemandStartStopSa>();
    sptr<OnDemandStartStopSa::SaLoadCallback> callback =
        new (std::nothrow) OnDemandStartStopSa::SaLoadCallback(onDemandStartStopSa);
    sptr<IRemoteObject> object {nullptr};
    FuzzedDataProvider provider(data, size);
    int32_t fuzzedInt32 = provider.ConsumeIntegral<int32_t>();
    callback->OnLoadSystemAbilitySuccess(fuzzedInt32, object);
    callback->OnLoadSystemAbilityFail(fuzzedInt32);
    OnDemandStartStopSa::IncreaseProcessingIpcCnt();
    OnDemandStartStopSa::DecreaseProcessingIpcCnt();
    OnDemandStartStopSa::IsSaBusy();
}

void FuzzSwitchOperation(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    auto fuzzedUint64 = provider.ConsumeIntegral<uint64_t>();
    auto fuzzedInt32 = provider.ConsumeIntegral<int32_t>();
    sptr<IInputMethodCore> core = new InputMethodCoreServiceImpl();
    sptr<SystemCmdChannelStub> stub = new SystemCmdChannelServiceImpl();
    auto info = std::make_shared<ImeInfo>();
 
    MessageParcel messData;
    messData.WriteRemoteObject(stub->AsObject());
    sptr<IRemoteObject> remoteObject = messData.ReadRemoteObject();
    auto fuzzedBool = static_cast<bool>(data[0] % 2);
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);

    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->UpdateUserInfo(fuzzedInt32);
 
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->RegisterProxyIme(fuzzedUint64, core, remoteObject);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->SwitchExtension(fuzzedInt32, info);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->SwitchSubType(fuzzedInt32, info);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->HandleFocusChanged(fuzzedBool,
        fuzzedUint64, fuzzedInt32, fuzzedInt32);
}
 
void FuzzHandleOperation(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    auto fuzzedInt32 = provider.ConsumeIntegral<int32_t>();
    InputType inputType = static_cast<InputType>(size);
    InputClientInfo clientInfo {};
 
    auto fuzzedBool = static_cast<bool>(data[0] % 2);
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
 
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->HandleScbStarted(fuzzedInt32, fuzzedInt32);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->HandleWmsDisconnected(fuzzedInt32, fuzzedInt32);
    DelayedSingleton<InputMethodSystemAbility>::GetInstance()->NeedHideWhenSwitchInputType(fuzzedInt32, inputType,
        fuzzedBool);
}
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    /* Run your code on data */
    OHOS::FuzzStringUtils(data, size);
    OHOS::FuzzOnDemandStartStopSa(data, size);
    OHOS::FuzzSwitchOperation(data, size);
    OHOS::FuzzHandleOperation(data, size);
    return 0;
}