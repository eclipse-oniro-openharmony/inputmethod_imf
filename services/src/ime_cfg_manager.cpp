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
#include "file_operator.h"
#include "ime_enabled_info_manager.h"
namespace OHOS {
namespace MiscServices {
namespace {
constexpr const char *IME_CFG_FILE_PATH = "/data/service/el1/public/imf/ime_cfg.json";
} // namespace
std::shared_ptr<AppExecFwk::EventHandler> ImeCfgManager::serviceHandler_ = nullptr;
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
        IMSA_HILOGD("ime cfg file not found.");
        return;
    }
    std::string cfg;
    bool ret = FileOperator::Read(IME_CFG_FILE_PATH, cfg);
    if (!ret) {
        IMSA_HILOGE("failed to ReadJsonFile!");
        return;
    }
    ParseImeCfg(cfg);
}

void ImeCfgManager::WriteImeCfg()
{
    auto content = PackageImeCfg();
    if (content.empty()) {
        IMSA_HILOGE("failed to Package imeCfg!");
        return;
    }
    if (serviceHandler_ == nullptr) {
        IMSA_HILOGE("serviceHandler_ is nullptr!");
        return;
    }
    auto task = [content]() {
        IMSA_HILOGD("start WriteJsonFile!");
        if (!FileOperator::Write(IME_CFG_FILE_PATH, content, O_CREAT | O_WRONLY | O_SYNC | O_TRUNC)) {
            IMSA_HILOGE("failed to WriteJsonFile!");
        }
    };
    serviceHandler_->PostTask(task, "WriteImeCfg", 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

bool ImeCfgManager::ParseImeCfg(const std::string &content)
{
    IMSA_HILOGD("content: %{public}s", content.c_str());
    ImePersistCfg cfg;
    auto ret = cfg.Unmarshall(content);
    if (!ret) {
        IMSA_HILOGE("Unmarshall failed!");
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
    IMSA_HILOGD("ret: %{public}d, content: %{public}s, size: %{public}zu", ret, content.c_str(),
        cfg.imePersistInfo.size());
    return content;
}

void ImeCfgManager::AddImeCfg(const ImePersistInfo &cfg)
{
    ImeEnabledInfoManager::GetInstance().SetCurrentIme(
        cfg.userId, cfg.currentIme, cfg.currentSubName, cfg.isDefaultImeSet);
}

void ImeCfgManager::ModifyImeCfg(const ImePersistInfo &cfg)
{
    ImeEnabledInfoManager::GetInstance().SetCurrentIme(
        cfg.userId, cfg.currentIme, cfg.currentSubName, cfg.isDefaultImeSet);
}

void ImeCfgManager::ModifyTempScreenLockImeCfg(int32_t userId, const std::string &ime)
{
    ImeEnabledInfoManager::GetInstance().SetTmpIme(userId, ime);
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
    return ImeEnabledInfoManager::GetInstance().GetCurrentImeCfg(userId);
}

bool ImeCfgManager::IsDefaultImeSet(int32_t userId)
{
    IMSA_HILOGI("ImeCfgManager::IsDefaultImeSet enter.");
    auto ret = ImeEnabledInfoManager::GetInstance().IsDefaultImeSet(userId);
    IMSA_HILOGI("isDefaultImeSet: %{public}d", ret);
    return ret;
}

void ImeCfgManager::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler)
{
    serviceHandler_ = eventHandler;
}
} // namespace MiscServices
} // namespace OHOS