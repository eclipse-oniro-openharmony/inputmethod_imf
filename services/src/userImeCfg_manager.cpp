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
#include "global.h"
namespace OHOS {
namespace MiscServices {
namespace {
const std::string USER_IME_CFG_JSON_PATH = "/system/etc/imf/currentIme.json";
}
std::mutex UserImeCfgManager::instanceLock_;
sptr<UserImeCfgManager> UserImeCfgManager::instance_;

void UserImeCfgManager::Init()
{
    //TODO 加载json文件，存入userCurrentIme_
}

void UserImeCfgManager::AddCurrentIme(int32_t userId, const std::string &currentImeCfg)
{
    //TODO json文件添加对应userId的value

    userCurrentIme_.insert_or_assign(userId, currentImeCfg);
}

void UserImeCfgManager::ModifyCurrentIme(int32_t userId, const std::string &currentImeCfg)
{
    //TODO json文件修改对应userId的value

    userCurrentIme_.insert_or_assign(userId, currentImeCfg);
}

void UserImeCfgManager::DeleteCurrentIme(int32_t userId)
{
    //TODO json文件删除对应userId的value

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
        IMSA_HILOGD("UserImeCfgManager::CurrentIme not found");
        return "";
    }
    return it->second;
}

sptr<UserImeCfgManager> UserImeCfgManager::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (!instance_) {
            IMSA_HILOGD("UserImeCfgManager::GetInstance instance_ is nullptr");
            instance_ = new UserImeCfgManager();
        }
    }
    return instance_;
}
} // namespace MiscServices
} // namespace OHOS