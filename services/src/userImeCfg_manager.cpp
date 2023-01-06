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

#include "userImeCfg_manager.h"

#include <fstream>
#include "file_ex.h"

namespace OHOS {
namespace MiscServices {
namespace {
const std::string USER_IME_CFG_JSON_PATH = "/system/etc/imf/currentIme.json";
}

void UserImeCfgManager::Init()
{
    nlohmann::json userImeCfgJson;
    if (!LoadUserImeCfg(USER_IME_CFG_JSON_PATH, userImeCfgJson)) {
        return;
    }
    for (auto iter = userImeCfgJson.begin(); iter != userImeCfgJson.end();) {
        for (auto &[userId, currentImeCfg] : iter.value().items()) {
            for (auto &[key, value] : currentImeCfg.value().items()) {
                if (key == "currentIme") {
                    userCurrentIme_.insert({ userId.stoi(), value });
                }
            }
        }
    }
}

void UserImeCfgManager::AddCurrentIme(int32_t userId, const std::string &currentImeCfg)
{
    //TODO 修改json文件处理

    userCurrentIme_.insert_or_assign(userId, currentImeCfg);
}

void UserImeCfgManager::ModifyCurrentIme(int32_t userId, const std::string &currentImeCfg)
{
    //TODO 修改json文件处理

    userCurrentIme_.insert_or_assign(userId, currentImeCfg);
}

void UserImeCfgManager::DeleteCurrentIme(int32_t userId)
{
    IMSA_HILOGD("UserImeCfgManager::start");
    //TODO 修改json文件处理

    auto it = userCurrentIme_.find(userId);
    if (it != userCurrentIme_.end()) {
        userCurrentIme_.erase(it);
    }
}

std::string UserImeCfgManager::GetCurrentIme(int32_t userId)
{
    IMSA_HILOGD("UserImeCfgManager::start");
    auto it = userCurrentIme_.find(userId);
    if (it == userCurrentIme_.end()) {
        IMSA_HILOGE("UserImeCfgManager::CurrentIme not found");
        return "";
    }
    return it->second;
}

sptr<UserImeCfgManager> UserImeCfgManager::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (!instance_) {
            IMSA_HILOGI("UserImeCfgManager::GetInstance instance_ is nullptr");
            instance_ = new UserImeCfgManager();
        }
    }
    return instance_;
}

bool UserImeCfgManager::LoadUserImeCfg(const std::string& filePath, nlohmann::json &userImeCfgJson)
{
    std::ifstream ifs(filePath.c_str());
    if (!ifs.good()) {
        IMSA_HILOGE("load json file failed");
        return false;
    }
    userImeCfgJson = nlohmann::json::parse(ifs, nullptr, false);
    if (userImeCfgJson.is_discarded()) {
        IMSA_HILOGE("parse failed");
        return false;
    }
    IMSA_HILOGI("userImeCfg json %{public}s", userImeCfgJson.dump().c_str());
    return true;
}
} // namespace MiscServices
} // namespace OHOS