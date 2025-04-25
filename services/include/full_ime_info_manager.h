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

#include "event_handler.h"
#include "input_method_property.h"
#include "timer.h"
namespace OHOS {
namespace MiscServices {

class FullImeInfoManager {
public:
    static FullImeInfoManager &GetInstance();
    int32_t RegularInit();
    int32_t Init();                                                // regular Init/boot complete/data share ready
    int32_t Switch(int32_t userId);                                // user switched
    int32_t Update();                                              // language change
    int32_t Delete(int32_t userId);                                // user removed
    int32_t Add(int32_t userId, const std::string &bundleName);    // package added
    int32_t Delete(int32_t userId, const std::string &bundleName); // package removed
    int32_t Update(int32_t userId, const std::string &bundleName); // package changed
    std::string Get(int32_t userId, uint32_t tokenId);
    bool Get(int32_t userId, const std::string &bundleName, FullImeInfo &fullImeInfo);
    bool Has(int32_t userId, const std::string &bundleName);
    int32_t Get(int32_t userId, std::vector<Property> &props);
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler);

private:
    FullImeInfoManager();
    ~FullImeInfoManager();
    int32_t AddIfNoCache(int32_t userId, std::vector<FullImeInfo> &infos);
    int32_t Add(int32_t userId, std::vector<FullImeInfo> &infos);
    int32_t AddPackage(int32_t userId, const std::string &bundleName, FullImeInfo &info);
    void PostEnableTask(const std::function<void()> &task, const std::string &taskName);
    std::mutex lock_;
    std::map<int32_t, std::vector<FullImeInfo>> fullImeInfos_;
    Utils::Timer timer_{ "imeInfoCacheInitTimer" };
    uint32_t timerId_{ 0 };
    std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_{ nullptr };
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_FULL_IME_INFO_MANAGER_H
