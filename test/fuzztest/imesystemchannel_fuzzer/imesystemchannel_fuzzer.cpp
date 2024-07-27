/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ime_system_channel.h"
#include "input_method_system_ability_proxy.h"
#undef private

#include "imesystemchannel_fuzzer.h"
#include "system_cmd_channel_stub.h"
#include <cstddef>
#include <cstdint>

#include "input_method_agent_stub.h"
#include "message_parcel.h"

using namespace OHOS::MiscServices;
namespace OHOS {
constexpr size_t THRESHOLD = 10;
constexpr int32_t PRIVATEDATAVALUE = 100;
void FuzzGetSmartMenuCfg()
{
    ImeSystemCmdChannel::GetInstance()->GetSmartMenuCfg();
}

void FuzzConnectSystemCmd()
{
    sptr<OnSystemCmdListener> listener = new (std::nothrow) OnSystemCmdListener();
    if (listener == nullptr) {
        return;
    }

    ImeSystemCmdChannel::GetInstance()->SetSystemCmdListener(listener);
    ImeSystemCmdChannel::GetInstance()->GetSystemCmdListener();
    ImeSystemCmdChannel::GetInstance()->ConnectSystemCmd(listener);
    ImeSystemCmdChannel::GetInstance()->RunConnectSystemCmd();
}

void FuzzSystemCmdAgent()
{
    ImeSystemCmdChannel::GetInstance()->GetSystemCmdAgent();
    ImeSystemCmdChannel::GetInstance()->ClearSystemCmdAgent();
}

void FuzzOnSystemCmdAgent()
{
    sptr<SystemCmdChannelStub> stub = new SystemCmdChannelStub();

    MessageParcel data;
    data.WriteRemoteObject(stub->AsObject());
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    ImeSystemCmdChannel::GetInstance()->OnConnectCmdReady(remoteObject);
    ImeSystemCmdChannel::GetInstance()->OnSystemCmdAgentDied(remoteObject);
}

void FuzzPrivateCommand(const uint8_t *data, size_t size)
{
    bool fuzzedBool = static_cast<bool>(data[0] % 2);

    std::unordered_map<std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = std::string("stringValue");
    PrivateDataValue privateDataValue2 = fuzzedBool;
    PrivateDataValue privateDataValue3 = PRIVATEDATAVALUE;
    privateCommand.emplace("value1", privateDataValue1);
    privateCommand.emplace("value2", privateDataValue2);
    privateCommand.emplace("value3", privateDataValue3);

    ImeSystemCmdChannel::GetInstance()->SendPrivateCommand(privateCommand);
    ImeSystemCmdChannel::GetInstance()->ReceivePrivateCommand(privateCommand);
}

void FuzzNotifyPanelStatus(const uint8_t *data, size_t size)
{
    bool fuzzedBool = static_cast<bool>(data[0] % 2);
    auto fuzzedUint32 = static_cast<uint32_t>(size);

    SysPanelStatus sysPanelStatus = { fuzzedBool, 0, fuzzedUint32, fuzzedUint32 };
    ImeSystemCmdChannel::GetInstance()->NotifyPanelStatus(sysPanelStatus);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return 0;
    }
    /* Run your code on data */

    OHOS::FuzzGetSmartMenuCfg();
    OHOS::FuzzConnectSystemCmd();
    OHOS::FuzzSystemCmdAgent();
    OHOS::FuzzOnSystemCmdAgent();
    OHOS::FuzzPrivateCommand(data, size);
    OHOS::FuzzNotifyPanelStatus(data, size);
    return 0;
}