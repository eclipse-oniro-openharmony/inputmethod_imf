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

#include "enableupgrademanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#define private public
#define protected public
#include "enable_upgrade_manager.h"
#include "full_ime_info_manager.h"
#undef private

using namespace OHOS::MiscServices;
namespace OHOS {

void FuzzAgentStub(const uint8_t *data, size_t size)
{
    static std::vector<FullImeInfo> imeInfos;
    static std::set<std::string> bundleNames;
    static std::vector<std::string> bundleNamesVec;
    static std::vector<ImeEnabledInfo> enabledInfos;
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto fuzzInt32 = static_cast<int32_t>(size);
    auto fuzzUint32 = static_cast<uint32_t>(size);
    auto fuzzedBool = static_cast<bool>(data[0] % 2);
    int32_t userId = fuzzInt32;
    FullImeInfo imeInfo = { .isNewIme = fuzzedBool, .tokenId = fuzzUint32, .appId = fuzzedString,
        .versionCode = fuzzUint32 };
    imeInfos.push_back(imeInfo);
    bundleNames.insert(fuzzedString);
    bundleNamesVec.push_back(fuzzedString);
    ImeEnabledInfo imeEnabeleInfo;
    imeEnabeleInfo.bundleName = fuzzedString;
    imeEnabeleInfo.extensionName = fuzzedString;
    imeEnabeleInfo.enabledStatus = static_cast<EnabledStatus>(fuzzInt32);
    imeEnabeleInfo.stateUpdateTime = fuzzedString;

    ImeEnabledCfg newEnabledCfg;
    newEnabledCfg.version = fuzzedString;
    newEnabledCfg.enabledInfos = enabledInfos;
    ImePersistInfo persisInfo;
    persisInfo.userId = userId;
    persisInfo.currentIme = fuzzedString;
    persisInfo.currentSubName = fuzzedString;
    persisInfo.tempScreenLockIme = fuzzedString;
    persisInfo.isDefaultImeSet = fuzzedBool;
    EnabledStatus initStatus = EnabledStatus::DISABLED;
    EnableUpgradeManager::GetInstance().Upgrade(userId, imeInfos);
    EnableUpgradeManager::GetInstance().GetEnabledTable(userId, bundleNames);
    EnableUpgradeManager::GetInstance().GetFullExperienceTable(userId, bundleNames);
    EnableUpgradeManager::GetInstance().MergeTwoTable(userId, enabledInfos);
    EnableUpgradeManager::GetInstance().PaddedByBundleMgr(userId, imeInfos, enabledInfos);
    EnableUpgradeManager::GetInstance().UpdateGlobalEnabledTable(userId, newEnabledCfg);
    EnableUpgradeManager::GetInstance().GetGlobalTableUserId(fuzzedString);
    EnableUpgradeManager::GetInstance().GenerateGlobalContent(userId, bundleNamesVec);
    EnableUpgradeManager::GetInstance().GetImePersistCfg(userId, persisInfo);
    EnableUpgradeManager::GetInstance().PaddedByImePersistCfg(userId, enabledInfos);
    EnableUpgradeManager::GetInstance().ComputeEnabledStatus(fuzzedString, initStatus);
}

void FuzzFullImeInfo(const uint8_t *data, size_t size)
{
    auto fuzzInt32 = static_cast<int32_t>(size);
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    auto fuzzedBool = static_cast<bool>(data[0] % 2);
    auto fuzzUint32 = static_cast<uint32_t>(size);
    static std::vector<FullImeInfo> infos;
    FullImeInfo imeInfo = { .isNewIme = fuzzedBool, .tokenId = fuzzUint32, .appId = fuzzedString,
        .versionCode = fuzzUint32 };
    infos.push_back(imeInfo);
    FullImeInfoManager::GetInstance().Delete(fuzzInt32);
    FullImeInfoManager::GetInstance().Has(fuzzInt32, fuzzedString);
    FullImeInfoManager::GetInstance().DeletePackage(fuzzInt32, fuzzedString);
}
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzAgentStub(data, size);
    OHOS::FuzzFullImeInfo(data, size);
    return 0;
}