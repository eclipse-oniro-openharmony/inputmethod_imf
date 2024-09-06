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

#include "settingsdatautils_fuzzer.h"

#include "settings_data_utils.h"

using namespace OHOS::MiscServices;
namespace OHOS {
void FuzzGetToken()
{
    SettingsDataUtils::GetInstance()->GetToken();
}

void FuzzDataShareHelper()
{
    auto helper = SettingsDataUtils::GetInstance()->CreateDataShareHelper();
    SettingsDataUtils::GetInstance()->ReleaseDataShareHelper(helper);
}

void FuzzCreateAndRegisterObserver(const std::string &key, SettingsDataObserver::CallbackFunc func)
{
    SettingsDataUtils::GetInstance()->CreateAndRegisterObserver(key, func);
}

void FuzzRegisterObserver(const std::string &key, SettingsDataObserver::CallbackFunc &func)
{
    sptr<SettingsDataObserver> observer = new SettingsDataObserver(key, func);
    SettingsDataUtils::GetInstance()->RegisterObserver(observer);
}

void FuzzUnregisterObserver(const std::string &key, SettingsDataObserver::CallbackFunc &func)
{
    sptr<SettingsDataObserver> observer = new SettingsDataObserver(key, func);
    SettingsDataUtils::GetInstance()->UnregisterObserver(observer);
}

void FuzzGenerateTargetUri(const std::string &key)
{
    SettingsDataUtils::GetInstance()->GenerateTargetUri(key);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    std::string fuzzedString(reinterpret_cast<const char *>(data), size);
    SettingsDataObserver::CallbackFunc func;

    OHOS::FuzzGetToken();
    OHOS::FuzzDataShareHelper();
    OHOS::FuzzCreateAndRegisterObserver(fuzzedString, func);
    OHOS::FuzzRegisterObserver(fuzzedString, func);
    OHOS::FuzzUnregisterObserver(fuzzedString, func);
    OHOS::FuzzGenerateTargetUri(fuzzedString);
    return 0;
}