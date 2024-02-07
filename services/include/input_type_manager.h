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

#ifndef INPUTMETHOD_IMF_INPUT_TYPE_MANAGER_H
#define INPUTMETHOD_IMF_INPUT_TYPE_MANAGER_H

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "block_data.h"
#include "input_method_utils.h"
#include "serializable.h"

namespace OHOS {
namespace MiscServices {
struct ImeIdentification {
    std::string bundleName;
    std::string subName;
    bool operator==(const ImeIdentification &ime) const
    {
        return (bundleName == ime.bundleName && subName == ime.subName);
    }
    bool operator<(const ImeIdentification &ime) const
    {
        return bundleName == ime.bundleName ? (subName < ime.subName) : (bundleName < ime.bundleName);
    }
};

struct InputTypeCfg {
    InputType type{};
    ImeIdentification ime;
    bool GetValue(cJSON *node)
    {
        int32_t typeTemp = -1;
        Serializable::GetValue(node, GET_NAME(inputType), typeTemp);
        type = static_cast<InputType>(typeTemp);
        Serializable::GetValue(node, GET_NAME(bundleName), ime.bundleName);
        Serializable::GetValue(node, GET_NAME(subtypeId), ime.subName);
        return true;
    }
};

class InputTypeManager {
public:
    static InputTypeManager &GetInstance();
    bool IsSupported(InputType type);
    bool IsInputType(const ImeIdentification &ime);
    bool IsStarted();
    bool IsCameraImeStarted();
    void Set(bool isStarted, const ImeIdentification &currentIme = {});
    ImeIdentification GetCurrentIme();
    int32_t GetImeByInputType(InputType type, ImeIdentification &ime);

private:
    bool Init();
    bool GetInputTypeFromFile(std::vector<InputTypeCfg> &configs);
    bool ParseInputType(const std::string &content, std::vector<InputTypeCfg> &configs);
    std::mutex stateLock_;
    bool isStarted_{ false };
    ImeIdentification currentTypeIme_;

    std::mutex typesLock_;
    std::map<InputType, ImeIdentification> inputTypes_;
    std::mutex listLock_;
    std::set<ImeIdentification> inputTypeImeList_;

    std::atomic_bool isTypeCfgReady_{ false };
    std::atomic_bool isInitInProgress_{ false };
    BlockData<bool> isInitSuccess_{ false };
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUTMETHOD_IMF_INPUT_TYPE_MANAGER_H
