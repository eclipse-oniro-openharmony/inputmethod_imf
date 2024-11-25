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

#ifndef SERVICES_INCLUDE_FULL_IME_INFO_MANAGER_H
#define SERVICES_INCLUDE_FULL_IME_INFO_MANAGER_H

#include <chrono>
#include <map>

#include "global.h"
#include "input_method_property.h"
#include "timer.h"
namespace OHOS {
namespace MiscServices {

class FullImeInfoManager {
public:
    static FullImeInfoManager &GetInstance();
    int32_t Init();                                             // osAccount start/bundle scan finished/regular update
    int32_t Add(int32_t userId);                                // user switched
    int32_t Update();                                           // language change
    int32_t Delete(int32_t userId);                             // user removed
    int32_t Add(int32_t userId, const std::string &bundleName); // package added
    int32_t Delete(int32_t userId, const std::string &bundleName); // package removed
    int32_t Update(int32_t userId, const std::string &bundleName); // package changed
    std::vector<FullImeInfo> Get(int32_t userId);
    std::string Get(int32_t userId, uint32_t tokenId);
    bool Get(const std::string &bundleName, int32_t userId, FullImeInfo &fullImeInfo);
    bool Has(int32_t userId, const std::string &bundleName);

private:
    FullImeInfoManager();
    ~FullImeInfoManager();
    std::mutex lock_;
    std::map<int32_t, std::vector<FullImeInfo>> fullImeInfos_;
    Utils::Timer timer_ { "imeInfoCacheInitTimer" };
    uint32_t timerId_ { 0 };
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_FULL_IME_INFO_MANAGER_H
