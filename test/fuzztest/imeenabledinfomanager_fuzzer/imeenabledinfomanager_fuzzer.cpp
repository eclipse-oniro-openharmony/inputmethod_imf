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

#include "imeenabledinfomanager_fuzzer.h"

#include "ime_enabled_info_manager.h"
#include "global.h"
#include <map>

using namespace OHOS::MiscServices;
namespace OHOS {
std::string GetString(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return "";
    }
    return std::string(reinterpret_cast<const char *>(data), size);
}
void FuzzInit(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    auto fuzzedString = GetString(data, size);
    std::map<int32_t, std::vector<FullImeInfo>> fullImeInfos;
    std::vector<FullImeInfo> imeInfos;
    for (size_t i = 0; i < size; i++) {
        FullImeInfo imeInfo;
        imeInfo.prop.name = fuzzedString + std::to_string(i);
        imeInfo.prop.id = std::to_string(i) + fuzzedString;
        imeInfos.push_back(imeInfo);
    }
    fullImeInfos.insert({ userId, imeInfos });
    ImeEnabledInfoManager::GetInstance().Init(fullImeInfos);
}
void FuzzSwitch(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    auto fuzzedString = GetString(data, size);
    std::vector<FullImeInfo> imeInfos;
    for (size_t i = 0; i < size; i++) {
        FullImeInfo imeInfo;
        imeInfo.prop.name = fuzzedString + std::to_string(i);
        imeInfo.prop.id = std::to_string(i) + fuzzedString;
        imeInfos.push_back(imeInfo);
    }
    ImeEnabledInfoManager::GetInstance().Switch(userId, imeInfos);
}
void FuzzDelete(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    ImeEnabledInfoManager::GetInstance().Delete(userId);
}
void FuzzAddPackage(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    auto fuzzedString = GetString(data, size);
    FullImeInfo imeInfo;
    imeInfo.prop.name = fuzzedString;
    imeInfo.prop.id = fuzzedString + "ext";
    ImeEnabledInfoManager::GetInstance().Add(userId, imeInfo);
}
void FuzzDeletePackage(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    auto fuzzedString = GetString(data, size);
    ImeEnabledInfoManager::GetInstance().Delete(userId, fuzzedString);
}
void FuzzUpdateEnabledStatus(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    auto enabledStatus = static_cast<EnabledStatus>(size);
    auto fuzzedString = GetString(data, size);
    ImeEnabledInfoManager::GetInstance().Update(userId, fuzzedString, fuzzedString + "ext", enabledStatus);
}
void FuzzGetEnabledState(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    auto fuzzedString = GetString(data, size);
    auto enabledStatus = EnabledStatus::DISABLED;
    ImeEnabledInfoManager::GetInstance().GetEnabledState(userId, fuzzedString, enabledStatus);
}
void FuzzGetEnabledStates(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    auto fuzzedString = GetString(data, size);
    std::vector<Property> props;
    for (size_t i = 0; i < size; i++) {
        Property prop;
        prop.name = fuzzedString + std::to_string(i);
        prop.id = std::to_string(i) + fuzzedString;
        props.push_back(prop);
    }
    ImeEnabledInfoManager::GetInstance().GetEnabledStates(userId, props);
}
void FuzzIsDefaultFullMode(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    auto fuzzedString = GetString(data, size);
    ImeEnabledInfoManager::GetInstance().IsDefaultFullMode(userId, fuzzedString);
}
void FuzzSetCurrentIme(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    auto fuzzedString = GetString(data, size);
    std::string imeId = fuzzedString;
    if (size % 2 == 0) {
        imeId = fuzzedString + "/" + "ext";
    }
    bool isSetByUser = false;
    if (data != nullptr && size != 0) {
        isSetByUser = static_cast<bool>(data[0] % 2); // remainder 2 can generate a random number of 0 or 1
    }
    ImeEnabledInfoManager::GetInstance().SetCurrentIme(userId, imeId, fuzzedString, isSetByUser);
}
void FuzzSetTmpIme(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    auto fuzzedString = GetString(data, size);
    std::string imeId = fuzzedString;
    if (size % 2 == 0) {
        imeId = fuzzedString + "/" + "ext";
    }
    ImeEnabledInfoManager::GetInstance().SetTmpIme(userId, imeId);
}
void FuzzGetCurrentImeCfg(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    ImeEnabledInfoManager::GetInstance().GetCurrentImeCfg(userId);
}
void FuzzIsDefaultImeSet(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    ImeEnabledInfoManager::GetInstance().IsDefaultImeSet(userId);
}
void FuzzOnFullExperienceTableChanged(const uint8_t *data, size_t size)
{
    auto userId = static_cast<int32_t>(size);
    ImeEnabledInfoManager::GetInstance().OnFullExperienceTableChanged(userId);
}
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    IMSA_HILOGI("run in.");
    OHOS::FuzzInit(data, size);
    OHOS::FuzzSwitch(data, size);
    OHOS::FuzzDelete(data, size);
    OHOS::FuzzAddPackage(data, size);
    OHOS::FuzzDeletePackage(data, size);
    OHOS::FuzzUpdateEnabledStatus(data, size);
    OHOS::FuzzGetEnabledState(data, size);
    OHOS::FuzzGetEnabledStates(data, size);
    OHOS::FuzzIsDefaultFullMode(data, size);
    OHOS::FuzzSetCurrentIme(data, size);
    OHOS::FuzzSetTmpIme(data, size);
    OHOS::FuzzGetCurrentImeCfg(data, size);
    OHOS::FuzzIsDefaultImeSet(data, size);
    OHOS::FuzzOnFullExperienceTableChanged(data, size);
    return 0;
}