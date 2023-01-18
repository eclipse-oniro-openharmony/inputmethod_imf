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
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <string>

#include "file_ex.h"
#include "global.h"
#include "parameter.h"
namespace OHOS {
namespace MiscServices {
namespace {
constexpr const char *IME_CFG_DIR = "/data/service/el1/public/imf/ime_cfg";
constexpr const char *IME_CFG_FILE_PATH = "/data/service/el1/public/imf/ime_cfg/ime_cfg.json";
static constexpr const char *DEFAULT_IME_KEY = "persist.sys.default_ime";
static constexpr int32_t CONFIG_LEN = 128;
static constexpr int32_t SUCCESS = 0;
using json = nlohmann::json;
} // namespace
ImeCfgManager &ImeCfgManager::GetInstance()
{
    static ImeCfgManager instance;
    return instance;
}

void ImeCfgManager::Init()
{
    std::string path(IME_CFG_DIR);
    if (CreateCachePath(path, S_IRWXU) != SUCCESS) {
        IMSA_HILOGE("CreateCachePath failed");
        return;
    }
    ReadImeCfgFile();
}

void ImeCfgManager::ReadImeCfgFile()
{
    json jsonConfigs;
    bool ret = ReadCacheFile(IME_CFG_FILE_PATH, jsonConfigs);
    if (!ret) {
        IMSA_HILOGE("ReadJsonFile failed");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    FromJson(jsonConfigs, imeConfigs_);
}

void ImeCfgManager::WriteImeCfgFile()
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    json jsonConfigs;
    ToJson(jsonConfigs, imeConfigs_);
    if (!WriteCacheFile(IME_CFG_FILE_PATH, jsonConfigs)) {
        IMSA_HILOGE("WriteJsonFile failed");
    }
}

void ImeCfgManager::AddImeCfg(const ImeCfg &cfg)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    imeConfigs_.push_back(cfg);
    WriteImeCfgFile();
}

void ImeCfgManager::ModifyImeCfg(const ImeCfg &cfg)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    for (auto &imeConfig : imeConfigs_) {
        if (imeConfig.userId == cfg.userId && !cfg.currentIme.empty()) {
            imeConfig.currentIme = cfg.currentIme;
        }
    }
    WriteImeCfgFile();
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
    WriteImeCfgFile();
}

ImeCfg ImeCfgManager::GetImeCfg(int32_t userId)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    for (auto &cfg : imeConfigs_) {
        if (cfg.userId == userId) {
            return cfg;
        }
    }
    return {};
}

std::string ImeCfgManager::GetDefaultIme()
{
    char value[CONFIG_LEN] = { 0 };
    auto code = GetParameter(DEFAULT_IME_KEY, "", value, CONFIG_LEN);
    return code > 0 ? value : "";
}

void ImeCfgManager::FromJson(const json &jsonConfigs, std::vector<ImeCfg> &configs)
{
    for (auto &jsonCfg : jsonConfigs["imeCfg_list"]) {
        ImeCfg cfg;
        FromJson(jsonCfg, cfg);
        configs.push_back(cfg);
    }
}

void ImeCfgManager::ToJson(json &jsonConfigs, const std::vector<ImeCfg> &configs)
{
    for (auto &cfg : configs) {
        json jsonCfg;
        ToJson(jsonCfg, cfg);
        jsonConfigs["imeCfg_list"].push_back(jsonCfg);
    }
}

int32_t ImeCfgManager::CreateCachePath(std::string &path, mode_t pathMode)
{
    if (IsCachePathExit(path)) {
        IMSA_HILOGI("dir: %{public}s exist", path.c_str());
        return SUCCESS;
    }
    return mkdir(path.c_str(), pathMode);
}

bool ImeCfgManager::IsCachePathExit(std::string &path)
{
    return access(path.c_str(), F_OK) == SUCCESS;
}

bool ImeCfgManager::ReadCacheFile(const std::string &path, json &jsonCfg)
{
    std::ifstream jsonFs(path);
    if (!jsonFs.is_open()) {
        IMSA_HILOGE("file read open failed");
        return false;
    }
    jsonCfg = json::parse(jsonFs, nullptr, false);
    if (jsonCfg.is_null() || jsonCfg.is_discarded()) {
        IMSA_HILOGE("json parse failed");
        return false;
    }
    IMSA_HILOGI("json: %{public}s", jsonCfg.dump().c_str());
    return true;
}
bool ImeCfgManager::WriteCacheFile(const std::string &path, const json &jsonCfg)
{
    std::ofstream jsonFs(path);
    if (!jsonFs.is_open()) {
        IMSA_HILOGE("file write open failed");
        return false;
    }
    constexpr int32_t width = 2;
    jsonFs << std::setw(width) << jsonCfg << std::endl;
    return true;
}
} // namespace MiscServices
} // namespace OHOS