/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "full_ime_info_manager.h"

#include "common_timer_errors.h"
#include "ime_enabled_info_manager.h"
#include "ime_info_inquirer.h"
#include "inputmethod_message_handler.h"
#include "message.h"
namespace OHOS {
namespace MiscServices {
constexpr uint32_t TIMER_TASK_INTERNAL = 1 * 60 * 60 * 1000; // updated hourly
FullImeInfoManager::~FullImeInfoManager()
{
    timer_.Unregister(timerId_);
    timer_.Shutdown();
    fullImeInfos_.clear();
}

FullImeInfoManager::FullImeInfoManager()
{
    uint32_t ret = timer_.Setup();
    if (ret != Utils::TIMER_ERR_OK) {
        IMSA_HILOGE("failed to create timer!");
        return;
    }
    timerId_ = timer_.Register(
        []() {
            Message *msg = new (std::nothrow) Message(MessageID::MSG_ID_REGULAR_UPDATE_IME_INFO, nullptr);
            if (msg != nullptr) {
                MessageHandler::Instance()->SendMessage(msg);
            }
        },
        TIMER_TASK_INTERNAL, false);
}

FullImeInfoManager &FullImeInfoManager::GetInstance()
{
    static FullImeInfoManager instance;
    return instance;
}

int32_t FullImeInfoManager::RegularInit()
{
    std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> fullImeInfos;
    auto ret = ImeInfoInquirer::GetInstance().QueryFullImeInfo(fullImeInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGW("failed to QueryFullImeInfo, ret:%{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> lock(lock_);
    fullImeInfos_.clear();
    for (const auto &infos : fullImeInfos) {
        fullImeInfos_.insert_or_assign(infos.first, infos.second);
    }
    return ErrorCode::NO_ERROR;
}

int32_t FullImeInfoManager::Switch(int32_t userId)
{
    std::vector<FullImeInfo> infos;
    auto ret = AddUser(userId, infos);
    ImeEnabledInfoManager::GetInstance().Switch(userId, infos);
    return ret;
}

int32_t FullImeInfoManager::Update()
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        fullImeInfos_.clear();
    }
    IMSA_HILOGI("run in.");
    std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> fullImeInfos;
    auto ret = ImeInfoInquirer::GetInstance().QueryFullImeInfo(fullImeInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGW("failed to QueryFullImeInfo, ret:%{public}d", ret);
        return ret;
    }
    {
        std::lock_guard<std::mutex> lock(lock_);
        for (const auto &infos : fullImeInfos) {
            fullImeInfos_.insert_or_assign(infos.first, infos.second);
        }
    }
    return ErrorCode::NO_ERROR;
}

int32_t FullImeInfoManager::Delete(int32_t userId)
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        fullImeInfos_.erase(userId);
    }
    ImeEnabledInfoManager::GetInstance().Delete(userId);
    return ErrorCode::NO_ERROR;
}

int32_t FullImeInfoManager::Add(int32_t userId, const std::string &bundleName)
{
    FullImeInfo info;
    auto ret = AddPackage(userId, bundleName, info);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    ImeEnabledInfoManager::GetInstance().Add(userId, info);
    return ErrorCode::NO_ERROR;
}

int32_t FullImeInfoManager::Delete(int32_t userId, const std::string &bundleName)
{
    auto ret = DeletePackage(userId, bundleName);
    ImeEnabledInfoManager::GetInstance().Delete(userId, bundleName);
    return ret;
}

int32_t FullImeInfoManager::Update(int32_t userId, const std::string &bundleName)
{
    FullImeInfo info;
    auto ret = ImeInfoInquirer::GetInstance().GetFullImeInfo(userId, bundleName, info);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to GetFullImeInfo failed, userId:%{public}d, bundleName:%{public}s, ret:%{public}d",
            userId, bundleName.c_str(), ret);
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    std::lock_guard<std::mutex> lock(lock_);
    auto it = fullImeInfos_.find(userId);
    if (it == fullImeInfos_.end()) {
        fullImeInfos_.insert({ userId, { info } });
        return ErrorCode::NO_ERROR;
    }
    auto iter = std::find_if(it->second.begin(), it->second.end(),
        [&bundleName](const FullImeInfo &info) { return bundleName == info.prop.name; });
    if (iter != it->second.end()) {
        it->second.erase(iter);
    }
    it->second.push_back(info);
    return ErrorCode::NO_ERROR;
}

int32_t FullImeInfoManager::Get(int32_t userId, std::vector<Property> &props)
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        auto it = fullImeInfos_.find(userId);
        if (it == fullImeInfos_.end()) {
            return {};
        }
        for (auto &fullImeInfo : it->second) {
            props.push_back(fullImeInfo.prop);
        }
    }
    auto ret = ImeEnabledInfoManager::GetInstance().GetEnabledStates(userId, props);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("get enabled status failed:%{public}d!", ret);
        return ErrorCode::ERROR_ENABLE_IME;
    }
    return ErrorCode::NO_ERROR;
}

