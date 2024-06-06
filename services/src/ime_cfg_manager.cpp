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

#include <algorithm>
#include <fcntl.h>
#include <ios>
#include <string>

#include "file_operator.h"
#include "global.h"
namespace OHOS {
namespace MiscServices {
namespace {
constexpr const char *IME_CFG_FILE_PATH = "/data/service/el1/public/imf/ime_cfg.json";
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
    if (!FileOperator::IsExist(IME_CFG_FILE_PATH)) {
        IMSA_HILOGD("ime cfg file not find");
        return;
    }
    std::string cfg;
    bool ret = FileOperator::Read(IME_CFG_FILE_PATH, cfg);
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
    if (!FileOperator::Write(IME_CFG_FILE_PATH, content, O_CREAT | O_WRONLY | O_SYNC | O_TRUNC)) {
        IMSA_HILOGE("WriteJsonFile failed");
    }
}

bool ImeCfgManager::ParseImeCfg(const std::string &content)
{
    IMSA_HILOGD("content:%{public}s", content.c_str());
    ImePersistCfg cfg;
    auto ret = cfg.Unmarshall(content);
    if (!ret) {
        IMSA_HILOGE("Unmarshall failed");
        return false;
    }
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    imeConfigs_ = cfg.imePersistInfo;
    return true;
}

std::string ImeCfgManager::PackageImeCfg()
{
    ImePersistCfg cfg;
    {
        std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
        cfg.imePersistInfo = imeConfigs_;
    }
    std::string content;
    auto ret = cfg.Marshall(content);
    IMSA_HILOGD("ret:%{public}d, content:%{public}s, size:%{public}zu", ret, content.c_str(),
        cfg.imePersistInfo.size());
    return content;
}

void ImeCfgManager::AddImeCfg(const ImePersistInfo &cfg)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    imeConfigs_.push_back(cfg);
    WriteImeCfg();
}

void ImeCfgManager::ModifyImeCfg(const ImePersistInfo &cfg)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    auto it = std::find_if(imeConfigs_.begin(), imeConfigs_.end(),
        [&cfg](const ImePersistInfo &imeCfg) { return imeCfg.userId == cfg.userId && !cfg.currentIme.empty(); });
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

ImePersistInfo ImeCfgManager::GetImeCfg(int32_t userId)
{
    std::lock_guard<std::recursive_mutex> lock(imeCfgLock_);
    auto it = std::find_if(imeConfigs_.begin(), imeConfigs_.end(),
        [userId](const ImePersistInfo &cfg) { return cfg.userId == userId; });
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