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
#include "enable_ime_data_parser.h"
#undef private

#include "enableimedataparse_fuzzer.h"

using namespace OHOS::MiscServices;
namespace OHOS {
constexpr size_t THRESHOLD = 10;
void FuzzInitialize(const int32_t userId)
{
    EnableImeDataParser::GetInstance()->Initialize(userId);
}

void FuzzOnUserChanged(const int32_t userId)
{
    EnableImeDataParser::GetInstance()->OnUserChanged(userId);
}

void FuzzCheckNeedSwitch(const std::string &key, SwitchInfo &switchInfo, const int32_t userId)
{
    EnableImeDataParser::GetInstance()->CheckNeedSwitch(key, switchInfo, userId);
    EnableImeDataParser::GetInstance()->CheckNeedSwitch(switchInfo, userId);
}

void FuzzGetEnableData(const std::string &key, std::vector<std::string> &enableVec, const int32_t userId)
{
    EnableImeDataParser::GetInstance()->GetEnableData(key, enableVec, userId);
}

void FuzzParseEnableIme(const std::string &valueStr, int32_t userId, std::vector<std::string> &enableVec)
{
    EnableImeDataParser::GetInstance()->ParseEnableIme(valueStr, userId, enableVec);
}

void FuzzParseEnableKeyboard(const std::string &valueStr, int32_t userId, std::vector<std::string> &enableVec)
{
    EnableImeDataParser::GetInstance()->ParseEnableKeyboard(valueStr, userId, enableVec);
}

void FuzzCheckTargetEnableName(const std::string &key, const std::string &targetName, std::string &nextIme,
                               const int32_t userId)
{
    EnableImeDataParser::GetInstance()->CheckTargetEnableName(key, targetName, nextIme, userId);
}
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return 0;
    }
    /* Run your code on data */
    const int32_t userId = static_cast<int32_t>(size);
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    SwitchInfo switchInfo;
    std::vector<std::string> enableVec;

    OHOS::FuzzInitialize(userId);
    OHOS::FuzzOnUserChanged(userId);
    OHOS::FuzzCheckNeedSwitch(fuzzedString, switchInfo, userId);
    OHOS::FuzzGetEnableData(fuzzedString, enableVec, userId);

    OHOS::FuzzParseEnableIme(fuzzedString, userId, enableVec);
    OHOS::FuzzParseEnableKeyboard(fuzzedString, userId, enableVec);
    OHOS::FuzzCheckTargetEnableName(fuzzedString, fuzzedString, fuzzedString, userId);
    return 0;
}