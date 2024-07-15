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

#include "fair_lock.h"
#include "input_method_property.h"
#include "timer.h"
namespace OHOS {
namespace MiscServices {

class FullImeInfoManager {
public:
    static FullImeInfoManager &GetInstance();
    int32_t Init();    // osaccount服务启动/包浏览完成/定时刷新,手機啓動和框架服務異常都會有osaccount服务通知，手機啓動和添加用戶都會有包浏览完成通知
    int32_t Add(int32_t userId);                                   // 用户切换
    int32_t Delete(int32_t userId);                                // 用戶移除
    int32_t Add(int32_t userId, const std::string &bundleName);    // 包安装
    int32_t Delete(int32_t userId, const std::string &bundleName); // 包移除
    int32_t update(int32_t userId, const std::string &bundleName); // 包变化
    int32_t UpdateAllLabel(int32_t userId);                        // 語言变化更新
    std::vector<FullImeInfo> Get(int32_t userId);
    void Print();
    void PrintSubProp(const std::vector<SubProperty> &subProps);

private:
    FullImeInfoManager();
    ~FullImeInfoManager();
    FairLock lock_;
    std::map<int32_t, std::vector<std::shared_ptr<FullImeInfo>>> fullImeInfos_;
    Utils::Timer timer_{ "imeInfoCacheInitTimer" };
    uint32_t timerId_{ 0 };
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_FULL_IME_INFO_MANAGER_H
