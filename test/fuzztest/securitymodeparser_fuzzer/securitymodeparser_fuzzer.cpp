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
#include "security_mode_parser.h"
#undef private

#include "securitymodeparser_fuzzer.h"

using namespace OHOS::MiscServices;
namespace OHOS {
void FuzzInitialize(const int32_t userId)
{
    SecurityModeParser::GetInstance()->Initialize(userId);
}

void FuzzGetSecurityMode(const std::string &bundleName, const int32_t userId)
{
    SecurityModeParser::GetInstance()->GetSecurityMode(bundleName, userId);
}

void FuzzUpdateFullModeList(const int32_t userId)
{
    SecurityModeParser::GetInstance()->UpdateFullModeList(userId);
}

void FuzzParseSecurityMode(const std::string &bundleName, const int32_t userId)
{
    SecurityModeParser::GetInstance()->ParseSecurityMode(bundleName, userId);
}

void FuzzIsFullMode(const std::string &bundleName)
{
    SecurityModeParser::GetInstance()->IsFullMode(bundleName);
}

} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    /* Run your code on data */
    const int32_t userId = static_cast<int32_t>(size);
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);

    OHOS::FuzzInitialize(userId);
    OHOS::FuzzGetSecurityMode(fuzzedString, userId);
    OHOS::FuzzUpdateFullModeList(userId);
    OHOS::FuzzParseSecurityMode(fuzzedString, userId);
    OHOS::FuzzIsFullMode(fuzzedString);
    return 0;
}
