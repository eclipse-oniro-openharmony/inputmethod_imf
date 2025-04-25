/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef SERVICES_INCLUDE_IME_CFG_MANAGER_H
#define SERVICES_INCLUDE_IME_CFG_MANAGER_H

#include <mutex>

#include "input_method_utils.h"
#include "serializable.h"
#include "ime_enabled_info_manager.h"
#include "enable_upgrade_manager.h"
namespace OHOS {
namespace MiscServices {

class ImeCfgManager {
public:
    static ImeCfgManager &GetInstance();
    void Init();
    void AddImeCfg(const ImePersistInfo &cfg);
    void ModifyImeCfg(const ImePersistInfo &cfg);
    void ModifyTempScreenLockImeCfg(int32_t userId, const std::string &ime);
    void DeleteImeCfg(int32_t userId);
    std::shared_ptr<ImeNativeCfg> GetCurrentImeCfg(int32_t userId);
    bool IsDefaultImeSet(int32_t userId);

private:
    ImeCfgManager() = default;
    ~ImeCfgManager() = default;
    void ReadImeCfg();
    void WriteImeCfg();
    ImePersistInfo GetImeCfg(int32_t userId);
    bool ParseImeCfg(const std::string &content);
    std::string PackageImeCfg();
    std::recursive_mutex imeCfgLock_;
    std::vector<ImePersistInfo> imeConfigs_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_IME_CFG_MANAGER_H
