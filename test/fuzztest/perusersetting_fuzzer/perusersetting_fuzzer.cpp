/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#include "perusersetting_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_ex.h>

#include "global.h"
#include "input_method_agent_stub.h"
#include "input_method_info.h"
#include "message_parcel.h"
#include "peruser_setting.h"

using namespace OHOS::MiscServices;
namespace OHOS {
    constexpr size_t THRESHOLD = 10;

    uint32_t ConvertToUint32(const uint8_t *ptr)
    {
        if (ptr == nullptr) {
            return 0;
        }
        uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
        return bigVar;
    }
    bool FuzzPerUserSetting(const uint8_t *rawData, size_t size)
    {
        std::string str(reinterpret_cast<const char *>(rawData), size);
        std::u16string imeId = Str8ToStr16(str);
        std::u16string packageName = Str8ToStr16(str);
        std::u16string key = Str8ToStr16(str);
        std::u16string value = Str8ToStr16(str);
        bool isSecurityIme = true;
        constexpr int32_t MAIN_USER_ID = 100;

        std::shared_ptr<PerUserSetting> userSetting = std::make_shared<PerUserSetting>(MAIN_USER_ID);

        userSetting->Initialize();
        userSetting->GetUserState();
        userSetting->GetCurrentInputMethod();
        userSetting->GetSecurityInputMethod();
        userSetting->GetNextInputMethod();
        userSetting->GetInputMethodSetting();
        userSetting->GetInputMethodProperty(imeId);
        userSetting->OnPackageAdded(packageName, isSecurityIme);
        userSetting->OnPackageRemoved(packageName, isSecurityIme);
        userSetting->OnSettingChanged(key, value);
        userSetting->OnAdvanceToNext();
        userSetting->OnUserLocked();

        return true;
    }
}
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return 0;
    }
    /* Run your code on data */
    OHOS::FuzzPerUserSetting(data, size);
    return 0;
}