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

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <ios>
#include <string>

#include "file_ex.h"
#include "global.h"
namespace OHOS {
namespace MiscServices {
namespace {
constexpr const char *IME_CFG_DIR = "/data/service/el1/public/imf/ime_cfg";
constexpr const char *IME_CFG_FILE_PATH = "/data/service/el1/public/imf/ime_cfg/ime_cfg.json";
static constexpr int32_t SUCCESS = 0;
static constexpr uint32_t MAX_FILE_LENGTH = 10000;
using json = nlohmann::json;
} // namespace
ImeCfgManager &ImeCfgManager::GetInstance()
{
    static ImeCfgManager instance;
    return instance;
}

void ImeCfgManager::Init()
{
    std::string filePath(IME_CFG_FILE_PATH);
    if (!IsCachePathExit(filePath)) {
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
    std::string cachePath(IME_CFG_DIR);
    if (CreateCachePath(cachePath, S_IRWXU) != SUCCESS) {
        IMSA_HILOGE("CreateCachePath failed");
        return;
    }
    if (!WriteCacheFile(IME_CFG_FILE_PATH, jsonConfigs)) {
        IMSA_HILOGE("WriteJsonFile failed");
    }
}

void ImeCfgManager::AddImeCfg(const ImePersistCfg &cfg)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    imeConfigs_.push_back(cfg);
    WriteImeCfgFile();
}

void ImeCfgManager::ModifyImeCfg(const ImePersistCfg &cfg)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    auto it = std::find_if(imeConfigs_.begin(), imeConfigs_.end(),
        [cfg](const ImePersistCfg &imeCfg) { return imeCfg.userId == cfg.userId && !cfg.currentIme.empty(); });
    if (it != imeConfigs_.end()) {
        *it = cfg;
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

ImePersistCfg ImeCfgManager::GetImeCfg(int32_t userId)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    auto it = std::find_if(
        imeConfigs_.begin(), imeConfigs_.end(), [userId](const ImePersistCfg &cfg) { return cfg.userId == userId; });
    if (it != imeConfigs_.end()) {
        return *it;
    }
    return {};
}

std::shared_ptr<ImeNativeCfg> ImeCfgManager::GetCurrentImeCfg(int32_t userId)
{
    auto cfg = GetImeCfg(userId);
    ImeNativeCfg info;
    info.subName = cfg.currentSubName;
    info.imeId = cfg.currentIme;
    auto pos = info.imeId.find('/');
    if (pos != std::string::npos && pos + 1 < info.imeId.size()) {
        info.bundleName = info.imeId.substr(0, pos);
        info.extName = info.imeId.substr(pos + 1);
    }
    return std::make_shared<ImeNativeCfg>(info);
}

void ImeCfgManager::FromJson(const json &jsonConfigs, std::vector<ImePersistCfg> &configs)
{
    if (!jsonConfigs.contains("imeCfg_list") || !jsonConfigs["imeCfg_list"].is_array()) {
        IMSA_HILOGE("imeCfg_list not find or abnormal");
        return;
    }
    for (auto &jsonCfg : jsonConfigs["imeCfg_list"]) {
        ImePersistCfg cfg;
        FromJson(jsonCfg, cfg);
        configs.push_back(cfg);
    }
}

void ImeCfgManager::ToJson(json &jsonConfigs, const std::vector<ImePersistCfg> &configs)
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
    auto fd = open(path.c_str(), O_RDONLY);
    if (fd <= 0) {
        IMSA_HILOGE("file open failed, fd: %{public}d", fd);
        return false;
    }
    char cfg[MAX_FILE_LENGTH] = { 0 };
    auto ret = read(fd, cfg, MAX_FILE_LENGTH);
    if (ret <= 0) {
        IMSA_HILOGE("file read failed, ret: %{public}zd", ret);
        close(fd);
        return false;
    }
    close(fd);

    if (cfg[0] == '\0') {
        IMSA_HILOGE("imeCfg is empty");
        return false;
    }
    jsonCfg = json::parse(cfg, nullptr, false);
    if (jsonCfg.is_null() || jsonCfg.is_discarded()) {
        IMSA_HILOGE("json parse failed.");
        return false;
    }
    IMSA_HILOGD("imeCfg json: %{public}s", jsonCfg.dump().c_str());
    return true;
}

bool ImeCfgManager::WriteCacheFile(const std::string &path, const json &jsonCfg)
{
    std::string cfg = jsonCfg.dump();
    if (cfg.empty()) {
        IMSA_HILOGE("imeCfg is empty");
        return false;
    }
    IMSA_HILOGD("imeCfg json: %{public}s", cfg.c_str());
    auto fd = open(path.c_str(), O_CREAT | O_WRONLY | O_SYNC | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd <= 0) {
        IMSA_HILOGE("file open failed, fd: %{public}d", fd);
        return false;
    }
    auto ret = write(fd, cfg.c_str(), cfg.size());
    if (ret <= 0) {
        IMSA_HILOGE("file write failed, ret: %{public}zd", ret);
        close(fd);
        return false;
    }
    close(fd);
    return true;
}
} // namespace MiscServices
} // namespace OHOS