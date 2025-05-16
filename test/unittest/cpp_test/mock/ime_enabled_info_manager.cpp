/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ime_enabled_info_manager.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <set>
#include <sstream>

#include "global.h"

namespace OHOS {
namespace MiscServices {
using namespace std::chrono;
ImeEnabledInfoManager &ImeEnabledInfoManager::GetInstance()
{
    static ImeEnabledInfoManager instance;
    return instance;
}

ImeEnabledInfoManager::~ImeEnabledInfoManager()
{
}

void ImeEnabledInfoManager::SetCurrentImeStatusChangedHandler(CurrentImeStatusChangedHandler handler)
{
}

void ImeEnabledInfoManager::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &eventHandler)
{
}

int32_t ImeEnabledInfoManager::RegularInit(const std::map<int32_t, std::vector<FullImeInfo>> &fullImeInfos)
{
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::Init(const std::map<int32_t, std::vector<FullImeInfo>> &fullImeInfos)
{
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::Delete(int32_t userId)
{
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::Switch(int32_t userId, const std::vector<FullImeInfo> &imeInfos)
{
    return ErrorCode::NO_ERROR;
}
int32_t ImeEnabledInfoManager::Add(int32_t userId, const FullImeInfo &imeInfo)
{
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::Delete(int32_t userId, const std::string &bundleName)
{
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::Update(
    int32_t userId, const std::string &bundleName, const std::string &extensionName, EnabledStatus status)
{
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledState(int32_t userId, const std::string &bundleName, EnabledStatus &status)
{
    return ErrorCode::NO_ERROR;
}

int32_t ImeEnabledInfoManager::GetEnabledStates(int32_t userId, std::vector<Property> &props)
{
    return ErrorCode::NO_ERROR;
}

bool ImeEnabledInfoManager::IsDefaultFullMode(int32_t userId, const std::string &bundleName)
{
    return true;
}

void ImeEnabledInfoManager::OnFullExperienceTableChanged(int32_t userId)
{
}
} // namespace MiscServices
} // namespace OHOS