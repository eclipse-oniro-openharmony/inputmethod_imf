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

#include "ime_cfg_manager.h"

#include <sys/stat.h>

#include <fstream>

#include "../../utils/include/file_manager.h"
#include "file_ex.h"
#include "global.h"
namespace OHOS {
namespace MiscServices {
namespace {
constexpr const char *IME_CFG_DIR = "/data/service/el1/public/imf/ime_cfg";
constexpr const char *IME_CFG_FILENAME = "ime_cfg.json";
constexpr const char *IME_CFG_FILE_PATH = "/data/service/el1/public/imf/ime_cfg/ime_cfg.json";
constexpr const int32_t SUCCESS = 0;
using json = nlohmann::json;
} // namespace
ImeCfgManager &ImeCfgManager::GetInstance()
{
    static ImeCfgManager instance;
    return instance;
}

void ImeCfgManager::Init()
{
    if (!CreateCfgFile()) {
        return;
    }
    ReadCfgFile();
}

bool ImeCfgManager::CreateCfgFile()
{
    FileInfo info{ IME_CFG_DIR, IME_CFG_FILENAME, S_IRWXU, S_IRWXU };
    auto errCode = FileManager::GetInstance().CreateCacheFile(info);
    if (errCode != SUCCESS) {
        IMSA_HILOGE("ImeCfgManager::CreateCacheFile failed");
        return false;
    }
    return true;
}
void ImeCfgManager::ReadCfgFile()
{
    json jsonConfigs;
    bool ret = FileManager::GetInstance().ReadJsonFile(IME_CFG_FILE_PATH, jsonConfigs);
    if (!ret) {
        IMSA_HILOGE("ImeCfgManager::ReadJsonFile failed");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    from_json(jsonConfigs, imeConfigs_);
}

void ImeCfgManager::WriteCfgFile()
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    json jsonConfigs;
    to_json(jsonConfigs, imeConfigs_);
    if (!FileManager::GetInstance().WriteJsonFile(IME_CFG_FILE_PATH, jsonConfigs)) {
        IMSA_HILOGE("ImeCfgManager::WriteJsonFile failed");
    }
}

void ImeCfgManager::AddImeCfg(const ImeCfg &cfg)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    imeConfigs_.push_back(cfg);
    WriteCfgFile();
}

void ImeCfgManager::ModifyImeCfg(const ImeCfg &cfg)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    for (auto &imeConfig : imeConfigs_) {
        if (imeConfig.userId == cfg.userId && !cfg.currentIme.empty()) {
            imeConfig.currentIme = cfg.currentIme;
        }
    }
    WriteCfgFile();
}

void ImeCfgManager::DeleteImeCfg(int32_t userId)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    for (auto iter = imeConfigs_.begin(); iter != imeConfigs_.end(); iter++) {
        if (iter->userId == userId) {
            imeConfigs_.erase(iter);
            break;
        }
    }
    WriteCfgFile();
}

ImeCfg ImeCfgManager::GetImeCfg(int32_t userId)
{
    IMSA_HILOGD("ImeCfgManager::start");
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    for (auto &cfg : imeConfigs_) {
        if (cfg.userId == userId) {
            return cfg;
        }
    }
    return {};
}

void ImeCfgManager::from_json(const json &jsonConfigs, std::vector<ImeCfg> &configs)
{
    for (auto &jsonCfg : jsonConfigs["imeCfg_list"]) {
        ImeCfg cfg;
        from_json(jsonCfg, cfg);
        configs.push_back(cfg);
    }
}

void ImeCfgManager::to_json(json &jsonConfigs, const std::vector<ImeCfg> &configs)
{
    for (auto &cfg : configs) {
        json jsonCfg;
        to_json(jsonCfg, cfg);
        jsonConfigs["imeCfg_list"].push_back(jsonCfg);
    }
}
} // namespace MiscServices
} // namespace OHOS