bool FullImeInfoManager::Get(int32_t userId, const std::string &bundleName, FullImeInfo &fullImeInfo)
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        auto it = fullImeInfos_.find(userId);
        if (it == fullImeInfos_.end()) {
            IMSA_HILOGD("user %{public}d info", userId);
            return false;
        }
        auto iter = std::find_if(it->second.begin(), it->second.end(),
            [&bundleName](const FullImeInfo &info) { return bundleName == info.prop.name; });
        if (iter == it->second.end()) {
            IMSA_HILOGD("ime: %{public}s not in cache", bundleName.c_str());
            return false;
        }
        fullImeInfo = *iter;
    }
    auto ret =
        ImeEnabledInfoManager::GetInstance().GetEnabledState(userId, fullImeInfo.prop.name, fullImeInfo.prop.status);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("[%{public}d, %{public}s] get enabled status failed:%{public}d!", userId, bundleName.c_str(), ret);
    }
    return true;
}

bool FullImeInfoManager::Has(int32_t userId, const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = fullImeInfos_.find(userId);
    if (it == fullImeInfos_.end()) {
        return false;
    }
    auto iter = std::find_if(it->second.begin(), it->second.end(),
        [&bundleName](const FullImeInfo &info) { return bundleName == info.prop.name; });
    return iter != it->second.end();
}

std::string FullImeInfoManager::Get(int32_t userId, uint32_t tokenId)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = fullImeInfos_.find(userId);
    if (it == fullImeInfos_.end()) {
        return "";
    }
    auto iter = std::find_if(
        it->second.begin(), it->second.end(), [&tokenId](const FullImeInfo &info) { return tokenId == info.tokenId; });
    if (iter == it->second.end()) {
        return "";
    }
    return (*iter).prop.name;
}

int32_t FullImeInfoManager::Init()
{
    std::map<int32_t, std::vector<FullImeInfo>> fullImeInfos;
    auto ret = Init(fullImeInfos);
    if (ret != ErrorCode::NO_ERROR) {
        return ret;
    }
    ImeEnabledInfoManager::GetInstance().Init(fullImeInfos);
    return ErrorCode::NO_ERROR;
}

int32_t FullImeInfoManager::Init(std::map<int32_t, std::vector<FullImeInfo>> &fullImeInfos)
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (!fullImeInfos_.empty()) {
            fullImeInfos = fullImeInfos_;
            return ErrorCode::NO_ERROR;
        }
    }
    std::vector<std::pair<int32_t, std::vector<FullImeInfo>>> imeInfos;
    auto ret = ImeInfoInquirer::GetInstance().QueryFullImeInfo(imeInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGW("failed to QueryFullImeInfo, ret:%{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> lock(lock_);
    fullImeInfos_.clear();
    for (const auto &infos : imeInfos) {
        fullImeInfos_.insert_or_assign(infos.first, infos.second);
    }
    fullImeInfos = fullImeInfos_;
    return ErrorCode::NO_ERROR;
}

int32_t FullImeInfoManager::AddUser(int32_t userId, std::vector<FullImeInfo> &infos)
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        auto it = fullImeInfos_.find(userId);
        if (it != fullImeInfos_.end()) {
            infos = it->second;
            return ErrorCode::NO_ERROR;
        }
    }
    auto ret = ImeInfoInquirer::GetInstance().QueryFullImeInfo(userId, infos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to QueryFullImeInfo, userId:%{public}d, ret:%{public}d", userId, ret);
        return ret;
    }
    std::lock_guard<std::mutex> lock(lock_);
    fullImeInfos_.insert_or_assign(userId, infos);
    return ErrorCode::NO_ERROR;
}

int32_t FullImeInfoManager::AddPackage(int32_t userId, const std::string &bundleName, FullImeInfo &info)
{
    auto ret = ImeInfoInquirer::GetInstance().GetFullImeInfo(userId, bundleName, info);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("failed to GetFullImeInfo failed, userId:%{public}d, bundleName:%{public}s, ret:%{public}d",
                    userId, bundleName.c_str(), ret);
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    std::lock_guard<std::mutex> lock(lock_);
    auto it = fullImeInfos_.find(userId);
    if (it == fullImeInfos_.end()) {
        fullImeInfos_.insert({ userId, { info } });
        return ErrorCode::NO_ERROR;
    }
    auto iter = std::find_if(it->second.begin(), it->second.end(),
                             [&bundleName](const FullImeInfo &info) { return bundleName == info.prop.name; });
    if (iter != it->second.end()) {
        it->second.erase(iter);
    }
    it->second.push_back(info);
    return ErrorCode::NO_ERROR;
}

int32_t FullImeInfoManager::DeletePackage(int32_t userId, const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = fullImeInfos_.find(userId);
    if (it == fullImeInfos_.end()) {
        return ErrorCode::NO_ERROR;
    }
    auto iter = std::find_if(it->second.begin(), it->second.end(),
        [&bundleName](const FullImeInfo &info) { return bundleName == info.prop.name; });
    if (iter == it->second.end()) {
        return ErrorCode::NO_ERROR;
    }
    it->second.erase(iter);
    if (it->second.empty()) {
        fullImeInfos_.erase(it->first);
    }
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS