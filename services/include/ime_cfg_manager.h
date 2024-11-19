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

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "serializable.h"
namespace OHOS {
namespace MiscServices {
struct ImePersistInfo : public Serializable {
    ImePersistInfo() = default;
    ImePersistInfo(int32_t userId, std::string currentIme, std::string currentSubName, bool isDefaultImeSet)
        : userId(userId), currentIme(std::move(currentIme)), currentSubName(std::move(currentSubName)),
        isDefaultImeSet(isDefaultImeSet){};
    static constexpr int32_t INVALID_USERID = -1;
    int32_t userId{ INVALID_USERID };
    std::string currentIme;
    std::string currentSubName;
    std::string tempScreenLockIme;
    bool isDefaultImeSet{ false };

    bool Marshal(cJSON *node) const override
    {
        auto ret = SetValue(node, GET_NAME(userId), userId);
        ret = SetValue(node, GET_NAME(currentIme), currentIme) && ret;
        ret = SetValue(node, GET_NAME(currentSubName), currentSubName) && ret;
        ret = SetValue(node, GET_NAME(tempScreenLockIme), tempScreenLockIme) && ret;
        ret = SetValue(node, GET_NAME(isDefaultImeSet), isDefaultImeSet) && ret;
        return ret;
    }
    bool Unmarshal(cJSON *node) override
    {
        auto ret = GetValue(node, GET_NAME(userId), userId);
        ret = GetValue(node, GET_NAME(currentIme), currentIme) && ret;
        ret = GetValue(node, GET_NAME(currentSubName), currentSubName) && ret;
        ret = GetValue(node, GET_NAME(tempScreenLockIme), tempScreenLockIme) && ret;
        ret = GetValue(node, GET_NAME(isDefaultImeSet), isDefaultImeSet) && ret;
        return ret;
    }
};

struct ImePersistCfg : public Serializable {
    std::vector<ImePersistInfo> imePersistInfo;
    bool Marshal(cJSON *node) const override
    {
        return SetValue(node, GET_NAME(imeCfgList), imePersistInfo);
    }
    bool Unmarshal(cJSON *node) override
    {
        return GetValue(node, GET_NAME(imeCfgList), imePersistInfo);
    }
};

struct ImeNativeCfg {
    std::string imeId;
    std::string bundleName;
    std::string subName;
    std::string extName;
};

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
