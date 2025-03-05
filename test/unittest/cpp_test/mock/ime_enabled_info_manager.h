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

#ifndef IME_ENABLED_INFO_MANAGER_H
#define IME_ENABLED_INFO_MANAGER_H

#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "event_handler.h"
#include "input_method_property.h"
#include "input_method_status.h"

namespace OHOS {
namespace MiscServices {
// old ime enabled cfg
struct EnabledImeCfg {
};
// old ime full experience cfg
struct SecurityModeCfg {
};

struct ImeEnabledInfo {
};
struct ImeEnabledCfg {
};
using EnabledStatusChangedHandler =
    std::function<void(int32_t userId, const std::string &bundleName, EnabledStatus oldStatus)>;
class ImeEnabledInfoManager {
public:
    static constexpr const char *ENABLE_IME = "settings.inputmethod.enable_ime";
    static constexpr const char *SECURITY_MODE = "settings.inputmethod.full_experience";
    static ImeEnabledInfoManager &GetInstance();
    void SetEnabledStatusChangedHandler(EnabledStatusChangedHandler handler);
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler);
    int32_t Init(const std::map<int32_t, std::vector<FullImeInfo>> &fullImeInfos);
    int32_t Add(int32_t userId, const std::vector<FullImeInfo> &imeInfos);
    int32_t Delete(int32_t userId);
    int32_t Add(int32_t userId, const FullImeInfo &imeInfo);
    int32_t Delete(int32_t userId, const std::string &bundleName);
    int32_t Update(
        int32_t userId, const std::string &bundleName, const std::string &extensionName, EnabledStatus status);
    int32_t GetEnabledState(int32_t userId, const std::string &bundleName, EnabledStatus &status);
    int32_t GetEnabledStates(int32_t userId, std::vector<Property> &props);
    bool IsDefaultFullMode(int32_t userId, const std::string &bundleName);
    void OnFullExperienceTableChanged(int32_t userId); // add for compatibility
private:
    ImeEnabledInfoManager() = default;
    ~ImeEnabledInfoManager();
};
} // namespace MiscServices
} // namespace OHOS

#endif // IME_ENABLED_INFO_MANAGER_H