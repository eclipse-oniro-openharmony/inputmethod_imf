/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "input_type_manager.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cinttypes>
#include <cstdio>
#include <fstream>
#include <ios>
#include <string>

#include "climits"
#include "config_policy_utils.h"
#include "input_method_config_parser.h"
#include "global.h"
#include "ime_cfg_manager.h"

namespace OHOS {
namespace MiscServices {
namespace {
const std::string SUPPORTED_INPUT_TYPE_LIST = "supportedInputTypeList";
const std::string INPUT_TYPE = "inputType";
const std::string BUNDLE_NAME = "bundleName";
const std::string SUBTYPE_ID = "subtypeId";
using json = nlohmann::json;
} // namespace

void from_json(const json &jsonObj, InputTypeCfg &cfg)
{
    if (!jsonObj.contains(INPUT_TYPE) || !jsonObj[INPUT_TYPE].is_number()) {
        IMSA_HILOGE("INPUT_TYPE is invalid");
        return;
    }
    cfg.type = static_cast<InputType>(jsonObj.at(INPUT_TYPE).get<int32_t>());
    if (!jsonObj.contains(BUNDLE_NAME) || !jsonObj[BUNDLE_NAME].is_string()) {
        IMSA_HILOGE("BUNDLE_NAME is invalid");
        return;
    }
    cfg.ime.bundleName = jsonObj.at(BUNDLE_NAME).get<std::string>();
    if (!jsonObj.contains(SUBTYPE_ID) || !jsonObj[SUBTYPE_ID].is_string()) {
        IMSA_HILOGE("SUBTYPE_ID is invalid");
        return;
    }
    cfg.ime.subName = jsonObj.at(SUBTYPE_ID).get<std::string>();
}

InputTypeManager &InputTypeManager::GetInstance()
{
    static InputTypeManager instance;
    return instance;
}

bool InputTypeManager::IsSupported(InputType type)
{
    if (!isTypeCfgReady_.load() && !Init()) {
        IMSA_HILOGE("init cfg failed");
        return false;
    }
    std::lock_guard<std::mutex> lock(typesLock_);
    return inputTypes_.find(type) != inputTypes_.end();
}

bool InputTypeManager::IsInputType(const ImeIdentification &ime)
{
    if (!isTypeCfgReady_.load() && !Init()) {
        IMSA_HILOGE("init cfg failed");
        return false;
    }
    std::lock_guard<std::mutex> lock(listLock_);
    return inputTypeImeList_.find(ime) != inputTypeImeList_.end();
}

int32_t InputTypeManager::GetImeByInputType(InputType type, ImeIdentification &ime)
{
    if (!isTypeCfgReady_.load() && !Init()) {
        IMSA_HILOGE("init cfg failed");
        return ErrorCode::ERROR_PARSE_CONFIG_FILE;
    }
    std::lock_guard<std::mutex> lock(typesLock_);
    auto iter = inputTypes_.find(type);
    if (iter == inputTypes_.end()) {
        IMSA_HILOGE("type: %{public}d not supported", type);
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    ime = iter->second;
    IMSA_HILOGI("type: %{public}d find ime: %{public}s|%{public}s", type, ime.bundleName.c_str(), ime.subName.c_str());
    return ErrorCode::NO_ERROR;
}

void InputTypeManager::Set(bool isStarted, const ImeIdentification &currentIme)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    isStarted_ = isStarted;
    currentTypeIme_ = currentIme;
}

bool InputTypeManager::IsStarted()
{
    std::lock_guard<std::mutex> lock(stateLock_);
    return isStarted_;
}

ImeIdentification InputTypeManager::GetCurrentIme()
{
    std::lock_guard<std::mutex> lock(stateLock_);
    return currentTypeIme_;
}

bool InputTypeManager::Init()
{
    IMSA_HILOGD("start");
    if (isInitInProgress_.load()) {
        return isInitSuccess_.GetValue();
    }
    isInitInProgress_.store(true);
    isInitSuccess_.Clear(false);
    std::vector<InputTypeCfg> configs;
    bool isSuccess = ImeConfigParse::ParseFromCustomSystem(SUPPORTED_INPUT_TYPE_LIST, configs);
    IMSA_HILOGD("ParseFromCustomSystem end isSuccess %{public}d", isSuccess);
    if (isSuccess) {
        std::lock_guard<std::mutex> lk(typesLock_);
        for (const auto& config : configs) {
            inputTypes_.insert({ config.type, config.ime });
        }
        for (const auto& cfg : inputTypes_) {
            std::lock_guard<std::mutex> lock(listLock_);
            inputTypeImeList_.insert(cfg.second);
        }
    } else {
        std::lock_guard<std::mutex> lk(typesLock_);
        inputTypes_.clear();
    }
    isTypeCfgReady_.store(isSuccess);
    isInitSuccess_.SetValue(isSuccess);
    isInitInProgress_.store(false);
    return isSuccess;
}
} // namespace MiscServices
} // namespace OHOS