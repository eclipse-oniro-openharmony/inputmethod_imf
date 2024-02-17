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

#include <algorithm>
#include <ios>
#include <string>

#include "file_operator.h"
#include "global.h"
namespace OHOS {
namespace MiscServices {
namespace {
constexpr const char *IME_CFG_DIR = "/data/service/el1/public/imf/ime_cfg";
constexpr const char *IME_CFG_FILE_PATH = "/data/service/el1/public/imf/ime_cfg/ime_cfg.json";
static constexpr int32_t SUCCESS = 0;
} // namespace
ImeCfgManager &ImeCfgManager::GetInstance()
{
    static ImeCfgManager instance;
    return instance;
}

void ImeCfgManager::Init()
{
    ReadImeCfg();
}

void ImeCfgManager::ReadImeCfg()
{
    std::string path(IME_CFG_FILE_PATH);
    if (!FileOperator::IsExist(path)) {
        IMSA_HILOGD("ime cfg file not find");
        return;
    }
    std::string cfg;
    bool ret = FileOperator::Read(path, cfg);
    if (!ret) {
        IMSA_HILOGE("ReadJsonFile failed");
        return;
    }
    ParseImeCfg(cfg);
}

void ImeCfgManager::WriteImeCfg()
{
    auto content = PackageImeCfg();
    if (content.empty()) {
        IMSA_HILOGE("Package imeCfg failed");
        return;
    }
    std::string path(IME_CFG_DIR);
    if (!FileOperator::IsExist(path)) {
        auto ret = FileOperator::Create(S_IRWXU, path);
        if (ret != SUCCESS) {
            IMSA_HILOGE("ime cfg dir create failed");
            return;
        }
    }
    if (!FileOperator::Write(O_CREAT | O_WRONLY | O_SYNC | O_TRUNC, IME_CFG_FILE_PATH, content)) {
        IMSA_HILOGE("WriteJsonFile failed");
    }
}

bool ImeCfgManager::ParseImeCfg(const std::string &content)
{
    ImePersistInfo info;
    auto ret = info.Unmarshall(content);
    if (!ret) {
        return false;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
        imeConfigs_ = info.imePersistCfg;
    }
    return true;
}

std::string ImeCfgManager::PackageImeCfg()
{
    ImePersistInfo info;
    {
        std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
        info.imePersistCfg = imeConfigs_;
    }
    return Serializable::Marshall(info);
}

void ImeCfgManager::AddImeCfg(const ImePersistCfg &cfg)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    imeConfigs_.push_back(cfg);
    WriteImeCfg();
}

void ImeCfgManager::ModifyImeCfg(const ImePersistCfg &cfg)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    auto it = std::find_if(imeConfigs_.begin(), imeConfigs_.end(),
        [cfg](const ImePersistCfg &imeCfg) { return imeCfg.userId == cfg.userId && !cfg.currentIme.empty(); });
    if (it != imeConfigs_.end()) {
        *it = cfg;
    }

    WriteImeCfg();
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
    WriteImeCfg();
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
} // namespace MiscServices
} // namespace OHOS