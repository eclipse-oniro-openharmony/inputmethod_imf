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

#include "systemcmdchannelstub_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "system_cmd_channel_stub.h"
#include "message_parcel.h"

using namespace OHOS::MiscServices;
namespace OHOS {
constexpr size_t THRESHOLD = 10;
constexpr int32_t OFFSET = 4;
constexpr int32_t PRIVATEDATAVALUE = 100;
const std::u16string AGENTSTUB_INTERFACE_TOKEN = u"ohos.miscservices.inputmethod.ISystemCmdChannel";
uint32_t ConvertToUint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return bigVar;
}
bool FuzzSystemCmdChannelStub(const uint8_t *rawData, size_t size)
{
    bool fuzzedBool = static_cast<bool>(rawData[0] % 2);
    auto fuzzedUint32 = static_cast<uint32_t>(size);

    uint32_t code = ConvertToUint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;

    SysPanelStatus sysPanelStatus = {fuzzedBool, 0, fuzzedUint32, fuzzedUint32};

    std::unordered_map <std::string, PrivateDataValue> privateCommand;
    PrivateDataValue privateDataValue1 = std::string("stringValue");
    PrivateDataValue privateDataValue2 = fuzzedBool;
    PrivateDataValue privateDataValue3 = PRIVATEDATAVALUE;
    privateCommand.emplace("value1", privateDataValue1);
    privateCommand.emplace("value2", privateDataValue2);
    privateCommand.emplace("value3", privateDataValue3);

    MessageParcel data;
    data.WriteInterfaceToken(AGENTSTUB_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    sptr <SystemCmdChannelStub> stub = new SystemCmdChannelStub();
    stub->SendPrivateCommand(privateCommand);
    stub->NotifyPanelStatus(sysPanelStatus);
    stub->OnRemoteRequest(code, data, reply, option);
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
    OHOS::FuzzSystemCmdChannelStub(data, size);
    return 0;
}